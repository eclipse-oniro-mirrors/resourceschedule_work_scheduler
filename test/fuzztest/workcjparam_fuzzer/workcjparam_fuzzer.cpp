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

#include "workcjparam_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

#include "bool_wrapper.h"
#include "double_wrapper.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "want_params.h"
#include "work_condition.h"
#include "work_info.h"
#include "work_scheduler_ffi.h"

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
constexpr int32_t PARAM_TYPE_F64 = 1;
constexpr int32_t PARAM_TYPE_STRING = 2;
constexpr int32_t PARAM_TYPE_BOOL = 3;
constexpr int32_t PARAM_TYPE_INVALID = 4;
constexpr int32_t BATTERY_LEVEL_SPREAD = 150;
constexpr int32_t TIMER_CYCLE_SPREAD = 16;
constexpr int32_t DOUBLE_SCALE = 100;
constexpr int32_t MIN_WORK_ID = 1;
constexpr int32_t NETWORK_RANGE = 8;
constexpr int32_t CHARGER_RANGE = 8;
constexpr int32_t BATTERY_STATUS_RANGE = 6;
constexpr int32_t STORAGE_RANGE = 6;
constexpr int8_t CJ_INT_TYPE = 0;
constexpr int8_t CJ_F64_TYPE = 1;
constexpr int8_t CJ_STRING_TYPE = 2;
constexpr int8_t CJ_BOOL_TYPE = 3;

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

char *AllocCString(const std::string &value)
{
    size_t len = value.size() + 1;
    char *buffer = static_cast<char *>(malloc(len));
    if (buffer == nullptr) {
        return nullptr;
    }
    if (value.empty()) {
        buffer[0] = '\0';
        return buffer;
    }
    return std::char_traits<char>::copy(buffer, value.c_str(), len);
}

void FreeRetWorkInfo(RetWorkInfo &cwork)
{
    free(cwork.bundleName);
    free(cwork.abilityName);
    cwork.bundleName = nullptr;
    cwork.abilityName = nullptr;
}

void FreeCArrParameters(CArrParameters &arrParam)
{
    if (arrParam.head == nullptr) {
        arrParam.size = 0;
        return;
    }
    for (int64_t i = 0; i < arrParam.size; ++i) {
        free(arrParam.head[i].key);
        free(arrParam.head[i].value);
        arrParam.head[i].key = nullptr;
        arrParam.head[i].value = nullptr;
    }
    free(arrParam.head);
    arrParam.head = nullptr;
    arrParam.size = 0;
}

void FillRetWorkInfo(FuzzData &provider, RetWorkInfo &cwork)
{
    cwork.workId = provider.GetInt32();
    std::string bundle = provider.GetString();
    std::string ability = provider.GetString();
    if (bundle.empty()) {
        bundle = "com.example.fuzz";
    }
    if (ability.empty()) {
        ability = "FuzzAbility";
    }
    cwork.bundleName = AllocCString(bundle);
    cwork.abilityName = AllocCString(ability);
    cwork.netWorkType = provider.GetInt32();
    cwork.isCharging = provider.GetInt32();
    cwork.chargerType = provider.GetInt32();
    cwork.batteryLevel = provider.GetInt32();
    cwork.batteryStatus = provider.GetInt32();
    cwork.storageRequest = provider.GetInt32();
    cwork.isRepeat = provider.GetInt32();
    cwork.repeatCycleTime = provider.GetInt32();
    cwork.repeatCount = provider.GetInt32();
    cwork.isPersisted = provider.GetBool();
    cwork.isDeepIdle = provider.GetInt32();
    cwork.idleWaitTime = provider.GetInt32();
}

bool FillCParameterIntValue(FuzzData &provider, CParameters &entry)
{
    int32_t *value = static_cast<int32_t *>(malloc(sizeof(int32_t)));
    if (value == nullptr) {
        return false;
    }
    *value = provider.GetInt32();
    entry.value = value;
    entry.size = sizeof(int32_t);
    return true;
}

bool FillCParameterDoubleValue(FuzzData &provider, CParameters &entry)
{
    double *value = static_cast<double *>(malloc(sizeof(double)));
    if (value == nullptr) {
        return false;
    }
    *value = static_cast<double>(provider.GetInt32()) / static_cast<double>(DOUBLE_SCALE);
    entry.value = value;
    entry.size = sizeof(double);
    return true;
}

bool FillCParameterStringValue(FuzzData &provider, CParameters &entry)
{
    std::string strVal = provider.GetString();
    entry.value = AllocCString(strVal);
    if (entry.value == nullptr) {
        return false;
    }
    entry.size = static_cast<int64_t>(strVal.size()) + 1;
    return true;
}

