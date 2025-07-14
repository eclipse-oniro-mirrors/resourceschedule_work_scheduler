/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "work_sched_hilog.h"
#include "want_params_wrapper.h"
#include "bool_wrapper.h"
#include "double_wrapper.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "work_sched_errors.h"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace ohos::resourceschedule::workScheduler;

namespace OHOS {
namespace WorkScheduler {
const int32_t BATTERY_LEVEL_MIN = 0;
const int32_t BATTERY_LEVEL_MAX = 100;
const int32_t TRUE_PARAM = 1;
const int32_t UNSET_INT_PARAM = -1;
// need to be same as WantParams
enum {
    VALUE_TYPE_NULL = -1,
    VALUE_TYPE_BOOLEAN = 1,
    VALUE_TYPE_INT = 5,
    VALUE_TYPE_DOUBLE = 8,
    VALUE_TYPE_STRING = 9,
};

int32_t Common::FindErrCode(const int32_t errCodeIn)
{
    auto iter = paramErrCodeMsgMap.find(errCodeIn);
    if (iter != paramErrCodeMsgMap.end()) {
        return E_PARAM_ERROR;
    }
    return errCodeIn > THRESHOLD ? errCodeIn / OFFSET : errCodeIn;
}

std::string Common::FindErrMsg(const int32_t errCode)
{
    if (errCode == ERR_OK) {
        return "";
    }
    auto iter = saErrCodeMsgMap.find(errCode);
    if (iter != saErrCodeMsgMap.end()) {
        std::string errMessage = "BussinessError ";
        int32_t errCodeInfo = FindErrCode(errCode);
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

bool Common::GetExtrasInfo(
    ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo)
{
    if (!aniWork.parameters.has_value()) {
        return true;
    }
    AAFwk::WantParams extraParams;
    auto aniParameters = aniWork.parameters.value();

    for (const auto &item : aniParameters) {
        std::string key = std::string(item.first);
        switch (item.second.get_tag()) {
            case ohos::resourceschedule::workScheduler::ParameType::tag_t::int_type: {
                int32_t intVal = item.second.get_int_type_ref();
                extraParams.SetParam(key, AAFwk::Integer::Box(intVal));
                break;
            }
            case ohos::resourceschedule::workScheduler::ParameType::tag_t::double_type: {
                double doubleVal = item.second.get_double_type_ref();
                extraParams.SetParam(key, AAFwk::Double::Box(doubleVal));
                break;
            }
            case ohos::resourceschedule::workScheduler::ParameType::tag_t::string_type: {
                std::string strVal = std::string(item.second.get_string_type_ref());
                extraParams.SetParam(key, AAFwk::String::Box(strVal));
                break;
            }
            case ohos::resourceschedule::workScheduler::ParameType::tag_t::bool_type: {
                bool boolVal = item.second.get_bool_type_ref();
                extraParams.SetParam(key, AAFwk::Boolean::Box(boolVal));
                break;
            }
            default: {
                WS_HILOGE("parameters type error.");
                set_business_error(E_PARAMETERS_TYPE_ERR, FindErrMsg(E_PARAMETERS_TYPE_ERR));
                return false;
            }
        }
    }
    workInfo.RequestExtras(extraParams);
    WS_HILOGD("Get parameters finished.");
    return true;
}

bool Common::GetNetWorkInfo(
    ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo)
{
    if (!aniWork.networkType.has_value()) {
        WS_HILOGD("Unset networkType.");
        return false;
    }
    auto networkType = aniWork.networkType.value().get_value();
    if (networkType >= WorkCondition::Network::NETWORK_TYPE_ANY &&
        networkType <= WorkCondition::Network::NETWORK_TYPE_ETHERNET) {
        workInfo.RequestNetworkType(WorkCondition::Network(networkType));
        return true;
    } else {
        WS_HILOGE("NetworkType set is invalid, just ignore set.");
        set_business_error(E_NETWORK_TYPE_ERR, FindErrMsg(E_NETWORK_TYPE_ERR));
        return false;
    }
}

bool Common::GetChargeInfo(
    ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo)
{
    if (!aniWork.isCharging.has_value()) {
        WS_HILOGD("Unset isCharging, ignore ChargerType set also.");
        return false;
    }
    if (!aniWork.isCharging.value()) {
        workInfo.RequestChargerType(false, WorkCondition::Charger::CHARGING_UNPLUGGED);
    } else {
        if (!aniWork.chargerType.has_value()) {
            workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_ANY);
            return true;
        }
        auto chargerType = aniWork.chargerType.value().get_value();
        if (chargerType >= WorkCondition::Charger::CHARGING_PLUGGED_ANY &&
            chargerType <= WorkCondition::Charger::CHARGING_PLUGGED_WIRELESS) {
            workInfo.RequestChargerType(true, WorkCondition::Charger(chargerType));
        } else {
            workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_ANY);
            WS_HILOGE("ChargeType info is invalid, just ignore set.");
            set_business_error(E_CHARGER_TYPE_ERR, FindErrMsg(E_CHARGER_TYPE_ERR));
        }
    }
    return true;
}

bool Common::GetBatteryInfo(
    ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo)
{
    bool hasCondition = false;
    // Get battery level info.
    if (!aniWork.batteryLevel.has_value()) {
        WS_HILOGD("Unset batteryLevel.");
    } else {
        auto batteryLevel = static_cast<int32_t>(aniWork.batteryLevel.value());
        if (batteryLevel >= BATTERY_LEVEL_MIN && batteryLevel <= BATTERY_LEVEL_MAX) {
            workInfo.RequestBatteryLevel(batteryLevel);
            hasCondition = true;
        } else {
            WS_HILOGE("BatteryLevel set is invalid, just ignore set.");
            set_business_error(E_BATTERY_LEVEL_ERR, FindErrMsg(E_BATTERY_LEVEL_ERR));
            return false;
        }
    }

    // Get battery status info.
    if (!aniWork.batteryStatus.has_value()) {
        WS_HILOGD("Unset batteryStatus.");
    } else {
        auto batteryStatus = aniWork.batteryStatus.value().get_value();
        if (batteryStatus >= WorkCondition::BatteryStatus::BATTERY_STATUS_LOW &&
            batteryStatus <= WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY) {
            workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus(batteryStatus));
            hasCondition = true;
        } else {
            WS_HILOGE("BatteryStatus set is invalid, just ignore set.");
            set_business_error(E_BATTERY_STATUS_ERR, FindErrMsg(E_BATTERY_STATUS_ERR));
        }
    }
    return hasCondition;
}

bool Common::GetStorageInfo(
    ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo)
{
    if (!aniWork.storageRequest.has_value()) {
        WS_HILOGD("Unset StorageRequest.");
        return false;
    }
    auto storageRequest = aniWork.storageRequest.value().get_value();
    if (storageRequest >= WorkCondition::Storage::STORAGE_LEVEL_LOW &&
            storageRequest <= WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY) {
        workInfo.RequestStorageLevel(WorkCondition::Storage(storageRequest));
        return true;
    } else {
        WS_HILOGE("StorageRequest set is invalid, just ignore set.");
        set_business_error(E_STORAGE_REQUEST_ERR, FindErrMsg(E_STORAGE_REQUEST_ERR));
        return false;
    }
}

bool Common::GetRepeatInfo(
    ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo)
{
    if (!aniWork.repeatCycleTime.has_value()) {
        WS_HILOGD("RepeatCycleTime not set, just ignore other repeat set.");
        return false;
    }
    if (!aniWork.isRepeat.has_value() && !aniWork.repeatCount.has_value()) {
        WS_HILOGD("Not set isRepeat or repeatCount, ignore.");
        return false;
    }

    auto repeatCycleTime = static_cast<int32_t>(aniWork.repeatCycleTime.value());
    auto repeatCount = static_cast<int32_t>(aniWork.repeatCount.value());
    if (aniWork.isRepeat.value()) {
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
            set_business_error(E_REPEAT_COUNT_ERR, FindErrMsg(E_REPEAT_COUNT_ERR));
            return false;
        }
        workInfo.RequestRepeatCycle(repeatCycleTime, repeatCount);
        return true;
    }
}

