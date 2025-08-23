/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "work_scheduler_ffi.h"
#include "workscheduler_srv_client.h"
#include "work_sched_errors.h"
#include "work_scheduler_log.h"
#include "want_params_wrapper.h"
#include "bool_wrapper.h"
#include "double_wrapper.h"
#include "int_wrapper.h"
#include "string_wrapper.h"

namespace OHOS {
namespace WorkScheduler {

int32_t InnerWrapWantParamsString(const sptr<AAFwk::IInterface> iIt, CParameters *p)
{
    AAFwk::IString *ao = AAFwk::IString::Query(iIt);
    if (ao == nullptr) {
        LOGE("ao is nullptr, no memory.");
        return ERR_NO_MEMORY;
    }
    
    std::string natValue = OHOS::AAFwk::String::Unbox(ao);
    p->value = MallocCString(natValue);
    p->size = static_cast<int64_t>(natValue.length()) + 1;
    return 0;
}

template <class T, class IT, class NativeT>
int32_t InnerWrapWantParamsT(const sptr<AAFwk::IInterface> iIt, CParameters *p)
{
    NativeT natValue = T::Unbox(IT::Query(iIt));
    NativeT *ptr = static_cast<NativeT *>(malloc(sizeof(NativeT)));
    if (ptr == nullptr) {
        LOGE("natValue ptr is nullptr, no memory.");
        return ERR_NO_MEMORY;
    }
    *ptr = natValue;
    p->value = static_cast<void*>(ptr);
    p->size = sizeof(NativeT);
    return 0;
}

int32_t GetWorkInfoV2(RetWorkInfoV2 cwork, WorkInfo& workInfo)
{
    auto ret = GetWorkInfo(cwork.v1, workInfo);
    if (ret != 0) {
        return ret;
    }
    return GetExtrasInfo(cwork, workInfo);
}

int32_t ParseWorkInfoV2(std::shared_ptr<WorkInfo> workInfo, RetWorkInfoV2& cwork)
{
    ParseWorkInfo(workInfo, cwork.v1);
    return ParseExtrasInfo(workInfo, cwork.parameters);
}

extern "C" {
    const int32_t BATTERY_LEVEL_MIN = 0;
    const int32_t BATTERY_LEVEL_MAX = 100;

    const int8_t INT_TYPE = 0;
    const int8_t F64_TYPE = 1;
    const int8_t STRING_TYPE = 2;
    const int8_t BOOL_TYPE = 3;
    // need to be same as WantParams
    enum {
        VALUE_TYPE_NULL = -1,
        VALUE_TYPE_BOOLEAN = 1,
        VALUE_TYPE_INT = 5,
        VALUE_TYPE_DOUBLE = 8,
        VALUE_TYPE_STRING = 9,
    };

    int32_t CJ_StartWork(RetWorkInfo work)
    {
        WorkInfo workInfo = WorkInfo();
        auto paraCode = GetWorkInfo(work, workInfo);
        if (paraCode != SUCCESS_CODE) {
            LOGE("WorkScheduler: CJ_StartWork parse parameter failed %{public}d", paraCode);
            return paraCode;
        }
        ErrCode errCode = WorkSchedulerSrvClient::GetInstance().StartWork(workInfo);
        return errCode;
    }

    int32_t CJ_StopWork(RetWorkInfo work, bool needCancel)
    {
        WorkInfo workInfo = WorkInfo();
        ErrCode errCode;
        auto paraCode = GetWorkInfo(work, workInfo);
        if (paraCode != SUCCESS_CODE) {
            LOGE("WorkScheduler: CJ_StopWork parse parameter failed %{public}d", paraCode);
            return paraCode;
        }
        if (needCancel) {
            errCode = WorkSchedulerSrvClient::GetInstance().StopAndCancelWork(workInfo);
        } else {
            errCode = WorkSchedulerSrvClient::GetInstance().StopWork(workInfo);
        }
        return errCode;
    }

    int32_t CJ_GetWorkStatus(int32_t workId, RetWorkInfo& result)
    {
        std::shared_ptr<WorkInfo> workInfo {nullptr};
        ErrCode errCode = WorkSchedulerSrvClient::GetInstance().GetWorkStatus(workId, workInfo);
        if (errCode != ERR_OK) {
            LOGE("WorkScheduler: CJ_GetWorkStatus failed %{public}d", errCode);
            return errCode;
        }
        ParseWorkInfo(workInfo, result);
        LOGI("WorkScheduler: CJ_GetWorkStatus success");
        return errCode;
    }

    RetArrRetWorkInfo CJ_ObtainAllWorks()
    {
        std::list<std::shared_ptr<WorkInfo>> workInfoList;
        ErrCode errCode = WorkSchedulerSrvClient::GetInstance().ObtainAllWorks(workInfoList);
        RetArrRetWorkInfo ret = { .code = errCode, .size = 0, .data = nullptr};
        if (errCode != ERR_OK) {
            LOGE("WorkScheduler: CJ_ObtainAllWorks failed ");
            return ret;
        }
        int64_t listSize = static_cast<int64_t>(workInfoList.size());
        if (listSize < 0 || listSize > UINT_MAX) {
            LOGE("Illegal listSize parameter");
            return ret;
        }
        auto data = static_cast<RetWorkInfo*>(malloc(sizeof(RetWorkInfo) * listSize));
        if (data == nullptr) {
            return ret;
        }
        ret.size = listSize;
        int index = 0;
        for (auto workInfo: workInfoList) {
            ParseWorkInfo(workInfo, data[index]);
            index++;
        }
        ret.data = data;
        return ret;
    }

    int32_t CJ_IsLastWorkTimeOut(int32_t workId, bool& result)
    {
        return WorkSchedulerSrvClient::GetInstance().IsLastWorkTimeout(workId, result);
    }

    int32_t CJ_StopAndClearWorks()
    {
        return WorkSchedulerSrvClient::GetInstance().StopAndClearWorks();
    }

    int32_t CJ_StartWorkV2(RetWorkInfoV2 work)
    {
        WorkInfo workInfo = WorkInfo();
        auto paraCode = GetWorkInfoV2(work, workInfo);
        if (paraCode != SUCCESS_CODE) {
            LOGE("WorkScheduler: CJ_StartWork parse parameter failed %{public}d", paraCode);
            return paraCode;
        }
        ErrCode errCode = WorkSchedulerSrvClient::GetInstance().StartWork(workInfo);
        return errCode;
    }

    int32_t CJ_StopWorkV2(RetWorkInfoV2 work, bool needCancel)
    {
        WorkInfo workInfo = WorkInfo();
        ErrCode errCode;
        auto paraCode = GetWorkInfoV2(work, workInfo);
        if (paraCode != SUCCESS_CODE) {
            LOGE("WorkScheduler: CJ_StopWork parse parameter failed %{public}d", paraCode);
            return paraCode;
        }
        if (needCancel) {
            errCode = WorkSchedulerSrvClient::GetInstance().StopAndCancelWork(workInfo);
        } else {
            errCode = WorkSchedulerSrvClient::GetInstance().StopWork(workInfo);
        }
        return errCode;
    }

    int32_t CJ_GetWorkStatusV2(int32_t workId, RetWorkInfoV2& result)
    {
        std::shared_ptr<WorkInfo> workInfo {nullptr};
        ErrCode errCode = WorkSchedulerSrvClient::GetInstance().GetWorkStatus(workId, workInfo);
        if (errCode != ERR_OK) {
            LOGE("WorkScheduler: CJ_GetWorkStatus failed %{public}d", errCode);
            return errCode;
        }
        errCode = ParseWorkInfoV2(workInfo, result);
        if (errCode != ERR_OK) {
            LOGE("WorkScheduler: CJ_GetWorkStatus ParseWorkInfoV2 failed %{public}d", errCode);
            return errCode;
        }
        LOGI("WorkScheduler: CJ_GetWorkStatus success");
        return errCode;
    }

    RetArrRetWorkInfoV2 CJ_ObtainAllWorksV2()
    {
        std::list<std::shared_ptr<WorkInfo>> workInfoList;
        ErrCode errCode = WorkSchedulerSrvClient::GetInstance().ObtainAllWorks(workInfoList);
        RetArrRetWorkInfoV2 ret = { .code = errCode, .size = 0, .data = nullptr};
        if (errCode != ERR_OK) {
            LOGE("WorkScheduler: CJ_ObtainAllWorks failed ");
            return ret;
        }
        int64_t listSize = static_cast<int64_t>(workInfoList.size());
        if (listSize < 0 || listSize > UINT_MAX) {
            LOGE("Illegal listSize parameter");
            return ret;
        }
        auto data = static_cast<RetWorkInfoV2*>(malloc(sizeof(RetWorkInfoV2) * listSize));
        if (data == nullptr) {
            return ret;
        }
        ret.size = listSize;
        int index = 0;
        for (auto workInfo: workInfoList) {
            errCode = ParseWorkInfoV2(workInfo, data[index]);
            if (errCode != ERR_OK) {
                LOGE("CJ_ObtainAllWorksV2: ParseWorkInfoV2 failed %{public}d", errCode);
                ret.code = errCode;
                return ret;
            }
            index++;
        }
        ret.data = data;
        return ret;
    }

    // extra is not set
    int32_t GetWorkInfo(RetWorkInfo cwork, WorkInfo& workInfo)
    {
        workInfo.SetWorkId(cwork.workId);
        workInfo.SetElement(std::string(cwork.bundleName), std::string(cwork.abilityName));
        workInfo.RequestPersisted(cwork.isPersisted);
        bool hasConditions = false;
        int32_t ret = GetNetWorkInfo(cwork, workInfo, hasConditions);
        if (ret != 0) {
            return ret;
        }
        ret = GetChargeInfo(cwork, workInfo, hasConditions);
        if (ret != 0) {
            return ret;
        }
        ret = GetBatteryInfo(cwork, workInfo, hasConditions);
        if (ret != 0) {
            return ret;
        }
        ret = GetStorageInfo(cwork, workInfo, hasConditions);
        if (ret != 0) {
            return ret;
        }
        ret = GetRepeatInfo(cwork, workInfo, hasConditions);
        if (ret != 0) {
            return ret;
        }

        if (!hasConditions) {
            LOGE("Set none conditions, so fail to init WorkInfo.");
            return E_CONDITION_EMPTY;
        }
        return 0;
    }

    int32_t GetExtrasInfo(RetWorkInfoV2 cwork, OHOS::WorkScheduler::WorkInfo& workInfo)
    {
        int32_t code = 0;
        CArrParameters cArrP = cwork.parameters;
        AAFwk::WantParams wants;
        if (cArrP.size == 0) {
            return code;
        }
        for (auto i = 0; i < cArrP.size; ++i) {
            std::string key = std::string(cArrP.head[i].key);
            switch (cArrP.head[i].valueType) {
                case INT_TYPE: {
                    int32_t *intVal = static_cast<int32_t *>(cArrP.head[i].value);
                    wants.SetParam(key, AAFwk::Integer::Box(*intVal));
                    break;
                }
                case F64_TYPE: {
                    double *doubleVal = static_cast<double *>(cArrP.head[i].value);
                    wants.SetParam(key, AAFwk::Double::Box(*doubleVal));
                    break;
                }
                case STRING_TYPE: {
                    std::string strVal(static_cast<char *>(cArrP.head[i].value));
                    wants.SetParam(key, AAFwk::String::Box(strVal));
                    break;
                }
                case BOOL_TYPE: {
                    bool *boolVal = static_cast<bool *>(cArrP.head[i].value);
                    wants.SetParam(key, AAFwk::Boolean::Box(*boolVal));
                    break;
                }
                default: {
                    LOGE("parameters type error.");
                    code = E_PARAMETERS_TYPE_ERR;
                    return code;
                }
            }
        }
        workInfo.RequestExtras(wants);
        return code;
    }
    
    int32_t GetNetWorkInfo(RetWorkInfo cwork, WorkInfo& workInfo, bool& hasCondition)
    {
        int32_t code = 0;
        if (cwork.netWorkType == UNSET_INT_PARAM) {
            LOGI("Unset networkType.");
        } else if (cwork.netWorkType >= WorkCondition::Network::NETWORK_TYPE_ANY &&
            cwork.netWorkType <= WorkCondition::Network::NETWORK_TYPE_ETHERNET) {
            workInfo.RequestNetworkType(WorkCondition::Network(cwork.netWorkType));
            hasCondition = true;
        } else {
            LOGE("NetworkType set is invalid, just ignore set.");
            code = E_NETWORK_TYPE_ERR;
        }
        return code;
    }

    int32_t GetChargeInfo(RetWorkInfo cwork, WorkInfo& workInfo, bool& hasCondition)
    {
        if (!cwork.isCharging) {
            workInfo.RequestChargerType(false, WorkCondition::Charger::CHARGING_UNPLUGGED);
        } else {
            if (cwork.chargerType == UNSET_INT_PARAM) {
                workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_ANY);
            } else if (cwork.chargerType >=  WorkCondition::Charger::CHARGING_PLUGGED_ANY &&
                cwork.chargerType <= WorkCondition::Charger::CHARGING_PLUGGED_WIRELESS) {
                workInfo.RequestChargerType(true, WorkCondition::Charger(cwork.chargerType));
                hasCondition = true;
            } else {
                workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_ANY);
                LOGE("ChargeType info is invalid, just ignore set.");
                return E_CHARGER_TYPE_ERR;
            }
        }
        return 0;
    }

