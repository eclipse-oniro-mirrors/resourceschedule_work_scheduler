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
constexpr uint64_t LONG_TIMEOUT = 10 * 60 * 1000;

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
            value = (value << 8) | GetU8();
        }
        return static_cast<int32_t>(value);
    }

    uint64_t GetUInt64()
    {
        uint64_t value = 0;
        for (size_t i = 0; i < sizeof(uint64_t); ++i) {
            value = (value << 8) | GetU8();
        }
        return value;
    }

    std::string GetString()
    {
        size_t len = GetU8() % (MAX_STRING_LEN + 1);
        std::string result;
        result.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            result.push_back(static_cast<char>('a' + (GetU8() % 26)));
        }
        return result;
    }

private:
    const uint8_t *data_ {nullptr};
    size_t size_ {0};
    size_t offset_ {0};
};

int32_t PositiveMod(int32_t value, int32_t mod)
{
    if (mod <= 0) {
        return 0;
    }
    int64_t result = static_cast<int64_t>(value) % mod;
    return static_cast<int32_t>(result < 0 ? result + mod : result);
}

std::shared_ptr<Condition> MakeCondition(FuzzData &provider, WorkCondition::Type type)
{
    auto condition = std::make_shared<Condition>();
    condition->enumVal = PositiveMod(provider.GetInt32(), ENUM_SPREAD);
    condition->intVal = provider.GetInt32();
    condition->uintVal = static_cast<uint32_t>(PositiveMod(provider.GetInt32(), 100000));
    condition->boolVal = provider.GetBool();
    condition->timeVal = static_cast<time_t>(provider.GetInt32());
    condition->strVal = provider.GetString();
    switch (type) {
        case WorkCondition::Type::NETWORK:
            condition->enumVal = PositiveMod(provider.GetInt32(), 7);
            break;
        case WorkCondition::Type::CHARGER:
            condition->enumVal = PositiveMod(provider.GetInt32(), 6);
            condition->boolVal = provider.GetBool();
            break;
        case WorkCondition::Type::BATTERY_STATUS:
            condition->enumVal = PositiveMod(provider.GetInt32(), 4);
            break;
        case WorkCondition::Type::BATTERY_LEVEL:
            condition->intVal = PositiveMod(provider.GetInt32(), 120);
            break;
        case WorkCondition::Type::STORAGE:
            condition->enumVal = PositiveMod(provider.GetInt32(), 4);
            break;
        case WorkCondition::Type::TIMER:
            condition->uintVal = 1 + static_cast<uint32_t>(PositiveMod(provider.GetInt32(), 100000));
            condition->intVal = PositiveMod(provider.GetInt32(), 8);
            condition->boolVal = provider.GetBool();
            break;
        case WorkCondition::Type::GROUP:
        case WorkCondition::Type::DEEP_IDLE:
            condition->strVal = provider.GetString();
            condition->intVal = PositiveMod(provider.GetInt32(), 4);
            condition->enumVal = PositiveMod(provider.GetInt32(), ENUM_SPREAD);
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
            workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(PositiveMod(provider.GetInt32(), 7)));
            break;
        case WorkCondition::Type::CHARGER:
            workInfo.RequestChargerType(provider.GetBool(),
                static_cast<WorkCondition::Charger>(PositiveMod(provider.GetInt32(), 6)));
            break;
        case WorkCondition::Type::BATTERY_STATUS:
            workInfo.RequestBatteryStatus(static_cast<WorkCondition::BatteryStatus>(PositiveMod(provider.GetInt32(), 4)));
            break;
        case WorkCondition::Type::BATTERY_LEVEL:
            workInfo.RequestBatteryLevel(PositiveMod(provider.GetInt32(), 120));
            break;
        case WorkCondition::Type::STORAGE:
            workInfo.RequestStorageLevel(static_cast<WorkCondition::Storage>(PositiveMod(provider.GetInt32(), 4)));
            break;
        case WorkCondition::Type::TIMER: {
            uint32_t interval = 1 + static_cast<uint32_t>(PositiveMod(provider.GetInt32(), 100000));
            if (provider.GetBool()) {
                workInfo.RequestRepeatCycle(interval);
            } else {
                workInfo.RequestRepeatCycle(interval, PositiveMod(provider.GetInt32(), 8));
            }
            workInfo.RequestBaseTime(static_cast<time_t>(provider.GetInt32()));
            break;
        }
        case WorkCondition::Type::DEEP_IDLE:
            workInfo.RequestDeepIdle(provider.GetBool());
            workInfo.SetDeepIdleTime(PositiveMod(provider.GetInt32(), 100000));
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
    params.SetParam(provider.GetString(), AAFwk::Integer::Box(provider.GetInt32()));
    params.SetParam(provider.GetString(), AAFwk::Boolean::Box(provider.GetBool()));
    params.SetParam(provider.GetString(), AAFwk::Double::Box(static_cast<double>(provider.GetInt32()) / 100.0));
    workInfo.RequestExtras(params);
}

