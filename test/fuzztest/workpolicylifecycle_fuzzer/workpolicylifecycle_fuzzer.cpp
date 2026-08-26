/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "workpolicylifecycle_fuzzer.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include "detector_value.h"
#include "event_runner.h"
#include "watchdog.h"
#include "work_condition.h"
#include "work_conn_manager.h"
#include "work_event_handler.h"
#include "work_info.h"
#include "work_policy_manager.h"
#include "work_queue_manager.h"
#include "work_sched_constants.h"
#include "work_sched_errors.h"
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
constexpr uint32_t REPEAT_SECONDS_PER_MINUTE = 60;
constexpr uint32_t MILLISECONDS_PER_SECOND = 1000;
constexpr uint32_t DEFAULT_REPEAT_CYCLE =
    DEFAULT_REPEAT_MINUTES * REPEAT_SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
constexpr int32_t MAX_WORKS_PER_INPUT = 4;
constexpr int32_t FUZZ_INVALID_VALUE = -1;
constexpr int32_t FUZZ_BOUND_TWO = 2;
constexpr int32_t FUZZ_BOUND_THREE = 3;
constexpr int32_t FUZZ_BOUND_FOUR = 4;
constexpr int32_t FUZZ_BOUND_FIVE = 5;
constexpr int32_t MAX_POLICY_LEVEL_FOR_FUZZ = 10;
constexpr int32_t SA_REFRESH_RATIO_FOR_FUZZ = 12;
constexpr int32_t MAX_RUNNING_COUNT_FOR_FUZZ = 16;
constexpr int32_t HOURS_PER_DAY = 24;
constexpr int32_t MAX_WATCHDOG_ID_FOR_FUZZ = 32;
constexpr int32_t SECONDS_PER_MINUTE = 60;
constexpr int32_t MAX_PERCENT_FOR_FUZZ = 100;
constexpr int32_t MAX_BATTERY_LEVEL_FOR_FUZZ = 110;
constexpr int32_t MAX_START_TIME_DRIFT_MS = 1000;
constexpr int32_t SECONDS_PER_HOUR = 3600;
constexpr int32_t MAX_MEMORY_DUMP_SIZE = 4096;
constexpr int32_t MAX_SA_ID_FOR_FUZZ = 20000;
constexpr int32_t MAX_WORK_ID_FOR_FUZZ = 100000;

class FuzzWorkConnManager final : public WorkConnManager {
public:
    explicit FuzzWorkConnManager(FuzzedDataProvider &provider) : provider_(provider) {}
    bool StopWork(std::shared_ptr<WorkStatus> workStatus, bool isTimeOut) override
    {
        return provider_.ConsumeBool() || isTimeOut || (workStatus != nullptr && workStatus->IsPaused());
    }

private:
    FuzzedDataProvider &provider_;
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
    switch (provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_BOUND_FOUR)) {
        case 0:
            return 0;
        case 1:
            return 1;
        case FUZZ_BOUND_TWO:
            return DEFAULT_REPEAT_CYCLE - 1;
        case FUZZ_BOUND_THREE:
            return DEFAULT_REPEAT_CYCLE;
        default:
            return provider.ConsumeIntegralInRange<uint32_t>(0, DEFAULT_REPEAT_CYCLE * FUZZ_BOUND_TWO);
    }
}

std::shared_ptr<WorkSchedulerService> PrepareService(FuzzedDataProvider &provider)
{
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (!service->eventRunner_) {
        service->eventRunner_ = AppExecFwk::EventRunner::Create(WORKSCHEDULER_SERVICE_RUNNER_NAME,
            AppExecFwk::ThreadMode::FFRT);
    }
    service->handler_ = std::make_shared<WorkEventHandler>(service->eventRunner_, service);
    service->workPolicyManager_ = std::make_shared<WorkPolicyManager>(service);
    service->workPolicyManager_->handler_ = service->handler_;
    service->workPolicyManager_->workConnManager_ = std::make_shared<FuzzWorkConnManager>(provider);
    service->workPolicyManager_->watchdog_ =
        std::make_shared<Watchdog>(service->workPolicyManager_, service->eventRunner_);
    service->workQueueManager_ = std::make_shared<WorkQueueManager>(service);
    service->whitelist_.clear();
    service->whitelist_.insert(DEFAULT_UID);
    service->ready_.store(true);
    service->checkBundle_.store(false);
    return service;
}

