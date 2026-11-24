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

#include "workscheducjworkinfo_fuzzer.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>
#include "string_wrapper.h"
#include "work_condition.h"
#include "work_info.h"
#include "work_scheduler_ffi.h"

namespace OHOS {
namespace WorkScheduler {
int32_t GetWorkInfoV2(RetWorkInfoV2 cwork, WorkInfo& workInfo);
int32_t ParseWorkInfoV2(std::shared_ptr<WorkInfo> workInfo, RetWorkInfoV2& cwork);
namespace {
constexpr size_t MAX_TEXT = 64;
constexpr size_t MAX_EXTRAS = 4;
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
constexpr int32_t MAX_UNKNOWN_PARAM_TYPE = 10;
constexpr int32_t MAX_BATTERY_LEVEL_FOR_FUZZ = 110;
constexpr int32_t SECONDS_PER_DAY = 86400;
constexpr int32_t MAX_WORK_ID_FOR_FUZZ = 100000;

struct CjParameterStorage {
    std::array<CParameters, MAX_EXTRAS> params {};
    std::array<std::array<char, MAX_TEXT>, MAX_EXTRAS> keys {};
    std::array<std::array<char, MAX_TEXT>, MAX_EXTRAS> strings {};
    std::array<int32_t, MAX_EXTRAS> ints {};
    std::array<double, MAX_EXTRAS> doubles {};
    std::array<bool, MAX_EXTRAS> bools {};
};

std::string ConsumeString(FuzzedDataProvider &provider, const std::string &fallback)
{
    std::string value = provider.ConsumeRandomLengthString(MAX_TEXT - 1);
    return value.empty() ? fallback : value;
}

void FillCString(std::array<char, MAX_TEXT> &buffer, const std::string &value)
{
    buffer.fill(0);
    std::string clipped = value.substr(0, MAX_TEXT - 1);
    if (!clipped.empty()) {
        std::copy(clipped.begin(), clipped.end(), buffer.begin());
    }
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

RetWorkInfo BuildRetWorkInfo(FuzzedDataProvider &provider, std::array<char, MAX_TEXT> &bundle,
    std::array<char, MAX_TEXT> &ability)
{
    RetWorkInfo work {};
    FillCString(bundle, ConsumeString(provider, "bundleName"));
    FillCString(ability, ConsumeString(provider, "abilityName"));
    work.workId = provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_WORK_ID_FOR_FUZZ);
    work.bundleName = bundle.data();
    work.abilityName = ability.data();
    work.netWorkType = provider.ConsumeBool() ? UNSET_INT_PARAM :
        ConsumeIntAround(provider, WorkCondition::Network::NETWORK_TYPE_ANY,
            WorkCondition::Network::NETWORK_TYPE_ETHERNET);
    work.isCharging = provider.ConsumeIntegralInRange<int32_t>(0, 1);
    work.chargerType = provider.ConsumeBool() ? UNSET_INT_PARAM :
        ConsumeIntAround(provider, WorkCondition::Charger::CHARGING_PLUGGED_ANY,
            WorkCondition::Charger::CHARGING_PLUGGED_WIRELESS);
    work.batteryLevel = provider.ConsumeBool() ? UNSET_INT_PARAM : provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_VALUE, MAX_BATTERY_LEVEL_FOR_FUZZ);
    work.batteryStatus = provider.ConsumeBool() ? UNSET_INT_PARAM :
        ConsumeIntAround(provider, WorkCondition::BatteryStatus::BATTERY_STATUS_LOW,
            WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
    work.storageRequest = provider.ConsumeBool() ? UNSET_INT_PARAM :
        ConsumeIntAround(provider, WorkCondition::Storage::STORAGE_LEVEL_LOW,
            WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY);
    work.isRepeat = provider.ConsumeIntegralInRange<int32_t>(0, 1);
    work.repeatCycleTime = provider.ConsumeBool() ? UNSET_INT_PARAM :
        static_cast<int32_t>(ConsumeRepeatCycle(provider));
    work.repeatCount = provider.ConsumeBool() ? UNSET_INT_PARAM : provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_VALUE, MAX_REPEAT_COUNT_FOR_FUZZ);
    work.isPersisted = provider.ConsumeBool();
    work.isDeepIdle = provider.ConsumeBool() ? UNSET_INT_PARAM : provider.ConsumeIntegralInRange<int32_t>(0, 1);
    work.idleWaitTime = provider.ConsumeBool() ? UNSET_INT_PARAM : provider.ConsumeIntegralInRange<int32_t>(
        0, SECONDS_PER_DAY);
    return work;
}

void BuildParameters(FuzzedDataProvider &provider, RetWorkInfoV2 &work, CjParameterStorage &storage)
{
    int64_t count = provider.ConsumeIntegralInRange<int64_t>(0, static_cast<int64_t>(MAX_EXTRAS));
    work.parameters.size = count;
    work.parameters.head = count == 0 ? nullptr : storage.params.data();
    for (int64_t i = 0; i < count; ++i) {
        FillCString(storage.keys[i], ConsumeString(provider, "key" + std::to_string(i)));
        storage.params[i].key = storage.keys[i].data();
        storage.params[i].size = 0;
        switch (provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_BOUND_FOUR)) {
            case 0:
                storage.ints[i] = provider.ConsumeIntegral<int32_t>();
                storage.params[i].valueType = 0;
                storage.params[i].value = &storage.ints[i];
                storage.params[i].size = sizeof(int32_t);
                break;
            case 1:
                storage.doubles[i] = provider.ConsumeFloatingPoint<double>();
                storage.params[i].valueType = 1;
                storage.params[i].value = &storage.doubles[i];
                storage.params[i].size = sizeof(double);
                break;
            case FUZZ_BOUND_TWO:
                FillCString(storage.strings[i], ConsumeString(provider, "value"));
                storage.params[i].valueType = FUZZ_BOUND_TWO;
                storage.params[i].value = storage.strings[i].data();
                storage.params[i].size = std::strlen(storage.strings[i].data()) + 1;
                break;
            case FUZZ_BOUND_THREE:
                storage.bools[i] = provider.ConsumeBool();
                storage.params[i].valueType = FUZZ_BOUND_THREE;
                storage.params[i].value = &storage.bools[i];
                storage.params[i].size = sizeof(bool);
                break;
            default:
                storage.ints[i] = provider.ConsumeIntegral<int32_t>();
                storage.params[i].valueType = provider.ConsumeIntegralInRange<int8_t>(
                    FUZZ_BOUND_FOUR, MAX_UNKNOWN_PARAM_TYPE);
                storage.params[i].value = &storage.ints[i];
                storage.params[i].size = sizeof(int32_t);
                break;
        }
    }
}

void ClearParsedParameters(CArrParameters &params)
{
    if (params.head == nullptr) {
        return;
    }
    for (int64_t i = 0; i < params.size; ++i) {
        free(params.head[i].key);
        free(params.head[i].value);
        params.head[i].key = nullptr;
        params.head[i].value = nullptr;
    }
    free(params.head);
    params.head = nullptr;
    params.size = 0;
}

std::shared_ptr<WorkInfo> BuildNativeWorkInfo(FuzzedDataProvider &provider)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(provider.ConsumeIntegralInRange<int32_t>(1, MAX_WORK_ID_FOR_FUZZ));
    workInfo->SetElement(ConsumeString(provider, "bundleName"), ConsumeString(provider, "abilityName"));
    workInfo->RequestPersisted(provider.ConsumeBool());
    workInfo->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_WIFI);
    AAFwk::WantParams extras;
    extras.SetParam("key1", AAFwk::String::Box(ConsumeString(provider, "value1")));
    workInfo->RequestExtras(extras);
    return workInfo;
}
} // namespace

