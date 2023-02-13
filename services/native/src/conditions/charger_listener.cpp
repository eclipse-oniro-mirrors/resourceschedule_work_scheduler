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
#include "conditions/charger_listener.h"

#include "battery_info.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
ChargerEventSubscriber::ChargerEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
    ChargerListener &listener) : EventFwk::CommonEventSubscriber(subscribeInfo), listener_(listener) {}

void ChargerEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const std::string action = data.GetWant().GetAction();
    WS_HILOGD("OnReceiveEvent get action: %{public}s", action.c_str());

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED) {
        int32_t type = data.GetCode();
        switch (type) {
            case static_cast<int32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_AC):
                WS_HILOGI("Condition changed: CHARGER_PLUGGED_AC");
                listener_.OnConditionChanged(WorkCondition::Type::CHARGER,
                    std::make_shared<DetectorValue>(WorkCondition::CHARGING_PLUGGED_AC,
                    0, 0, std::string()));
                break;
            case static_cast<int32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB):
                WS_HILOGI("Condition changed: CHARGER_PLUGGED_USB");
                listener_.OnConditionChanged(WorkCondition::Type::CHARGER,
                    std::make_shared<DetectorValue>(WorkCondition::CHARGING_PLUGGED_USB,
                    0, 0, std::string()));
                break;
            case static_cast<int32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS):
                WS_HILOGI("Condition changed: CHARGER_WIRELESS");
                listener_.OnConditionChanged(WorkCondition::Type::CHARGER,
                    std::make_shared<DetectorValue>(WorkCondition::CHARGING_PLUGGED_WIRELESS,
                    0, 0, std::string()));
                break;
            default:
                break;
        }
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED) {
        int32_t type = data.GetCode();
        switch (type) {
            case static_cast<int32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_NONE):
            case static_cast<int32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_BUTT):
                WS_HILOGI("Condition changed: CHARGER_PLUGGED_UNPLUGGED");
                listener_.OnConditionChanged(WorkCondition::Type::CHARGER,
                    std::make_shared<DetectorValue>(WorkCondition::CHARGING_UNPLUGGED, 0, 0, std::string()));
                break;
            default:
                break;
        }
    }
}

std::shared_ptr<EventFwk::CommonEventSubscriber> CreateChargerEventSubscriber(ChargerListener &listener)
{
    EventFwk::MatchingSkills skills = EventFwk::MatchingSkills();
    skills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    skills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    EventFwk::CommonEventSubscribeInfo info(skills);
    return std::make_shared<ChargerEventSubscriber>(info, listener);
}

ChargerListener::ChargerListener(std::shared_ptr<WorkQueueManager> workQueueManager)
{
    workQueueManager_ = workQueueManager;
}

ChargerListener::~ChargerListener()
{
    this->Stop();
}

bool ChargerListener::Start()
{
    WS_HILOGI("Charger listener start");
    this->commonEventSubscriber = CreateChargerEventSubscriber(*this);
    return EventFwk::CommonEventManager::SubscribeCommonEvent(this->commonEventSubscriber);
}

bool ChargerListener::Stop()
{
    WS_HILOGI("Charger listener stop");
    if (this->commonEventSubscriber != nullptr) {
        bool result = EventFwk::CommonEventManager::UnSubscribeCommonEvent(this->commonEventSubscriber);
        if (result) {
            this->commonEventSubscriber = nullptr;
        }
        return result;
    }
    return true;
}

void ChargerListener::OnConditionChanged(WorkCondition::Type conditionType,
    std::shared_ptr<DetectorValue> conditionVal)
{
    if (workQueueManager_ != nullptr) {
        workQueueManager_->OnConditionChanged(conditionType, conditionVal);
    } else {
        WS_HILOGE("workQueueManager_ is nullptr.");
    }
}
} // namespace WorkScheduler
} // namespace OHOS