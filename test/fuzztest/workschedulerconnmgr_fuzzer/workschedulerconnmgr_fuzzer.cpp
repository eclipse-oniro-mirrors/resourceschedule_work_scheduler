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

#include "workschedulerconnmgr_fuzzer.h"

#include <cstdint>
#include <cstring>

#include "securec.h"
#include <memory>
#include <string>

#include "work_conn_manager.h"
#include "work_status.h"
#include "work_info.h"
#include "work_condition.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr int32_t DEFAULT_UID = 10008;
constexpr size_t NETWORK_TYPE_COUNT = 8;
constexpr size_t CHARGER_TYPE_COUNT = 5;
constexpr size_t BATTERY_STATUS_COUNT = 4;
constexpr size_t STORAGE_LEVEL_COUNT = 4;


int32_t FuzzInt32(const uint8_t* data, size_t size, size_t offset = 0)
{
    if (data == nullptr || size < offset + sizeof(int32_t)) {
        return 0;
    }
    int32_t value = 0;
    memcpy_s(&value, sizeof(value), data + offset, sizeof(value));
    return value;
}

WorkInfo BuildFuzzWorkInfoWithConditions(const uint8_t* data, size_t size, int32_t index)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(FuzzInt32(data, size, sizeof(int32_t) * index));
    workInfo.SetElement("fuzz_bundle_" + std::to_string(index), "fuzz_ability_" + std::to_string(index));
    workInfo.RequestPersisted((size > static_cast<size_t>(index)) && ((data[index] & 1) != 0));
    workInfo.RefreshUid(FuzzInt32(data, size, sizeof(int32_t)));

    // Set various conditions based on fuzz data to exercise WriteStartWorkEvent paths
    uint8_t selector = (size > sizeof(int32_t)) ? data[sizeof(int32_t)] : 0;
    if (selector & 0x01) {
        workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(
            FuzzInt32(data, size) % NETWORK_TYPE_COUNT));
    }
    if (selector & 0x02) {
        workInfo.RequestChargerType(static_cast<WorkCondition::Charger>(
            FuzzInt32(data, size, sizeof(int32_t)) % CHARGER_TYPE_COUNT));
    }
    if (selector & 0x04) {
        workInfo.RequestBatteryStatus(static_cast<WorkCondition::BatteryStatus>(
            FuzzInt32(data, size, sizeof(int32_t) * 2) % BATTERY_STATUS_COUNT));
    }
    if (selector & 0x08) {
        workInfo.RequestBatteryLevel(FuzzInt32(data, size, sizeof(int32_t) * 3));
    }
    if (selector & 0x10) {
        workInfo.RequestStorageLevel(static_cast<WorkCondition::Storage>(
            FuzzInt32(data, size, sizeof(int32_t) * 4) % STORAGE_LEVEL_COUNT));
    }
    if (selector & 0x20) {
        workInfo.RequestRepeatCycle(static_cast<uint32_t>(FuzzInt32(data, size, sizeof(int32_t) * 5)));
    }
    if (selector & 0x40) {
        workInfo.RequestDeepIdle(WorkCondition::DeepIdle::DEEP_IDLE_IN);
    }
    return workInfo;
}

std::shared_ptr<WorkStatus> BuildFuzzWorkStatusWithConditions(const uint8_t* data, size_t size, int32_t index)
{
    WorkInfo workInfo = BuildFuzzWorkInfoWithConditions(data, size, index);
    int32_t uid = FuzzInt32(data, size, sizeof(int32_t) * 2);
    if (uid == 0) {
        uid = DEFAULT_UID;
    }
    return std::make_shared<WorkStatus>(workInfo, uid);
}

void FuzzStartWork(WorkConnManager& connMgr, const uint8_t* data, size_t size)
{
    for (int32_t i = 0; i < 3; ++i) {
        auto workStatus = BuildFuzzWorkStatusWithConditions(data, size, i);
        (void)connMgr.StartWork(workStatus);
    }
}

void FuzzStopWork(WorkConnManager& connMgr, const uint8_t* data, size_t size)
{
    auto workStatus = BuildFuzzWorkStatusWithConditions(data, size, 0);
    bool isTimeOut = (size > 0) && ((data[0] & 1) != 0);
    (void)connMgr.StopWork(workStatus, isTimeOut);
}

void FuzzWriteStartWorkEvent(WorkConnManager& connMgr, const uint8_t* data, size_t size)
{
    // Test with different condition combinations to exercise all WriteStartWorkEvent paths
    for (int32_t i = 0; i < 4; ++i) {
        auto workStatus = BuildFuzzWorkStatusWithConditions(data, size, i);
        connMgr.WriteStartWorkEvent(workStatus);
    }
}

void FuzzRepeatedStartStop(WorkConnManager& connMgr, const uint8_t* data, size_t size)
{
    // Test repeated start/stop cycles to exercise connection map management
    auto workStatus = BuildFuzzWorkStatusWithConditions(data, size, 0);
    (void)connMgr.StartWork(workStatus);
    (void)connMgr.StartWork(workStatus); // Duplicate start - exercises existing connection path
    (void)connMgr.StopWork(workStatus, false);
    (void)connMgr.StopWork(workStatus, true); // Stop after already stopped
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t)) {
        return false;
    }

    WorkConnManager connMgr;
    FuzzStartWork(connMgr, data, size);
    FuzzStopWork(connMgr, data, size);
    FuzzWriteStartWorkEvent(connMgr, data, size);
    FuzzRepeatedStartStop(connMgr, data, size);
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
