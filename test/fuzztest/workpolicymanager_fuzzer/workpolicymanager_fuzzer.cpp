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

#include "workpolicymanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "bool_wrapper.h"
#include "detector_value.h"
#include "double_wrapper.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "want_params.h"
#include "watchdog.h"
#include "work_condition.h"
#include "work_conn_manager.h"
#include "work_info.h"
#include "work_policy_manager.h"
#include "work_queue_manager.h"
#include "work_sched_data_manager.h"
#include "work_sched_errors.h"
#include "work_sched_utils.h"
#include "work_scheduler_service.h"
#include "work_status.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t MIN_SIZE = 4;
constexpr size_t MAX_STRING_LEN = 24;
constexpr uint32_t BITS_PER_BYTE = 8;
constexpr uint32_t ALPHABET_COUNT = 26;
constexpr int32_t BASE_UID = 10000;
constexpr int32_t UID_RANGE = 6;
constexpr int32_t WORK_ID_RANGE = 64;
constexpr int32_t USER_ID_RANGE = 4;
constexpr int32_t STATUS_RANGE = 4;
constexpr int32_t NETWORK_RANGE = 7;
constexpr int32_t CHARGER_RANGE = 6;
constexpr int32_t BATTERY_STATUS_RANGE = 4;
constexpr int32_t BATTERY_LEVEL_RANGE = 120;
constexpr int32_t STORAGE_RANGE = 4;
constexpr int32_t TIMER_INTERVAL_RANGE = 100000;
constexpr int32_t TIMER_CYCLE_RANGE = 8;
constexpr int32_t MAX_WORK_PER_UID = 10;
constexpr int32_t EXTRA_WORK_COUNT = 3;
constexpr int32_t WATCHDOG_ID_BASE = 1;
constexpr int32_t WATCHDOG_TIME_RANGE = 120000;
constexpr int32_t DUMP_MEMORY_RANGE = 4096;
constexpr int32_t DUMP_CPU_RANGE = 100;
constexpr int32_t DUMP_THERMAL_RANGE = 8;
constexpr int32_t GROUP_RANGE = 8;
constexpr int32_t DOUBLE_SCALE = 100;

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

class MockWorkConnManager : public WorkConnManager {
public:
    bool StopWork(std::shared_ptr<WorkStatus> workStatus, bool isTimeOut) override
    {
        lastStoppedWork_ = workStatus;
        lastTimeout_ = isTimeOut;
        return stopResult_;
    }

    bool stopResult_ {true};
    bool lastTimeout_ {false};
    std::shared_ptr<WorkStatus> lastStoppedWork_ {nullptr};
};

template<int32_t MOD>
int32_t PositiveMod(int32_t value)
{
    return static_cast<int32_t>(static_cast<uint32_t>(value) % static_cast<uint32_t>(MOD));
}

void AddExtras(FuzzData &provider, WorkInfo &workInfo)
{
    AAFwk::WantParams params;
    params.SetParam("executeImmediate", AAFwk::String::Box(provider.GetBool() ? "false" : "true"));
    params.SetParam(provider.GetString(), AAFwk::Integer::Box(provider.GetInt32()));
    params.SetParam(provider.GetString(), AAFwk::Boolean::Box(provider.GetBool()));
    params.SetParam(provider.GetString(), AAFwk::Double::Box(static_cast<double>(provider.GetInt32()) /
        static_cast<double>(DOUBLE_SCALE)));
    workInfo.RequestExtras(params);
}

void FillWorkInfo(FuzzData &provider, WorkInfo &workInfo, int32_t index, int32_t uid)
{
    workInfo.SetWorkId(index + PositiveMod<WORK_ID_RANGE>(provider.GetInt32()) + 1);
    std::string bundleName = provider.GetBool() ? provider.GetString() : "";
    std::string abilityName = provider.GetBool() ? provider.GetString() : "";
    workInfo.SetElement(bundleName, abilityName);
    workInfo.RefreshUid(uid);
    workInfo.RequestPersisted(provider.GetBool());
    workInfo.SetCallBySystemApp(provider.GetBool());
    workInfo.SetEarliestStartTime(PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32()));

    if (provider.GetBool()) {
        workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(PositiveMod<NETWORK_RANGE>(
            provider.GetInt32())));
    }
    if (provider.GetBool()) {
        workInfo.RequestChargerType(provider.GetBool(), static_cast<WorkCondition::Charger>(
            PositiveMod<CHARGER_RANGE>(provider.GetInt32())));
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
    }
    if (provider.GetBool()) {
        workInfo.RequestDeepIdle(provider.GetBool());
        if (provider.GetBool()) {
            workInfo.RefreshSaId(provider.GetInt32());
        }
    }
    if (provider.GetBool()) {
        AddExtras(provider, workInfo);
    }
    if (workInfo.GetConditionMap()->empty()) {
        workInfo.RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    }
}