bool Common::GetDeepIdleInfo(
    ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo)
{
    if (!aniWork.isDeepIdle.has_value()) {
        WS_HILOGD("isDeepIdle not set");
        return false;
    }
    auto isDeepIdle = aniWork.isDeepIdle.value();
    workInfo.RequestDeepIdle(isDeepIdle == TRUE_PARAM);
    return true;
}

bool Common::GetWorkInfo(
    ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo)
{
    workInfo.SetWorkId(static_cast<int32_t>(aniWork.workId));
    workInfo.SetElement(std::string(aniWork.bundleName), std::string(aniWork.abilityName));
    workInfo.RequestPersisted(aniWork.isPersisted.has_value() ? aniWork.isPersisted.value() : false);
    // Get extra parameters.
    if (!GetExtrasInfo(aniWork, workInfo)) {
        return false;
    }
    // Get condition info.
    bool hasConditions = false;
    if (GetNetWorkInfo(aniWork, workInfo)) {
        hasConditions = true;
    }
    if (GetChargeInfo(aniWork, workInfo)) {
        hasConditions = true;
    }
    if (GetBatteryInfo(aniWork, workInfo)) {
        hasConditions = true;
    }
    if (GetStorageInfo(aniWork, workInfo)) {
        hasConditions = true;
    }
    if (GetRepeatInfo(aniWork, workInfo)) {
        hasConditions = true;
    }
    if (GetDeepIdleInfo(aniWork, workInfo)) {
        hasConditions = true;
    }

    if (!hasConditions) {
        WS_HILOGE("Set none conditions, so fail to init WorkInfo.");
        set_business_error(E_CONDITION_EMPTY, FindErrMsg(E_CONDITION_EMPTY));
        return false;
    }
    return true;
}

