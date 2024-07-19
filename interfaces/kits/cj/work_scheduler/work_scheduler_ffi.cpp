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

namespace OHOS {
namespace WorkScheduler {

extern "C" {
    const int32_t BATTERY_LEVEL_MIN = 0;
    const int32_t BATTERY_LEVEL_MAX = 100;

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
        auto paraCode = GetWorkInfo(work, workInfo);
        if (paraCode != SUCCESS_CODE) {
            LOGE("WorkScheduler: CJ_StopWork parse parameter failed %{public}d", paraCode);
            return paraCode;
        }
        ErrCode errCode = WorkSchedulerSrvClient::GetInstance().StopWork(workInfo);
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