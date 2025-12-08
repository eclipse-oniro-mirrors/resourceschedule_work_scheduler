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

#include "workschedulerstartwork_fuzzer.h"

#include "iwork_sched_service.h"
#include "work_scheduler_service.h"
#include "work_sched_common.h"
#include "work_condition.h"
#include "work_sched_errors.h"
#include "scheduler_bg_task_subscriber.h"
#include "work_bundle_group_change_callback.h"
#include "work_conn_manager.h"
#include "conditions/battery_status_listener.h"
#include "conditions/charger_listener.h"
#include "conditions/battery_level_listener.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "battery_info.h"
#include "conditions/screen_listener.h"
#include "work_queue_manager.h"
#include "conditions/network_listener.h"
#include "conditions/storage_listener.h"
#include "net_supplier_info.h"
#include "work_standby_state_change_callback.h"
#include "work_policy_manager.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
    const std::u16string WORK_SCHEDULER_STUB_TOKEN = u"ohos.workscheduler.iworkschedservice";
    static std::shared_ptr<WorkSchedulerService> workSchedulerService_;
    const uint32_t allowType = 8;
    const int32_t DEFAULT_VALUE = -1;
    const int32_t BEARER_CELLULAR = 0;
    const int32_t BEARER_WIFI = 1;
    const int32_t BEARER_BLUETOOTH = 2;
    const int32_t BEARER_ETHERNET = 3;
    const int32_t BEARER_WIFI_AWARE = 5;

    bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
    {
        return true;
    }

    WorkInfo CreateWorkInfo()
    {
        int32_t uid = 20008;
        int32_t workId = 10000;
        uint32_t timeInterval = 20 * 60 * 1000;
        int32_t battery = 20;
        WorkInfo workInfo = WorkInfo();
        workInfo.SetWorkId(workId);
        workInfo.SetElement("bundleName", "MainWorkSchedulerAbility");
        workInfo.RequestPersisted(true);
        workInfo.RequestRepeatCycle(timeInterval);
        workInfo.RefreshUid(uid);
        workInfo.RequestDeepIdle(true);
        workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
        workInfo.RequestBatteryLevel(battery);
        workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
        workInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW);
        workInfo.RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_WIFI);
        return workInfo;
    }

    void OnStorageChange()
    {
        std::vector<std::string> argsInStr;
        std::string result;

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-d");
        argsInStr.push_back("storage");
        argsInStr.push_back("invalid");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-d");
        argsInStr.push_back("storage");
        argsInStr.push_back("ok");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-d");
        argsInStr.push_back("storage");
        argsInStr.push_back("low");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    }

    void OnNetworkChange()
    {
        std::vector<std::string> argsInStr;
        std::string result;

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-d");
        argsInStr.push_back("network");
        argsInStr.push_back("invalid");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-d");
        argsInStr.push_back("network");
        argsInStr.push_back("disconnect");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-d");
        argsInStr.push_back("network");
        argsInStr.push_back("wifi");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    }

    void OnChargingChange()
    {
        std::vector<std::string> argsInStr;
        std::string result;

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-d");
        argsInStr.push_back("charging");
        argsInStr.push_back("none");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-d");
        argsInStr.push_back("charging");
        argsInStr.push_back("usb");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-d");
        argsInStr.push_back("charging");
        argsInStr.push_back("wireless");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-d");
        argsInStr.push_back("charging");
        argsInStr.push_back("ac");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    }

    void OnBatteryStatusChange()
    {
        std::vector<std::string> argsInStr;
        std::string result;

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-d");
        argsInStr.push_back("batteryStatus");
        argsInStr.push_back("ok");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-d");
        argsInStr.push_back("batteryStatus");
        argsInStr.push_back("low");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    }

    void OnTimerChange()
    {
        std::vector<std::string> argsInStr;
        std::string result;

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-repeat_time_min");
        argsInStr.push_back("10000");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-watchdog_time");
        argsInStr.push_back("5000");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
        
        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-min_interval");
        argsInStr.push_back("20000");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    }

    void OnBatteryLevelListener()
    {
        int32_t capacity = 20;
        BatteryLevelListener batteryLevelListener(workSchedulerService_->workQueueManager_, workSchedulerService_);
        batteryLevelListener.Start();

        EventFwk::CommonEventData data;
        batteryLevelListener.commonEventSubscriber->OnReceiveEvent(data);

        EventFwk::Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
        data.SetWant(want);
        batteryLevelListener.commonEventSubscriber->OnReceiveEvent(data);

        want.SetParam(PowerMgr::BatteryInfo::COMMON_EVENT_KEY_CAPACITY, capacity);
        data.SetWant(want);
        batteryLevelListener.commonEventSubscriber->OnReceiveEvent(data);
        batteryLevelListener.Stop();
    }

    void OnBatteryStatusListener()
    {
        BatteryStatusListener batteryStatusListener(workSchedulerService_->workQueueManager_);

        batteryStatusListener.Start();

        EventFwk::CommonEventData data;
        batteryStatusListener.commonEventSubscriber->OnReceiveEvent(data);

        EventFwk::Want want;

        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY);
        data.SetWant(want);
        batteryStatusListener.commonEventSubscriber->OnReceiveEvent(data);

        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW);
        data.SetWant(want);
        batteryStatusListener.commonEventSubscriber->OnReceiveEvent(data);
        batteryStatusListener.Stop();
    }

    void OnChargerListener()
    {
        ChargerListener chargerListener(workSchedulerService_->workQueueManager_);

        chargerListener.Start();

        EventFwk::CommonEventData data;
        chargerListener.commonEventSubscriber->OnReceiveEvent(data);

        EventFwk::Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        data.SetWant(want);
        data.SetCode(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_NONE));
        chargerListener.commonEventSubscriber->OnReceiveEvent(data);

        data.SetCode(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB));
        chargerListener.commonEventSubscriber->OnReceiveEvent(data);

        data.SetCode(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS));
        chargerListener.commonEventSubscriber->OnReceiveEvent(data);

        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
        data.SetWant(want);
        data.SetCode(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_AC));
        chargerListener.commonEventSubscriber->OnReceiveEvent(data);
        chargerListener.Stop();
    }

    void OnScreenListener()
    {
        ScreenListener screenListener(workSchedulerService_->workQueueManager_, workSchedulerService_);
        
        screenListener.Start();
        EventFwk::CommonEventData data;
        screenListener.commonEventSubscriber->OnReceiveEvent(data);

        EventFwk::Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
        data.SetWant(want);
        screenListener.commonEventSubscriber->OnReceiveEvent(data);

        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
        data.SetWant(want);
        screenListener.commonEventSubscriber->OnReceiveEvent(data);
        screenListener.Stop();
    }

    void OnStorageListener()
    {
        StorageListener storageListener(workSchedulerService_->workQueueManager_);
        storageListener.Start();

        EventFwk::CommonEventData data;
        EventFwk::Want want;
        storageListener.commonEventSubscriber->OnReceiveEvent(data);

        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_LOW);
        data.SetWant(want);
        storageListener.commonEventSubscriber->OnReceiveEvent(data);

        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_OK);
        data.SetWant(want);
        storageListener.commonEventSubscriber->OnReceiveEvent(data);
        storageListener.Stop();
    }

    void OnNetworkListener()
    {
        NetworkListener networkListener(workSchedulerService_->workQueueManager_);
        networkListener.Start();
        EventFwk::CommonEventData data;
        EventFwk::Want want;
        data.SetCode(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED);
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
        data.SetWant(want);
        EventFwk::CommonEventManager::PublishCommonEvent(data);
        networkListener.commonEventSubscriber->OnReceiveEvent(data);

        want.SetParam("NetType", BEARER_CELLULAR);
        data.SetWant(want);
        EventFwk::CommonEventManager::PublishCommonEvent(data);
        networkListener.commonEventSubscriber->OnReceiveEvent(data);

        want.SetParam("NetType", BEARER_WIFI);
        data.SetWant(want);
        EventFwk::CommonEventManager::PublishCommonEvent(data);
        networkListener.commonEventSubscriber->OnReceiveEvent(data);

        want.SetParam("NetType", BEARER_BLUETOOTH);
        data.SetWant(want);
        EventFwk::CommonEventManager::PublishCommonEvent(data);
        networkListener.commonEventSubscriber->OnReceiveEvent(data);

        want.SetParam("NetType", BEARER_ETHERNET);
        data.SetWant(want);
        EventFwk::CommonEventManager::PublishCommonEvent(data);
        networkListener.commonEventSubscriber->OnReceiveEvent(data);

        want.SetParam("NetType", BEARER_WIFI_AWARE);
        data.SetWant(want);
        EventFwk::CommonEventManager::PublishCommonEvent(data);
        networkListener.commonEventSubscriber->OnReceiveEvent(data);

        want.SetParam("NetType", DEFAULT_VALUE);
        data.SetWant(want);
        EventFwk::CommonEventManager::PublishCommonEvent(data);
        networkListener.commonEventSubscriber->OnReceiveEvent(data);
        networkListener.Stop();
    }

    void OnReceiveEvent()
    {
        OnBatteryLevelListener();
        OnBatteryStatusListener();
        OnChargerListener();
        OnScreenListener();
        OnStorageListener();
        OnNetworkListener();
    }

    void TriggerWork(WorkInfo &workInfo)
    {
        OnStorageChange();
        OnNetworkChange();
        OnChargingChange();
        OnBatteryStatusChange();
        OnTimerChange();

        OnReceiveEvent();

        std::vector<std::string> argsInStr;
        std::string result;
        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-k");
        argsInStr.push_back("settings.power.suspend_sources");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-t");
        argsInStr.push_back(workInfo.GetBundleName());
        argsInStr.push_back(workInfo.GetAbilityName());
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);

        workSchedulerService_->PauseRunningWorks(workInfo.GetUid());
        workSchedulerService_->ResumePausedWorks(workInfo.GetUid());

        std::shared_ptr<WorkConnManager> workConnManager = std::make_shared<WorkConnManager>();
        std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workInfo, workInfo.GetUid());
        workConnManager->WriteStartWorkEvent(workStatus);
    }

    void OnProcEfficiencyResourcesChange()
    {
        SchedulerBgTaskSubscriber subscriber;
        subscriber.OnProcEfficiencyResourcesApply(nullptr);
        subscriber.OnProcEfficiencyResourcesReset(nullptr);
        subscriber.OnAppEfficiencyResourcesApply(nullptr);
        subscriber.OnAppEfficiencyResourcesReset(nullptr);

        auto resourceInfo = std::make_shared<BackgroundTaskMgr::ResourceCallbackInfo>(0, 0, 0xFFFF, "name");
        subscriber.OnProcEfficiencyResourcesApply(resourceInfo);
        subscriber.OnProcEfficiencyResourcesReset(resourceInfo);
        subscriber.OnAppEfficiencyResourcesApply(resourceInfo);
        subscriber.OnAppEfficiencyResourcesReset(resourceInfo);
    }

    void OnWorkStandbyStateChange()
    {
        workSchedulerService_->standbyStateObserver_->OnDeviceIdleMode(true, false);
        workSchedulerService_->standbyStateObserver_->OnDeviceIdleMode(true, true);
        workSchedulerService_->standbyStateObserver_->OnDeviceIdleMode(false, true);
        workSchedulerService_->standbyStateObserver_->OnAllowListChanged(0, "bundlename", allowType, true);
    }

    void OnWorkBundleGroupChange()
    {
        int32_t value = 2;
        OHOS::DeviceUsageStats::AppGroupCallbackInfo appGroupCallbackInfo1(0, 1, value, 0, "bundlename");
        workSchedulerService_->groupObserver_->OnAppGroupChanged(appGroupCallbackInfo1);
        OHOS::DeviceUsageStats::AppGroupCallbackInfo appGroupCallbackInfo2(0, value, 1, 0, "bundlename");
        workSchedulerService_->groupObserver_->OnAppGroupChanged(appGroupCallbackInfo2);
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        MessageParcel dataMessageParcel;
        dataMessageParcel.WriteInterfaceToken(WORK_SCHEDULER_STUB_TOKEN);
        dataMessageParcel.WriteBuffer(data, size);
        dataMessageParcel.RewindRead(0);
        MessageParcel reply;
        MessageOption option;
        workSchedulerService_ = DelayedSingleton<WorkSchedulerService>::GetInstance();
        uint32_t code = static_cast<int32_t>(IWorkSchedServiceIpcCode::COMMAND_START_WORK);
        
        workSchedulerService_->OnStart();
        workSchedulerService_->InitBgTaskSubscriber();
        if (!workSchedulerService_->ready_) {
            workSchedulerService_->ready_ = true;
        }
        if (workSchedulerService_->workQueueManager_ == nullptr) {
            return false;
        }
        if (workSchedulerService_->standbyStateObserver_ == nullptr ||
            workSchedulerService_->groupObserver_ == nullptr) {
            return false;
        }
        if (workSchedulerService_->workPolicyManager_ == nullptr ||
            workSchedulerService_->workPolicyManager_->workConnManager_ == nullptr) {
            return false;
        }
        OnProcEfficiencyResourcesChange();
        OnWorkStandbyStateChange();
        OnWorkBundleGroupChange();
        if (workSchedulerService_->checkBundle_) {
            workSchedulerService_->checkBundle_ = false;
        }

        WorkInfo workInfo = CreateWorkInfo();
        if (!dataMessageParcel.WriteParcelable(&workInfo)) {
            return false;
        }
        workSchedulerService_->OnRemoteRequest(code, dataMessageParcel, reply, option);
        workSchedulerService_->TriggerWorkIfConditionReady();
        TriggerWork(workInfo);
        workSchedulerService_->StopAndClearWorks();
        workSchedulerService_->OnStop();
        return true;
    }
} // WorkScheduler
} // OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}