std::shared_ptr<WorkStatus> BuildWorkStatus(FuzzData &provider, int32_t index, int32_t uid)
{
    WorkInfo workInfo;
    FillWorkInfo(provider, workInfo, index, uid);
    auto workStatus = std::make_shared<WorkStatus>(workInfo, uid);
    workStatus->userId_ = PositiveMod<USER_ID_RANGE>(provider.GetInt32());
    workStatus->MarkStatus(static_cast<WorkStatus::Status>(PositiveMod<STATUS_RANGE>(provider.GetInt32())));
    uint64_t now = WorkSchedUtils::GetCurrentTimeMs() + 1;
    workStatus->workStartTime_ = provider.GetUInt64() % now;
    workStatus->workWatchDogTime_ = 1 + static_cast<uint64_t>(PositiveMod<WATCHDOG_TIME_RANGE>(provider.GetInt32()));
    workStatus->duration_ = provider.GetUInt64() % static_cast<uint64_t>(WATCHDOG_TIME_RANGE);
    workStatus->paused_ = provider.GetBool();
    workStatus->SetMinIntervalByDump(1 + PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32()));
    return workStatus;
}

std::shared_ptr<WorkPolicyManager> BuildPolicyManager(FuzzData &provider)
{
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    auto policyManager = std::make_shared<WorkPolicyManager>(service);
    policyManager->workConnManager_ = std::make_shared<MockWorkConnManager>();
    policyManager->watchdog_ = std::make_shared<Watchdog>(policyManager, nullptr);
    service->workPolicyManager_ = policyManager;
    if (service->workQueueManager_ == nullptr) {
        service->workQueueManager_ = std::make_shared<WorkQueueManager>(service);
    }
    policyManager->SetMemoryByDump(PositiveMod<DUMP_MEMORY_RANGE>(provider.GetInt32()));
    policyManager->SetCpuUsageByDump(PositiveMod<DUMP_CPU_RANGE>(provider.GetInt32()));
    policyManager->SetThermalLevelByDump(PositiveMod<DUMP_THERMAL_RANGE>(provider.GetInt32()));
    policyManager->SetMaxRunningCountByDump(1 + PositiveMod<MAX_WORK_PER_UID>(provider.GetInt32()));
    policyManager->SetWatchdogTimeByDump(1 + PositiveMod<WATCHDOG_TIME_RANGE>(provider.GetInt32()));
    return policyManager;
}

void PrepareDataManager(FuzzData &provider, const std::shared_ptr<WorkStatus> &workStatus)
{
    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    dataManager->SetDeviceSleep(provider.GetBool());
    dataManager->SetDeepIdle(provider.GetBool());
    dataManager->OnDeviceStandyWhitelistChanged(workStatus->bundleName_, provider.GetBool());
    dataManager->OnDeviceStandyRestrictlistChanged(workStatus->bundleName_, provider.GetBool());
    dataManager->AddGroup(workStatus->bundleName_, workStatus->userId_,
        PositiveMod<GROUP_RANGE>(provider.GetInt32()));
}

void AddQueueScenarios(FuzzData &provider, const std::shared_ptr<WorkPolicyManager> &policyManager,
    std::vector<std::shared_ptr<WorkStatus>> &works)
{
    int32_t uid = BASE_UID + PositiveMod<UID_RANGE>(provider.GetInt32());
    for (int32_t i = 0; i < MAX_WORK_PER_UID + EXTRA_WORK_COUNT; ++i) {
        auto workStatus = BuildWorkStatus(provider, i, uid);
        PrepareDataManager(provider, workStatus);
        works.emplace_back(workStatus);
        policyManager->AddWork(workStatus, uid);
    }
    if (!works.empty()) {
        policyManager->AddWork(works.front(), works.front()->uid_);
        policyManager->FindWorkStatus(*works.front()->workInfo_, works.front()->uid_);
        policyManager->FindWorkStatus(works.front()->uid_, works.front()->workInfo_->GetWorkId());
        policyManager->FindSA(works.front()->workInfo_->GetSaId(), works.front()->uid_);
        policyManager->FindWork(works.front()->uid_);
        policyManager->FindWork(works.front()->userId_, works.front()->bundleName_);
    }
}

