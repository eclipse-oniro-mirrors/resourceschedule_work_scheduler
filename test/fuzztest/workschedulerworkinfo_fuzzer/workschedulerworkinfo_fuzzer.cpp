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

#include "workschedulerworkinfo_fuzzer.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "securec.h"
#include <string>

#include "parcel.h"
#include "work_info.h"
#include "work_condition.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t MAX_FUZZ_STRING_LEN = 512;
constexpr size_t NETWORK_TYPE_COUNT = 8;
constexpr size_t CHARGER_TYPE_COUNT = 7;
constexpr size_t BATTERY_STATUS_COUNT = 4;
constexpr size_t STORAGE_LEVEL_COUNT = 4;
constexpr size_t BYTE_CHECK_INDEX_1 = 1;
constexpr size_t BYTE_CHECK_INDEX_2 = 2;
constexpr size_t BYTE_CHECK_INDEX_3 = 3;
constexpr size_t OFFSET_UID = 5;
constexpr size_t OFFSET_SA_ID = 6;
constexpr size_t OFFSET_PREINSTALLED = 7;
constexpr size_t OFFSET_DEEP_IDLE_TIME = 8;
constexpr size_t OFFSET_TRIGGER_TYPE = 9;
constexpr size_t OFFSET_BATTERY_LEVEL = 2;
constexpr size_t OFFSET_REPEAT_CYCLE = 3;
constexpr size_t OFFSET_REPEAT_CYCLE_COUNT = 4;

std::string FuzzString(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return "";
    }
    size_t len = std::min(size, MAX_FUZZ_STRING_LEN);
    return std::string(reinterpret_cast<const char*>(data), len);
}

int32_t FuzzInt32(const uint8_t* data, size_t size, size_t offset = 0)
{
    if (data == nullptr || size < offset + sizeof(int32_t)) {
        return 0;
    }
    int32_t value = 0;
    memcpy_s(&value, sizeof(value), data + offset, sizeof(value));
    return value;
}

void FuzzWorkInfoSetters(WorkInfo& workInfo, const uint8_t* data, size_t size)
{
    workInfo.SetWorkId(FuzzInt32(data, size));
    std::string fuzzStr = FuzzString(data, size);
    if (!fuzzStr.empty()) {
        workInfo.SetElement(fuzzStr, fuzzStr);
    }
    workInfo.RequestPersisted((size > 0) && ((data[0] & 1) != 0));
    workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(FuzzInt32(data, size) % NETWORK_TYPE_COUNT));
    workInfo.RequestChargerType((size > BYTE_CHECK_INDEX_1) && ((data[BYTE_CHECK_INDEX_1] & 1) != 0),
        static_cast<WorkCondition::Charger>(FuzzInt32(data, size, sizeof(int32_t)) % CHARGER_TYPE_COUNT));
    workInfo.RequestBatteryLevel(
        FuzzInt32(data, size, sizeof(int32_t) * OFFSET_BATTERY_LEVEL));
    workInfo.RequestBatteryStatus(static_cast<WorkCondition::BatteryStatus>(
        FuzzInt32(data, size, BYTE_CHECK_INDEX_1) % BATTERY_STATUS_COUNT));
    workInfo.RequestStorageLevel(static_cast<WorkCondition::Storage>(
        FuzzInt32(data, size, BYTE_CHECK_INDEX_2) % STORAGE_LEVEL_COUNT));
    uint32_t interval = static_cast<uint32_t>(
        FuzzInt32(data, size, sizeof(int32_t) * OFFSET_REPEAT_CYCLE));
    if (interval > 0) {
        workInfo.RequestRepeatCycle(interval,
            FuzzInt32(data, size, sizeof(int32_t) * OFFSET_REPEAT_CYCLE_COUNT));
    }
    workInfo.RequestDeepIdle(true);
    workInfo.RefreshUid(FuzzInt32(data, size, sizeof(int32_t) * OFFSET_UID));
    workInfo.RefreshSaId(FuzzInt32(data, size, sizeof(int32_t) * OFFSET_SA_ID));
    workInfo.SetPreinstalled((size > BYTE_CHECK_INDEX_2) && ((data[BYTE_CHECK_INDEX_2] & 1) != 0));
    workInfo.SetIsInnerApply((size > BYTE_CHECK_INDEX_3) && ((data[BYTE_CHECK_INDEX_3] & 1) != 0));
    workInfo.SetEarliestStartTime(FuzzInt32(data, size, sizeof(int32_t) * OFFSET_PREINSTALLED));
    workInfo.SetDeepIdleTime(FuzzInt32(data, size, sizeof(int32_t) * OFFSET_DEEP_IDLE_TIME));
    workInfo.SetTriggerType(FuzzInt32(data, size, sizeof(int32_t) * OFFSET_TRIGGER_TYPE));
}

void FuzzMarshallingRoundTrip(WorkInfo& workInfo)
{
    MessageParcel parcel;
    if (!workInfo.Marshalling(parcel)) {
        return;
    }
    MessageParcel parcelIn;
    const void* parcelData = reinterpret_cast<const void*>(parcel.GetData());
    if (parcelData == nullptr || !parcelIn.WriteBuffer(parcelData, parcel.GetDataSize())) {
        return;
    }
    parcelIn.RewindRead(0);
    WorkInfo* out = WorkInfo::Unmarshalling(parcelIn);
    if (out != nullptr) {
        std::string dumpResult;
        out->Dump(dumpResult);
        (void)out->GetBriefInfo();
        (void)out->ParseToJsonStr();
        delete out;
    }
}

void FuzzJsonPaths(WorkInfo& workInfo, const uint8_t* data, size_t size)
{
    std::string jsonStr = FuzzString(data, size);
    nlohmann::json jsonObj = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObj.is_discarded()) {
        return;
    }
    workInfo.ParseFromJson(jsonObj);
    if (jsonObj.is_object()) {
        workInfo.ParseElementFromJson(jsonObj);
    }
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    WorkInfo workInfo;
    FuzzWorkInfoSetters(workInfo, data, size);
    FuzzMarshallingRoundTrip(workInfo);
    FuzzJsonPaths(workInfo, data, size);
    std::string dump;
    workInfo.Dump(dump);
    (void)workInfo.ParseToJsonStr();
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
