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
#include <string>

#include "parcel.h"
#include "work_info.h"
#include "work_condition.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t MAX_FUZZ_STRING_LEN = 512;

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
    memcpy(&value, data + offset, sizeof(value));
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
    workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(FuzzInt32(data, size) % 8));
    workInfo.RequestChargerType((size > 1) && ((data[1] & 1) != 0),
        static_cast<WorkCondition::Charger>(FuzzInt32(data, size, sizeof(int32_t)) % 7));
    workInfo.RequestBatteryLevel(FuzzInt32(data, size, sizeof(int32_t) * 2));
    workInfo.RequestBatteryStatus(static_cast<WorkCondition::BatteryStatus>(FuzzInt32(data, size, 1) % 4));
    workInfo.RequestStorageLevel(static_cast<WorkCondition::Storage>(FuzzInt32(data, size, 2) % 4));
    uint32_t interval = static_cast<uint32_t>(FuzzInt32(data, size, sizeof(int32_t) * 3));
    if (interval > 0) {
        workInfo.RequestRepeatCycle(interval, FuzzInt32(data, size, sizeof(int32_t) * 4));
    }
    workInfo.RequestDeepIdle(true);
    workInfo.RefreshUid(FuzzInt32(data, size, sizeof(int32_t) * 5));
    workInfo.RefreshSaId(FuzzInt32(data, size, sizeof(int32_t) * 6));
    workInfo.SetPreinstalled((size > 2) && ((data[2] & 1) != 0));
    workInfo.SetIsInnerApply((size > 3) && ((data[3] & 1) != 0));
    workInfo.SetEarliestStartTime(FuzzInt32(data, size, sizeof(int32_t) * 7));
    workInfo.SetDeepIdleTime(FuzzInt32(data, size, sizeof(int32_t) * 8));
    workInfo.SetTriggerType(FuzzInt32(data, size, sizeof(int32_t) * 9));
}

void FuzzMarshallingRoundTrip(WorkInfo& workInfo)
{
    MessageParcel parcel;
    if (!workInfo.Marshalling(parcel)) {
        return;
    }
    MessageParcel parcelIn;
    parcelIn.WriteBuffer(parcel.GetData(), parcel.GetDataSize());
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