void AddConditions(WorkInfo &workInfo, FuzzedDataProvider &provider)
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
            FUZZ_INVALID_VALUE, MAX_BATTERY_LEVEL_FOR_FUZZ));
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
                FUZZ_INVALID_VALUE, MAX_POLICY_LEVEL_FOR_FUZZ));
        }
    }
    if ((mask & (1 << WorkCondition::Type::DEEP_IDLE)) != 0) {
        workInfo.RequestDeepIdle(provider.ConsumeBool());
        workInfo.deepIdleTime_ = provider.ConsumeIntegralInRange<int32_t>(
            0, HOURS_PER_DAY * SECONDS_PER_MINUTE * SECONDS_PER_MINUTE);
    }
    if (workInfo.GetConditionMap()->empty()) {
        workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
        workInfo.RequestBatteryLevel(provider.ConsumeIntegralInRange<int32_t>(0, MAX_PERCENT_FOR_FUZZ));
    }
}

std::shared_ptr<WorkStatus> BuildWorkStatus(FuzzedDataProvider &provider, int32_t index)
{
    WorkInfo workInfo;
    int32_t uid = provider.ConsumeBool() ? DEFAULT_UID : DEFAULT_UID + index + 1;
    workInfo.SetWorkId(provider.ConsumeIntegralInRange<int32_t>(1, MAX_WORK_ID_FOR_FUZZ) + index);
    workInfo.SetElement(ConsumeText(provider, "bundleName" + std::to_string(index)),
        ConsumeText(provider, "abilityName" + std::to_string(index)));
    workInfo.RefreshUid(uid);
    workInfo.RequestPersisted(false);
    workInfo.SetCallBySystemApp(provider.ConsumeBool());
    workInfo.earliestStartTime_ = provider.ConsumeIntegralInRange<int32_t>(0, SECONDS_PER_HOUR);
    if (provider.ConsumeIntegralInRange<int32_t>(0, SA_REFRESH_RATIO_FOR_FUZZ) == 0) {
        workInfo.RefreshSaId(provider.ConsumeIntegralInRange<int32_t>(1, MAX_SA_ID_FOR_FUZZ));
    }
    AddConditions(workInfo, provider);

    auto workStatus = std::make_shared<WorkStatus>(workInfo, uid);
    workStatus->workWatchDogTime_ = provider.ConsumeIntegralInRange<uint64_t>(1, LONG_WATCHDOG_TIME);
    workStatus->workStartTime_ = WorkSchedUtils::GetCurrentTimeMs() -
        provider.ConsumeIntegralInRange<uint64_t>(0, MAX_START_TIME_DRIFT_MS);
    return workStatus;
}

void DriveConditionReady(const std::shared_ptr<WorkSchedulerService> &service,
    const std::vector<std::shared_ptr<WorkStatus>> &works, FuzzedDataProvider &provider)
{
    auto readyWorks = std::make_shared<std::vector<std::shared_ptr<WorkStatus>>>();
    for (auto &workStatus : works) {
        if (provider.ConsumeBool()) {
            workStatus->MarkStatus(WorkStatus::Status::CONDITION_READY);
            readyWorks->emplace_back(workStatus);
        }
    }
    service->OnConditionReady(readyWorks);
    service->workPolicyManager_->OnConditionReady(readyWorks);
    if (!works.empty()) {
        auto selected = works[provider.ConsumeIntegralInRange<size_t>(0, works.size() - 1)];
        selected->MarkStatus(WorkStatus::Status::CONDITION_READY);
        service->workPolicyManager_->RealStartWork(selected);
    }
}

void DrivePauseResume(const std::shared_ptr<WorkPolicyManager> &policyManager,
    const std::vector<std::shared_ptr<WorkStatus>> &works, FuzzedDataProvider &provider)
{
    uint32_t watchdogId = provider.ConsumeIntegralInRange<uint32_t>(1, MAX_WATCHDOG_ID_FOR_FUZZ);
    for (auto &workStatus : works) {
        if (!provider.ConsumeBool()) {
            continue;
        }
        workStatus->MarkStatus(WorkStatus::Status::RUNNING);
        workStatus->paused_ = provider.ConsumeBool();
        workStatus->workWatchDogTime_ = provider.ConsumeIntegralInRange<uint64_t>(1, LONG_WATCHDOG_TIME);
        workStatus->workStartTime_ = WorkSchedUtils::GetCurrentTimeMs() -
        provider.ConsumeIntegralInRange<uint64_t>(0, MAX_START_TIME_DRIFT_MS);
        policyManager->watchdogIdMap_[watchdogId++] = workStatus;
    }
    policyManager->PauseRunningWorks(DEFAULT_UID);
    policyManager->PauseRunningWorks(DEFAULT_UID + provider.ConsumeIntegralInRange<int32_t>(1, FUZZ_BOUND_THREE));
    policyManager->ResumePausedWorks(DEFAULT_UID);
    policyManager->ResumePausedWorks(DEFAULT_UID + provider.ConsumeIntegralInRange<int32_t>(1, FUZZ_BOUND_THREE));
}

