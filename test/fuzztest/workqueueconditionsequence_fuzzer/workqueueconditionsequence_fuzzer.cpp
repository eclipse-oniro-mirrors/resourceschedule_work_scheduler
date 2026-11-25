/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "workqueueconditionsequence_fuzzer.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include "conditions/icondition_listener.h"
#include "detector_value.h"
#include "event_runner.h"
#include "watchdog.h"
#include "work_condition.h"
#include "work_event_handler.h"
#include "work_info.h"
#include "work_policy_manager.h"
#include "work_queue_manager.h"
#include "work_sched_constants.h"
#include "work_sched_utils.h"
#include "work_scheduler_service.h"
#include "work_status.h"

void OHOS::RefBase::DecStrongRef(void const *obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr const char *WORKSCHEDULER_SERVICE_RUNNER_NAME = "WorkSchedulerService";
constexpr size_t MAX_STRING_SIZE = 48;
constexpr int32_t DEFAULT_UID = 10000;
constexpr uint32_t DEFAULT_REPEAT_MINUTES = 20;
constexpr uint32_t SHORT_REPEAT_MINUTES = 5;
constexpr uint32_t REPEAT_SECONDS_PER_MINUTE = 60;
constexpr uint32_t MILLISECONDS_PER_SECOND = 1000;
constexpr uint32_t DEFAULT_REPEAT_CYCLE =
    DEFAULT_REPEAT_MINUTES * REPEAT_SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
constexpr uint32_t SHORT_REPEAT_CYCLE =
    SHORT_REPEAT_MINUTES * REPEAT_SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
constexpr int32_t MAX_WORKS_PER_INPUT = 5;
constexpr int32_t MIN_DETECTOR_BATTERY_LEVEL = -10;
constexpr int32_t MIN_REQUEST_BATTERY_LEVEL = -5;
constexpr int32_t FUZZ_INVALID_VALUE = -1;
constexpr int32_t FUZZ_BOUND_TWO = 2;
constexpr int32_t FUZZ_BOUND_THREE = 3;
constexpr int32_t FUZZ_BOUND_FOUR = 4;
constexpr int32_t FUZZ_BOUND_FIVE = 5;
constexpr int32_t MAX_REPEAT_COUNT_FOR_FUZZ = 8;
constexpr int32_t MAX_NORMAL_BATTERY_LEVEL = 100;
constexpr int32_t MAX_BATTERY_LEVEL_FOR_FUZZ = 120;
constexpr int32_t SECONDS_PER_HOUR = 3600;
constexpr int32_t MAX_UID_FOR_FUZZ = 30000;
constexpr int32_t MAX_WORK_ID_FOR_FUZZ = 100000;
constexpr int32_t MAX_TIMEOUT_WINDOW_MS = 600000;

class NoopConditionListener final : public IConditionListener {
public:
    void OnConditionChanged(WorkCondition::Type conditionType, std::shared_ptr<DetectorValue> conditionVal) override {}
    bool Start() override
    {
        return true;
    }
    bool Stop() override
    {
        return true;
    }
};

std::string ConsumeText(FuzzedDataProvider &provider, const std::string &fallback)
{
    std::string text = provider.ConsumeRandomLengthString(MAX_STRING_SIZE);
    if (text.empty()) {
        return fallback;
    }
    std::replace_if(text.begin(), text.end(), [](char chr) { return chr == '\0' || chr == '\n' || chr == '\r'; }, '_');
    return text;
}

int32_t ConsumeEnumEdge(FuzzedDataProvider &provider, int32_t minValue, int32_t maxValue)
{
    switch (provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_BOUND_FIVE)) {
        case 0:
            return minValue - 1;
        case 1:
            return minValue;
        case FUZZ_BOUND_TWO:
            return maxValue;
        case FUZZ_BOUND_THREE:
            return maxValue + 1;
        case FUZZ_BOUND_FOUR:
            return provider.ConsumeIntegralInRange<int32_t>(minValue, maxValue);
        default:
            return provider.ConsumeIntegral<int32_t>();
    }
}

