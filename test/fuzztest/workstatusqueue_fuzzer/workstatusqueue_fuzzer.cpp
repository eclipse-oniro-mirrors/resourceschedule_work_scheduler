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

#include "workstatusqueue_fuzzer.h"

#include <algorithm>
#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include "detector_value.h"
#include "work_condition.h"
#include "work_info.h"
#include "work_queue.h"
#include "work_sched_utils.h"
#include "work_status.h"

void OHOS::RefBase::DecStrongRef(void const *obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t MAX_STRING_SIZE = 48;
constexpr int32_t DEFAULT_UID = 10000;
constexpr int32_t DEFAULT_USER_ID = 100;
constexpr int32_t DEFAULT_SA_ID = 1905;
constexpr uint32_t DEFAULT_REPEAT_MINUTES = 20;
constexpr uint32_t MIN_REPEAT_MINUTES = 5;
constexpr uint32_t REPEAT_SECONDS_PER_MINUTE = 60;
constexpr uint32_t MILLISECONDS_PER_SECOND = 1000;
constexpr uint32_t DEFAULT_REPEAT_CYCLE =
    DEFAULT_REPEAT_MINUTES * REPEAT_SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
constexpr uint32_t MIN_REPEAT_CYCLE =
    MIN_REPEAT_MINUTES * REPEAT_SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
constexpr int32_t MAX_WORKS_PER_INPUT = 6;
constexpr int32_t MIN_BATTERY_LEVEL_FOR_FUZZ = -10;
constexpr int32_t FUZZ_INVALID_LOW_VALUE = -2;
constexpr int32_t FUZZ_INVALID_VALUE = -1;
constexpr int32_t FUZZ_BOUND_TWO = 2;
constexpr int32_t FUZZ_BOUND_THREE = 3;
constexpr int32_t FUZZ_BOUND_FOUR = 4;
constexpr int32_t FUZZ_BOUND_FIVE = 5;
constexpr int32_t MAX_PRIORITY_FOR_FUZZ = 10;
constexpr int32_t MAX_BATTERY_LEVEL_FOR_FUZZ = 120;
constexpr int32_t MAX_UID_FOR_FUZZ = 20000;
constexpr int32_t MAX_WORK_ID_FOR_FUZZ = 100000;
constexpr int32_t MAX_DEEP_IDLE_TIME_FOR_FUZZ = 600000;

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
            return MIN_REPEAT_CYCLE - 1;
        case FUZZ_BOUND_THREE:
            return MIN_REPEAT_CYCLE;
        case FUZZ_BOUND_FOUR:
            return DEFAULT_REPEAT_CYCLE;
        default:
            return provider.ConsumeIntegralInRange<uint32_t>(0, DEFAULT_REPEAT_CYCLE * FUZZ_BOUND_TWO);
    }
}

void AddRequestedConditions(WorkInfo &workInfo, FuzzedDataProvider &provider)
{
    uint16_t mask = provider.ConsumeIntegral<uint16_t>();
    if ((mask & (1 << WorkCondition::Type::NETWORK)) != 0) {
        workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(ConsumeEnumEdge(provider,
            WorkCondition::Network::NETWORK_TYPE_ANY, WorkCondition::Network::NETWORK_TYPE_ETHERNET)));
    }
    if ((mask & (1 << WorkCondition::Type::CHARGER)) != 0) {
        workInfo.RequestChargerType(provider.ConsumeBool(), static_cast<WorkCondition::Charger>(ConsumeEnumEdge(
            provider, WorkCondition::Charger::CHARGING_PLUGGED_ANY, WorkCondition::Charger::CHARGING_UNPLUGGED)));
    }
    if ((mask & (1 << WorkCondition::Type::BATTERY_STATUS)) != 0) {
        workInfo.RequestBatteryStatus(static_cast<WorkCondition::BatteryStatus>(ConsumeEnumEdge(provider,
            WorkCondition::BatteryStatus::BATTERY_STATUS_LOW,
            WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY)));
    }
    if ((mask & (1 << WorkCondition::Type::BATTERY_LEVEL)) != 0) {
        workInfo.RequestBatteryLevel(provider.ConsumeIntegralInRange<int32_t>(
            MIN_BATTERY_LEVEL_FOR_FUZZ, MAX_BATTERY_LEVEL_FOR_FUZZ));
    }
    if ((mask & (1 << WorkCondition::Type::STORAGE)) != 0) {
        workInfo.RequestStorageLevel(static_cast<WorkCondition::Storage>(ConsumeEnumEdge(provider,
            WorkCondition::Storage::STORAGE_LEVEL_LOW, WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY)));
    }
    if ((mask & (1 << WorkCondition::Type::TIMER)) != 0) {
        workInfo.RequestRepeatCycle(ConsumeRepeatCycle(provider), provider.ConsumeIntegralInRange<int32_t>(
            FUZZ_INVALID_VALUE, FUZZ_BOUND_FIVE));
    }
    if ((mask & (1 << WorkCondition::Type::DEEP_IDLE)) != 0) {
        workInfo.RequestDeepIdle(provider.ConsumeBool());
    }
}

