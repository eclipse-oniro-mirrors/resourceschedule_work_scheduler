/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "common.h"
#include "common_want.h"

#include "errors.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
const int32_t NAME_MAXIMUM_LIMIT = 128;
const int32_t RESULT_PARAMS_NUM = 2;
const int32_t UNSET_INT_PARAM = -1;
const std::string UNSET_STRING_PARAM = "";
const int32_t TRUE_PARAM = 1;
const int32_t FALSE_PARAM = 0;
const int32_t BATTERY_LEVEL_MIN = 0;
const int32_t BATTERY_LEVEL_MAX = 100;

AsyncWorkData::AsyncWorkData(napi_env napiEnv)
{
    env = napiEnv;
}

AsyncWorkData::~AsyncWorkData()
{
    if (callback) {
        WS_HILOGD("callback delete");
        napi_delete_reference(env, callback);
        callback = nullptr;
    }
    if (asyncWork) {
        WS_HILOGD("asyncWork delete");
        napi_delete_async_work(env, asyncWork);
        asyncWork = nullptr;
    }
}

napi_value Common::NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

bool Common::GetBaseWorkInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    // Get workid.
    int32_t workId = Common::GetIntProperty(env, objValue, "workId");
    if (workId == UNSET_INT_PARAM || workId < 0) {
        WS_HILOGE("Work id is invalid, failed.");
        return false;
    }

    // Get bundleName and abilityName.
    std::string bundleName = Common::GetStringProperty(env, objValue, "bundleName");
    std::string abilityName = Common::GetStringProperty(env, objValue, "abilityName");
    if (bundleName == UNSET_STRING_PARAM || abilityName == UNSET_STRING_PARAM) {
        WS_HILOGE("BundleName or abilityName is invalid, failed.");
        return false;
    }

    workInfo.SetWorkId(workId);
    workInfo.SetElement(bundleName, abilityName);

    // Get persist param. if not set, it will be used false.
    workInfo.RequestPersisted(Common::GetBoolProperty(env, objValue, "isPersisted"));
    return true;
}

bool Common::GetNetWorkInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    bool hasCondition = false;
    int32_t networkType = Common::GetIntProperty(env, objValue, "networkType");
    if (networkType == UNSET_INT_PARAM) {
        WS_HILOGI("Unset networkType.");
    } else if (networkType >= WorkCondition::Network::NETWORK_TYPE_ANY &&
        networkType <= WorkCondition::Network::NETWORK_TYPE_ETHERNET) {
        workInfo.RequestNetworkType(WorkCondition::Network(networkType));
        hasCondition = true;
    } else {
        WS_HILOGE("NetworkType set is invalid, just ignore set.");
    }
    return hasCondition;
}

bool Common::GetChargeInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    bool hasCondition = false;
    int32_t isCharging = Common::GetBoolToIntProperty(env, objValue, "isCharging");
    int32_t chargerType = Common::GetIntProperty(env, objValue, "chargerType");
    if (isCharging == UNSET_INT_PARAM) {
        WS_HILOGI("Unset isCharging, ignore ChargerType set also.");
    } else if (isCharging == FALSE_PARAM) {
        workInfo.RequestChargerType(false, WorkCondition::Charger::CHARGING_UNPLUGGED);
        hasCondition = true;
    }  else {
        if (chargerType == UNSET_INT_PARAM) {
            workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_ANY);
        } else if (chargerType >=  WorkCondition::Charger::CHARGING_PLUGGED_ANY &&
            chargerType <= WorkCondition::Charger::CHARGING_PLUGGED_WIRELESS) {
            workInfo.RequestChargerType(true, WorkCondition::Charger(chargerType));
        } else {
            workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_ANY);
            WS_HILOGE("ChargeType info is invalid, just ignore set.");
        }
        hasCondition = true;
    }
    return hasCondition;
}

