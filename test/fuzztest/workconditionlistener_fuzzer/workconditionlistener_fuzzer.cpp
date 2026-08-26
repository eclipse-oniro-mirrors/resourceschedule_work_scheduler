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

#include "workconditionlistener_fuzzer.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "conditions/battery_level_listener.h"
#include "conditions/battery_status_listener.h"
#include "conditions/charger_listener.h"
#include "conditions/group_listener.h"
#include "conditions/network_listener.h"
#include "conditions/screen_listener.h"
#include "conditions/storage_listener.h"
#include "conditions/timer_listener.h"
#include "detector_value.h"
#include "event_runner.h"
#include "watchdog.h"
#include "work_condition.h"
#include "work_event_handler.h"
#include "work_info.h"
#include "work_policy_manager.h"
#include "work_queue_manager.h"
#include "work_sched_constants.h"
#include "work_scheduler_service.h"
#include "work_status.h"

void OHOS::RefBase::DecStrongRef(void const *obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr const char *WORKSCHEDULER_SERVICE_RUNNER_NAME = "WorkSchedulerService";
constexpr size_t MAX_STRING_SIZE = 48;
constexpr int32_t DEFAULT_UID = 10000;
constexpr int32_t DEFAULT_SA_ID = 1905;
constexpr int32_t DEFAULT_REPEAT_MINUTES = 20;
constexpr int32_t REPEAT_SECONDS_PER_MINUTE = 60;
constexpr int32_t MILLISECONDS_PER_SECOND = 1000;
constexpr int32_t DEFAULT_REPEAT_CYCLE =
    DEFAULT_REPEAT_MINUTES * REPEAT_SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
constexpr int32_t BEARER_CELLULAR = 0;
constexpr int32_t BEARER_WIFI = 1;
constexpr int32_t BEARER_BLUETOOTH = 2;
constexpr int32_t BEARER_ETHERNET = 3;
constexpr int32_t BEARER_WIFI_AWARE = 5;
constexpr int32_t MIN_BATTERY_LEVEL_FOR_FUZZ = -5;
constexpr int32_t MIN_NETWORK_TYPE_FOR_FUZZ = -3;
constexpr int32_t FUZZ_INVALID_LOW_VALUE = -2;
constexpr int32_t FUZZ_INVALID_VALUE = -1;
constexpr int32_t MAX_REPEAT_COUNT_FOR_FUZZ = 3;
constexpr int32_t MAX_PREPARED_WORKS_FOR_FUZZ = 4;
constexpr int32_t MAX_EVENT_ROUNDS_FOR_FUZZ = 5;
constexpr int32_t MAX_CHARGE_STATE_FOR_FUZZ = 6;
constexpr int32_t MAX_GROUP_FOR_FUZZ = 10;
constexpr int32_t MAX_NETWORK_TYPE_FOR_FUZZ = 12;
constexpr int32_t MAX_BATTERY_LEVEL_FOR_FUZZ = 120;
constexpr int32_t MAX_USER_ID_FOR_FUZZ = 200;
constexpr int32_t MAX_LISTENER_WORK_ID_FOR_FUZZ = 1000;

std::string ConsumeText(FuzzedDataProvider &provider, const std::string &fallback)
{
    std::string text = provider.ConsumeRandomLengthString(MAX_STRING_SIZE);
    if (text.empty()) {
        return fallback;
    }
    std::replace_if(text.begin(), text.end(), [](char chr) { return chr == '\0' || chr == '\n' || chr == '\r'; }, '_');
    return text;
}

std::shared_ptr<WorkSchedulerService> PrepareService()
{
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (!service->eventRunner_) {
        service->eventRunner_ = AppExecFwk::EventRunner::Create(WORKSCHEDULER_SERVICE_RUNNER_NAME,
            AppExecFwk::ThreadMode::FFRT);
    }
    if (!service->handler_) {
        service->handler_ = std::make_shared<WorkEventHandler>(service->eventRunner_, service);
    }
    service->workQueueManager_ = std::make_shared<WorkQueueManager>(service);
    service->workPolicyManager_ = std::make_shared<WorkPolicyManager>(service);
    service->workPolicyManager_->handler_ = service->handler_;
    service->workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
    service->workPolicyManager_->watchdog_ =
        std::make_shared<Watchdog>(service->workPolicyManager_, service->eventRunner_);
    service->ready_.store(true);
    service->checkBundle_.store(false);
    service->whitelist_.clear();
    service->whitelist_.insert(DEFAULT_UID);
    return service;
}

WorkInfo BuildWorkInfo(FuzzedDataProvider &provider)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(provider.ConsumeIntegralInRange<int32_t>(1, MAX_LISTENER_WORK_ID_FOR_FUZZ));
    workInfo.SetElement(ConsumeText(provider, "bundle.listener.demo"), ConsumeText(provider, "AbilityListener"));
    workInfo.RefreshUid(DEFAULT_UID);
    workInfo.RequestPersisted(provider.ConsumeBool());
    workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(provider.ConsumeIntegralInRange<int32_t>(
        WorkCondition::Network::NETWORK_TYPE_ANY, WorkCondition::Network::NETWORK_TYPE_ETHERNET)));
    workInfo.RequestStorageLevel(static_cast<WorkCondition::Storage>(provider.ConsumeIntegralInRange<int32_t>(
        WorkCondition::Storage::STORAGE_LEVEL_LOW, WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY)));
    workInfo.RequestRepeatCycle(provider.ConsumeIntegralInRange<uint32_t>(0, DEFAULT_REPEAT_CYCLE),
        provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_REPEAT_COUNT_FOR_FUZZ));
    if (provider.ConsumeBool()) {
        workInfo.RefreshSaId(DEFAULT_SA_ID);
        workInfo.RequestDeepIdle(true);
        workInfo.deepIdleTime_ = provider.ConsumeIntegralInRange<int32_t>(0, DEFAULT_REPEAT_CYCLE);
    }
    return workInfo;
}