bool FillCParameterBoolValue(FuzzData &provider, CParameters &entry)
{
    bool *value = static_cast<bool *>(malloc(sizeof(bool)));
    if (value == nullptr) {
        return false;
    }
    *value = provider.GetBool();
    entry.value = value;
    entry.size = sizeof(bool);
    return true;
}

bool FillCParameterValue(FuzzData &provider, CParameters &entry)
{
    entry.valueType = static_cast<int8_t>(provider.GetU8() % (PARAM_TYPE_INVALID + 1));
    entry.value = nullptr;
    entry.size = 0;
    switch (entry.valueType) {
        case CJ_INT_TYPE:
            return FillCParameterIntValue(provider, entry);
        case CJ_F64_TYPE:
            return FillCParameterDoubleValue(provider, entry);
        case CJ_STRING_TYPE:
            return FillCParameterStringValue(provider, entry);
        case CJ_BOOL_TYPE:
            return FillCParameterBoolValue(provider, entry);
        default:
            return true;
    }
}

bool FillCParameterEntry(FuzzData &provider, int64_t index, CParameters *head)
{
    std::string key = provider.GetString();
    if (key.empty()) {
        key = "key" + std::to_string(index);
    }
    head[index].key = AllocCString(key);
    if (head[index].key == nullptr) {
        return false;
    }
    return FillCParameterValue(provider, head[index]);
}

void BuildCArrParameters(FuzzData &provider, CArrParameters &arrParam)
{
    int64_t requestCount = provider.GetU8() % EXTRA_PARAM_COUNT;
    if (requestCount == 0) {
        arrParam.size = 0;
        arrParam.head = nullptr;
        return;
    }
    arrParam.head = static_cast<CParameters *>(malloc(sizeof(CParameters) * requestCount));
    if (arrParam.head == nullptr) {
        arrParam.size = 0;
        return;
    }
    int64_t validCount = 0;
    for (int64_t i = 0; i < requestCount; ++i) {
        if (!FillCParameterEntry(provider, validCount, arrParam.head)) {
            free(arrParam.head[validCount].key);
            free(arrParam.head[validCount].value);
            arrParam.head[validCount].key = nullptr;
            arrParam.head[validCount].value = nullptr;
            continue;
        }
        ++validCount;
    }
    arrParam.size = validCount;
    if (validCount == 0) {
        free(arrParam.head);
        arrParam.head = nullptr;
    }
}

void AddNativeWorkInfoExtras(FuzzData &provider, WorkInfo &workInfo)
{
    if (!provider.GetBool()) {
        return;
    }
    AAFwk::WantParams params;
    int32_t count = provider.GetU8() % EXTRA_PARAM_COUNT;
    for (int32_t i = 0; i < count; ++i) {
        std::string key = provider.GetString();
        if (key.empty()) {
            key = "param" + std::to_string(i);
        }
        switch (provider.GetU8() % EXTRA_PARAM_COUNT) {
            case PARAM_TYPE_INT:
                params.SetParam(key, AAFwk::Integer::Box(provider.GetInt32()));
                break;
            case PARAM_TYPE_F64:
                params.SetParam(key, AAFwk::Double::Box(static_cast<double>(provider.GetInt32()) /
                    static_cast<double>(DOUBLE_SCALE)));
                break;
            case PARAM_TYPE_STRING:
                params.SetParam(key, AAFwk::String::Box(provider.GetString()));
                break;
            case PARAM_TYPE_BOOL:
                params.SetParam(key, AAFwk::Boolean::Box(provider.GetBool()));
                break;
            default:
                params.SetParam(key, AAFwk::Integer::Box(provider.GetInt32()));
                break;
        }
    }
    workInfo.RequestExtras(params);
}