uint32_t ConsumeRepeatCycle(FuzzedDataProvider &provider)
{
    switch (provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_BOUND_FIVE)) {
        case 0:
            return 0;
        case 1:
            return 1;
        case FUZZ_BOUND_TWO:
            return SHORT_REPEAT_CYCLE - 1;
        case FUZZ_BOUND_THREE:
            return SHORT_REPEAT_CYCLE;
        case FUZZ_BOUND_FOUR:
            return DEFAULT_REPEAT_CYCLE;
        default:
            return provider.ConsumeIntegralInRange<uint32_t>(0, DEFAULT_REPEAT_CYCLE * FUZZ_BOUND_TWO);
    }
}

void RegisterNoopListeners(const std::shared_ptr<WorkQueueManager> &manager)
{
    manager->listenerMap_.clear();
    auto listener = std::make_shared<NoopConditionListener>();
    for (int32_t type = WorkCondition::Type::NETWORK; type <= WorkCondition::Type::STANDBY; ++type) {
        manager->AddListener(static_cast<WorkCondition::Type>(type), listener);
    }
}

std::shared_ptr<WorkSchedulerService> PrepareService()
{
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (!service->eventRunner_) {
        service->eventRunner_ = AppExecFwk::EventRunner::Create(WORKSCHEDULER_SERVICE_RUNNER_NAME,
            AppExecFwk::ThreadMode::FFRT);
    }
    if (!service->handler_) {
        service->handler_ = std::make_shared<WorkEventHandler>(service->eventRunner_, service);
    }
    service->workPolicyManager_ = std::make_shared<WorkPolicyManager>(service);
    service->workPolicyManager_->handler_ = service->handler_;
    service->workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
    service->workPolicyManager_->watchdog_ =
        std::make_shared<Watchdog>(service->workPolicyManager_, service->eventRunner_);
    service->workQueueManager_ = std::make_shared<WorkQueueManager>(service);
    RegisterNoopListeners(service->workQueueManager_);
    service->whitelist_.clear();
    service->whitelist_.insert(DEFAULT_UID);
    service->ready_.store(true);
    service->checkBundle_.store(false);
    return service;
}

void AddRequestedConditions(WorkInfo &workInfo, FuzzedDataProvider &provider)
{
    uint8_t mask = provider.ConsumeIntegral<uint8_t>();
    if ((mask & (1 << WorkCondition::Type::NETWORK)) != 0) {
        workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(ConsumeEnumEdge(provider,
            WorkCondition::Network::NETWORK_TYPE_ANY, WorkCondition::Network::NETWORK_TYPE_ETHERNET)));
    }
    if ((mask & (1 << WorkCondition::Type::CHARGER)) != 0) {
        workInfo.RequestChargerType(provider.ConsumeBool(), static_cast<WorkCondition::Charger>(
            ConsumeEnumEdge(provider, WorkCondition::Charger::CHARGING_PLUGGED_ANY,
                WorkCondition::Charger::CHARGING_UNPLUGGED)));
    }
    if ((mask & (1 << WorkCondition::Type::BATTERY_STATUS)) != 0) {
        workInfo.RequestBatteryStatus(static_cast<WorkCondition::BatteryStatus>(ConsumeEnumEdge(provider,
            WorkCondition::BatteryStatus::BATTERY_STATUS_LOW,
            WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY)));
    }
    if ((mask & (1 << WorkCondition::Type::BATTERY_LEVEL)) != 0) {
        workInfo.RequestBatteryLevel(provider.ConsumeIntegralInRange<int32_t>(
            MIN_REQUEST_BATTERY_LEVEL, MAX_BATTERY_LEVEL_FOR_FUZZ));
    }
    if ((mask & (1 << WorkCondition::Type::STORAGE)) != 0) {
        workInfo.RequestStorageLevel(static_cast<WorkCondition::Storage>(ConsumeEnumEdge(provider,
            WorkCondition::Storage::STORAGE_LEVEL_LOW, WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY)));
    }
    if ((mask & (1 << WorkCondition::Type::TIMER)) != 0) {
        if (provider.ConsumeBool()) {
            workInfo.RequestRepeatCycle(ConsumeRepeatCycle(provider));
        } else {
            workInfo.RequestRepeatCycle(ConsumeRepeatCycle(provider), provider.ConsumeIntegralInRange<int32_t>(
                FUZZ_INVALID_VALUE, MAX_REPEAT_COUNT_FOR_FUZZ));
        }
    }
    if ((mask & (1 << WorkCondition::Type::DEEP_IDLE)) != 0) {
        workInfo.RequestDeepIdle(provider.ConsumeBool());
    }
    if (workInfo.GetConditionMap()->empty()) {
        workInfo.RequestBatteryLevel(provider.ConsumeIntegralInRange<int32_t>(0, MAX_NORMAL_BATTERY_LEVEL));
        workInfo.RequestRepeatCycle(DEFAULT_REPEAT_CYCLE);
    }
}

