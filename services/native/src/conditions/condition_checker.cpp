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
#include "conditions/condition_checker.h"
#ifdef COMMUNICATION_NETMANAGER_BASE_ENABLE
#include "net_conn_client.h"
#endif
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
#include "battery_srv_client.h"
#include "battery_info.h"
#endif
#include "work_sched_hilog.h"

using namespace OHOS::NetManagerStandard;

namespace OHOS {
namespace WorkScheduler {
ConditionChecker::ConditionChecker(const std::shared_ptr<WorkQueueManager> workQueueManager)
{
    workQueueManager_ = workQueueManager;
}

void ConditionChecker::CheckAllStatus()
{
    CheckNetworkStatus();
    CheckChargerStatus();
    CheckBatteryStatus();
}


void ConditionChecker::CheckNetworkStatus()
{
#ifdef COMMUNICATION_NETMANAGER_BASE_ENABLE
    WS_HILOGD("enter");
    NetHandle netHandle;
    int ret = NetConnClient::GetInstance().GetDefaultNet(netHandle);
    if (ret != NETMANAGER_SUCCESS) {
        WS_HILOGE("GetDefaultNet failed %{public}d", ret);
        return;
    }
    NetAllCapabilities netAllCap;
    ret = NetConnClient::GetInstance().GetNetCapabilities(netHandle, netAllCap);
    if (ret != NETMANAGER_SUCCESS) {
        WS_HILOGE("GetNetCapbilities failed, ret = %{public}d", ret);
        return;
    }
    if (netAllCap.netCaps_.count(NetCap::NET_CAPABILITY_INTERNET)) {
        if (netAllCap.bearerTypes_.count(BEARER_ETHERNET)) {
            WS_HILOGI("BEARER_ETHERNET");
            workQueueManager_->OnConditionChanged(WorkCondition::Type::NETWORK,
                std::make_shared<DetectorValue>(WorkCondition::NETWORK_TYPE_ETHERNET, 0, 0, std::string()));
        } else if (netAllCap.bearerTypes_.count(BEARER_WIFI)) {
            WS_HILOGI("BEARER_WIFI");
            workQueueManager_->OnConditionChanged(WorkCondition::Type::NETWORK,
                std::make_shared<DetectorValue>(WorkCondition::NETWORK_TYPE_WIFI, 0, 0, std::string()));
        } else if (netAllCap.bearerTypes_.count(BEARER_CELLULAR)) {
            WS_HILOGI("BEARER_CELLULAR");
            workQueueManager_->OnConditionChanged(WorkCondition::Type::NETWORK,
                std::make_shared<DetectorValue>(WorkCondition::NETWORK_TYPE_MOBILE, 0, 0, std::string()));
        }
    } else {
        WS_HILOGI("no network");
    }
#endif
}

void ConditionChecker::CheckChargerStatus()
{
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    WS_HILOGD("enter");
    auto type = PowerMgr::BatterySrvClient::GetInstance().GetPluggedType();
    switch (type) {
        case PowerMgr::BatteryPluggedType::PLUGGED_TYPE_AC:
            WS_HILOGI("CHARGER_PLUGGED_AC");
            workQueueManager_->OnConditionChanged(WorkCondition::Type::CHARGER,
                std::make_shared<DetectorValue>(WorkCondition::CHARGING_PLUGGED_AC,
                0, true, std::string()));
            break;
        case PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB:
            WS_HILOGI("CHARGER_PLUGGED_USB");
            workQueueManager_->OnConditionChanged(WorkCondition::Type::CHARGER,
                std::make_shared<DetectorValue>(WorkCondition::CHARGING_PLUGGED_USB,
                0, true, std::string()));
            break;
        case PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS:
            WS_HILOGI("CHARGER_WIRELESS");
            workQueueManager_->OnConditionChanged(WorkCondition::Type::CHARGER,
                std::make_shared<DetectorValue>(WorkCondition::CHARGING_PLUGGED_WIRELESS,
                0, true, std::string()));
            break;

        case PowerMgr::BatteryPluggedType::PLUGGED_TYPE_NONE:
        case PowerMgr::BatteryPluggedType::PLUGGED_TYPE_BUTT:
            WS_HILOGI("CHARGER_PLUGGED_UNPLUGGED");
            workQueueManager_->OnConditionChanged(WorkCondition::Type::CHARGER,
                std::make_shared<DetectorValue>(WorkCondition::CHARGING_UNPLUGGED, 0, false, std::string()));
            break;
        default:
            break;
    }
#endif
}
void CheckBatteryStatus()
{
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    WS_HILOGD("enter");
    int32_t defaultCapacity = -1;
    int32_t batteryCapacityLow = 20;
    auto capacity = PowerMgr::BatterySrvClient::GetInstance().GetCapacity();
    WS_HILOGD("capacity = %{public}d", capacity);
    if (capacity == defaultCapacity) {
        return;
    } else if (capacity < batteryCapacityLow) {
        WS_HILOGI("BATTERY_STATUS_LOW");
        workQueueManager_->OnConditionChanged(WorkCondition::Type::BATTERY_STATUS,
            std::make_shared<DetectorValue>(WorkCondition::BATTERY_STATUS_LOW, 0, 0, std::string()));
    } else {
        WS_HILOGI("BATTERY_STATUS_OKAY");
        workQueueManager_->OnConditionChanged(WorkCondition::Type::BATTERY_STATUS,
            std::make_shared<DetectorValue>(WorkCondition::BATTERY_STATUS_OKAY, 0, 0, std::string()));
    }
#endif
}
} // namespace WorkScheduler
} // namespace OHOS