void AddPresetWork(const std::shared_ptr<WorkSchedulerService> &service, FuzzedDataProvider &provider)
{
    for (int32_t index = 0; index < provider.ConsumeIntegralInRange<int32_t>(1, MAX_PREPARED_WORKS_FOR_FUZZ); ++index) {
        auto workInfo = BuildWorkInfo(provider);
        workInfo.SetWorkId(index + 1);
        auto workStatus = std::make_shared<WorkStatus>(workInfo, DEFAULT_UID);
        service->workQueueManager_->AddWork(workStatus);
        service->workPolicyManager_->AddWork(workStatus, DEFAULT_UID);
    }
}

EventFwk::CommonEventData BuildCommonEventData(FuzzedDataProvider &provider)
{
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    const std::vector<std::string> actions = {
        EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE,
        EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_LOW,
        EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_OK,
        EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_FULL,
        EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED,
        EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF,
        EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED,
        ConsumeText(provider, "invalid.listener.action"),
    };
    want.SetAction(actions[provider.ConsumeIntegralInRange<size_t>(0, actions.size() - 1)]);
    const std::vector<int32_t> networkTypes = {
        FUZZ_INVALID_VALUE, BEARER_CELLULAR, BEARER_WIFI, BEARER_BLUETOOTH, BEARER_ETHERNET, BEARER_WIFI_AWARE,
        provider.ConsumeIntegralInRange<int32_t>(MIN_NETWORK_TYPE_FOR_FUZZ, MAX_NETWORK_TYPE_FOR_FUZZ),
    };
    want.SetParam("NetType", networkTypes[provider.ConsumeIntegralInRange<size_t>(0, networkTypes.size() - 1)]);
    want.SetParam("soc", provider.ConsumeIntegralInRange<int32_t>(
        MIN_BATTERY_LEVEL_FOR_FUZZ, MAX_BATTERY_LEVEL_FOR_FUZZ));
    want.SetParam("chargeState", provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_LOW_VALUE, MAX_CHARGE_STATE_FOR_FUZZ));
    want.SetParam("batteryLow", provider.ConsumeBool());
    data.SetWant(want);
    data.SetCode(provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_LOW_VALUE, MAX_CHARGE_STATE_FOR_FUZZ));
    return data;
}

