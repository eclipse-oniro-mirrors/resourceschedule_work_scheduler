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

#include "workinfojsonparcel_fuzzer.h"

#include <memory>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>
#include "message_parcel.h"
#include "nlohmann/json.hpp"
#include "string_wrapper.h"
#include "want_params.h"
#include "work_condition.h"
#include "work_info.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t MAX_STRING_SIZE = 64;
constexpr uint32_t DEFAULT_REPEAT_MINUTES = 20;
constexpr uint32_t MIN_REPEAT_MINUTES = 5;
constexpr uint32_t REPEAT_SECONDS_PER_MINUTE = 60;
constexpr uint32_t MILLISECONDS_PER_SECOND = 1000;
constexpr uint32_t DEFAULT_REPEAT_CYCLE =
    DEFAULT_REPEAT_MINUTES * REPEAT_SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
constexpr uint32_t MIN_REPEAT_CYCLE =
    MIN_REPEAT_MINUTES * REPEAT_SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
constexpr int32_t FUZZ_INVALID_VALUE = -1;
constexpr int32_t FUZZ_BOUND_TWO = 2;
constexpr int32_t FUZZ_BOUND_THREE = 3;
constexpr int32_t FUZZ_BOUND_FOUR = 4;
constexpr int32_t FUZZ_BOUND_FIVE = 5;
constexpr int32_t MAX_REPEAT_COUNT_FOR_FUZZ = 8;
constexpr int32_t PARAM_TYPE_STRING_FOR_FUZZ = 9;
constexpr int32_t MAX_BATTERY_LEVEL_FOR_FUZZ = 110;
constexpr int32_t MAX_RAW_JSON_SIZE = 512;
constexpr int32_t MAX_SA_ID_FOR_FUZZ = 20000;
constexpr int32_t MAX_UID_FOR_FUZZ = 30000;
constexpr int32_t SECONDS_PER_DAY = 86400;
constexpr int32_t MAX_WORK_ID_FOR_FUZZ = 100000;

std::string ConsumeString(FuzzedDataProvider &provider, const std::string &fallback)
{
    std::string value = provider.ConsumeRandomLengthString(MAX_STRING_SIZE);
    return value.empty() ? fallback : value;
}

int32_t ConsumeIntAround(FuzzedDataProvider &provider, int32_t minValue, int32_t maxValue)
{
    switch (provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_BOUND_FOUR)) {
        case 0:
            return minValue - 1;
        case 1:
            return minValue;
        case FUZZ_BOUND_TWO:
            return maxValue;
        case FUZZ_BOUND_THREE:
            return maxValue + 1;
        default:
            return provider.ConsumeIntegralInRange<int32_t>(minValue, maxValue);
    }
}

uint32_t ConsumeRepeatCycle(FuzzedDataProvider &provider)
{
    switch (provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_BOUND_FIVE)) {
        case 0:
            return 0;
        case 1:
            return 1;
        case FUZZ_BOUND_TWO:
            return MIN_REPEAT_CYCLE - 1;
        case FUZZ_BOUND_THREE:
            return MIN_REPEAT_CYCLE;
        case FUZZ_BOUND_FOUR:
            return DEFAULT_REPEAT_CYCLE;
        default:
            return provider.ConsumeIntegral<uint32_t>();
    }
}