void RunWorkSchedCjWorkInfoFuzzer(FuzzedDataProvider &provider)
{
    std::array<char, MAX_TEXT> bundle {};
    std::array<char, MAX_TEXT> ability {};
    RetWorkInfo work = BuildRetWorkInfo(provider, bundle, ability);
    WorkInfo parsed;
    GetWorkInfo(work, parsed);

    CjParameterStorage storage;
    RetWorkInfoV2 workV2 {};
    workV2.v1 = work;
    BuildParameters(provider, workV2, storage);
    WorkInfo parsedV2;
    GetWorkInfoV2(workV2, parsedV2);
    GetExtrasInfo(workV2, parsedV2);

    auto sharedWorkInfo = BuildNativeWorkInfo(provider);
    RetWorkInfo parsedBack {};
    ParseWorkInfo(sharedWorkInfo, parsedBack);
    RetWorkInfoV2 parsedBackV2 {};
    ParseWorkInfoV2(sharedWorkInfo, parsedBackV2);
    CArrParameters parsedParams {};
    ParseExtrasInfo(sharedWorkInfo, parsedParams);
    ClearParsedParameters(parsedParams);
    ClearParsedParameters(parsedBackV2.parameters);
    free(parsedBack.bundleName);
    free(parsedBack.abilityName);
    free(parsedBackV2.v1.bundleName);
    free(parsedBackV2.v1.abilityName);
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    OHOS::WorkScheduler::RunWorkSchedCjWorkInfoFuzzer(provider);
    return 0;
}