void DriveCommonSubscriber(const std::shared_ptr<EventFwk::CommonEventSubscriber> &subscriber,
    FuzzedDataProvider &provider)
{
    if (!subscriber) {
        return;
    }
    for (int32_t index = 0; index < provider.ConsumeIntegralInRange<int32_t>(1, MAX_EVENT_ROUNDS_FOR_FUZZ); ++index) {
        auto data = BuildCommonEventData(provider);
        EventFwk::CommonEventManager::PublishCommonEvent(data);
        subscriber->OnReceiveEvent(data);
    }
}

template<typename Listener>
void DriveListener(Listener &listener, WorkCondition::Type type, FuzzedDataProvider &provider)
{
    listener.Start();
    for (int32_t index = 0; index < provider.ConsumeIntegralInRange<int32_t>(1, MAX_EVENT_ROUNDS_FOR_FUZZ); ++index) {
        auto value = std::make_shared<DetectorValue>(provider.ConsumeIntegralInRange<int32_t>(
            FUZZ_INVALID_LOW_VALUE, MAX_BATTERY_LEVEL_FOR_FUZZ),
            provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_USER_ID_FOR_FUZZ), provider.ConsumeBool(),
            ConsumeText(provider, "bundle.listener.demo"));
        listener.OnConditionChanged(type, value);
    }
    listener.Stop();
}
} // namespace

bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
{
    uid = DEFAULT_UID;
    return true;
}

void RunWorkConditionListenerFuzzer(FuzzedDataProvider &provider)
{
    auto service = PrepareService();
    AddPresetWork(service, provider);
    auto manager = service->workQueueManager_;

    auto networkListener = std::make_shared<NetworkListener>(manager);
    DriveListener(*networkListener, WorkCondition::Type::NETWORK, provider);
    networkListener->Start();
    DriveCommonSubscriber(networkListener->commonEventSubscriber, provider);
    networkListener->Stop();

    auto storageListener = std::make_shared<StorageListener>(manager);
    DriveListener(*storageListener, WorkCondition::Type::STORAGE, provider);
    storageListener->Start();
    DriveCommonSubscriber(storageListener->commonEventSubscriber, provider);
    storageListener->Stop();

    auto timerListener = std::make_shared<TimerListener>(manager, service->eventRunner_);
    DriveListener(*timerListener, WorkCondition::Type::TIMER, provider);

    auto groupListener = std::make_shared<GroupListener>(manager, service->eventRunner_);
    DriveListener(*groupListener, WorkCondition::Type::GROUP, provider);
    manager->OnConditionChanged(WorkCondition::Type::GROUP, std::make_shared<DetectorValue>(
        provider.ConsumeIntegralInRange<int32_t>(
            FUZZ_INVALID_VALUE, MAX_GROUP_FOR_FUZZ), provider.ConsumeIntegralInRange<int32_t>(0, MAX_USER_ID_FOR_FUZZ),
        true, ConsumeText(provider, "bundle.listener.demo")));

    auto screenListener = std::make_shared<ScreenListener>(manager, service);
    DriveListener(*screenListener, WorkCondition::Type::DEEP_IDLE, provider);
    screenListener->Start();
    screenListener->StartTimer();
    DriveCommonSubscriber(screenListener->commonEventSubscriber, provider);
    screenListener->StopTimer();
    screenListener->Stop();

#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    auto chargerListener = std::make_shared<ChargerListener>(manager);
    DriveListener(*chargerListener, WorkCondition::Type::CHARGER, provider);
    chargerListener->Start();
    DriveCommonSubscriber(chargerListener->commonEventSubscriber, provider);
    chargerListener->Stop();
    auto batteryStatusListener = std::make_shared<BatteryStatusListener>(manager);
    DriveListener(*batteryStatusListener, WorkCondition::Type::BATTERY_STATUS, provider);
    batteryStatusListener->Start();
    DriveCommonSubscriber(batteryStatusListener->commonEventSubscriber, provider);
    batteryStatusListener->Stop();
    auto batteryLevelListener = std::make_shared<BatteryLevelListener>(manager, service);
    DriveListener(*batteryLevelListener, WorkCondition::Type::BATTERY_LEVEL, provider);
    batteryLevelListener->Start();
    DriveCommonSubscriber(batteryLevelListener->commonEventSubscriber, provider);
    batteryLevelListener->Stop();
#endif

    std::string result;
    manager->Dump(result);
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    OHOS::WorkScheduler::RunWorkConditionListenerFuzzer(provider);
    return 0;
}
