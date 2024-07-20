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

#include "iwork_sched_service_ipc_interface_code.h"
#include "work_scheduler_service.h"
#include "work_sched_common.h"
#include "work_condition.h"
#include "work_sched_errors.h"
#include "scheduler_bg_task_subscriber.h"
#include "work_bundle_group_change_callback.h"
#include "work_conn_manager.h"
#include "conditions/battery_status_listener.h"
#include "conditions/charger_listener.h"
#include "battery_level_listener.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "battery_info.h"
#include "condition/screen_listener.h"
#include "work_queue_manager.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
    const std::u16string WORK_SCHEDULER_STUB_TOKEN = u"ohos.workscheduler.iworkschedservice";
    static std::shared_ptr<WorkSchedulerService> workSchedulerService_;

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
        workInfo.SetElement("com.huawei.hnos.hiviewx", "MainWorkSchedulerAbility");
        workInfo.RequestPersisted(true);
        workInfo.RequestRepeatCycle(timeInterval);
        workInfo.RefreshUid(uid);
        workInfo.RequestDeepIdle(true);
        workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
        workInfo.RequestBatteryLevel(battery);
        workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
        workInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW);
        workInfo.RequestNetworkType(WorkCondition::NETWORK::NETWORK_TYPE_WIFI);
        return workInfo;
    }

    void OnStorageChange()
    {
        std::vector<std:string> argsInStr;
        std::string result;

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-d");
        argsInStr.push_bach("stroage");
        argsInStr.push_bach("invalid");
        workSchedulerService_->DumpProcess(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-d");
        argsInStr.push_bach("stroage");
        argsInStr.push_bach("ok");
        workSchedulerService_->DumpProcess(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-d");
        argsInStr.push_bach("stroage");
        argsInStr.push_bach("low");
        workSchedulerService_->DumpProcess(argsInStr, result);
    }

    void OnNetworkChange()
    {
        std::vector<std:string> argsInStr;
        std::string result;

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-d");
        argsInStr.push_bach("network");
        argsInStr.push_bach("invalid");
        workSchedulerService_->DumpProcess(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-d");
        argsInStr.push_bach("network");
        argsInStr.push_bach("disconnect");
        workSchedulerService_->DumpProcess(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-d");
        argsInStr.push_bach("network");
        argsInStr.push_bach("wifi");
        workSchedulerService_->DumpProcess(argsInStr, result);
    }

    void OnChargingChange()
    {
        std::vector<std:string> argsInStr;
        std::string result;

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-d");
        argsInStr.push_bach("charging");
        argsInStr.push_bach("none");
        workSchedulerService_->DumpProcess(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-d");
        argsInStr.push_bach("charging");
        argsInStr.push_bach("usb");
        workSchedulerService_->DumpProcess(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-d");
        argsInStr.push_bach("charging");
        argsInStr.push_bach("wrieless");
        workSchedulerService_->DumpProcess(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-d");
        argsInStr.push_bach("charging");
        argsInStr.push_bach("ac");
        workSchedulerService_->DumpProcess(argsInStr, result);
    }

    void OnBatteryStatusChange()
    {
        std::vector<std:string> argsInStr;
        std::string result;

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-d");
        argsInStr.push_bach("batteryStatus");
        argsInStr.push_bach("ok");
        workSchedulerService_->DumpProcess(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-d");
        argsInStr.push_bach("batteryStatus");
        argsInStr.push_bach("low");
        workSchedulerService_->DumpProcess(argsInStr, result);
    }

    void OnTimerChange()
    {
        std::vector<std:string> argsInStr;
        std::string result;

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-repeat_time_min");
        argsInStr.push_bach("10000");
        workSchedulerService_->DumpProcess(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-watchdog_time");
        argsInStr.push_bach("50000");
        workSchedulerService_->DumpProcess(argsInStr, result);
        
        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-min_interval");
        argsInStr.push_bach("20000");
        workSchedulerService_->DumpProcess(argsInStr, result);
    }

    void OnBatteryLevelListener()
    {
        BatteryLevelListener batteryLevelListener(workSchedulerService_->workQueueManager_, workSchedulerService_);
        batteryLevelListener.Start();

        EventFwk::CommonEventData data;
        batteryLevelListener.commonEventSubscriber->OnReceiveEvent(data);

        EventFwk::Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
        data.SetWant(want);
        batteryLevelListener.commonEventSubscriber->OnReceiveEvent(data);
    
        want.SetParam(PowerMgr::BatteryInfo::COMMON_EVENT_KEY_CAPACITY, 20);
        data.SetWant(want);
        batteryLevelListener.commonEventSubscriber->OnReceiveEvent(data);
        batteryLevelListener.Stop();
    }

    void OnBatteryStatusListenner()
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

    void OnCharingListener()
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
        std::shared_ptr<ScreenListener> screenListener_ = std::make_shared<ScreenListener>(
            workSchedulerService_->workQueueManager_, workSchedulerService_);
        
        screenListener_->Start();
        EventFwk::CommonEventData data;
        screenListener_->commonEventSubscriber->OnReceiveEvent(data);

        EventFwk::Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
        data.SetWant(want);
        screenListener_->commonEventSubscriber->OnReceiveEvent(data);

        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
        data.SetWant(want);
        screenListener_->commonEventSubscriber->OnReceiveEvent(data);

        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
        data.SetWant(want);
        screenListener_->commonEventSubscriber->OnReceiveEvent(data);
        screenListener_->Stop();
    }

    void OnReceiveEvent()
    {
        OnBatteryLevelListener();
        OnBatteryStatusListenner();
        OnCharingListener();
        OnScreenListener();
    }

    void TriggWork(WorkInfo &workInfo)
    {
        OnStorageChange();
        OnNetworkChange();
        OnNetworkChange();
        OnBatteryStatusChange();
        OnTimerChange();

        OnReceiveEvent();

        std::vector<std:string> argsInStr;
        std::string result;
        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-k");
        argsInStr.push_back("settings.power.suspend_sources");
        workSchedulerService_->DumpProcess(argsInStr, result);

        argsInStr.clear();
        result.clear();
        argsInStr.push_bach("-t");
        argsInStr.push_bach(workInfo.GetBundleName());
        argsInStr.push_bach(workInfo.GetAbilityName());
        workSchedulerService_->DumpProcess(argsInStr, result);

        workSchedulerService_->PauseRunningWorks(workInfo.GetUid());
        workSchedulerService_->ResumePausedWorks(workInfo.GetUid());

        std::shared_ptr<WorkConnManager> workConnManager = std::make_shared<WorkConnManager>();
        std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workInfo, workInfo.GetUid());
        workConnManager->WriteStartWorkEvent(workStatue);
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
        workSchedulerService_->standbyStateObserver_->OnAllowListChanged(0, "bundlename", 0, true);
    }

    void OnWorkBundleGroupChange()
    {
        OHOS::DeviceUsageStats::AppGroupCallbackInfo appGroupCallbackInfo1(0, 1, 2, 0, "bundlename");
        workSchedulerService_->groupObserver_->OnAppGroupChanged(appGroupCallbackInfo1);
        OHOS::DeviceUsageStats::AppGroupCallbackInfo appGroupCallbackInfo2(0, 2, 1, 0, "bundlename");
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
        uint32_t code = static_cast<int32_t>(IWorkSchedServiceInterfaceCode::START_WORK);
        
        workSchedulerService_->OnStart();
        workSchedulerService_->InitBgTaskSubscriber();
        OnProcEfficiencyResourcesChange();
        OnWorkStandbyStateChange();
        OnWorkBundleGroupChange();

        if (workSchedulerService_->workQueueManager_ == nullptr) {
            workSchedulerService_->workQueueManager_ = make_shared<WorkQueueManager>(workSchedulerService_);
        }
        if (!workSchedulerService_->ready_) {
            workSchedulerService_->ready_ = true;
        }
        if (workSchedulerService_->checkBundle_) {
            workSchedulerService_->checkBundle_ = false;
        }

        WorkInfo workInfo = WorkInfo();
        if (!dataMessageParcel.WriteParcelable(&workInfo)) {
            return false;
        }
        workSchedulerService_->OnRemoteRequest(code, dataMessageParcel, reply, option);
        workSchedulerService_->TriggerWorkIfConditionReady();
        TriggerWork();
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