void DriveStopAndWatchdog(const std::shared_ptr<WorkSchedulerService> &service,
    const std::vector<std::shared_ptr<WorkStatus>> &works, FuzzedDataProvider &provider)
{
    if (works.empty()) {
        return;
    }
    auto workStatus = works[provider.ConsumeIntegralInRange<size_t>(0, works.size() - 1)];
    bool needCancel = provider.ConsumeBool();
    bool isTimeout = provider.ConsumeBool();
    service->workPolicyManager_->StopWork(workStatus, workStatus->uid_, needCancel, isTimeout);
    service->StopWorkInner(workStatus, workStatus->uid_, needCancel, isTimeout);
    service->WatchdogTimeOut(workStatus);
    service->workPolicyManager_->WatchdogTimeOut(provider.ConsumeIntegralInRange<uint32_t>(
        0, MAX_WATCHDOG_ID_FOR_FUZZ));
    service->workPolicyManager_->RemoveWatchDog(workStatus);
}
} // namespace

bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
{
    uid = DEFAULT_UID;
    return true;
}

void RunWorkPolicyLifecycleFuzzer(FuzzedDataProvider &provider)
{
    auto service = PrepareService(provider);
    auto policyManager = service->workPolicyManager_;
    std::vector<std::shared_ptr<WorkStatus>> works;
    int32_t workCount = provider.ConsumeIntegralInRange<int32_t>(1, MAX_WORKS_PER_INPUT);
    for (int32_t index = 0; index < workCount; ++index) {
        auto workStatus = BuildWorkStatus(provider, index);
        works.emplace_back(workStatus);
        policyManager->AddWork(workStatus, workStatus->uid_);
        service->workQueueManager_->AddWork(workStatus);
    }

    switch (provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_BOUND_FIVE)) {
        case 0:
            DriveConditionReady(service, works, provider);
            DrivePauseResume(policyManager, works, provider);
            DriveStopAndWatchdog(service, works, provider);
            break;
        case 1:
            DrivePauseResume(policyManager, works, provider);
            DriveConditionReady(service, works, provider);
            break;
        case FUZZ_BOUND_TWO:
            DriveStopAndWatchdog(service, works, provider);
            DrivePauseResume(policyManager, works, provider);
            break;
        default:
            DriveConditionReady(service, works, provider);
            DriveStopAndWatchdog(service, works, provider);
            DrivePauseResume(policyManager, works, provider);
            break;
    }

    std::string dumpResult;
    policyManager->Dump(dumpResult);
    int32_t queryUid = DEFAULT_UID;
    static_cast<void>(policyManager->ObtainAllWorks(queryUid));
    static_cast<void>(policyManager->GetAllRunningWorks());
    static_cast<void>(policyManager->GetAllWorkStatus(queryUid));
    static_cast<void>(policyManager->GetDeepIdleWorks());
    policyManager->SetMemoryByDump(provider.ConsumeIntegralInRange<int32_t>(0, MAX_MEMORY_DUMP_SIZE));
    policyManager->SetWatchdogTimeByDump(provider.ConsumeIntegralInRange<int32_t>(0, LONG_WATCHDOG_TIME));
    policyManager->SetCpuUsageByDump(provider.ConsumeIntegralInRange<int32_t>(0, MAX_PERCENT_FOR_FUZZ));
    policyManager->SetMaxRunningCountByDump(provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_VALUE, MAX_RUNNING_COUNT_FOR_FUZZ));
    policyManager->SetThermalLevelByDump(provider.ConsumeIntegralInRange<int32_t>(0, MAX_POLICY_LEVEL_FOR_FUZZ));
    for (auto &workStatus : works) {
        policyManager->RemoveWork(workStatus, workStatus->uid_);
    }
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    OHOS::WorkScheduler::RunWorkPolicyLifecycleFuzzer(provider);
    return 0;
}