bool Common::GetBatteryInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    bool hasCondition = false;
    // Get battery level info.
    int32_t batteryLevel = Common::GetIntProperty(env, objValue, "batteryLevel");
    if (batteryLevel == UNSET_INT_PARAM) {
        WS_HILOGI("Unset batteryLevel.");
    } else if (batteryLevel >= BATTERY_LEVEL_MIN && batteryLevel <= BATTERY_LEVEL_MAX) {
        workInfo.RequestBatteryLevel(batteryLevel);
        hasCondition = true;
    } else {
        WS_HILOGE("BatteryLevel set is invalid, just ignore set.");
    }

    // Get battery status info.
    int32_t batteryStatus = Common::GetIntProperty(env, objValue, "batteryStatus");
    if (batteryStatus == UNSET_INT_PARAM) {
        WS_HILOGI("Unset batteryStatus.");
    } else if (batteryStatus >= WorkCondition::BatteryStatus::BATTERY_STATUS_LOW &&
        batteryStatus <= WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY) {
        workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus(batteryStatus));
        hasCondition = true;
    } else {
        WS_HILOGE("BatteryStatus set is invalid, just ignore set.");
    }
    return hasCondition;
}

bool Common::GetStorageInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    bool hasCondition = false;
    int32_t storageRequest = Common::GetIntProperty(env, objValue, "storageRequest");
    if (storageRequest == UNSET_INT_PARAM) {
        WS_HILOGI("Unset StorageRequest.");
    } else if (storageRequest >= WorkCondition::Storage::STORAGE_LEVEL_LOW
        && storageRequest <= WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY) {
        workInfo.RequestStorageLevel(WorkCondition::Storage(storageRequest));
        hasCondition = true;
    } else {
        WS_HILOGE("StorageRequest set is invalid, just ignore set.");
    }
    return hasCondition;
}

bool Common::GetRepeatInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    int32_t repeatCycleTime = Common::GetIntProperty(env, objValue, "repeatCycleTime");
    if (repeatCycleTime == UNSET_INT_PARAM) {
        WS_HILOGI("RepeatCycleTime not set, just ignore other repeat set.");
        return false;
    }

    bool isRepeat = Common::GetBoolProperty(env, objValue, "isRepeat");
    int32_t repeatCount = Common::GetIntProperty(env, objValue, "repeatCount");
    if (!isRepeat && repeatCount == UNSET_INT_PARAM) {
        WS_HILOGI("Not set isRepeat or repeatCount, ignore.");
        return false;
    }
    if (isRepeat) {
        if (repeatCount > 0) {
            WS_HILOGI("RepeatCount has been set , ignore isRepeat.");
            workInfo.RequestRepeatCycle(repeatCycleTime, repeatCount);
        } else {
            workInfo.RequestRepeatCycle(repeatCycleTime);
        }
        return true;
    } else {
        if (repeatCount < 0) {
            WS_HILOGE("RepeatCount is invalid, ignore.");
            return false;
        }
        workInfo.RequestRepeatCycle(repeatCycleTime, repeatCount);
        return true;
    }
}

bool Common::GetExtrasInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    napi_value extras = nullptr;
    napi_status getExtrasStatus = napi_get_named_property(env, objValue, "parameters", &extras);
    if (getExtrasStatus != napi_ok) {
        return true;
    }
    AAFwk::WantParams extraParams;
    if (!UnwrapWantParams(env, extras, extraParams)) {
        return false;
    }
    workInfo.RequestExtras(extraParams);
    WS_HILOGD("Get parameters finished.");
    return true;
}


bool Common::GetWorkInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    // Get base info.
    if (!GetBaseWorkInfo(env, objValue, workInfo)) {
        return false;
    }
    // Get extra parameters.
    if (!GetExtrasInfo(env, objValue, workInfo)) {
        return false;
    }

    // Get condition info.
    bool hasConditions = false;
    if (GetNetWorkInfo(env, objValue, workInfo)) {
        hasConditions = true;
    }
    if (GetChargeInfo(env, objValue, workInfo)) {
        hasConditions = true;
    }
    if (GetBatteryInfo(env, objValue, workInfo)) {
        hasConditions = true;
    }
    if (GetStorageInfo(env, objValue, workInfo)) {
        hasConditions = true;
    }
    if (GetRepeatInfo(env, objValue, workInfo)) {
        hasConditions = true;
    }

    if (!hasConditions) {
        WS_HILOGE("Set none conditions, so fail to init WorkInfo.");
        return false;
    }
    return true;
}

