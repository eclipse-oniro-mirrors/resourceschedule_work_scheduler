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

#include "worktaihecommon_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "bool_wrapper.h"
#include "common.h"
#include "double_wrapper.h"
#include "int_wrapper.h"
#include "ohos.resourceschedule.workScheduler.impl.hpp"
#include "string_wrapper.h"
#include "want_params.h"
#include "work_condition.h"
#include "work_info.h"
#include "work_sched_errors.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t MIN_SIZE = 4;
constexpr size_t MAX_STRING_LEN = 32;
constexpr int32_t ENUM_SPREAD = 12;
constexpr uint32_t BITS_PER_BYTE = 8;
constexpr uint32_t ALPHABET_COUNT = 26;
constexpr uint32_t DECIMAL_COUNT = 10;
constexpr uint8_t DIGIT_BRANCH_MASK = 3;
constexpr int32_t EXTRA_PARAM_COUNT = 6;
constexpr int32_t PARAM_TYPE_INT = 0;
constexpr int32_t PARAM_TYPE_DOUBLE = 1;
constexpr int32_t PARAM_TYPE_STRING = 2;
constexpr int32_t PARAM_TYPE_BOOL = 3;
constexpr int32_t BATTERY_LEVEL_SPREAD = 150;
constexpr int32_t TIMER_CYCLE_SPREAD = 16;
constexpr int32_t DOUBLE_SCALE = 100;
constexpr int32_t MIN_WORK_ID = 1;
constexpr int32_t NETWORK_RANGE = 8;
constexpr int32_t CHARGER_RANGE = 6;
constexpr int32_t BATTERY_STATUS_RANGE = 6;
constexpr int32_t STORAGE_RANGE = 6;

using AniWorkInfo = ::ohos::resourceschedule::workScheduler::WorkInfo;
using AniParameType = ::ohos::resourceschedule::workScheduler::ParameType;
using AniNetworkType = ::ohos::resourceschedule::workScheduler::NetworkType;
using AniChargingType = ::ohos::resourceschedule::workScheduler::ChargingType;
using AniBatteryStatus = ::ohos::resourceschedule::workScheduler::BatteryStatus;
using AniStorageRequest = ::ohos::resourceschedule::workScheduler::StorageRequest;