std::shared_ptr<WorkStatus> BuildWorkStatus(FuzzedDataProvider &provider, int32_t index)
{
    WorkInfo workInfo;
    int32_t uid = provider.ConsumeBool() ? DEFAULT_UID : provider.ConsumeIntegralInRange<int32_t>(1, MAX_UID_FOR_FUZZ);
    workInfo.SetWorkId(provider.ConsumeIntegralInRange<int32_t>(1, MAX_WORK_ID_FOR_FUZZ) + index);
    workInfo.SetElement(ConsumeText(provider, "bundleName" + std::to_string(index)),
        ConsumeText(provider, "abilityName" + std::to_string(index)));
    workInfo.RefreshUid(uid);
    workInfo.RequestPersisted(false);
    workInfo.SetCallBySystemApp(provider.ConsumeBool());
    workInfo.earliestStartTime_ = provider.ConsumeIntegralInRange<int32_t>(0, SECONDS_PER_HOUR);
    AddRequestedConditions(workInfo, provider);

    auto workStatus = std::make_shared<WorkStatus>(workInfo, uid);
    workStatus->userId_ = provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_BOUND_THREE);
    if (provider.ConsumeBool()) {
        workStatus->SetTimeout(provider.ConsumeBool());
        workStatus->workStartTime_ = WorkSchedUtils::GetCurrentTimeMs() -
            provider.ConsumeIntegralInRange<uint64_t>(0, MAX_TIMEOUT_WINDOW_MS);
        workStatus->workWatchDogTime_ = provider.ConsumeIntegralInRange<uint64_t>(0, MAX_TIMEOUT_WINDOW_MS);
    }
    return workStatus;
}

std::shared_ptr<DetectorValue> BuildDetectorValue(WorkCondition::Type type,
    const std::shared_ptr<WorkStatus> &workStatus, FuzzedDataProvider &provider)
{
    int32_t intValue = 0;
    int32_t timeValue = provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_BOUND_THREE);
    bool boolValue = provider.ConsumeBool();
    std::string stringValue = ConsumeText(provider, workStatus ? workStatus->bundleName_ : "bundleName");

    switch (type) {
        case WorkCondition::Type::NETWORK:
            intValue = ConsumeEnumEdge(provider, WorkCondition::Network::NETWORK_TYPE_ANY,
                WorkCondition::Network::NETWORK_UNKNOWN);
            break;
        case WorkCondition::Type::CHARGER:
            intValue = ConsumeEnumEdge(provider, WorkCondition::Charger::CHARGING_PLUGGED_ANY,
                WorkCondition::Charger::CHARGING_UNKNOWN);
            break;
        case WorkCondition::Type::BATTERY_STATUS:
            intValue = ConsumeEnumEdge(provider, WorkCondition::BatteryStatus::BATTERY_STATUS_LOW,
                WorkCondition::BatteryStatus::BATTERY_UNKNOWN);
            break;
        case WorkCondition::Type::BATTERY_LEVEL:
            intValue = provider.ConsumeIntegralInRange<int32_t>(MIN_DETECTOR_BATTERY_LEVEL, MAX_BATTERY_LEVEL_FOR_FUZZ);
            break;
        case WorkCondition::Type::STORAGE:
            intValue = ConsumeEnumEdge(provider, WorkCondition::Storage::STORAGE_LEVEL_LOW,
                WorkCondition::Storage::STORAGE_UNKNOWN);
            break;
        case WorkCondition::Type::GROUP:
            intValue = provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_BOUND_FIVE);
            if (workStatus && provider.ConsumeBool()) {
                timeValue = workStatus->userId_;
                stringValue = workStatus->bundleName_;
                boolValue = true;
            }
            break;
        case WorkCondition::Type::DEEP_IDLE:
            intValue = workStatus && workStatus->workInfo_->IsSA() ? workStatus->workInfo_->GetSaId() : DEFAULT_SA_ID;
            break;
        case WorkCondition::Type::STANDBY:
            boolValue = provider.ConsumeBool();
            break;
        case WorkCondition::Type::TIMER:
        case WorkCondition::Type::UNKNOWN:
        default:
            intValue = provider.ConsumeIntegral<int32_t>();
            break;
    }
    return std::make_shared<DetectorValue>(intValue, timeValue, boolValue, stringValue);
}

