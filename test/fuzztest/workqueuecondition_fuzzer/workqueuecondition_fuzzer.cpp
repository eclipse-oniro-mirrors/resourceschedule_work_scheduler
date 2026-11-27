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

#include "workqueuecondition_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "conditions/icondition_listener.h"
#include "detector_value.h"
#include "work_condition.h"
#include "work_info.h"
#include "work_queue.h"
#include "work_queue_manager.h"
#include "work_sched_data_manager.h"
#include "work_scheduler_service.h"
#include "work_status.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t MIN_SIZE = 4;
constexpr size_t MAX_STRING_LEN = 24;
constexpr int32_t MAX_WORK_COUNT = 8;
constexpr int32_t TYPE_COUNT = static_cast<int32_t>(WorkCondition::Type::UNKNOWN) + 1;
constexpr uint32_t BITS_PER_BYTE = 8;
constexpr uint32_t ALPHABET_COUNT = 26;
constexpr int32_t DETECTOR_VALUE_RANGE = 16;
constexpr int32_t DETECTOR_TIME_RANGE = 100000;
constexpr int32_t BASE_UID = 10000;
constexpr int32_t UID_RANGE = 8;
constexpr int32_t WORK_ID_RANGE = 32;
constexpr int32_t USER_ID_RANGE = 4;
constexpr int32_t STATUS_RANGE = 4;
constexpr int32_t NETWORK_RANGE = 7;
constexpr int32_t CHARGER_RANGE = 6;
constexpr int32_t BATTERY_STATUS_RANGE = 4;
constexpr int32_t BATTERY_LEVEL_RANGE = 120;
constexpr int32_t STORAGE_RANGE = 4;
constexpr int32_t TIMER_INTERVAL_RANGE = 100000;
constexpr int32_t TIMER_CYCLE_RANGE = 5;
constexpr int32_t GROUP_RANGE = 8;
constexpr int32_t MIN_INTERVAL_RANGE = 10000;

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

class DummyConditionListener : public IConditionListener {
public:
    void OnConditionChanged(WorkCondition::Type conditionType, std::shared_ptr<DetectorValue> conditionVal) override
    {
        lastType_ = conditionType;
        lastValue_ = conditionVal;
    }

    bool Start() override
    {
        started_ = true;
        return true;
    }

    bool Stop() override
    {
        started_ = false;
        return true;
    }

    bool started_ {false};
    WorkCondition::Type lastType_ {WorkCondition::Type::UNKNOWN};
    std::shared_ptr<DetectorValue> lastValue_ {nullptr};
};

template<int32_t MOD>
int32_t PositiveMod(int32_t value)
{
    return static_cast<int32_t>(static_cast<uint32_t>(value) % static_cast<uint32_t>(MOD));
}

std::shared_ptr<DetectorValue> BuildDetectorValue(FuzzData &provider)
{
    int32_t intVal = PositiveMod<DETECTOR_VALUE_RANGE>(provider.GetInt32());
    int32_t timeVal = PositiveMod<DETECTOR_TIME_RANGE>(provider.GetInt32());
    bool boolVal = provider.GetBool();
    std::string strVal = provider.GetString();
    return std::make_shared<DetectorValue>(intVal, timeVal, boolVal, strVal);
}

void FillWorkInfo(FuzzData &provider, WorkInfo &workInfo, int32_t index)
{
    int32_t uid = BASE_UID + PositiveMod<UID_RANGE>(provider.GetInt32());
    workInfo.SetWorkId(index + 1 + PositiveMod<WORK_ID_RANGE>(provider.GetInt32()));
    std::string bundleName = provider.GetString();
    std::string abilityName = provider.GetString();
    workInfo.SetElement(bundleName, abilityName);
    workInfo.RefreshUid(uid);
    workInfo.RequestPersisted(provider.GetBool());
    workInfo.SetEarliestStartTime(PositiveMod<MIN_INTERVAL_RANGE>(provider.GetInt32()));

    if (provider.GetBool()) {
        workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(PositiveMod<NETWORK_RANGE>(
            provider.GetInt32())));
    }
    if (provider.GetBool()) {
        bool isCharging = provider.GetBool();
        WorkCondition::Charger chargerType = static_cast<WorkCondition::Charger>(
            PositiveMod<CHARGER_RANGE>(provider.GetInt32()));
        workInfo.RequestChargerType(isCharging, chargerType);
    }
    if (provider.GetBool()) {
        workInfo.RequestBatteryStatus(static_cast<WorkCondition::BatteryStatus>(
            PositiveMod<BATTERY_STATUS_RANGE>(provider.GetInt32())));
    }
    if (provider.GetBool()) {
        workInfo.RequestBatteryLevel(PositiveMod<BATTERY_LEVEL_RANGE>(provider.GetInt32()));
    }
    if (provider.GetBool()) {
        workInfo.RequestStorageLevel(static_cast<WorkCondition::Storage>(PositiveMod<STORAGE_RANGE>(
            provider.GetInt32())));
    }
    if (provider.GetBool()) {
        uint32_t interval = 1 + static_cast<uint32_t>(PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32()));
        if (provider.GetBool()) {
            workInfo.RequestRepeatCycle(interval);
        } else {
            workInfo.RequestRepeatCycle(interval, PositiveMod<TIMER_CYCLE_RANGE>(provider.GetInt32()));
        }
        workInfo.RequestBaseTime(static_cast<time_t>(provider.GetInt32()));
    }
    if (provider.GetBool()) {
        workInfo.RequestDeepIdle(provider.GetBool());
        if (provider.GetBool()) {
            workInfo.RefreshSaId(provider.GetInt32());
        }
    }
    if (workInfo.GetConditionMap()->empty()) {
        workInfo.RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    }
}

