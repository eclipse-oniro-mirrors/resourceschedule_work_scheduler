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
#include "work_sched_errors.h"

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
bool g_hasParamError = false;

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
    int32_t workId = GetIntProperty(env, objValue, "workId", E_WORKID_ERR);
    if (workId == UNSET_INT_PARAM || workId < 0) {
        WS_HILOGE("Work id is invalid, failed.");
        HandleParamErr(env, E_WORKID_ERR);
        return false;
    }

    // Get bundleName and abilityName.
    std::string bundleName = GetStringProperty(env, objValue, "bundleName", E_BUNDLE_OR_ABILITY_NAME_ERR);
    std::string abilityName = GetStringProperty(env, objValue, "abilityName", E_BUNDLE_OR_ABILITY_NAME_ERR);
    if (bundleName == UNSET_STRING_PARAM || abilityName == UNSET_STRING_PARAM) {
        WS_HILOGE("BundleName or abilityName is invalid, failed.");
        HandleParamErr(env, E_BUNDLE_OR_ABILITY_NAME_ERR);
        return false;
    }

    workInfo.SetWorkId(workId);
    workInfo.SetElement(bundleName, abilityName);

    // Get persist param. if not set, it will be used false.
    workInfo.RequestPersisted(GetBoolProperty(env, objValue, "isPersisted", E_IS_PERSISTED_ERR));
    return true;
}

bool Common::GetNetWorkInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    bool hasCondition = false;
    int32_t networkType = GetIntProperty(env, objValue, "networkType", E_NETWORK_TYPE_ERR);
    if (networkType == UNSET_INT_PARAM) {
        WS_HILOGI("Unset networkType.");
    } else if (networkType >= WorkCondition::Network::NETWORK_TYPE_ANY &&
        networkType <= WorkCondition::Network::NETWORK_TYPE_ETHERNET) {
        workInfo.RequestNetworkType(WorkCondition::Network(networkType));
        hasCondition = true;
    } else {
        WS_HILOGE("NetworkType set is invalid, just ignore set.");
        HandleParamErr(env, E_NETWORK_TYPE_ERR);
    }
    return hasCondition;
}

bool Common::GetChargeInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    bool hasCondition = false;
    int32_t isCharging = GetBoolToIntProperty(env, objValue, "isCharging", E_IS_CHARGING_ERR);
    int32_t chargerType = GetIntProperty(env, objValue, "chargerType", E_CHARGER_TYPE_ERR);
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
            HandleParamErr(env, E_CHARGER_TYPE_ERR);
        }
        hasCondition = true;
    }
    return hasCondition;
}

bool Common::GetBatteryInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    bool hasCondition = false;
    // Get battery level info.
    int32_t batteryLevel = GetIntProperty(env, objValue, "batteryLevel", E_BATTERY_LEVEL_ERR);
    if (batteryLevel == UNSET_INT_PARAM) {
        WS_HILOGI("Unset batteryLevel.");
    } else if (batteryLevel >= BATTERY_LEVEL_MIN && batteryLevel <= BATTERY_LEVEL_MAX) {
        workInfo.RequestBatteryLevel(batteryLevel);
        hasCondition = true;
    } else {
        WS_HILOGE("BatteryLevel set is invalid, just ignore set.");
        HandleParamErr(env, E_BATTERY_LEVEL_ERR);
    }

    // Get battery status info.
    int32_t batteryStatus = GetIntProperty(env, objValue, "batteryStatus", E_BATTERY_STATUS_ERR);
    if (batteryStatus == UNSET_INT_PARAM) {
        WS_HILOGI("Unset batteryStatus.");
    } else if (batteryStatus >= WorkCondition::BatteryStatus::BATTERY_STATUS_LOW &&
        batteryStatus <= WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY) {
        workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus(batteryStatus));
        hasCondition = true;
    } else {
        WS_HILOGE("BatteryStatus set is invalid, just ignore set.");
        HandleParamErr(env, E_BATTERY_STATUS_ERR);
    }
    return hasCondition;
}

bool Common::GetStorageInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    bool hasCondition = false;
    int32_t storageRequest = GetIntProperty(env, objValue, "storageRequest", E_STORAGE_REQUEST_ERR);
    if (storageRequest == UNSET_INT_PARAM) {
        WS_HILOGI("Unset StorageRequest.");
    } else if (storageRequest >= WorkCondition::Storage::STORAGE_LEVEL_LOW
            && storageRequest <= WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY) {
        workInfo.RequestStorageLevel(WorkCondition::Storage(storageRequest));
        hasCondition = true;
    } else {
        WS_HILOGE("StorageRequest set is invalid, just ignore set.");
        HandleParamErr(env, E_STORAGE_REQUEST_ERR);
    }
    return hasCondition;
}

