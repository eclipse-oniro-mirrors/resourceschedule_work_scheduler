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

#include "workschedulerpolicymgr_fuzzer.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "work_policy_manager.h"
#include "work_scheduler_service.h"
#include "work_queue_manager.h"
#include "work_conn_manager.h"
#include "work_status.h"
#include "work_info.h"
#include "work_condition.h"
#include "detector_value.h"
#include "policy_type.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr int32_t DEFAULT_UID = 10008;

int32_t FuzzInt32(const uint8_t* data, size_t size, size_t offset = 0)
{
    if (data == nullptr || size < offset + sizeof(int32_t)) {
        return 0;
    }
    int32_t value = 0;
    memcpy(&value, data + offset, sizeof(value));
    return value;
}

WorkInfo BuildFuzzWorkInfo(const uint8_t* data, size_t size, int32_t index)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(FuzzInt32(data, size, sizeof(int32_t) * index));
    workInfo.SetElement("fuzz_bundle_" + std::to_string(index), "fuzz_ability_" + std::to_string(index));
    workInfo.RequestPersisted((size > static_cast<size_t>(index)) && ((data[index] & 1) != 0));
    workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(FuzzInt32(data, size) % 8));
    workInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY);
    workInfo.RefreshUid(FuzzInt32(data, size, sizeof(int32_t)));
    return workInfo;
}

std::shared_ptr<WorkStatus> BuildFuzzWorkStatus(const uint8_t* data, size_t size, int32_t index)
{
    WorkInfo workInfo = BuildFuzzWorkInfo(data, size, index);
    int32_t uid = FuzzInt32(data, size, sizeof(int32_t) * 2);
    if (uid == 0) {
        uid = DEFAULT_UID;
    }
    return std::make_shared<WorkStatus>(workInfo, uid);
}

void FuzzAddAndFind(WorkPolicyManager& policyMgr, const uint8_t* data, size_t size)
{
    int32_t uid = FuzzInt32(data, size);
    if (uid == 0) {
        uid = DEFAULT_UID;
    }

    for (int32_t i = 0; i < 4; ++i) {
        auto workStatus = BuildFuzzWorkStatus(data, size, i);
        policyMgr.AddWork(workStatus, uid);
    }

    WorkInfo queryInfo = BuildFuzzWorkInfo(data, size, 0);
    (void)policyMgr.FindWorkStatus(queryInfo, uid);
    (void)policyMgr.FindWorkStatus(uid, FuzzInt32(data, size, sizeof(int32_t)));
    (void)policyMgr.FindSA(FuzzInt32(data, size, sizeof(int32_t) * 2), uid);
    (void)policyMgr.ObtainAllWorks(uid);
    (void)policyMgr.GetAllWorkStatus(uid);

    int32_t workId = FuzzInt32(data, size, sizeof(int32_t) * 3);
    (void)policyMgr.GetWorkStatus(uid, workId);
    (void)policyMgr.GetAllRunningWorks();
    (void)policyMgr.FindWork(uid);
}

void FuzzPolicyChanged(WorkPolicyManager& policyMgr, const uint8_t* data, size_t size)
{
    PolicyType policyTypes[] = {
        PolicyType::APP_REMOVED,
        PolicyType::APP_DATA_CLEAR,
        PolicyType::APP_ADDED,
        PolicyType::APP_CHANGED,
        PolicyType::USER_SWITCHED,
        PolicyType::USER_STARTED
    };
    for (auto policyType : policyTypes) {
        auto detectorVal = std::make_shared<DetectorValue>(
            FuzzInt32(data, size),
            FuzzInt32(data, size, sizeof(int32_t)),
            (size > 0) && ((data[0] & 1) != 0),
            "fuzz_str"
        );
        policyMgr.OnPolicyChanged(policyType, detectorVal);
    }
}

void FuzzDumpAndConfig(WorkPolicyManager& policyMgr, const uint8_t* data, size_t size)
{
    std::string result;
    policyMgr.Dump(result);

    int32_t memory = FuzzInt32(data, size);
    policyMgr.SetMemoryByDump(memory);
    (void)policyMgr.GetDumpSetMemory();

    int32_t watchdogTime = FuzzInt32(data, size, sizeof(int32_t));
    policyMgr.SetWatchdogTimeByDump(watchdogTime);
    policyMgr.SetWatchdogTime(watchdogTime);
    (void)policyMgr.GetWatchdogTime();

    int32_t cpu = FuzzInt32(data, size, sizeof(int32_t) * 2);
    policyMgr.SetCpuUsageByDump(cpu);
    (void)policyMgr.GetDumpSetCpuUsage();

    int32_t maxCount = FuzzInt32(data, size, sizeof(int32_t) * 3);
    policyMgr.SetMaxRunningCountByDump(maxCount);
    (void)policyMgr.GetDumpSetMaxRunningCount();

    int32_t thermalLevel = FuzzInt32(data, size, sizeof(int32_t) * 4);
    policyMgr.SetThermalLevelByDump(thermalLevel);
    (void)policyMgr.GetDumpSetThermalLevel();

    int32_t delayTime = FuzzInt32(data, size, sizeof(int32_t) * 5);
    policyMgr.SendRetrigger(delayTime);

    int32_t interval = FuzzInt32(data, size, sizeof(int32_t) * 6);
    policyMgr.SetMinIntervalByDump(interval);

    policyMgr.CheckWorkToRun();
    policyMgr.TriggerIdeWork();
}

void FuzzPauseResume(WorkPolicyManager& policyMgr, const uint8_t* data, size_t size)
{
    int32_t uid = FuzzInt32(data, size);
    if (uid == 0) {
        uid = DEFAULT_UID;
    }
    (void)policyMgr.PauseRunningWorks(uid);
    (void)policyMgr.ResumePausedWorks(uid);
    (void)policyMgr.StopAndClearWorks(uid);
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t)) {
        return false;
    }

    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (service == nullptr) {
        return false;
    }

    auto policyMgr = std::make_shared<WorkPolicyManager>(service);
    if (policyMgr == nullptr) {
        return false;
    }

    FuzzAddAndFind(*policyMgr, data, size);
    FuzzPolicyChanged(*policyMgr, data, size);
    FuzzDumpAndConfig(*policyMgr, data, size);
    FuzzPauseResume(*policyMgr, data, size);
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