    int32_t GetBatteryInfo(RetWorkInfo cwork, WorkInfo& workInfo, bool& hasCondition)
    {
        if (cwork.batteryLevel == UNSET_INT_PARAM) {
            LOGI("Unset batteryLevel.");
        } else if (cwork.batteryLevel >= BATTERY_LEVEL_MIN && cwork.batteryLevel <= BATTERY_LEVEL_MAX) {
            workInfo.RequestBatteryLevel(cwork.batteryLevel);
        } else {
            LOGE("BatteryLevel set is invalid, just ignore set.");
            return E_BATTERY_LEVEL_ERR;
        }

        if (cwork.batteryStatus == UNSET_INT_PARAM) {
            LOGI("Unset batteryStatus.");
        } else if (cwork.batteryStatus >= WorkCondition::BatteryStatus::BATTERY_STATUS_LOW &&
            cwork.batteryStatus <= WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY) {
            workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus(cwork.batteryStatus));
            hasCondition = true;
        } else {
            LOGE("BatteryStatus set is invalid, just ignore set.");
            return E_BATTERY_STATUS_ERR;
        }
        return 0;
    }

    int32_t GetStorageInfo(RetWorkInfo cwork, WorkInfo& workInfo, bool& hasCondition)
    {
        if (cwork.storageRequest == UNSET_INT_PARAM) {
            LOGI("Unset StorageRequest.");
        } else if (cwork.storageRequest >= WorkCondition::Storage::STORAGE_LEVEL_LOW &&
                cwork.storageRequest <= WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY) {
            workInfo.RequestStorageLevel(WorkCondition::Storage(cwork.storageRequest));
            hasCondition = true;
        } else {
            LOGE("StorageRequest set is invalid, just ignore set.");
            return E_STORAGE_REQUEST_ERR;
        }
        return 0;
    }

    int32_t GetRepeatInfo(RetWorkInfo cwork, WorkInfo& workInfo, bool& hasCondition)
    {
        if (cwork.repeatCycleTime == UNSET_INT_PARAM) {
            LOGI("RepeatCycleTime not set, just ignore other repeat set.");
            return 0;
        }

        if (!cwork.isRepeat && cwork.repeatCount == UNSET_INT_PARAM) {
            LOGI("Not set isRepeat or repeatCount, ignore.");
            return 0;
        }
        if (cwork.isRepeat) {
            if (cwork.repeatCount > 0) {
                LOGI("RepeatCount has been set , ignore isRepeat.");
                workInfo.RequestRepeatCycle(cwork.repeatCycleTime, cwork.repeatCount);
            } else {
                workInfo.RequestRepeatCycle(cwork.repeatCycleTime);
            }
            hasCondition = true;
            return 0;
        } else {
            if (cwork.repeatCount < 0) {
                LOGE("RepeatCount is invalid, ignore.");
                return E_REPEAT_COUNT_ERR;
            }
            workInfo.RequestRepeatCycle(cwork.repeatCycleTime, cwork.repeatCount);
            hasCondition = true;
            return 0;
        }
    }

    void ParseWorkInfo(std::shared_ptr<WorkInfo> workInfo, RetWorkInfo& cwork)
    {
        cwork.workId = workInfo->GetWorkId();
        cwork.bundleName = MallocCString(workInfo->GetBundleName());
        cwork.abilityName = MallocCString(workInfo->GetAbilityName());
        cwork.isPersisted = workInfo->IsPersisted();
        cwork.netWorkType = workInfo->GetNetworkType();
        auto chargerType = workInfo->GetChargerType();
        if (chargerType >= WorkCondition::Charger::CHARGING_UNPLUGGED) {
            cwork.isCharging = false;
            cwork.chargerType = -1;
        } else {
            cwork.isCharging = true;
            cwork.chargerType = chargerType;
        }
        cwork.batteryLevel = workInfo->GetBatteryLevel();
        cwork.batteryStatus = workInfo->GetBatteryStatus();
        cwork.storageRequest = workInfo->GetStorageLevel();
        cwork.isRepeat = workInfo->IsRepeat();
        cwork.repeatCycleTime = static_cast<int32_t>(workInfo->GetTimeInterval());
        cwork.repeatCount = workInfo->GetCycleCount();
        cwork.isDeepIdle = -1;
        cwork.idleWaitTime = -1;
    }

    void ClearParametersPtr(CParameters **ptr, int count, bool isKey) 
    {
        CParameters *p = *ptr;
        for (int i= 0; i < count; i++) {
            free(p[i].key);
            free(p[i].value);
            p[i].key = nullptr;
            p[i].value = nullptr;
        }
        if (!isKey) {
            free(p[count].key);
            p[count].key = nullptr;
        }
        free(*ptr);
        *ptr = nullptr;
    }

    int32_t ConvertToCArrParameters(std::map<std::string, sptr<AAFwk::IInterface>>& extrasMap, CArrParameters& arrParam)
    {
        ErrCode errCode;
        int typeId = VALUE_TYPE_NULL;
        int i = 0;
        int32_t mallocSize = static_cast<int32_t>(sizeof(CParameters) * arrParam.size);
        arrParam.head = static_cast<CParameters *>(malloc(mallocSize));
        if (!arrParam.head) {
            errCode = ERR_NO_MEMORY;
            arrParam.size = 0;
            return errCode;
        }
        for (auto it : extrasMap) {
            typeId = AAFwk::WantParams::GetDataType(it.second);
            arrParam.head[i].key = MallocCString(it.first);
            switch (typeId) {
                case VALUE_TYPE_INT: {
                    arrParam.head[i].valueType = INT_TYPE;
                    errCode = InnerWrapWantParamsT<AAFwk::Integer, AAFwk::IInteger, int>(it.second, &arrParam.head[i]);
                    break;
                }
                case VALUE_TYPE_DOUBLE: {
                    arrParam.head[i].valueType = F64_TYPE;
                    errCode = InnerWrapWantParamsT<AAFwk::Double, AAFwk::IDouble, double>(it.second, &arrParam.head[i]);
                    break;
                }
                case VALUE_TYPE_BOOLEAN: {
                    arrParam.head[i].valueType = BOOL_TYPE;
                    errCode = InnerWrapWantParamsT<AAFwk::Boolean, AAFwk::IBoolean, bool>(it.second, &arrParam.head[i]);
                    break;
                }
                case VALUE_TYPE_STRING: {
                    arrParam.head[i].valueType = STRING_TYPE;
                    errCode = InnerWrapWantParamsString(it.second, &arrParam.head[i]);
                    break;
                }
                default: {
                    LOGE("parameters type not supported.");
                    break;
                }
            }

            if (errCode == ERR_NO_MEMORY) {
                ClearParametersPtr(&arrParam.head, i, true);
                return errCode;
            }
            ++i;
        }
        return errCode;
    }

    int32_t ParseExtrasInfo(std::shared_ptr<WorkInfo> workInfo, CArrParameters &arrParam)
    {
        // init arrParam
        arrParam.size = 0;
        arrParam.head = nullptr;
        std::shared_ptr<AAFwk::WantParams> extras = workInfo->GetExtras();
        if (extras.get() == nullptr) {
            LOGI("extras map is not initialized.");
            return ERR_NO_MEMORY;
        }
        auto extrasMap = extras->GetParams();
        arrParam.size = static_cast<int64_t>(extrasMap.size());
        if (extrasMap.size() == 0) {
            LOGI("extras parameters is 0.");
            return ERR_NO_MEMORY;
        }
        return ConvertToCArrParameters(extrasMap, arrParam);
    }

    char* MallocCString(const std::string& origin)
    {
        if (origin.empty()) {
            return nullptr;
        }
        auto len = origin.length() + 1;
        char* res = static_cast<char*>(malloc(sizeof(char) * len));
        if (res == nullptr) {
            return nullptr;
        }
        return std::char_traits<char>::copy(res, origin.c_str(), len);
    }
}
} // WorkScheduler
} // OHOS