bool Common::GetRepeatInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    int32_t repeatCycleTime = GetIntProperty(env, objValue, "repeatCycleTime", E_REPEAT_CYCLE_TIME_TYPE_ERR);
    if (repeatCycleTime == UNSET_INT_PARAM) {
        WS_HILOGI("RepeatCycleTime not set, just ignore other repeat set.");
        return false;
    }

    bool isRepeat = GetBoolProperty(env, objValue, "isRepeat", E_IS_REPEAT_ERR);
    int32_t repeatCount = GetIntProperty(env, objValue, "repeatCount", E_REPEAT_COUNT_ERR);
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
            HandleParamErr(env, E_REPEAT_COUNT_ERR);
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
        HandleParamErr(env, E_PARAMETERS_TYPE_ERR);
        return false;
    }
    workInfo.RequestExtras(extraParams);
    WS_HILOGD("Get parameters finished.");
    return true;
}


bool Common::GetWorkInfo(napi_env env, napi_value objValue, WorkInfo &workInfo)
{
    g_hasParamError = false;
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
    // if param error occurs when get workInfo
    if (g_hasParamError) {
        return false;
    }
    if (!hasConditions) {
        WS_HILOGE("Set none conditions, so fail to init WorkInfo.");
        HandleParamErr(env, E_CONDITION_EMPTY);
        return false;
    }
    return true;
}

int32_t Common::GetIntProperty(napi_env env, napi_value object,
    const std::string &propertyName, ErrCode errCode)
{
    int32_t intValue = UNSET_INT_PARAM;
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus != napi_ok) {
        return intValue;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_undefined) {
        WS_HILOGD("Unset %{public}s.", propertyName.c_str());
        return intValue;
    } else if (valueType != napi_number) {
        WS_HILOGE("%{public}s type error, number expect.", propertyName.c_str());
        HandleParamErr(env, errCode);
        return intValue;
    }
    napi_get_value_int32(env, value, &intValue);
    return intValue;
}

bool Common::GetBoolProperty(napi_env env, napi_value object,
    const std::string &propertyName, ErrCode errCode)
{
    bool boolValue = false;
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus != napi_ok) {
        return boolValue;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_undefined) {
        WS_HILOGD("Unset %{public}s.", propertyName.c_str());
        return boolValue;
    } else if (valueType != napi_boolean) {
        WS_HILOGE("%{public}s type error, boolean expect.", propertyName.c_str());
        HandleParamErr(env, errCode);
        return boolValue;
    }
    napi_get_value_bool(env, value, &boolValue);
    return boolValue;
}

int32_t Common::GetBoolToIntProperty(napi_env env, napi_value object,
    const std::string &propertyName, ErrCode errCode)
{
    bool boolValue = false;
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus != napi_ok) {
        return UNSET_INT_PARAM;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_undefined) {
        WS_HILOGD("Unset %{public}s.", propertyName.c_str());
        return UNSET_INT_PARAM;
    } else if (valueType != napi_boolean) {
        WS_HILOGE("%{public}s type error, boolean expect.", propertyName.c_str());
        HandleParamErr(env, errCode);
        return UNSET_INT_PARAM;
    }
    napi_status getIntStatus = napi_get_value_bool(env, value, &boolValue);
    if (getIntStatus == napi_ok) {
        return boolValue ? TRUE_PARAM : FALSE_PARAM;
    }
    return UNSET_INT_PARAM;
}

std::string Common::GetStringProperty(napi_env env, napi_value object,
    const std::string &propertyName, ErrCode errCode)
{
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus != napi_ok) {
        return UNSET_STRING_PARAM;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_undefined) {
        WS_HILOGD("Unset %{public}s.", propertyName.c_str());
        return UNSET_STRING_PARAM;
    } else if (valueType != napi_string) {
        WS_HILOGE("%{public}s type error, string expect.", propertyName.c_str());
        HandleParamErr(env, errCode);
        return UNSET_STRING_PARAM;
    }
    char chars[NAME_MAXIMUM_LIMIT] = {0};
    size_t charLength = 0;
    napi_status getStringStatus = napi_get_value_string_utf8(env, value, chars, NAME_MAXIMUM_LIMIT, &charLength);
    if (getStringStatus == napi_ok && charLength > 0) {
        return std::string(chars, charLength);
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

napi_value Common::GetCallbackErrorValue(napi_env env, int32_t errCode, const std::string errMsg)
{
    if (errCode == ERR_OK) {
        return NapiGetNull(env);
    }
    napi_value error = nullptr;
    napi_value eCode = nullptr;
    napi_value eMsg = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_string_utf8(env, errMsg.c_str(),
        errMsg.length(), &eMsg));
    NAPI_CALL(env, napi_create_object(env, &error));
    NAPI_CALL(env, napi_set_named_property(env, error, "code", eCode));
    NAPI_CALL(env, napi_set_named_property(env, error, "message", eMsg));
    return error;
}

