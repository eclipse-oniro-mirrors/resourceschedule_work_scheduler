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

#include "workschedulerworkstatus_fuzzer.h"

#include <cstdint>
#include <cstring>

#include "securec.h"
#include <memory>
#include <string>

#include "work_status.h"
#include "work_info.h"
#include "work_condition.h"
#include "detector_value.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr int32_t DEFAULT_UID = 10008;
constexpr size_t NETWORK_TYPE_COUNT = 7;
constexpr size_t CONDITION_TYPE_COUNT = 10;
constexpr size_t BATTERY_STATUS_COUNT = 4;
constexpr size_t STORAGE_LEVEL_COUNT = 4;
constexpr size_t BYTE_CHECK_INDEX_1 = 1;
constexpr size_t BYTE_CHECK_INDEX_2 = 2;
constexpr size_t OFFSET_REPEAT_CYCLE = 3;
constexpr size_t OFFSET_UID = 4;
constexpr size_t OFFSET_MIN_INTERVAL_GROUP = 6;
constexpr size_t OFFSET_MIN_INTERVAL_DUMP = 7;


int32_t FuzzInt32(const uint8_t* data, size_t size, size_t offset = 0)
{
    if (data == nullptr || size < offset + sizeof(int32_t)) {
        return 0;
    }
    int32_t value = 0;
    memcpy_s(&value, sizeof(value), data + offset, sizeof(value));
    return value;
}

WorkInfo BuildFuzzWorkInfo(const uint8_t* data, size_t size)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(FuzzInt32(data, size));
    workInfo.SetElement("fuzz_bundle", "fuzz_ability");
    workInfo.RequestPersisted((size > 0) && ((data[0] & 1) != 0));
    workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(
        FuzzInt32(data, size, sizeof(int32_t)) % NETWORK_TYPE_COUNT));
    workInfo.RequestBatteryStatus(static_cast<WorkCondition::BatteryStatus>(
        FuzzInt32(data, size, BYTE_CHECK_INDEX_1) % BATTERY_STATUS_COUNT));
    workInfo.RequestStorageLevel(static_cast<WorkCondition::Storage>(
        FuzzInt32(data, size, BYTE_CHECK_INDEX_2) % STORAGE_LEVEL_COUNT));
    workInfo.RequestRepeatCycle(static_cast<uint32_t>(FuzzInt32(data, size, sizeof(int32_t) * OFFSET_REPEAT_CYCLE)));
    workInfo.RefreshUid(FuzzInt32(data, size, sizeof(int32_t) * OFFSET_UID));
    return workInfo;
}

void FuzzConditionChange(WorkStatus& workStatus, const uint8_t* data, size_t size)
{
    auto condition = std::make_shared<Condition>();
    condition->enumVal = FuzzInt32(data, size);
    condition->intVal = FuzzInt32(data, size, sizeof(int32_t));
    condition->boolVal = (size > 1) && ((data[1] & 1) != 0);
    WorkCondition::Type type = static_cast<WorkCondition::Type>(
        FuzzInt32(data, size, sizeof(int32_t) * 2) % CONDITION_TYPE_COUNT);
    workStatus.OnConditionChanged(type, condition);
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    WorkInfo workInfo = BuildFuzzWorkInfo(data, size);
    int32_t uid = FuzzInt32(data, size, sizeof(int32_t) * 5);
    if (uid == 0) {
        uid = DEFAULT_UID;
    }
    WorkStatus workStatus(workInfo, uid);
    std::string result;
    workStatus.Dump(result);
    workStatus.DumpCondition(result);
    (void)workStatus.IsReady();
    (void)workStatus.IsSAReady();
    (void)workStatus.IsReadyStatus();
    (void)workStatus.IsRunning();
    (void)workStatus.IsRemoved();
    (void)workStatus.IsRepeating();
    (void)workStatus.IsPaused();
    (void)workStatus.IsLastWorkTimeout();
    workStatus.MarkStatus(WorkStatus::Status::CONDITION_READY);
    (void)workStatus.GetStatus();
    workStatus.SetMinIntervalByGroup(FuzzInt32(data, size, sizeof(int32_t) * OFFSET_MIN_INTERVAL_GROUP));
    workStatus.SetMinIntervalByDump(FuzzInt32(data, size, sizeof(int32_t) * OFFSET_MIN_INTERVAL_DUMP));
    (void)workStatus.GetMinInterval();
    FuzzConditionChange(workStatus, data, size);
    WorkStatus::ClearUidLastTimeMap(uid);
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
