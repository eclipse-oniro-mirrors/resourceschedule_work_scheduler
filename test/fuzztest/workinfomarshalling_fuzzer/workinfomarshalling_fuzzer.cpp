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

#include "workinfomarshalling_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "message_parcel.h"
#include "nlohmann/json.hpp"
#include "bool_wrapper.h"
#include "double_wrapper.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "want_params.h"
#include "work_condition.h"
#include "work_info.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t MIN_SIZE = 4;
constexpr size_t MAX_STRING_LEN = 32;
constexpr int32_t ENUM_SPREAD = 8;
constexpr uint32_t MIN_REPEAT_TIME = 1;
constexpr uint32_t MAX_REPEAT_TIME = 24 * 60 * 60 * 1000;

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

    uint32_t GetUInt32()
    {
        return static_cast<uint32_t>(GetInt32());
    }

    std::string GetString(size_t maxLen = MAX_STRING_LEN)
    {
        size_t len = GetU8() % (maxLen + 1);
        std::string result;
        result.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            char ch = static_cast<char>('a' + (GetU8() % 26));
            if ((GetU8() & 3) == 0) {
                ch = static_cast<char>('0' + (GetU8() % 10));
            }
            result.push_back(ch);
        }
        return result;
    }

private:
    const uint8_t *data_ {nullptr};
    size_t size_ {0};
    size_t offset_ {0};
};

template<typename T>
T PickEnum(FuzzData &provider, int32_t spread = ENUM_SPREAD)
{
    if (spread <= 0) {
        return static_cast<T>(0);
    }
    int64_t value = static_cast<int64_t>(provider.GetInt32()) % spread;
    return static_cast<T>(value < 0 ? value + spread : value);
}

int32_t PositiveMod(int32_t value, int32_t mod)
{
    if (mod <= 0) {
        return 0;
    }
    int64_t result = static_cast<int64_t>(value) % mod;
    return static_cast<int32_t>(result < 0 ? result + mod : result);
}

void AddExtras(FuzzData &provider, WorkInfo &workInfo)
{
    AAFwk::WantParams params;
    int32_t count = provider.GetU8() % 4;
    for (int32_t i = 0; i < count; ++i) {
        std::string key = provider.GetString();
        if (key.empty()) {
            key = "executeImmediate";
        }
        switch (provider.GetU8() % 4) {
            case 0:
                params.SetParam(key, AAFwk::Integer::Box(provider.GetInt32()));
                break;
            case 1:
                params.SetParam(key, AAFwk::String::Box(provider.GetBool() ? "false" : provider.GetString()));
                break;
            case 2:
                params.SetParam(key, AAFwk::Boolean::Box(provider.GetBool()));
                break;
            default:
                params.SetParam(key, AAFwk::Double::Box(static_cast<double>(provider.GetInt32()) / 100.0));
                break;
        }
    }
    workInfo.RequestExtras(params);
}

WorkInfo BuildWorkInfo(FuzzData &provider, bool allowExtras = true)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(provider.GetInt32());
    workInfo.SetElement(provider.GetString(), provider.GetString());
    workInfo.RequestPersisted(provider.GetBool());
    workInfo.RefreshUid(provider.GetInt32());
    workInfo.RefreshAppIndex(PositiveMod(provider.GetInt32(), 128));
    workInfo.SetPreinstalled(provider.GetBool());
    workInfo.SetCallBySystemApp(provider.GetBool());
    workInfo.SetEarliestStartTime(PositiveMod(provider.GetInt32(), MAX_REPEAT_TIME));
    if (provider.GetBool()) {
        workInfo.RefreshSaId(provider.GetInt32());
    }
    if (provider.GetBool()) {
        workInfo.RequestNetworkType(PickEnum<WorkCondition::Network>(provider));
    }
    if (provider.GetBool()) {
        workInfo.RequestChargerType(provider.GetBool(), PickEnum<WorkCondition::Charger>(provider));
    }
    if (provider.GetBool()) {
        workInfo.RequestBatteryLevel(PositiveMod(provider.GetInt32(), 150));
    }
    if (provider.GetBool()) {
        workInfo.RequestBatteryStatus(PickEnum<WorkCondition::BatteryStatus>(provider));
    }
    if (provider.GetBool()) {
        workInfo.RequestStorageLevel(PickEnum<WorkCondition::Storage>(provider));
    }
    if (provider.GetBool()) {
        uint32_t interval = MIN_REPEAT_TIME + provider.GetUInt32() % MAX_REPEAT_TIME;
        if (provider.GetBool()) {
            workInfo.RequestRepeatCycle(interval);
        } else {
            workInfo.RequestRepeatCycle(interval, PositiveMod(provider.GetInt32(), 16));
        }
        workInfo.RequestBaseTime(static_cast<time_t>(provider.GetUInt32()));
    }
    if (provider.GetBool()) {
        workInfo.RequestDeepIdle(provider.GetBool());
        workInfo.SetDeepIdleTime(PositiveMod(provider.GetInt32(), MAX_REPEAT_TIME));
    }
    if (allowExtras && provider.GetBool()) {
        AddExtras(provider, workInfo);
    }
    return workInfo;
}

