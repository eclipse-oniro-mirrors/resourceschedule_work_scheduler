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

int32_t PositiveMod(int32_t value, int32_t mod)
{
    if (mod <= 0) {
        return 0;
    }
    int64_t result = static_cast<int64_t>(value) % mod;
    return static_cast<int32_t>(result < 0 ? result + mod : result);
}

std::shared_ptr<DetectorValue> BuildDetectorValue(FuzzData &provider)
{
    return std::make_shared<DetectorValue>(
        PositiveMod(provider.GetInt32(), 16), PositiveMod(provider.GetInt32(), 100000),
        provider.GetBool(), provider.GetString());
}

void FillWorkInfo(FuzzData &provider, WorkInfo &workInfo, int32_t index)
{
    int32_t uid = 10000 + PositiveMod(provider.GetInt32(), 8);
    workInfo.SetWorkId(index + 1 + PositiveMod(provider.GetInt32(), 32));
    workInfo.SetElement(provider.GetString(), provider.GetString());
    workInfo.RefreshUid(uid);
    workInfo.RequestPersisted(provider.GetBool());
    workInfo.SetEarliestStartTime(PositiveMod(provider.GetInt32(), 10000));

    if (provider.GetBool()) {
        workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(PositiveMod(provider.GetInt32(), 7)));
    }
    if (provider.GetBool()) {
        workInfo.RequestChargerType(provider.GetBool(), static_cast<WorkCondition::Charger>(
            PositiveMod(provider.GetInt32(), 6)));
    }
    if (provider.GetBool()) {
        workInfo.RequestBatteryStatus(static_cast<WorkCondition::BatteryStatus>(PositiveMod(provider.GetInt32(), 4)));
    }
    if (provider.GetBool()) {
        workInfo.RequestBatteryLevel(PositiveMod(provider.GetInt32(), 120));
    }
    if (provider.GetBool()) {
        workInfo.RequestStorageLevel(static_cast<WorkCondition::Storage>(PositiveMod(provider.GetInt32(), 4)));
    }
    if (provider.GetBool()) {
        uint32_t interval = 1 + static_cast<uint32_t>(PositiveMod(provider.GetInt32(), 100000));
        if (provider.GetBool()) {
            workInfo.RequestRepeatCycle(interval);
        } else {
            workInfo.RequestRepeatCycle(interval, PositiveMod(provider.GetInt32(), 5));
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
    int32_t uid = workInfo.GetUid() > 0 ? workInfo.GetUid() : 10000 + index;
    auto workStatus = std::make_shared<WorkStatus>(workInfo, uid);
    workStatus->userId_ = PositiveMod(provider.GetInt32(), 4);
    workStatus->MarkStatus(static_cast<WorkStatus::Status>(PositiveMod(provider.GetInt32(), 4)));
    workStatus->workStartTime_ = static_cast<uint64_t>(PositiveMod(provider.GetInt32(), 100000));
    workStatus->workWatchDogTime_ = static_cast<uint64_t>(PositiveMod(provider.GetInt32(), 100000));
    workStatus->duration_ = static_cast<uint64_t>(PositiveMod(provider.GetInt32(), 100000));
    workStatus->paused_ = provider.GetBool();
    workStatus->SetMinIntervalByDump(1 + PositiveMod(provider.GetInt32(), 10000));
    return workStatus;
}

void PrepareDataManager(FuzzData &provider)
{
    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    std::string bundleName = provider.GetString();
    if (bundleName.empty()) {
        bundleName = "bundle";
    }
    dataManager->SetDeviceSleep(provider.GetBool());
    dataManager->SetDeepIdle(provider.GetBool());
    dataManager->OnDeviceStandyWhitelistChanged(bundleName, provider.GetBool());
    dataManager->OnDeviceStandyRestrictlistChanged(provider.GetString(), provider.GetBool());
    dataManager->AddGroup(bundleName, PositiveMod(provider.GetInt32(), 4), PositiveMod(provider.GetInt32(), 8));
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

    manager->SetTimeCycle(static_cast<uint32_t>(PositiveMod(provider.GetInt32(), 100000)));
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