void ExerciseReadyAndStop(FuzzData &provider, const std::shared_ptr<WorkPolicyManager> &policyManager,
    const std::vector<std::shared_ptr<WorkStatus>> &works)
{
    auto readyWorks = std::make_shared<std::vector<std::shared_ptr<WorkStatus>>>();
    for (auto &workStatus : works) {
        if (provider.GetBool()) {
            workStatus->MarkStatus(WorkStatus::Status::CONDITION_READY);
        }
        readyWorks->emplace_back(workStatus);
    }
    policyManager->OnConditionReady(nullptr);
    policyManager->OnConditionReady(readyWorks);
    policyManager->CheckWorkToRun();
    if (works.empty()) {
        return;
    }

    auto target = works.front();
    target->MarkStatus(WorkStatus::Status::RUNNING);
    policyManager->StopWork(target, target->uid_, provider.GetBool(), provider.GetBool());
    policyManager->RemoveWork(target, target->uid_);
    policyManager->StopAndClearWorks(target->uid_);
}

void ExerciseWatchdogAndPause(FuzzData &provider, const std::shared_ptr<WorkPolicyManager> &policyManager,
    const std::vector<std::shared_ptr<WorkStatus>> &works)
{
    uint32_t watchdogId = WATCHDOG_ID_BASE;
    for (auto &workStatus : works) {
        workStatus->MarkStatus(WorkStatus::Status::RUNNING);
        workStatus->paused_ = provider.GetBool();
        workStatus->workWatchDogTime_ =
            1 + static_cast<uint64_t>(PositiveMod<WATCHDOG_TIME_RANGE>(provider.GetInt32()));
        workStatus->workStartTime_ = WorkSchedUtils::GetCurrentTimeMs();
        policyManager->watchdogIdMap_.emplace(watchdogId++, workStatus);
    }
    int32_t uid = works.empty() ? BASE_UID : works.front()->uid_;
    policyManager->PauseRunningWorks(uid);
    policyManager->ResumePausedWorks(uid);
    if (!works.empty()) {
        policyManager->RemoveWatchDog(works.front());
        policyManager->WatchdogTimeOut(WATCHDOG_ID_BASE);
    }
}

void ExerciseDumpAndIde(FuzzData &provider, const std::shared_ptr<WorkPolicyManager> &policyManager,
    const std::vector<std::shared_ptr<WorkStatus>> &works)
{
    std::string result;
    policyManager->Dump(result);
    policyManager->DumpTriggerWork(BASE_UID, PositiveMod<WORK_ID_RANGE>(provider.GetInt32()), result);
    if (works.empty()) {
        policyManager->TriggerIdeWork();
        return;
    }
    auto target = works.back();
    target->bundleName_.clear();
    target->abilityName_.clear();
    target->workInfo_->SetElement("", "");
    policyManager->AddWork(target, target->uid_);
    policyManager->DumpCheckIdeWorkToRun(target->bundleName_, target->abilityName_);
    policyManager->DumpTriggerWork(target->uid_, target->workInfo_->GetWorkId(), result);
    policyManager->ideDebugList.emplace_back(target);
    policyManager->TriggerIdeWork();
    target->MarkStatus(WorkStatus::Status::RUNNING);
    policyManager->ideDebugList.emplace_back(target);
    policyManager->TriggerIdeWork();
}

void ExercisePolicyChanges(FuzzData &provider, const std::shared_ptr<WorkPolicyManager> &policyManager,
    const std::vector<std::shared_ptr<WorkStatus>> &works)
{
    auto detectorVal = std::make_shared<DetectorValue>(
        works.empty() ? BASE_UID : works.front()->uid_,
        PositiveMod<TIMER_INTERVAL_RANGE>(provider.GetInt32()), provider.GetBool(), provider.GetString());
    policyManager->OnPolicyChanged(PolicyType::APP_REMOVED, detectorVal);
    std::string dump;
    policyManager->DumpConditionReadyQueue(dump);
    policyManager->DumpUidQueueMap(dump);
    policyManager->GetAllRunningWorks();
    if (!works.empty()) {
        int32_t uid = works.front()->uid_;
        int32_t workId = works.front()->workInfo_->GetWorkId();
        bool timeout = false;
        policyManager->IsLastWorkTimeout(workId, uid, timeout);
        policyManager->GetAllWorkStatus(uid);
        policyManager->ObtainAllWorks(uid);
        policyManager->GetWorkStatus(uid, workId);
        policyManager->GetDeepIdleWorks();
    }
}

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size < MIN_SIZE) {
            return false;
        }
        FuzzData provider(data, size);
        auto policyManager = BuildPolicyManager(provider);
        std::vector<std::shared_ptr<WorkStatus>> works;
        AddQueueScenarios(provider, policyManager, works);
        ExerciseReadyAndStop(provider, policyManager, works);
        ExerciseWatchdogAndPause(provider, policyManager, works);
        ExerciseDumpAndIde(provider, policyManager, works);
        ExercisePolicyChanges(provider, policyManager, works);
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