void AddConditions(WorkInfo &workInfo, FuzzedDataProvider &provider)
{
    uint8_t mask = provider.ConsumeIntegral<uint8_t>();
    if ((mask & (1 << WorkCondition::Type::NETWORK)) != 0) {
        workInfo.RequestNetworkType(WorkCondition::Network(ConsumeIntAround(provider,
            WorkCondition::Network::NETWORK_TYPE_ANY, WorkCondition::Network::NETWORK_TYPE_ETHERNET)));
    }
    if ((mask & (1 << WorkCondition::Type::CHARGER)) != 0) {
        workInfo.RequestChargerType(provider.ConsumeBool(), WorkCondition::Charger(ConsumeIntAround(provider,
            WorkCondition::Charger::CHARGING_PLUGGED_ANY, WorkCondition::Charger::CHARGING_PLUGGED_WIRELESS)));
    }
    if ((mask & (1 << WorkCondition::Type::BATTERY_STATUS)) != 0) {
        workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus(ConsumeIntAround(provider,
            WorkCondition::BatteryStatus::BATTERY_STATUS_LOW,
            WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY)));
    }
    if ((mask & (1 << WorkCondition::Type::BATTERY_LEVEL)) != 0) {
        workInfo.RequestBatteryLevel(provider.ConsumeIntegralInRange<int32_t>(
            FUZZ_INVALID_VALUE, MAX_BATTERY_LEVEL_FOR_FUZZ));
    }
    if ((mask & (1 << WorkCondition::Type::STORAGE)) != 0) {
        workInfo.RequestStorageLevel(WorkCondition::Storage(ConsumeIntAround(provider,
            WorkCondition::Storage::STORAGE_LEVEL_LOW, WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY)));
    }
    if ((mask & (1 << WorkCondition::Type::TIMER)) != 0) {
        if (provider.ConsumeBool()) {
            workInfo.RequestRepeatCycle(ConsumeRepeatCycle(provider));
        } else {
            workInfo.RequestRepeatCycle(ConsumeRepeatCycle(provider), provider.ConsumeIntegralInRange<int32_t>(
                FUZZ_INVALID_VALUE, MAX_REPEAT_COUNT_FOR_FUZZ));
        }
    }
    if ((mask & (1 << WorkCondition::Type::DEEP_IDLE)) != 0) {
        workInfo.RequestDeepIdle(provider.ConsumeBool());
    }
    if (workInfo.GetConditionMap()->empty()) {
        workInfo.RequestRepeatCycle(DEFAULT_REPEAT_CYCLE);
    }
}

WorkInfo BuildWorkInfo(FuzzedDataProvider &provider)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_WORK_ID_FOR_FUZZ));
    workInfo.SetElement(ConsumeString(provider, "bundleName"), ConsumeString(provider, "abilityName"));
    workInfo.RequestPersisted(provider.ConsumeBool());
    workInfo.RefreshUid(provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_UID_FOR_FUZZ));
    workInfo.SetCallBySystemApp(provider.ConsumeBool());
    workInfo.SetPreinstalled(provider.ConsumeBool());
    if (provider.ConsumeBool()) {
        workInfo.RefreshSaId(provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_SA_ID_FOR_FUZZ));
    }
    AddConditions(workInfo, provider);
    return workInfo;
}

void FillJsonBasicFields(nlohmann::json &root, FuzzedDataProvider &provider)
{
    root["workId"] = provider.ConsumeBool() ? nlohmann::json(provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_VALUE, MAX_WORK_ID_FOR_FUZZ)) :
        nlohmann::json(ConsumeString(provider, "1"));
    if (provider.ConsumeBool()) {
        root["bundleName"] = ConsumeString(provider, "bundleName");
    }
    if (provider.ConsumeBool()) {
        root["abilityName"] = ConsumeString(provider, "abilityName");
    }
    root["uid"] = provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_UID_FOR_FUZZ);
    if (provider.ConsumeBool()) {
        root["saId"] = provider.ConsumeBool() ? nlohmann::json(provider.ConsumeIntegralInRange<int32_t>(
            FUZZ_INVALID_VALUE, MAX_SA_ID_FOR_FUZZ)) :
            nlohmann::json(ConsumeString(provider, "1"));
        root["residentSa"] = provider.ConsumeBool() ? nlohmann::json(provider.ConsumeBool()) :
            nlohmann::json(ConsumeString(provider, "true"));
    }
    root["persisted"] = provider.ConsumeBool();
    root["preinstalled"] = provider.ConsumeBool();
    root["uriKey"] = ConsumeString(provider, "uriKey");
    root["callBySystemApp"] = provider.ConsumeBool();
    root["earliestStartTime"] = provider.ConsumeBool() ?
        nlohmann::json(provider.ConsumeIntegralInRange<int32_t>(0, SECONDS_PER_DAY)) :
        nlohmann::json(ConsumeString(provider, "time"));
}