class FuzzData {
public:
    explicit FuzzData(const uint8_t *data, size_t size) : data_(data), size_(size) {}

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

template<int32_t MOD>
int32_t PositiveMod(int32_t value)
{
    return static_cast<int32_t>(static_cast<uint32_t>(value) % static_cast<uint32_t>(MOD));
}

void FillAniParameters(FuzzData &provider, AniWorkInfo &aniWork)
{
    if (!provider.GetBool()) {
        return;
    }
    std::map<std::string, AniParameType> params;
    int32_t count = provider.GetU8() % EXTRA_PARAM_COUNT;
    for (int32_t i = 0; i < count; ++i) {
        std::string key = provider.GetString();
        if (key.empty()) {
            key = "param" + std::to_string(i);
        }
        switch (provider.GetU8() % EXTRA_PARAM_COUNT) {
            case PARAM_TYPE_INT:
                params.emplace(key, AniParameType::make_int_type(provider.GetInt32()));
                break;
            case PARAM_TYPE_DOUBLE:
                params.emplace(key, AniParameType::make_double_type(
                    static_cast<double>(provider.GetInt32()) / static_cast<double>(DOUBLE_SCALE)));
                break;
            case PARAM_TYPE_STRING:
                params.emplace(key, AniParameType::make_string_type(provider.GetString()));
                break;
            case PARAM_TYPE_BOOL:
                params.emplace(key, AniParameType::make_bool_type(provider.GetBool()));
                break;
            default:
                params.emplace(key, AniParameType::make_int_type(provider.GetInt32()));
                break;
        }
    }
    aniWork.parameters = taihe::optional<std::map<std::string, AniParameType>>(std::in_place, params);
}

void FillAniWorkInfoBase(FuzzData &provider, AniWorkInfo &aniWork, bool forceValidBase)
{
    aniWork.workId = provider.GetInt32();
    std::string bundle = provider.GetString();
    std::string ability = provider.GetString();
    if (forceValidBase || provider.GetBool()) {
        if (bundle.empty()) {
            bundle = "com.example.fuzz";
        }
        if (ability.empty()) {
            ability = "FuzzAbility";
        }
    }
    aniWork.bundleName = bundle;
    aniWork.abilityName = ability;
    if (provider.GetBool()) {
        aniWork.isPersisted = taihe::optional<bool>(std::in_place, provider.GetBool());
    }
    if (provider.GetBool()) {
        aniWork.earliestStartTime = taihe::optional<int32_t>(std::in_place, provider.GetInt32());
    }
}

void FillAniWorkInfoConditions(FuzzData &provider, AniWorkInfo &aniWork)
{
    if (provider.GetBool()) {
        aniWork.networkType = taihe::optional<AniNetworkType>(
            std::in_place, AniNetworkType::key_t(PositiveMod<NETWORK_RANGE>(provider.GetInt32())));
    }
    if (provider.GetBool()) {
        aniWork.isCharging = taihe::optional<bool>(std::in_place, provider.GetBool());
        if (provider.GetBool()) {
            aniWork.chargerType = taihe::optional<AniChargingType>(
                std::in_place, AniChargingType::key_t(PositiveMod<CHARGER_RANGE>(provider.GetInt32())));
        }
    }
    if (provider.GetBool()) {
        aniWork.batteryLevel = taihe::optional<int32_t>(
            std::in_place, PositiveMod<BATTERY_LEVEL_SPREAD>(provider.GetInt32()));
    }
    if (provider.GetBool()) {
        aniWork.batteryStatus = taihe::optional<AniBatteryStatus>(
            std::in_place, AniBatteryStatus::key_t(PositiveMod<BATTERY_STATUS_RANGE>(provider.GetInt32())));
    }
    if (provider.GetBool()) {
        aniWork.storageRequest = taihe::optional<AniStorageRequest>(
            std::in_place, AniStorageRequest::key_t(PositiveMod<STORAGE_RANGE>(provider.GetInt32())));
    }
    if (provider.GetBool()) {
        aniWork.repeatCycleTime = taihe::optional<int32_t>(std::in_place, provider.GetInt32());
        if (provider.GetBool()) {
            aniWork.isRepeat = taihe::optional<bool>(std::in_place, provider.GetBool());
        }
        aniWork.repeatCount = taihe::optional<int32_t>(std::in_place, provider.GetInt32());
    }
    if (provider.GetBool()) {
        aniWork.isDeepIdle = taihe::optional<bool>(std::in_place, provider.GetBool());
        aniWork.idleWaitTime = taihe::optional<int32_t>(std::in_place, provider.GetInt32());
    }
}

AniWorkInfo BuildAniWorkInfo(FuzzData &provider, bool forceValidBase = false)
{
    AniWorkInfo aniWork;
    FillAniWorkInfoBase(provider, aniWork, forceValidBase);
    FillAniWorkInfoConditions(provider, aniWork);
    FillAniParameters(provider, aniWork);
    return aniWork;
}

WorkInfo BuildNativeWorkInfo(FuzzData &provider)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(MIN_WORK_ID + PositiveMod<ENUM_SPREAD>(provider.GetInt32()));
    workInfo.SetElement("com.example.fuzz", "FuzzAbility");
    workInfo.RequestPersisted(provider.GetBool());
    workInfo.RequestNetworkType(
        static_cast<WorkCondition::Network>(PositiveMod<NETWORK_RANGE>(provider.GetInt32())));
    if (provider.GetBool()) {
        workInfo.RequestChargerType(true,
            static_cast<WorkCondition::Charger>(PositiveMod<CHARGER_RANGE>(provider.GetInt32())));
    } else {
        workInfo.RequestChargerType(false, WorkCondition::Charger::CHARGING_UNPLUGGED);
    }
    workInfo.RequestBatteryLevel(PositiveMod<BATTERY_LEVEL_SPREAD>(provider.GetInt32()));
    workInfo.RequestBatteryStatus(
        static_cast<WorkCondition::BatteryStatus>(PositiveMod<BATTERY_STATUS_RANGE>(provider.GetInt32())));
    workInfo.RequestStorageLevel(
        static_cast<WorkCondition::Storage>(PositiveMod<STORAGE_RANGE>(provider.GetInt32())));
    uint32_t interval = MIN_WORK_ID + static_cast<uint32_t>(PositiveMod<ENUM_SPREAD>(provider.GetInt32()));
    if (provider.GetBool()) {
        workInfo.RequestRepeatCycle(interval);
    } else {
        workInfo.RequestRepeatCycle(interval, PositiveMod<TIMER_CYCLE_SPREAD>(provider.GetInt32()));
    }
    if (provider.GetBool()) {
        workInfo.RequestDeepIdle(provider.GetBool());
    }
    if (provider.GetBool()) {
        AAFwk::WantParams params;
        params.SetParam("executeImmediate", AAFwk::Boolean::Box(provider.GetBool()));
        params.SetParam("count", AAFwk::Integer::Box(provider.GetInt32()));
        params.SetParam("ratio", AAFwk::Double::Box(static_cast<double>(provider.GetInt32()) /
            static_cast<double>(DOUBLE_SCALE)));
        params.SetParam("tag", AAFwk::String::Box(provider.GetString()));
        workInfo.RequestExtras(params);
    }
    return workInfo;
}

