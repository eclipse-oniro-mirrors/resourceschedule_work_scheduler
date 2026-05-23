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
constexpr uint32_t BITS_PER_BYTE = 8;
constexpr uint32_t ALPHABET_COUNT = 26;
constexpr uint32_t DECIMAL_COUNT = 10;
constexpr uint8_t DIGIT_BRANCH_MASK = 3;
constexpr int32_t EXTRA_PARAM_COUNT = 4;
constexpr int32_t PARAM_TYPE_INT = 0;
constexpr int32_t PARAM_TYPE_STRING = 1;
constexpr int32_t PARAM_TYPE_BOOL = 2;
constexpr int32_t APP_INDEX_RANGE = 128;
constexpr int32_t BATTERY_LEVEL_SPREAD = 150;
constexpr int32_t TIMER_CYCLE_SPREAD = 16;
constexpr int32_t DOUBLE_SCALE = 100;
constexpr uint32_t MIN_REPEAT_TIME = 1;
constexpr uint32_t HOURS_PER_DAY = 24;
constexpr uint32_t MINUTES_PER_HOUR = 60;
constexpr uint32_t SECONDS_PER_MINUTE = 60;
constexpr uint32_t MILLISECONDS_PER_SECOND = 1000;
constexpr uint32_t MAX_REPEAT_TIME =
    HOURS_PER_DAY * MINUTES_PER_HOUR * SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;

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
            char ch = static_cast<char>('a' + (GetU8() % ALPHABET_COUNT));
            if ((GetU8() & DIGIT_BRANCH_MASK) == 0) {
                ch = static_cast<char>('0' + (GetU8() % DECIMAL_COUNT));
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

template<typename T, int32_t SPREAD = ENUM_SPREAD>
T PickEnum(FuzzData &provider)
{
    uint32_t value = static_cast<uint32_t>(provider.GetInt32()) % static_cast<uint32_t>(SPREAD);
    return static_cast<T>(value);
}

template<int32_t MOD>
int32_t PositiveMod(int32_t value)
{
    return static_cast<int32_t>(static_cast<uint32_t>(value) % static_cast<uint32_t>(MOD));
}

void AddExtras(FuzzData &provider, WorkInfo &workInfo)
{
    AAFwk::WantParams params;
    int32_t count = provider.GetU8() % EXTRA_PARAM_COUNT;
    for (int32_t i = 0; i < count; ++i) {
        std::string key = provider.GetString();
        if (key.empty()) {
            key = "executeImmediate";
        }
        switch (provider.GetU8() % EXTRA_PARAM_COUNT) {
            case PARAM_TYPE_INT:
                params.SetParam(key, AAFwk::Integer::Box(provider.GetInt32()));
                break;
            case PARAM_TYPE_STRING:
                params.SetParam(key, AAFwk::String::Box(provider.GetBool() ? "false" : provider.GetString()));
                break;
            case PARAM_TYPE_BOOL:
                params.SetParam(key, AAFwk::Boolean::Box(provider.GetBool()));
                break;
            default:
                params.SetParam(key, AAFwk::Double::Box(static_cast<double>(provider.GetInt32()) /
                    static_cast<double>(DOUBLE_SCALE)));
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
    workInfo.RefreshAppIndex(PositiveMod<APP_INDEX_RANGE>(provider.GetInt32()));
    workInfo.SetPreinstalled(provider.GetBool());
    workInfo.SetCallBySystemApp(provider.GetBool());
    workInfo.SetEarliestStartTime(PositiveMod<static_cast<int32_t>(MAX_REPEAT_TIME)>(provider.GetInt32()));
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
        workInfo.RequestBatteryLevel(PositiveMod<BATTERY_LEVEL_SPREAD>(provider.GetInt32()));
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
            workInfo.RequestRepeatCycle(interval, PositiveMod<TIMER_CYCLE_SPREAD>(provider.GetInt32()));
        }
        workInfo.RequestBaseTime(static_cast<time_t>(provider.GetUInt32()));
    }
    if (provider.GetBool()) {
        workInfo.RequestDeepIdle(provider.GetBool());
        workInfo.SetDeepIdleTime(PositiveMod<static_cast<int32_t>(MAX_REPEAT_TIME)>(provider.GetInt32()));
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

void FillJsonBase(FuzzData &provider, nlohmann::json &root)
{
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
    root["appIndex"] = PositiveMod<APP_INDEX_RANGE>(provider.GetInt32());
    root["earliestStartTime"] = provider.GetInt32();
    root["createTime"] = provider.GetUInt32();
    root["uriKey"] = provider.GetString();
}

void FillJsonConditions(FuzzData &provider, nlohmann::json &root)
{
    nlohmann::json conditions;
    if (provider.GetBool()) {
        conditions["network"] = PositiveMod<ENUM_SPREAD>(provider.GetInt32());
    }
    if (provider.GetBool()) {
        conditions["isCharging"] = provider.GetBool();
        conditions["chargerType"] = PositiveMod<ENUM_SPREAD>(provider.GetInt32());
    }
    if (provider.GetBool()) {
        conditions["batteryLevel"] = PositiveMod<BATTERY_LEVEL_SPREAD>(provider.GetInt32());
    }
    if (provider.GetBool()) {
        conditions["batteryStatus"] = PositiveMod<ENUM_SPREAD>(provider.GetInt32());
    }
    if (provider.GetBool()) {
        conditions["storage"] = PositiveMod<ENUM_SPREAD>(provider.GetInt32());
    }
    if (provider.GetBool()) {
        conditions["isDeepIdle"] = provider.GetBool();
        conditions["deepIdleTime"] = provider.GetInt32();
    }
    if (provider.GetBool()) {
        conditions["timer"] = provider.GetInt32();
        conditions["repeat"] = provider.GetBool();
        conditions["cycle"] = PositiveMod<TIMER_CYCLE_SPREAD>(provider.GetInt32());
        conditions["baseTime"] = provider.GetInt32();
    }
    root["conditions"] = conditions;
}

void FillJsonParameters(FuzzData &provider, nlohmann::json &root)
{
    nlohmann::json parameters;
    nlohmann::json parameterTypes;
    int32_t count = provider.GetU8() % EXTRA_PARAM_COUNT;
    for (int32_t i = 0; i < count; ++i) {
        std::string key = provider.GetString();
        if (key.empty()) {
            key = "key" + std::to_string(i);
        }
        parameters[key] = provider.GetString();
        parameterTypes[key] = PositiveMod<TIMER_CYCLE_SPREAD>(provider.GetInt32());
    }
    if (provider.GetBool()) {
        root["parameters"] = parameters;
        root["parametersType"] = parameterTypes;
    }
}

nlohmann::json BuildJson(FuzzData &provider)
{
    nlohmann::json root;
    FillJsonBase(provider, root);
    FillJsonConditions(provider, root);
    FillJsonParameters(provider, root);
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

