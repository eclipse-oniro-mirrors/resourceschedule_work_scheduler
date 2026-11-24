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

#include "workstatuscondition_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "bool_wrapper.h"
#include "detector_value.h"
#include "double_wrapper.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "want_params.h"
#include "work_condition.h"
#include "work_info.h"
#include "work_sched_data_manager.h"
#include "work_sched_utils.h"
#include "work_scheduler_service.h"
#include "work_status.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t MIN_SIZE = 4;
constexpr size_t MAX_STRING_LEN = 24;
constexpr int32_t TYPE_COUNT = static_cast<int32_t>(WorkCondition::Type::UNKNOWN) + 1;
constexpr int32_t ENUM_SPREAD = 8;
constexpr uint32_t BITS_PER_BYTE = 8;
constexpr uint32_t ALPHABET_COUNT = 26;
constexpr int32_t NETWORK_RANGE = 7;
constexpr int32_t CHARGER_RANGE = 6;
constexpr int32_t BATTERY_STATUS_RANGE = 4;
constexpr int32_t BATTERY_LEVEL_RANGE = 120;
constexpr int32_t STORAGE_RANGE = 4;
constexpr int32_t TIMER_INTERVAL_RANGE = 100000;
constexpr int32_t TIMER_CYCLE_RANGE = 8;
constexpr int32_t USER_ID_RANGE = 4;
constexpr int32_t STATUS_RANGE = 4;
constexpr int32_t WORK_ID_RANGE = 64;
constexpr int32_t BASE_UID = 10000;
constexpr int32_t UID_RANGE = 16;
constexpr int32_t APP_INDEX_RANGE = 128;
constexpr int32_t FUZZ_STATUS_COUNT = 4;
constexpr int32_t MILLISECONDS_PER_SECOND = 1000;
constexpr int32_t SECONDS_PER_MINUTE = 60;
constexpr int32_t LONG_TIMEOUT_MINUTES = 10;
constexpr int32_t DOUBLE_SCALE = 100;
constexpr uint64_t LONG_TIMEOUT = LONG_TIMEOUT_MINUTES * SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;

class FuzzData {
public:
    FuzzData(const uint8_t *data, size_t size) : data_(data), size_(size) {}

    uint8_t GetU8()
    {
        if (offset_ >= size_) {
            return 0;
        }
        return data_[offset_++];
    }

    bool GetBool()
    {
        return (GetU8() & 1) != 0;
    }

    int32_t GetInt32()
    {
        uint32_t value = 0;
        for (size_t i = 0; i < sizeof(uint32_t); ++i) {
            value = (value << BITS_PER_BYTE) | GetU8();
        }
        return static_cast<int32_t>(value);
    }

    uint64_t GetUInt64()
    {
        uint64_t value = 0;
        for (size_t i = 0; i < sizeof(uint64_t); ++i) {
            value = (value << BITS_PER_BYTE) | GetU8();
        }
        return value;
    }

    std::string GetString()
    {
        size_t len = GetU8() % (MAX_STRING_LEN + 1);
        std::string result;
        result.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            result.push_back(static_cast<char>('a' + (GetU8() % ALPHABET_COUNT)));
        }
        return result;
    }

private:
    const uint8_t *data_ {nullptr};
    size_t size_ {0};
    size_t offset_ {0};
};

template<int32_t MOD>
int32_t PositiveMod(int32_t value)
{
    return static_cast<int32_t>(static_cast<uint32_t>(value) % static_cast<uint32_t>(MOD));
}

std::shared_ptr<Condition> MakeCondition(FuzzData &provider, WorkCondition::Type type)
{
    auto condition = std::make_shared<Condition>();
    condition->enumVal = PositiveMod<ENUM_SPREAD>(provider.GetInt32());
    condition->intVal = provider.GetInt32();
    condition->uintVal = static_cast<uint32_t>(PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32()));
    condition->boolVal = provider.GetBool();
    condition->timeVal = static_cast<time_t>(provider.GetInt32());
    condition->strVal = provider.GetString();
    switch (type) {
        case WorkCondition::Type::NETWORK:
            condition->enumVal = PositiveMod<NETWORK_RANGE>(provider.GetInt32());
            break;
        case WorkCondition::Type::CHARGER:
            condition->enumVal = PositiveMod<CHARGER_RANGE>(provider.GetInt32());
            condition->boolVal = provider.GetBool();
            break;
        case WorkCondition::Type::BATTERY_STATUS:
            condition->enumVal = PositiveMod<BATTERY_STATUS_RANGE>(provider.GetInt32());
            break;
        case WorkCondition::Type::BATTERY_LEVEL:
            condition->intVal = PositiveMod<BATTERY_LEVEL_RANGE>(provider.GetInt32());
            break;
        case WorkCondition::Type::STORAGE:
            condition->enumVal = PositiveMod<STORAGE_RANGE>(provider.GetInt32());
            break;
        case WorkCondition::Type::TIMER:
            condition->uintVal = 1 + static_cast<uint32_t>(PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32()));
            condition->intVal = PositiveMod<TIMER_CYCLE_RANGE>(provider.GetInt32());
            condition->boolVal = provider.GetBool();
            break;
        case WorkCondition::Type::GROUP:
        case WorkCondition::Type::DEEP_IDLE:
            condition->strVal = provider.GetString();
            condition->intVal = PositiveMod<USER_ID_RANGE>(provider.GetInt32());
            condition->enumVal = PositiveMod<ENUM_SPREAD>(provider.GetInt32());
            condition->boolVal = provider.GetBool();
            break;
        default:
            break;
    }
    return condition;
}

