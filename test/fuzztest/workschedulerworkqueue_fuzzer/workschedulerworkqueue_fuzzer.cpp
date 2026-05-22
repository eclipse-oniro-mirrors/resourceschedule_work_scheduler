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

#include "workschedulerworkqueue_fuzzer.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "work_queue.h"
#include "work_status.h"
#include "work_info.h"
#include "work_condition.h"
#include "detector_value.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr int32_t DEFAULT_UID = 10008;
constexpr size_t MAX_WORK_COUNT = 16;

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

void FuzzPushAndQuery(WorkQueue& queue, const uint8_t* data, size_t size)
{
    size_t workCount = std::min(static_cast<size_t>(data[0] % MAX_WORK_COUNT) + 1, MAX_WORK_COUNT);
    std::vector<std::shared_ptr<WorkStatus>> workVector;
    for (size_t i = 0; i < workCount; ++i) {
        workVector.emplace_back(BuildFuzzWorkStatus(data, size, static_cast<int32_t>(i)));
    }
    auto workStatusVector = std::make_shared<std::vector<std::shared_ptr<WorkStatus>>>(workVector);
    queue.Push(workStatusVector);

    for (size_t i = 0; i < workCount; ++i) {
        queue.Push(BuildFuzzWorkStatus(data, size, static_cast<int32_t>(i + MAX_WORK_COUNT)));
    }

    (void)queue.GetSize();
    (void)queue.GetWorkList();
    (void)queue.GetRunningCount();
    (void)queue.GetRunningWorks();
    (void)queue.GetDeepIdleWorks();

    std::string result;
    queue.GetWorkIdStr(result);
    queue.Dump(result);
}

void FuzzFindAndRemove(WorkQueue& queue, const uint8_t* data, size_t size)
{
    int32_t workId = FuzzInt32(data, size);
    std::string workIdStr = std::to_string(workId);
    (void)queue.Find(workIdStr);
    (void)queue.FindSA(workId);

    int32_t userId = FuzzInt32(data, size, sizeof(int32_t));
    std::string bundleName = "fuzz_bundle";
    (void)queue.Find(userId, bundleName);

    auto workStatus = BuildFuzzWorkStatus(data, size, 0);
    (void)queue.Contains(std::make_shared<std::string>(workStatus->workId_));
    (void)queue.Remove(workStatus);
    (void)queue.CancelWork(workStatus);
}

void FuzzConditionChanged(WorkQueue& queue, const uint8_t* data, size_t size)
{
    for (int32_t type = 0; type < static_cast<int32_t>(WorkCondition::Type::UNKNOWN); ++type) {
        WorkCondition::Type condType = static_cast<WorkCondition::Type>(type);
        auto detectorVal = std::make_shared<DetectorValue>(
            FuzzInt32(data, size),
            FuzzInt32(data, size, sizeof(int32_t)),
            (size > 0) && ((data[0] & 1) != 0),
            "fuzz_str"
        );
        (void)queue.OnConditionChanged(condType, detectorVal);
    }
}

void FuzzMiscOperations(WorkQueue& queue, const uint8_t* data, size_t size)
{
    int64_t interval = static_cast<int64_t>(FuzzInt32(data, size));
    queue.SetMinIntervalByDump(interval);
    queue.RemoveUnReady();
    queue.GetWorkToRunByPriority();
    queue.ClearAll();
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t)) {
        return false;
    }

    WorkQueue queue;
    FuzzPushAndQuery(queue, data, size);
    FuzzFindAndRemove(queue, data, size);
    FuzzConditionChanged(queue, data, size);
    FuzzMiscOperations(queue, data, size);
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