void FuzzRawParcel(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);
    std::unique_ptr<WorkInfo> read(WorkInfo::Unmarshalling(parcel));
    if (read != nullptr) {
        std::string result;
        read->Dump(result);
        read->ParseToJsonStr();
    }
}

void FuzzStructuredParcel(FuzzData &provider)
{
    WorkInfo workInfoWithExtras = BuildWorkInfo(provider, true);
    MessageParcel extrasParcel;
    workInfoWithExtras.Marshalling(extrasParcel);

    WorkInfo workInfo = BuildWorkInfo(provider, false);
    MessageParcel parcel;
    workInfo.Marshalling(parcel);
    parcel.RewindRead(0);
    std::unique_ptr<WorkInfo> read(WorkInfo::Unmarshalling(parcel));
    if (read != nullptr) {
        MessageParcel secondParcel;
        read->Marshalling(secondParcel);
        std::string result;
        read->Dump(result);
        read->ParseToJsonStr();
    }
}

nlohmann::json BuildJson(FuzzData &provider)
{
    nlohmann::json root;
    if (provider.GetBool()) {
        root["workId"] = provider.GetInt32();
    }
    if (provider.GetBool()) {
        root["bundleName"] = provider.GetString();
        root["abilityName"] = provider.GetString();
    }
    if (provider.GetBool()) {
        root["saId"] = provider.GetInt32();
        root["residentSa"] = provider.GetBool();
    }
    root["uid"] = provider.GetInt32();
    root["persisted"] = provider.GetBool();
    root["preinstalled"] = provider.GetBool();
    root["callBySystemApp"] = provider.GetBool();
    root["appIndex"] = PositiveMod(provider.GetInt32(), 128);
    root["earliestStartTime"] = provider.GetInt32();
    root["createTime"] = provider.GetUInt32();
    root["uriKey"] = provider.GetString();

    nlohmann::json conditions;
    if (provider.GetBool()) {
        conditions["network"] = PositiveMod(provider.GetInt32(), ENUM_SPREAD);
    }
    if (provider.GetBool()) {
        conditions["isCharging"] = provider.GetBool();
        conditions["chargerType"] = PositiveMod(provider.GetInt32(), ENUM_SPREAD);
    }
    if (provider.GetBool()) {
        conditions["batteryLevel"] = PositiveMod(provider.GetInt32(), 150);
    }
    if (provider.GetBool()) {
        conditions["batteryStatus"] = PositiveMod(provider.GetInt32(), ENUM_SPREAD);
    }
    if (provider.GetBool()) {
        conditions["storage"] = PositiveMod(provider.GetInt32(), ENUM_SPREAD);
    }
    if (provider.GetBool()) {
        conditions["isDeepIdle"] = provider.GetBool();
        conditions["deepIdleTime"] = provider.GetInt32();
    }
    if (provider.GetBool()) {
        conditions["timer"] = provider.GetInt32();
        conditions["repeat"] = provider.GetBool();
        conditions["cycle"] = PositiveMod(provider.GetInt32(), 16);
        conditions["baseTime"] = provider.GetInt32();
    }
    root["conditions"] = conditions;

    nlohmann::json parameters;
    nlohmann::json parameterTypes;
    int32_t count = provider.GetU8() % 4;
    for (int32_t i = 0; i < count; ++i) {
        std::string key = provider.GetString();
        if (key.empty()) {
            key = "key" + std::to_string(i);
        }
        parameters[key] = provider.GetString();
        parameterTypes[key] = PositiveMod(provider.GetInt32(), 16);
    }
    if (provider.GetBool()) {
        root["parameters"] = parameters;
        root["parametersType"] = parameterTypes;
    }
    return root;
}

void FuzzJson(FuzzData &provider, const uint8_t *data, size_t size)
{
    WorkInfo workInfo;
    nlohmann::json parsed = nlohmann::json::parse(
        std::string(reinterpret_cast<const char *>(data), size), nullptr, false);
    if (!parsed.is_discarded()) {
        workInfo.ParseFromJson(parsed);
    }

    WorkInfo structuredInfo;
    structuredInfo.ParseFromJson(BuildJson(provider));
    structuredInfo.ParseToJsonStr();
    std::string result;
    structuredInfo.Dump(result);
}
} // namespace

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size < MIN_SIZE) {
            return false;
        }
        FuzzData provider(data, size);
        FuzzRawParcel(data, size);
        FuzzStructuredParcel(provider);
        FuzzJson(provider, data, size);
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