int32_t Common::GetIntProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    int32_t intValue = UNSET_INT_PARAM;
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus == napi_ok) {
        napi_status getIntStatus = napi_get_value_int32(env, value, &intValue);
        if (getIntStatus == napi_ok) {
            return intValue;
        }
    }
    return intValue;
}

bool Common::GetBoolProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    bool boolValue = false;
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus == napi_ok) {
        napi_status getIntStatus = napi_get_value_bool(env, value, &boolValue);
        if (getIntStatus == napi_ok) {
            return boolValue;
        }
    }
    return boolValue;
}

int32_t Common::GetBoolToIntProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    bool boolValue = false;
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus == napi_ok) {
        napi_status getIntStatus = napi_get_value_bool(env, value, &boolValue);
        if (getIntStatus == napi_ok) {
            return boolValue ? TRUE_PARAM : FALSE_PARAM;
        }
    }
    return UNSET_INT_PARAM;
}

std::string Common::GetStringProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus == napi_ok) {
        char chars[NAME_MAXIMUM_LIMIT] = {0};
        size_t charLength = 0;
        napi_status getStringStatus =
            napi_get_value_string_utf8(env, value, chars, NAME_MAXIMUM_LIMIT, &charLength);
        if (getStringStatus == napi_ok && charLength > 0) {
            return std::string(chars, charLength);
        }
    }
    return UNSET_STRING_PARAM;
}

bool Common::MatchValueType(napi_env env, napi_value value, napi_valuetype targetType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    return valueType == targetType;
}

napi_value Common::JSParaError(const napi_env &env, const napi_ref &callback)
{
    if (callback) {
        return Common::NapiGetNull(env);
    } else {
        napi_value promise = nullptr;
        napi_deferred deferred = nullptr;
        napi_create_promise(env, &deferred, &promise);
        napi_resolve_deferred(env, deferred, Common::NapiGetNull(env));
        return promise;
    }
}

void Common::PaddingAsyncWorkData(
    const napi_env &env, const napi_ref &callback, AsyncWorkData &info, napi_value &promise)
{
    if (callback) {
        info.callback = callback;
        info.isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        info.deferred = deferred;
        info.isCallback = false;
    }
}