void AddRequestCondition(FuzzData &provider, WorkInfo &workInfo, WorkCondition::Type type)
{
    switch (type) {
        case WorkCondition::Type::NETWORK:
            workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(PositiveMod<NETWORK_RANGE>(
                provider.GetInt32())));
            break;
        case WorkCondition::Type::CHARGER: {
            bool isCharging = provider.GetBool();
            WorkCondition::Charger chargerType = static_cast<WorkCondition::Charger>(
                PositiveMod<CHARGER_RANGE>(provider.GetInt32()));
            workInfo.RequestChargerType(isCharging, chargerType);
            break;
        }
        case WorkCondition::Type::BATTERY_STATUS:
            workInfo.RequestBatteryStatus(static_cast<WorkCondition::BatteryStatus>(
                PositiveMod<BATTERY_STATUS_RANGE>(provider.GetInt32())));
            break;
        case WorkCondition::Type::BATTERY_LEVEL:
            workInfo.RequestBatteryLevel(PositiveMod<BATTERY_LEVEL_RANGE>(provider.GetInt32()));
            break;
        case WorkCondition::Type::STORAGE:
            workInfo.RequestStorageLevel(static_cast<WorkCondition::Storage>(PositiveMod<STORAGE_RANGE>(
                provider.GetInt32())));
            break;
        case WorkCondition::Type::TIMER: {
            uint32_t interval = 1 + static_cast<uint32_t>(PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32()));
            if (provider.GetBool()) {
                workInfo.RequestRepeatCycle(interval);
            } else {
                workInfo.RequestRepeatCycle(interval, PositiveMod<TIMER_CYCLE_RANGE>(provider.GetInt32()));
            }
            workInfo.RequestBaseTime(static_cast<time_t>(provider.GetInt32()));
            break;
        }
        case WorkCondition::Type::DEEP_IDLE:
            workInfo.RequestDeepIdle(provider.GetBool());
            workInfo.SetDeepIdleTime(PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32()));
            break;
        default:
            break;
    }
}

void AddExtras(FuzzData &provider, WorkInfo &workInfo)
{
    AAFwk::WantParams params;
    std::string executeImmediate = provider.GetBool() ? "false" : "true";
    params.SetParam("executeImmediate", AAFwk::String::Box(executeImmediate));
    std::string intKey = provider.GetString();
    int32_t intValue = provider.GetInt32();
    params.SetParam(intKey, AAFwk::Integer::Box(intValue));
    std::string boolKey = provider.GetString();
    bool boolValue = provider.GetBool();
    params.SetParam(boolKey, AAFwk::Boolean::Box(boolValue));
    std::string doubleKey = provider.GetString();
    double doubleValue = static_cast<double>(provider.GetInt32()) / static_cast<double>(DOUBLE_SCALE);
    params.SetParam(doubleKey, AAFwk::Double::Box(doubleValue));
    workInfo.RequestExtras(params);
}

WorkInfo BuildWorkInfo(FuzzData &provider, int32_t index)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(index + 1 + PositiveMod<WORK_ID_RANGE>(provider.GetInt32()));
    std::string bundleName = provider.GetString();
    std::string abilityName = provider.GetString();
    workInfo.SetElement(bundleName.empty() ? "bundle" : bundleName, abilityName.empty() ? "ability" : abilityName);
    workInfo.RefreshUid(BASE_UID + PositiveMod<UID_RANGE>(provider.GetInt32()));
    workInfo.RefreshAppIndex(PositiveMod<APP_INDEX_RANGE>(provider.GetInt32()));
    workInfo.RequestPersisted(provider.GetBool());
    workInfo.SetPreinstalled(provider.GetBool());
    workInfo.SetCallBySystemApp(provider.GetBool());
    workInfo.SetEarliestStartTime(PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32()));
    if (provider.GetBool()) {
        workInfo.RefreshSaId(provider.GetInt32());
    }

    int32_t conditionCount = 1 + (provider.GetU8() % TYPE_COUNT);
    for (int32_t i = 0; i < conditionCount; ++i) {
        AddRequestCondition(provider, workInfo, static_cast<WorkCondition::Type>(i % TYPE_COUNT));
    }
    if (provider.GetBool()) {
        AddExtras(provider, workInfo);
    }
    if (workInfo.GetConditionMap()->empty()) {
        workInfo.RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    }
    return workInfo;
}