WorkInfo BuildWorkInfo(FuzzData &provider, int32_t index)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(index + 1 + PositiveMod(provider.GetInt32(), 64));
    std::string bundleName = provider.GetString();
    std::string abilityName = provider.GetString();
    workInfo.SetElement(bundleName.empty() ? "bundle" : bundleName, abilityName.empty() ? "ability" : abilityName);
    workInfo.RefreshUid(10000 + PositiveMod(provider.GetInt32(), 16));
    workInfo.RefreshAppIndex(PositiveMod(provider.GetInt32(), 128));
    workInfo.RequestPersisted(provider.GetBool());
    workInfo.SetPreinstalled(provider.GetBool());
    workInfo.SetCallBySystemApp(provider.GetBool());
    workInfo.SetEarliestStartTime(PositiveMod(provider.GetInt32(), 100000));
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
    workStatus->userId_ = PositiveMod(provider.GetInt32(), 4);
    workStatus->MarkStatus(static_cast<WorkStatus::Status>(PositiveMod(provider.GetInt32(), 4)));
    workStatus->workStartTime_ = provider.GetUInt64() % (WorkSchedUtils::GetCurrentTimeMs() + 1);
    workStatus->workWatchDogTime_ = provider.GetBool() ? LONG_TIMEOUT + provider.GetUInt64() % LONG_TIMEOUT :
        provider.GetUInt64() % LONG_TIMEOUT;
    workStatus->duration_ = provider.GetUInt64() % LONG_TIMEOUT;
    workStatus->paused_ = provider.GetBool();
    workStatus->lastTimeout_ = provider.GetBool();
    workStatus->SetTimeout(provider.GetBool());
    workStatus->SetMinIntervalByDump(1 + PositiveMod(provider.GetInt32(), 100000));
    return workStatus;
}

void PrepareDataManager(FuzzData &provider, const std::shared_ptr<WorkStatus> &workStatus)
{
    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    dataManager->SetDeviceSleep(provider.GetBool());
    dataManager->SetDeepIdle(provider.GetBool());
    dataManager->OnDeviceStandyWhitelistChanged(workStatus->bundleName_, provider.GetBool());
    dataManager->OnDeviceStandyRestrictlistChanged(workStatus->bundleName_, provider.GetBool());
    dataManager->AddGroup(workStatus->bundleName_, workStatus->userId_, PositiveMod(provider.GetInt32(), ENUM_SPREAD));
    DelayedSingleton<WorkSchedulerService>::GetInstance()->UpdateEffiResApplyInfo(workStatus->uid_, provider.GetBool());
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
        if (workStatus->conditionMap_.count(type) == 0 && workStatus->workInfo_->GetConditionMap()->count(type) > 0) {
            workStatus->conditionMap_[type] = MakeCondition(provider, type);
        }
        if (workStatus->conditionMap_.count(type) > 0 && workStatus->workInfo_->GetConditionMap()->count(type) > 0) {
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
    workStatus->SetMinIntervalByGroup(PositiveMod(provider.GetInt32(), ENUM_SPREAD));
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
        for (int32_t i = 0; i < 4; ++i) {
            auto workStatus = BuildWorkStatus(provider, i);
            ExerciseStatus(provider, workStatus);
        }
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