napi_value Common::GetNapiWorkInfo(napi_env env, std::shared_ptr<WorkInfo> &workInfo)
{
    if (workInfo == nullptr) {
        return NapiGetNull(env);
    }
    napi_value napiWork = nullptr;
    napi_create_object(env, &napiWork);

    // Set base info.
    napi_value napiWorkId = nullptr;
    napi_value napiBundleName = nullptr;
    napi_value napiAbilityName = nullptr;
    napi_create_int32(env, workInfo->GetWorkId(), &napiWorkId);
    napi_create_string_utf8(env, workInfo->GetBundleName().c_str(), workInfo->GetBundleName().length(),
        &napiBundleName);
    napi_create_string_utf8(env, workInfo->GetAbilityName().c_str(), workInfo->GetAbilityName().length(),
        &napiAbilityName);
    napi_set_named_property(env, napiWork, "workId", napiWorkId);
    napi_set_named_property(env, napiWork, "bundleName", napiBundleName);
    napi_set_named_property(env, napiWork, "abilityName", napiAbilityName);

    // Set isPersisted.
    napi_value napiIsPersisted = nullptr;
    napi_get_boolean(env, workInfo->IsPersisted(), &napiIsPersisted);
    napi_set_named_property(env, napiWork, "isPersisted", napiIsPersisted);

    // Set net info.
    if (workInfo->GetNetworkType() != WorkCondition::Network::NETWORK_UNKNOWN) {
        napi_value napiNetworkType = nullptr;
        napi_create_int32(env, static_cast<int32_t>(workInfo->GetNetworkType()), &napiNetworkType);
        napi_set_named_property(env, napiWork, "networkType", napiNetworkType);
    }

    // Set charge info.
    WorkCondition::Charger charger = workInfo-> GetChargerType();
    if (charger != WorkCondition::Charger::CHARGING_UNKNOWN) {
        napi_value napiIsCharging = nullptr;
        if (charger == WorkCondition::Charger::CHARGING_UNPLUGGED) {
            napi_get_boolean(env, false, &napiIsCharging);
            napi_set_named_property(env, napiWork, "isCharging", napiIsCharging);
        } else {
            napi_get_boolean(env, true, &napiIsCharging);
            napi_set_named_property(env, napiWork, "isCharging", napiIsCharging);
            napi_value napiChargerType = nullptr;
            napi_create_int32(env, static_cast<int32_t>(charger), &napiChargerType);
            napi_set_named_property(env, napiWork, "chargerType", napiChargerType);
        }
    }

    // Set batteryLevel info.
    if (workInfo->GetBatteryLevel() >= 0) {
        napi_value napiBatteryLevel = nullptr;
        napi_create_int32(env, workInfo->GetBatteryLevel(), &napiBatteryLevel);
        napi_set_named_property(env, napiWork, "batteryLevel", napiBatteryLevel);
    }

    // Set batteryStatus info.
    if (workInfo->GetBatteryStatus() != WorkCondition::BatteryStatus::BATTERY_UNKNOWN) {
        napi_value napiBatteryStatus = nullptr;
        napi_create_int32(env, static_cast<int32_t>(workInfo->GetBatteryStatus()), &napiBatteryStatus);
        napi_set_named_property(env, napiWork, "batteryStatus", napiBatteryStatus);
    }

    // Set storage info.
    if (workInfo->GetStorageLevel() != WorkCondition::Storage::STORAGE_UNKNOWN) {
        napi_value napiStorageRequest = nullptr;
        napi_create_int32(env, static_cast<int32_t>(workInfo->GetStorageLevel()), &napiStorageRequest);
        napi_set_named_property(env, napiWork, "storageRequest", napiStorageRequest);
    }

    // Set timer info.
    uint32_t timeInterval = workInfo->GetTimeInterval();
    if (timeInterval > 0) {
        napi_value napiTimer = nullptr;
        napi_create_int32(env, static_cast<int32_t>(timeInterval), &napiTimer);
        napi_set_named_property(env, napiWork, "repeatCycleTime", napiTimer);
        if  (workInfo->IsRepeat()) {
            napi_value napiIsRepeat = nullptr;
            napi_get_boolean(env, true, &napiIsRepeat);
            napi_set_named_property(env, napiWork, "isRepeat", napiIsRepeat);
        } else {
            napi_value napiCount = nullptr;
            napi_create_int32(env, workInfo->GetCycleCount(), &napiCount);
            napi_set_named_property(env, napiWork, "repeatCount", napiCount);
        }
    }

    if (workInfo->GetExtras()) {
        napi_value parameters = WrapWantParams(env, *workInfo->GetExtras());
        napi_set_named_property(env, napiWork, "parameters", parameters);
    }
    return napiWork;
}

napi_value Common::GetCallbackErrorValue(napi_env env, int32_t errCode)
{
    if (errCode == ERR_OK) {
        return NapiGetNull(env);
    }
    napi_value result = nullptr;
    napi_value eCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "data", eCode));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));
    return result;
}

void Common::SetCallback(
    const napi_env &env, const napi_ref &callbackIn, const int32_t &errorCode, const napi_value &result)
{
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    napi_value results[RESULT_PARAMS_NUM] = {nullptr};
    results[0] = GetCallbackErrorValue(env, errorCode);
    results[1] = result;
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback,
        RESULT_PARAMS_NUM, &results[0], &resultout));
}

napi_value Common::SetPromise(
    const napi_env &env, const AsyncWorkData &info, const napi_value &result)
{
    if (info.errorCode == ERR_OK) {
        napi_resolve_deferred(env, info.deferred, result);
    } else {
        napi_value res = nullptr;
        napi_value eCode = nullptr;
        NAPI_CALL(env, napi_create_int32(env, info.errorCode, &eCode));
        NAPI_CALL(env, napi_create_object(env, &res));
        NAPI_CALL(env, napi_set_named_property(env, res, "data", eCode));
        NAPI_CALL(env, napi_set_named_property(env, res, "code", eCode));
        napi_reject_deferred(env, info.deferred, res);
    }
    return result;
}

void Common::ReturnCallbackPromise(const napi_env &env, const AsyncWorkData &info, const napi_value &result)
{
    if (info.isCallback) {
        SetCallback(env, info.callback, info.errorCode, result);
    } else {
        SetPromise(env, info, result);
    }
}
} // namespace WorkScheduler
} // namespace OHOS