void Common::SetCallback(
    const napi_env &env, const napi_ref &callbackIn, int32_t errCode, const napi_value &result)
{
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    napi_value results[RESULT_PARAMS_NUM] = {nullptr};
    if (errCode == ERR_OK) {
        results[0] = NapiGetNull(env);
    } else {
        std::string errMsg = FindErrMsg(env, errCode);
        int32_t errCodeInfo = FindErrCode(env, errCode);
        results[0] = GetCallbackErrorValue(env, errCodeInfo, errMsg);
    }
    results[1] = result;
    NAPI_CALL_RETURN_VOID(env,
        napi_call_function(env, undefined, callback, RESULT_PARAMS_NUM, &results[0], &resultout));
}

napi_value Common::SetPromise(
    const napi_env &env, const AsyncWorkData &info, const napi_value &result)
{
    if (info.errorCode == ERR_OK) {
        napi_resolve_deferred(env, info.deferred, result);
    } else {
        int32_t errCodeInfo = FindErrCode(env, info.errorCode);
        std::string errMsg = FindErrMsg(env, info.errorCode);
        napi_value error = nullptr;
        napi_value eCode = nullptr;
        napi_value eMsg = nullptr;
        NAPI_CALL(env, napi_create_int32(env, errCodeInfo, &eCode));
        NAPI_CALL(env, napi_create_string_utf8(env, errMsg.c_str(),
            errMsg.length(), &eMsg));
        NAPI_CALL(env, napi_create_object(env, &error));
        NAPI_CALL(env, napi_set_named_property(env, error, "data", eCode));
        NAPI_CALL(env, napi_set_named_property(env, error, "code", eCode));
        NAPI_CALL(env, napi_set_named_property(env, error, "message", eMsg));
        napi_reject_deferred(env, info.deferred, error);
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

void Common::HandleErrCode(const napi_env &env, int32_t errCode)
{
    WS_HILOGI("HandleErrCode errCode = %{public}d", errCode);
    if (errCode == ERR_OK) {
        return;
    }
    std::string errMsg = FindErrMsg(env, errCode);
    int32_t errCodeInfo = FindErrCode(env, errCode);
    if (errMsg != "") {
        napi_throw_error(env, std::to_string(errCodeInfo).c_str(), errMsg.c_str());
    }
}

void Common::HandleParamErr(const napi_env &env, int32_t errCode)
{
    WS_HILOGI("HandleParamErr errCode = %{public}d", errCode);
    if (errCode == ERR_OK) {
        return;
    }
    auto iter = paramErrCodeMsgMap.find(errCode);
    if (iter != paramErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError 401: Parameter error. ";
        errMessage.append(iter->second);
        napi_throw_error(env, std::to_string(E_PARAM_ERROR).c_str(), errMessage.c_str());
        g_hasParamError = true;
    }
}

std::string Common::FindErrMsg(const napi_env &env, int32_t errCode)
{
    if (errCode == ERR_OK) {
        return "";
    }
    auto iter = saErrCodeMsgMap.find(errCode);
    if (iter != saErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError ";
        int32_t errCodeInfo = FindErrCode(env, errCode);
        errMessage.append(std::to_string(errCodeInfo)).append(": ").append(iter->second);
        return errMessage;
    }
    iter = paramErrCodeMsgMap.find(errCode);
    if (iter != paramErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError 401: Parameter error. ";
        errMessage.append(iter->second);
        return errMessage;
    }
    return "Inner error.";
}

int32_t Common::FindErrCode(const napi_env &env, int32_t errCodeIn)
{
    auto iter = paramErrCodeMsgMap.find(errCodeIn);
    if (iter != paramErrCodeMsgMap.end()) {
        return E_PARAM_ERROR;
    }
    return errCodeIn > THRESHOLD ? errCodeIn / OFFSET : errCodeIn;
}
} // namespace WorkScheduler
} // namespace OHOS