void FillNativeWorkInfoConditions(FuzzData &provider, WorkInfo &workInfo)
{
    if (provider.GetBool()) {
        workInfo.RequestNetworkType(
            static_cast<WorkCondition::Network>(PositiveMod<NETWORK_RANGE>(provider.GetInt32())));
    }
    if (provider.GetBool()) {
        workInfo.RequestChargerType(provider.GetBool(),
            static_cast<WorkCondition::Charger>(PositiveMod<CHARGER_RANGE>(provider.GetInt32())));
    }
    if (provider.GetBool()) {
        workInfo.RequestBatteryLevel(PositiveMod<BATTERY_LEVEL_SPREAD>(provider.GetInt32()));
    }
    if (provider.GetBool()) {
        workInfo.RequestBatteryStatus(
            static_cast<WorkCondition::BatteryStatus>(PositiveMod<BATTERY_STATUS_RANGE>(provider.GetInt32())));
    }
    if (provider.GetBool()) {
        workInfo.RequestStorageLevel(
            static_cast<WorkCondition::Storage>(PositiveMod<STORAGE_RANGE>(provider.GetInt32())));
    }
    if (provider.GetBool()) {
        uint32_t interval = MIN_WORK_ID + static_cast<uint32_t>(PositiveMod<ENUM_SPREAD>(provider.GetInt32()));
        if (provider.GetBool()) {
            workInfo.RequestRepeatCycle(interval);
        } else {
            workInfo.RequestRepeatCycle(interval, PositiveMod<TIMER_CYCLE_SPREAD>(provider.GetInt32()));
        }
    }
}

WorkInfo BuildNativeWorkInfo(FuzzData &provider)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(MIN_WORK_ID + PositiveMod<ENUM_SPREAD>(provider.GetInt32()));
    std::string bundle = provider.GetString();
    std::string ability = provider.GetString();
    if (bundle.empty()) {
        bundle = "com.example.fuzz";
    }
    if (ability.empty()) {
        ability = "FuzzAbility";
    }
    workInfo.SetElement(bundle, ability);
    workInfo.RequestPersisted(provider.GetBool());
    FillNativeWorkInfoConditions(provider, workInfo);
    AddNativeWorkInfoExtras(provider, workInfo);
    return workInfo;
}

void FuzzGetWorkInfo(FuzzData &provider)
{
    RetWorkInfo cwork = {};
    FillRetWorkInfo(provider, cwork);
    WorkInfo workInfo;
    GetWorkInfo(cwork, workInfo);
    FreeRetWorkInfo(cwork);
}

void FuzzGetConditionParts(FuzzData &provider)
{
    RetWorkInfo cwork = {};
    FillRetWorkInfo(provider, cwork);
    WorkInfo workInfo;
    bool hasCondition = false;
    GetNetWorkInfo(cwork, workInfo, hasCondition);
    GetChargeInfo(cwork, workInfo, hasCondition);
    GetBatteryInfo(cwork, workInfo, hasCondition);
    GetStorageInfo(cwork, workInfo, hasCondition);
    GetRepeatInfo(cwork, workInfo, hasCondition);
    FreeRetWorkInfo(cwork);
}

void FuzzGetExtrasInfo(FuzzData &provider)
{
    RetWorkInfoV2 cworkV2 = {};
    FillRetWorkInfo(provider, cworkV2.v1);
    BuildCArrParameters(provider, cworkV2.parameters);
    WorkInfo workInfo;
    GetExtrasInfo(cworkV2, workInfo);
    FreeCArrParameters(cworkV2.parameters);
    FreeRetWorkInfo(cworkV2.v1);
}

void FuzzParseWorkInfo(FuzzData &provider)
{
    WorkInfo nativeInfo = BuildNativeWorkInfo(provider);
    auto workPtr = std::make_shared<WorkInfo>(nativeInfo);
    RetWorkInfo cwork = {};
    ParseWorkInfo(workPtr, cwork);
    if (cwork.bundleName != nullptr && cwork.abilityName != nullptr) {
        WorkInfo roundTrip;
        GetWorkInfo(cwork, roundTrip);
    }
    FreeRetWorkInfo(cwork);
}

void FuzzParseExtrasInfo(FuzzData &provider)
{
    WorkInfo nativeInfo = BuildNativeWorkInfo(provider);
    auto workPtr = std::make_shared<WorkInfo>(nativeInfo);
    CArrParameters arrParam = {};
    ParseExtrasInfo(workPtr, arrParam);
    FreeCArrParameters(arrParam);

    WorkInfo emptyInfo;
    auto emptyPtr = std::make_shared<WorkInfo>(emptyInfo);
    CArrParameters emptyArr = {};
    ParseExtrasInfo(emptyPtr, emptyArr);
    FreeCArrParameters(emptyArr);
}

void FuzzMallocCString(FuzzData &provider)
{
    std::string value = provider.GetString();
    char *result = MallocCString(value);
    free(result);
    char *emptyResult = MallocCString("");
    free(emptyResult);
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < MIN_SIZE) {
        return false;
    }
    FuzzData provider(data, size);
    FuzzGetWorkInfo(provider);
    FuzzGetConditionParts(provider);
    FuzzGetExtrasInfo(provider);
    FuzzParseWorkInfo(provider);
    FuzzParseExtrasInfo(provider);
    FuzzMallocCString(provider);
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