std::vector<WorkCondition::Type> BuildConditionSequence(FuzzedDataProvider &provider)
{
    std::vector<WorkCondition::Type> sequence = {
        WorkCondition::Type::NETWORK,
        WorkCondition::Type::STORAGE,
        WorkCondition::Type::CHARGER,
        WorkCondition::Type::BATTERY_STATUS,
        WorkCondition::Type::BATTERY_LEVEL,
        WorkCondition::Type::TIMER,
        WorkCondition::Type::GROUP,
        WorkCondition::Type::DEEP_IDLE,
        WorkCondition::Type::STANDBY,
    };
    std::rotate(sequence.begin(), sequence.begin() + provider.ConsumeIntegralInRange<size_t>(0, sequence.size() - 1),
        sequence.end());
    return sequence;
}
} // namespace

bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
{
    uid = DEFAULT_UID;
    return true;
}

void RunWorkQueueConditionFuzzer(FuzzedDataProvider &provider)
{
    auto service = PrepareService();
    auto manager = service->workQueueManager_;
    std::vector<std::shared_ptr<WorkStatus>> works;
    int32_t workCount = provider.ConsumeIntegralInRange<int32_t>(1, MAX_WORKS_PER_INPUT);
    for (int32_t index = 0; index < workCount; ++index) {
        auto workStatus = BuildWorkStatus(provider, index);
        works.emplace_back(workStatus);
        manager->AddWork(workStatus);
    }

    auto sequence = BuildConditionSequence(provider);
    for (auto type : sequence) {
        auto pivot = works[provider.ConsumeIntegralInRange<size_t>(0, works.size() - 1)];
        auto detectorValue = BuildDetectorValue(type, pivot, provider);
        auto readyWorks = manager->GetReayQueue(type, detectorValue);
        if (!readyWorks.empty()) {
            service->OnConditionReady(std::make_shared<std::vector<std::shared_ptr<WorkStatus>>>(readyWorks));
        }
        if (provider.ConsumeBool()) {
            manager->OnConditionChanged(type, detectorValue);
        }
    }

    std::string dumpResult;
    manager->Dump(dumpResult);
    manager->SetTimeCycle(provider.ConsumeIntegralInRange<uint32_t>(0, DEFAULT_REPEAT_CYCLE * FUZZ_BOUND_TWO));
    manager->SetTimeRetrigger(provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_VALUE, static_cast<int32_t>(DEFAULT_REPEAT_CYCLE)));
    static_cast<void>(manager->GetTimeCycle());
    static_cast<void>(manager->GetTimeRetrigger());

    auto selected = works[provider.ConsumeIntegralInRange<size_t>(0, works.size() - 1)];
    if (provider.ConsumeBool()) {
        selected->SetTimeout(true);
        selected->workStartTime_ = WorkSchedUtils::GetCurrentTimeMs() - DEFAULT_REPEAT_CYCLE;
        manager->ClearTimeOutWorkStatus();
    }
    manager->CancelWork(selected);
    if (!works.empty()) {
        manager->RemoveWork(works.front());
    }
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    OHOS::WorkScheduler::RunWorkQueueConditionFuzzer(provider);
    return 0;
}