WorkInfo BuildWorkInfo(FuzzedDataProvider &provider, int32_t index)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(provider.ConsumeBool() ? index : provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_LOW_VALUE, MAX_WORK_ID_FOR_FUZZ));
    workInfo.SetElement(ConsumeText(provider, "bundle.queue.demo"), ConsumeText(provider, "AbilityQueue"));
    workInfo.RequestPersisted(provider.ConsumeBool());
    workInfo.RefreshUid(provider.ConsumeBool() ? DEFAULT_UID : provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_VALUE, MAX_UID_FOR_FUZZ));
    workInfo.RefreshAppIndex(provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_BOUND_THREE));
    workInfo.SetCallBySystemApp(provider.ConsumeBool());
    workInfo.SetPreinstalled(provider.ConsumeBool());
    if (provider.ConsumeBool()) {
        workInfo.RefreshSaId(provider.ConsumeIntegralInRange<int32_t>(0, DEFAULT_SA_ID));
        workInfo.RequestDeepIdle(true);
        workInfo.deepIdleTime_ = provider.ConsumeIntegralInRange<int32_t>(
            FUZZ_INVALID_VALUE, MAX_DEEP_IDLE_TIME_FOR_FUZZ);
    }
    AddRequestedConditions(workInfo, provider);
    return workInfo;
}

std::shared_ptr<DetectorValue> BuildDetectorValue(WorkCondition::Type type,
    const std::shared_ptr<WorkStatus> &pivot, FuzzedDataProvider &provider)
{
    int32_t intValue = provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_LOW_VALUE, MAX_BATTERY_LEVEL_FOR_FUZZ);
    int32_t extraValue = provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_VALUE, DEFAULT_USER_ID + FUZZ_BOUND_THREE);
    bool boolValue = provider.ConsumeBool();
    std::string stringValue = ConsumeText(provider, "bundle.queue.demo");
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
        case WorkCondition::Type::STORAGE:
            intValue = ConsumeEnumEdge(provider, WorkCondition::Storage::STORAGE_LEVEL_LOW,
                WorkCondition::Storage::STORAGE_UNKNOWN);
            break;
        case WorkCondition::Type::GROUP:
            if (pivot && provider.ConsumeBool()) {
                extraValue = pivot->userId_;
                stringValue = pivot->bundleName_;
                boolValue = true;
            }
            break;
        case WorkCondition::Type::DEEP_IDLE:
            intValue = pivot && pivot->workInfo_->IsSA() ? pivot->workInfo_->GetSaId() : DEFAULT_SA_ID;
            break;
        default:
            break;
    }
    return std::make_shared<DetectorValue>(intValue, extraValue, boolValue, stringValue);
}

std::vector<WorkCondition::Type> BuildConditionSequence(FuzzedDataProvider &provider)
{
    std::vector<WorkCondition::Type> sequence = {
        WorkCondition::Type::NETWORK,
        WorkCondition::Type::CHARGER,
        WorkCondition::Type::BATTERY_STATUS,
        WorkCondition::Type::BATTERY_LEVEL,
        WorkCondition::Type::STORAGE,
        WorkCondition::Type::TIMER,
        WorkCondition::Type::GROUP,
        WorkCondition::Type::DEEP_IDLE,
        WorkCondition::Type::STANDBY,
    };
    std::rotate(sequence.begin(), sequence.begin() + provider.ConsumeIntegralInRange<size_t>(0, sequence.size() - 1),
        sequence.end());
    if (provider.ConsumeBool()) {
        sequence.emplace_back(WorkCondition::Type::UNKNOWN);
    }
    return sequence;
}

