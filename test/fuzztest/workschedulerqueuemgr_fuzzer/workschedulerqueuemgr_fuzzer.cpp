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

#include "workschedulerqueuemgr_fuzzer.h"

#include <cstdint>
#include <cstring>

#include "securec.h"
#include <memory>
#include <string>
#include <vector>
#include <list>

#include "work_queue_manager.h"
#include "work_scheduler_service.h"
#include "work_status.h"
#include "work_info.h"
#include "work_condition.h"
#include "detector_value.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr int32_t DEFAULT_UID = 10008;
constexpr size_t NETWORK_TYPE_COUNT = 8;
constexpr int32_t ADD_WORK_COUNT = 4;
constexpr int32_t CLEAR_WORK_COUNT = 3;


int32_t FuzzInt32(const uint8_t* data, size_t size, size_t offset = 0)
{
    if (data == nullptr || size < offset + sizeof(int32_t)) {
        return 0;
    }
    int32_t value = 0;
    memcpy_s(&value, sizeof(value), data + offset, sizeof(value));
    return value;
}

WorkInfo BuildFuzzWorkInfo(const uint8_t* data, size_t size, int32_t index)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(FuzzInt32(data, size, sizeof(int32_t) * index));
    workInfo.SetElement("fuzz_bundle_" + std::to_string(index), "fuzz_ability_" + std::to_string(index));
    workInfo.RequestPersisted((size > static_cast<size_t>(index)) && ((data[index] & 1) != 0));
    workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(FuzzInt32(data, size) % NETWORK_TYPE_COUNT));
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

void FuzzAddAndQuery(WorkQueueManager& queueMgr, const uint8_t* data, size_t size)
{
    for (int32_t i = 0; i < ADD_WORK_COUNT; ++i) {
        auto workStatus = BuildFuzzWorkStatus(data, size, i);
        queueMgr.AddWork(workStatus);
    }

    std::string result;
    queueMgr.Dump(result);

    (void)queueMgr.GetTimeCycle();
    (void)queueMgr.GetTimeRetrigger();
}

void FuzzRemoveAndCancel(WorkQueueManager& queueMgr, const uint8_t* data, size_t size)
{
    auto workStatus = BuildFuzzWorkStatus(data, size, 0);
    (void)queueMgr.RemoveWork(workStatus);

    auto cancelStatus = BuildFuzzWorkStatus(data, size, 1);
    (void)queueMgr.CancelWork(cancelStatus);
}

void FuzzConditionChanged(WorkQueueManager& queueMgr, const uint8_t* data, size_t size)
{
    for (int32_t type = 0; type < static_cast<int32_t>(WorkCondition::Type::UNKNOWN); ++type) {
        WorkCondition::Type condType = static_cast<WorkCondition::Type>(type);
        auto detectorVal = std::make_shared<DetectorValue>(
            FuzzInt32(data, size),
            FuzzInt32(data, size, sizeof(int32_t)),
            (size > 0) && ((data[0] & 1) != 0),
            "fuzz_str"
        );
        queueMgr.OnConditionChanged(condType, detectorVal);
    }
}

void FuzzTimeConfig(WorkQueueManager& queueMgr, const uint8_t* data, size_t size)
{
    uint32_t timeCycle = static_cast<uint32_t>(FuzzInt32(data, size));
    queueMgr.SetTimeCycle(timeCycle);
    (void)queueMgr.GetTimeCycle();

    int32_t timeRetrigger = FuzzInt32(data, size, sizeof(int32_t));
    queueMgr.SetTimeRetrigger(timeRetrigger);
    (void)queueMgr.GetTimeRetrigger();

    int64_t interval = static_cast<int64_t>(FuzzInt32(data, size, sizeof(int32_t) * 2));
    queueMgr.SetMinIntervalByDump(interval);
}

void FuzzStopAndClear(WorkQueueManager& queueMgr, const uint8_t* data, size_t size)
{
    std::list<std::shared_ptr<WorkStatus>> workList;
    for (int32_t i = 0; i < CLEAR_WORK_COUNT; ++i) {
        workList.emplace_back(BuildFuzzWorkStatus(data, size, i));
    }
    (void)queueMgr.StopAndClearWorks(workList);
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

    auto queueMgr = std::make_shared<WorkQueueManager>(service);
    if (queueMgr == nullptr) {
        return false;
    }

    queueMgr->Init();
    FuzzAddAndQuery(*queueMgr, data, size);
    FuzzRemoveAndCancel(*queueMgr, data, size);
    FuzzConditionChanged(*queueMgr, data, size);
    FuzzTimeConfig(*queueMgr, data, size);
    FuzzStopAndClear(*queueMgr, data, size);
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