std::shared_ptr<WorkStatus> BuildWorkStatus(FuzzData &provider, int32_t index)
{
    WorkInfo workInfo = BuildWorkInfo(provider, index);
    auto workStatus = std::make_shared<WorkStatus>(workInfo, workInfo.GetUid());
    workStatus->userId_ = PositiveMod<USER_ID_RANGE>(provider.GetInt32());
    workStatus->MarkStatus(static_cast<WorkStatus::Status>(PositiveMod<STATUS_RANGE>(provider.GetInt32())));
    uint64_t now = WorkSchedUtils::GetCurrentTimeMs() + 1;
    workStatus->workStartTime_ = provider.GetUInt64() % now;
    workStatus->workWatchDogTime_ = provider.GetBool() ? LONG_TIMEOUT + provider.GetUInt64() % LONG_TIMEOUT :
        provider.GetUInt64() % LONG_TIMEOUT;
    workStatus->duration_ = provider.GetUInt64() % LONG_TIMEOUT;
    workStatus->paused_ = provider.GetBool();
    workStatus->lastTimeout_ = provider.GetBool();
    workStatus->SetTimeout(provider.GetBool());
    workStatus->SetMinIntervalByDump(1 + PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32()));
    return workStatus;
}

void PrepareDataManager(FuzzData &provider, const std::shared_ptr<WorkStatus> &workStatus)
{
    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    dataManager->SetDeviceSleep(provider.GetBool());
    dataManager->SetDeepIdle(provider.GetBool());
    dataManager->OnDeviceStandyWhitelistChanged(workStatus->bundleName_, provider.GetBool());
    dataManager->OnDeviceStandyRestrictlistChanged(workStatus->bundleName_, provider.GetBool());
    dataManager->AddGroup(workStatus->bundleName_, workStatus->userId_,
        PositiveMod<ENUM_SPREAD>(provider.GetInt32()));
    DelayedSingleton<WorkSchedulerService>::GetInstance()->UpdateEffiResApplyInfo(
        workStatus->uid_, provider.GetBool());
}

void ResetDataManager()
{
    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    dataManager->ClearDeviceStandyWhitelist();
    dataManager->ClearDeviceStandyRestrictlist();
    dataManager->ClearAllGroup();
}

void FillCurrentConditions(FuzzData &provider, const std::shared_ptr<WorkStatus> &workStatus)
{
    auto requestMap = workStatus->workInfo_->GetConditionMap();
    for (auto &item : *requestMap) {
        if (provider.GetBool()) {
            workStatus->conditionMap_[item.first] = item.second;
        } else {
            workStatus->conditionMap_[item.first] = MakeCondition(provider, item.first);
        }
    }
}

void ExerciseStatus(FuzzData &provider, const std::shared_ptr<WorkStatus> &workStatus)
{
    PrepareDataManager(provider, workStatus);
    FillCurrentConditions(provider, workStatus);

    for (int32_t i = 0; i < TYPE_COUNT; ++i) {
        auto type = static_cast<WorkCondition::Type>(i);
        auto condition = MakeCondition(provider, type);
        workStatus->OnConditionChanged(type, condition);
        workStatus->ToString(type);
    }

    workStatus->IsReady();
    workStatus->IsSAReady();
    workStatus->IsConditionReady();
    for (int32_t i = 0; i < TYPE_COUNT; ++i) {
        auto type = static_cast<WorkCondition::Type>(i);
        bool missingCurrentCondition = workStatus->conditionMap_.count(type) == 0;
        bool hasRequestCondition = workStatus->workInfo_->GetConditionMap()->count(type) > 0;
        if (missingCurrentCondition && hasRequestCondition) {
            workStatus->conditionMap_[type] = MakeCondition(provider, type);
        }
        if (workStatus->conditionMap_.count(type) > 0 && hasRequestCondition) {
            workStatus->IsBatteryAndNetworkReady(type);
            workStatus->IsChargerReady(type);
            workStatus->IsStorageReady(type);
            workStatus->IsTimerReady(type);
            workStatus->IsNapReady(type);
        }
    }
    workStatus->NeedRemove();
    workStatus->IsUriKeySwitchOn();
    workStatus->IsStandbyExemption();
    workStatus->TimeUntilLast();
    workStatus->SetMinIntervalByGroup(PositiveMod<ENUM_SPREAD>(provider.GetInt32()));
    workStatus->UpdateUidLastTimeMap();
    WorkStatus::ClearUidLastTimeMap(workStatus->uid_);
    workStatus->HasTimeout();
    workStatus->IsTimeout();
    workStatus->SetTimeout(provider.GetBool());
    workStatus->IsSpecial();
    workStatus->CheckEarliestStartTime();
    workStatus->IsNeedDiscreteScheduled();
    workStatus->GetPriority();
    std::string dump;
    workStatus->Dump(dump);
    workStatus->DumpCondition(dump);
}
} // namespace

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size < MIN_SIZE) {
            return false;
        }
        FuzzData provider(data, size);
        for (int32_t i = 0; i < FUZZ_STATUS_COUNT; ++i) {
            auto workStatus = BuildWorkStatus(provider, i);
            ExerciseStatus(provider, workStatus);
        }
        ResetDataManager();
        return true;
    }
} // namespace WorkScheduler
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