void FuzzGetBaseWorkInfo(FuzzData &provider)
{
    AniWorkInfo aniWork = BuildAniWorkInfo(provider, true);
    aniWork.workId = MIN_WORK_ID + PositiveMod<ENUM_SPREAD>(provider.GetInt32());
    WorkInfo workInfo;
    Common::GetBaseWorkInfo(aniWork, workInfo);

    AniWorkInfo invalidWork = BuildAniWorkInfo(provider);
    WorkInfo invalidNative;
    Common::GetBaseWorkInfo(invalidWork, invalidNative);
}

void FuzzGetConditionApis(FuzzData &provider)
{
    AniWorkInfo aniWork = BuildAniWorkInfo(provider, true);
    WorkInfo workInfo;
    Common::GetNetworkInfo(aniWork, workInfo);
    Common::GetChargeInfo(aniWork, workInfo);
    Common::GetBatteryInfo(aniWork, workInfo);
    Common::GetStorageInfo(aniWork, workInfo);
    Common::GetRepeatInfo(aniWork, workInfo);
    Common::GetDeepIdleInfo(aniWork, workInfo);
}

void FuzzGetExtrasInfo(FuzzData &provider)
{
    AniWorkInfo aniWork = BuildAniWorkInfo(provider, true);
    WorkInfo workInfo;
    Common::GetExtrasInfo(aniWork, workInfo);
}

void FuzzGetWorkInfo(FuzzData &provider)
{
    AniWorkInfo aniWork = BuildAniWorkInfo(provider);
    WorkInfo workInfo;
    Common::GetWorkInfo(aniWork, workInfo);

    AniWorkInfo validWork = BuildAniWorkInfo(provider, true);
    validWork.workId = MIN_WORK_ID + PositiveMod<ENUM_SPREAD>(provider.GetInt32());
    validWork.networkType = taihe::optional<AniNetworkType>(
        std::in_place, AniNetworkType::key_t(WorkCondition::Network::NETWORK_TYPE_WIFI));
    WorkInfo validNative;
    Common::GetWorkInfo(validWork, validNative);
}

void FuzzParseWorkInfo(FuzzData &provider)
{
    WorkInfo nativeInfo = BuildNativeWorkInfo(provider);
    auto workPtr = std::make_shared<WorkInfo>(nativeInfo);
    AniWorkInfo aniWork;
    Common::ParseWorkInfo(workPtr, aniWork);

    WorkInfo unpluggedInfo = BuildNativeWorkInfo(provider);
    unpluggedInfo.RequestChargerType(false, WorkCondition::Charger::CHARGING_UNPLUGGED);
    auto unpluggedPtr = std::make_shared<WorkInfo>(unpluggedInfo);
    AniWorkInfo unpluggedAni;
    Common::ParseWorkInfo(unpluggedPtr, unpluggedAni);
}

void FuzzParseExtrasInfo(FuzzData &provider)
{
    WorkInfo nativeInfo = BuildNativeWorkInfo(provider);
    auto workPtr = std::make_shared<WorkInfo>(nativeInfo);
    AniWorkInfo aniWork;
    Common::ParseExtrasInfo(workPtr, aniWork);

    WorkInfo emptyInfo;
    auto emptyPtr = std::make_shared<WorkInfo>(emptyInfo);
    AniWorkInfo emptyAni;
    Common::ParseExtrasInfo(emptyPtr, emptyAni);
}

void FuzzErrHelpers(FuzzData &provider)
{
    int32_t errCode = provider.GetInt32();
    Common::FindErrCode(errCode);
    Common::FindErrMsg(errCode);
    Common::FindErrMsg(E_NETWORK_TYPE_ERR);
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < MIN_SIZE) {
        return false;
    }
    FuzzData provider(data, size);
    FuzzGetBaseWorkInfo(provider);
    FuzzGetConditionApis(provider);
    FuzzGetExtrasInfo(provider);
    FuzzGetWorkInfo(provider);
    FuzzParseWorkInfo(provider);
    FuzzParseExtrasInfo(provider);
    FuzzErrHelpers(provider);
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