nlohmann::json BuildJsonConditions(FuzzedDataProvider &provider)
{
    nlohmann::json conditions = nlohmann::json::object();
    conditions["network"] = ConsumeIntAround(provider, WorkCondition::Network::NETWORK_TYPE_ANY,
        WorkCondition::Network::NETWORK_TYPE_ETHERNET);
    conditions["isCharging"] = provider.ConsumeBool();
    conditions["chargerType"] = ConsumeIntAround(provider, WorkCondition::Charger::CHARGING_PLUGGED_ANY,
        WorkCondition::Charger::CHARGING_PLUGGED_WIRELESS);
    conditions["batteryLevel"] = provider.ConsumeBool() ?
        nlohmann::json(provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_BATTERY_LEVEL_FOR_FUZZ)) :
        nlohmann::json(ConsumeString(provider, "low"));
    conditions["batteryStatus"] = ConsumeIntAround(provider, WorkCondition::BatteryStatus::BATTERY_STATUS_LOW,
        WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
    conditions["storage"] = ConsumeIntAround(provider, WorkCondition::Storage::STORAGE_LEVEL_LOW,
        WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY);
    conditions["isDeepIdle"] = provider.ConsumeBool();
    conditions["timer"] = ConsumeRepeatCycle(provider);
    conditions["repeat"] = provider.ConsumeBool();
    conditions["cycle"] = provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_REPEAT_COUNT_FOR_FUZZ);
    conditions["baseTime"] = provider.ConsumeIntegralInRange<int64_t>(0, SECONDS_PER_DAY);
    return conditions;
}

void FillJsonParameters(nlohmann::json &root, FuzzedDataProvider &provider)
{
    if (!provider.ConsumeBool()) {
        return;
    }
    root["parameters"] = {{"key1", "value1"}, {"key2", "42"}, {"key3", "true"}};
    root["parametersType"] = {{"key1", PARAM_TYPE_STRING_FOR_FUZZ}, {"key2", FUZZ_BOUND_FIVE}, {"key3", 1}};
}

nlohmann::json BuildJsonWorkInfo(FuzzedDataProvider &provider)
{
    nlohmann::json root = nlohmann::json::object();
    FillJsonBasicFields(root, provider);
    root["conditions"] = BuildJsonConditions(provider);
    FillJsonParameters(root, provider);
    return root;
}
} // namespace

void RunWorkInfoJsonParcelFuzzer(FuzzedDataProvider &provider)
{
    WorkInfo jsonWorkInfo;
    jsonWorkInfo.ParseFromJson(BuildJsonWorkInfo(provider));
    std::string jsonDump = jsonWorkInfo.ParseToJsonStr();
    jsonWorkInfo.Dump(jsonDump);

    std::string rawJson = provider.ConsumeRandomLengthString(MAX_RAW_JSON_SIZE);
    nlohmann::json parsed = nlohmann::json::parse(rawJson, nullptr, false);
    WorkInfo rawWorkInfo;
    rawWorkInfo.ParseFromJson(parsed);
    std::string rawDump;
    rawWorkInfo.Dump(rawDump);

    WorkInfo parcelWorkInfo = BuildWorkInfo(provider);
    AAFwk::WantParams extras;
    extras.SetParam("key1", AAFwk::String::Box(ConsumeString(provider, "value1")));
    parcelWorkInfo.RequestExtras(extras);
    MessageParcel parcel;
    if (parcelWorkInfo.Marshalling(parcel)) {
        std::vector<uint8_t> tail = provider.ConsumeRemainingBytes<uint8_t>();
        if (!tail.empty()) {
            parcel.WriteBuffer(tail.data(), tail.size());
        }
        parcel.RewindRead(0);
        std::unique_ptr<WorkInfo> unmarshalled(WorkInfo::Unmarshalling(parcel));
        if (unmarshalled != nullptr) {
            std::string result = unmarshalled->ParseToJsonStr();
            unmarshalled->Dump(result);
        }
    }
    jsonDump.clear();
    rawDump.clear();
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    OHOS::WorkScheduler::RunWorkInfoJsonParcelFuzzer(provider);
    return 0;
}