bool Common::ConvertToAniParameters(std::map<std::string, sptr<AAFwk::IInterface>>& extrasMap,
    ::ohos::resourceschedule::workScheduler::WorkInfo &aniWork)
{
    int typeId = VALUE_TYPE_NULL;
    auto aniParams = aniWork.parameters.has_value() ? aniWork.parameters.value() : map<string, ParameType>{};
    for (const auto &iter : extrasMap) {
        typeId = AAFwk::WantParams::GetDataType(iter.second);
        switch (typeId) {
            case VALUE_TYPE_INT: {
                AAFwk::IInteger *intPtr = AAFwk::IInteger::Query(iter.second);
                if (intPtr == nullptr) {
                    return false;
                }
                aniParams.emplace(iter.first, ParameType::make_int_type(AAFwk::Integer::Unbox(intPtr)));
                break;
            }
            case VALUE_TYPE_DOUBLE: {
                AAFwk::IDouble *doublePtr = AAFwk::IDouble::Query(iter.second);
                if (doublePtr == nullptr) {
                    return false;
                }
                aniParams.emplace(iter.first, ParameType::make_double_type(AAFwk::Double::Unbox(doublePtr)));
                break;
            }
            case VALUE_TYPE_BOOLEAN: {
                AAFwk::IBoolean *boolPtr = AAFwk::IBoolean::Query(iter.second);
                if (boolPtr == nullptr) {
                    return false;
                }
                aniParams.emplace(iter.first, ParameType::make_bool_type(AAFwk::Boolean::Unbox(boolPtr)));
                break;
            }
            case VALUE_TYPE_STRING: {
                AAFwk::IString *strPtr = AAFwk::IString::Query(iter.second);
                if (strPtr == nullptr) {
                    return false;
                }
                aniParams.emplace(iter.first, ParameType::make_string_type(AAFwk::String::Unbox(strPtr)));
                break;
            }
            default: {
                WS_HILOGE("parameters type not supported.");
                return false;
            }
        }
    }
    aniWork.parameters = optional<map<string, ParameType>>(std::in_place, aniParams);
    return true;
}

void Common::ParseExtrasInfo(std::shared_ptr<OHOS::WorkScheduler::WorkInfo> workInfo,
    ::ohos::resourceschedule::workScheduler::WorkInfo &aniWork)
{
    std::shared_ptr<AAFwk::WantParams> extras = workInfo->GetExtras();
    if (extras.get() == nullptr) {
        WS_HILOGE("extras map is not initialized.");
        aniWork.parameters = {};
        return;
    }
    auto extrasMap = extras->GetParams();
    if (extrasMap.size() == 0) {
        aniWork.parameters = {};
        WS_HILOGE("extras parameters is 0.");
        return;
    }
    if (ConvertToAniParameters(extrasMap, aniWork)) {
        aniWork.parameters = {};
        WS_HILOGE("convert extra parameters failed.");
        return;
    }
}

void Common::ParseWorkInfo(std::shared_ptr<OHOS::WorkScheduler::WorkInfo> workInfo,
    ::ohos::resourceschedule::workScheduler::WorkInfo &aniWork)
{
    aniWork.isPersisted = optional<bool>(std::in_place, workInfo->IsPersisted());
    aniWork.networkType = optional<NetworkType>(std::in_place, NetworkType::key_t(workInfo->GetNetworkType()));
    if (workInfo->GetChargerType() >= WorkCondition::Charger::CHARGING_UNPLUGGED) {
        aniWork.isCharging = optional<bool>(std::in_place, false);
        aniWork.chargerType = optional<ChargingType>(std::in_place, ChargingType::key_t(UNSET_INT_PARAM));
    } else {
        aniWork.isCharging = optional<bool>(std::in_place, true);
        aniWork.chargerType = optional<ChargingType>(std::in_place, ChargingType::key_t(workInfo->GetChargerType()));
    }
    aniWork.batteryLevel = optional<double>(std::in_place, workInfo->GetBatteryLevel());
    aniWork.batteryStatus =
        optional<BatteryStatus>(std::in_place, BatteryStatus::key_t(workInfo->GetBatteryStatus()));
    aniWork.storageRequest =
        optional<StorageRequest>(std::in_place, StorageRequest::key_t(workInfo->GetStorageLevel()));
    aniWork.repeatCycleTime = optional<double>(std::in_place, workInfo->GetTimeInterval());
    aniWork.isRepeat = optional<bool>(std::in_place, workInfo->IsRepeat());
    aniWork.repeatCount = optional<double>(std::in_place, workInfo->GetCycleCount());
    aniWork.isDeepIdle = optional<bool>(std::in_place, UNSET_INT_PARAM);
    aniWork.idleWaitTime = optional<double>(std::in_place, UNSET_INT_PARAM);
    ParseExtrasInfo(workInfo, aniWork);
}
}  // namespace WorkScheduler
}  // namespace OHOS