std::shared_ptr<WorkStatus> BuildWorkStatus(FuzzData &provider, int32_t index)
{
    WorkInfo workInfo;
    FillWorkInfo(provider, workInfo, index);
    int32_t uid = workInfo.GetUid() > 0 ? workInfo.GetUid() : BASE_UID + index;
    auto workStatus = std::make_shared<WorkStatus>(workInfo, uid);
    workStatus->userId_ = PositiveMod<USER_ID_RANGE>(provider.GetInt32());
    workStatus->MarkStatus(static_cast<WorkStatus::Status>(PositiveMod<STATUS_RANGE>(provider.GetInt32())));
    workStatus->workStartTime_ = static_cast<uint64_t>(PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32()));
    workStatus->workWatchDogTime_ = static_cast<uint64_t>(PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32()));
    workStatus->duration_ = static_cast<uint64_t>(PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32()));
    workStatus->paused_ = provider.GetBool();
    workStatus->SetMinIntervalByDump(1 + PositiveMod<MIN_INTERVAL_RANGE>(provider.GetInt32()));
    return workStatus;
}

void PrepareDataManager(FuzzData &provider)
{
    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    std::string bundleName = provider.GetString();
    if (bundleName.empty()) {
        bundleName = "bundle";
    }
    std::string restrictBundle = provider.GetString();
    bool isSleep = provider.GetBool();
    bool isDeepIdle = provider.GetBool();
    bool inWhitelist = provider.GetBool();
    bool inRestrictlist = provider.GetBool();
    int32_t userId = PositiveMod<USER_ID_RANGE>(provider.GetInt32());
    int32_t appGroup = PositiveMod<GROUP_RANGE>(provider.GetInt32());
    dataManager->SetDeviceSleep(isSleep);
    dataManager->SetDeepIdle(isDeepIdle);
    dataManager->OnDeviceStandyWhitelistChanged(bundleName, inWhitelist);
    dataManager->OnDeviceStandyRestrictlistChanged(restrictBundle, inRestrictlist);
    dataManager->AddGroup(bundleName, userId, appGroup);
}

void ResetDataManager()
{
    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    dataManager->ClearDeviceStandyWhitelist();
    dataManager->ClearDeviceStandyRestrictlist();
    dataManager->ClearAllGroup();
}

void FuzzWorkQueue(FuzzData &provider)
{
    WorkQueue queue;
    std::vector<std::shared_ptr<WorkStatus>> works;
    int32_t count = 1 + (provider.GetU8() % MAX_WORK_COUNT);
    for (int32_t i = 0; i < count; ++i) {
        auto work = BuildWorkStatus(provider, i);
        works.emplace_back(work);
        queue.Push(work);
    }

    auto vectorPtr = std::make_shared<std::vector<std::shared_ptr<WorkStatus>>>(works);
    queue.Push(vectorPtr);

    for (int32_t i = 0; i < TYPE_COUNT; ++i) {
        WorkCondition::Type type = static_cast<WorkCondition::Type>(i);
        auto value = BuildDetectorValue(provider);
        queue.ParseCondition(type, value);
        queue.OnConditionChanged(type, value);
    }

    if (!works.empty()) {
        queue.Contains(std::make_shared<std::string>(works.front()->workId_));
        queue.Find(works.front()->workId_);
        queue.FindSA(works.front()->workInfo_->GetSaId());
        queue.Find(works.front()->userId_, works.front()->bundleName_);
        queue.GetWorkToRunByPriority();
        queue.GetRunningWorks();
        queue.GetDeepIdleWorks();
        queue.GetRunningCount();
        std::string dump;
        queue.GetWorkIdStr(dump);
        queue.Dump(dump);
        queue.SetMinIntervalByDump(provider.GetInt32());
        queue.RemoveUnReady();
        queue.CancelWork(works.front());
        queue.Remove(works.front());
    }
    queue.ClearAll();
}

void FuzzWorkQueueManager(FuzzData &provider)
{
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    auto manager = std::make_shared<WorkQueueManager>(service);
    for (int32_t i = 0; i < TYPE_COUNT; ++i) {
        manager->AddListener(static_cast<WorkCondition::Type>(i), std::make_shared<DummyConditionListener>());
    }
    int32_t count = 1 + (provider.GetU8() % MAX_WORK_COUNT);
    std::list<std::shared_ptr<WorkStatus>> workList;
    for (int32_t i = 0; i < count; ++i) {
        auto work = BuildWorkStatus(provider, i);
        manager->AddWork(work);
        workList.emplace_back(work);
    }

    manager->SetTimeCycle(static_cast<uint32_t>(PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32())));
    manager->SetTimeRetrigger(provider.GetInt32());
    for (int32_t i = 0; i < TYPE_COUNT; ++i) {
        auto type = static_cast<WorkCondition::Type>(i);
        auto value = BuildDetectorValue(provider);
        manager->GetReayQueue(type, value);
        manager->OnConditionChanged(type, value);
    }
    std::string dump;
    manager->Dump(dump);
    manager->SetMinIntervalByDump(provider.GetInt32());
    manager->ClearTimeOutWorkStatus();
    manager->StopAndClearWorks(workList);
}
} // namespace

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size < MIN_SIZE) {
            return false;
        }
        FuzzData provider(data, size);
        PrepareDataManager(provider);
        FuzzWorkQueue(provider);
        FuzzWorkQueueManager(provider);
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