void DriveWorkStatus(std::shared_ptr<WorkStatus> workStatus, WorkQueue &queue, FuzzedDataProvider &provider)
{
    workStatus->priority_ = provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_LOW_VALUE, MAX_PRIORITY_FOR_FUZZ);
    workStatus->duration_ = provider.ConsumeIntegralInRange<uint64_t>(0, DEFAULT_REPEAT_CYCLE * FUZZ_BOUND_THREE);
    workStatus->paused_ = provider.ConsumeBool();
    workStatus->lastTimeout_ = provider.ConsumeBool();
    workStatus->SetTimeout(provider.ConsumeBool());
    workStatus->SetMinIntervalByGroup(provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, FUZZ_BOUND_FIVE));
    workStatus->SetMinIntervalByDump(provider.ConsumeIntegralInRange<int64_t>(
        FUZZ_INVALID_VALUE, DEFAULT_REPEAT_CYCLE * FUZZ_BOUND_TWO));
    workStatus->MarkRound();
    workStatus->UpdateUidLastTimeMap();
    workStatus->MarkStatus(static_cast<WorkStatus::Status>(provider.ConsumeIntegralInRange<int32_t>(
        WorkStatus::Status::WAIT_CONDITION, WorkStatus::Status::REMOVED)));

    std::string result;
    workStatus->Dump(result);
    workStatus->DumpCondition(result);
    static_cast<void>(workStatus->GetStatus());
    static_cast<void>(workStatus->IsReady());
    static_cast<void>(workStatus->IsSAReady());
    static_cast<void>(workStatus->IsReadyStatus());
    static_cast<void>(workStatus->IsRunning());
    static_cast<void>(workStatus->IsRemoved());
    static_cast<void>(workStatus->IsRepeating());
    static_cast<void>(workStatus->IsPaused());
    static_cast<void>(workStatus->IsLastWorkTimeout());
    static_cast<void>(workStatus->HasTimeout());
    static_cast<void>(workStatus->IsTimeout());
    static_cast<void>(workStatus->NeedRemove());
    static_cast<void>(workStatus->GetMinInterval());
    static_cast<void>(workStatus->TimeUntilLast());

    for (auto type : BuildConditionSequence(provider)) {
        auto detectorValue = BuildDetectorValue(type, workStatus, provider);
        auto condition = queue.ParseCondition(type, detectorValue);
        WorkCondition::Type typeRef = type;
        workStatus->OnConditionChanged(typeRef, condition);
        queue.OnConditionChanged(type, detectorValue);
    }
    workStatus->UpdateTimerIfNeed();
}
} // namespace

void RunWorkStatusQueueFuzzer(FuzzedDataProvider &provider)
{
    WorkQueue queue;
    auto statusVector = std::make_shared<std::vector<std::shared_ptr<WorkStatus>>>();
    int32_t workCount = provider.ConsumeIntegralInRange<int32_t>(1, MAX_WORKS_PER_INPUT);
    for (int32_t index = 0; index < workCount; ++index) {
        WorkInfo workInfo = BuildWorkInfo(provider, index + 1);
        auto workStatus = std::make_shared<WorkStatus>(workInfo,
            provider.ConsumeBool() ? DEFAULT_UID : provider.ConsumeIntegralInRange<int32_t>(
                FUZZ_INVALID_VALUE, MAX_UID_FOR_FUZZ));
        statusVector->emplace_back(workStatus);
        if (provider.ConsumeBool()) {
            queue.Push(workStatus);
        }
    }
    queue.Push(statusVector);

    for (auto &workStatus : *statusVector) {
        DriveWorkStatus(workStatus, queue, provider);
    }

    std::string result;
    queue.GetWorkIdStr(result);
    queue.Dump(result);
    queue.SetMinIntervalByDump(provider.ConsumeIntegralInRange<int64_t>(
        FUZZ_INVALID_VALUE, DEFAULT_REPEAT_CYCLE * FUZZ_BOUND_TWO));
    static_cast<void>(queue.GetSize());
    static_cast<void>(queue.GetRunningCount());
    static_cast<void>(queue.GetRunningWorks());
    static_cast<void>(queue.GetDeepIdleWorks());

    auto selected = statusVector->at(provider.ConsumeIntegralInRange<size_t>(0, statusVector->size() - 1));
    std::string workId = selected->workId_;
    static_cast<void>(queue.Find(workId));
    static_cast<void>(queue.FindSA(selected->workInfo_->GetSaId()));
    static_cast<void>(queue.Contains(std::make_shared<std::string>(workId)));
    static_cast<void>(queue.Find(selected->userId_, selected->bundleName_));
    static_cast<void>(WorkStatus::MakeWorkId(selected->workInfo_->GetWorkId(), selected->uid_));

    if (provider.ConsumeBool()) {
        selected->workStartTime_ = WorkSchedUtils::GetCurrentTimeMs() - provider.ConsumeIntegralInRange<uint64_t>(0,
            DEFAULT_REPEAT_CYCLE * FUZZ_BOUND_THREE);
        selected->SetTimeout(true);
    }
    queue.RemoveUnReady();
    queue.CancelWork(selected);
    queue.Remove(selected);
    WorkStatus::ClearUidLastTimeMap(selected->uid_);
    if (provider.ConsumeBool()) {
        queue.ClearAll();
    }
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    OHOS::WorkScheduler::RunWorkStatusQueueFuzzer(provider);
    return 0;
}
