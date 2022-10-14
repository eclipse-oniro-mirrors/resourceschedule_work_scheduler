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
#include "conditions/battery_status_listener.h"

#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
BatteryStatusEventSubscriber::BatteryStatusEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
    BatteryStatusListener &listener) : EventFwk::CommonEventSubscriber(subscribeInfo), listener_(listener) {}

void BatteryStatusEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const std::string action = data.GetWant().GetAction();
    WS_HILOGD("OnReceiveEvent get action: %{public}s", action.c_str());

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW) {
        WS_HILOGI("Condition changed: BATTERY_STATUS_LOW");
        listener_.OnConditionChanged(WorkCondition::Type::BATTERY_STATUS,
            std::make_shared<DetectorValue>(WorkCondition::BATTERY_STATUS_LOW, 0, 0, std::string()));
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY) {
        WS_HILOGI("Condition changed: BATTERY_STATUS_OKAY");
        listener_.OnConditionChanged(WorkCondition::Type::BATTERY_STATUS,
            std::make_shared<DetectorValue>(WorkCondition::BATTERY_STATUS_OKAY, 0, 0, std::string()));
    } else {
        WS_HILOGI("OnReceiveEvent action is invalid");
    }
}

std::shared_ptr<EventFwk::CommonEventSubscriber> CreateBatteryEventSubscriber(BatteryStatusListener &listener)
{
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW);
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY);
    EventFwk::CommonEventSubscribeInfo info(skill);
    return std::make_shared<BatteryStatusEventSubscriber>(info, listener);
}

BatteryStatusListener::BatteryStatusListener(std::shared_ptr<WorkQueueManager> workQueueManager)
{
    workQueueManager_ = workQueueManager;
}

BatteryStatusListener::~BatteryStatusListener()
{
    this->Stop();
}

bool BatteryStatusListener::Start()
{
    WS_HILOGD("Battery status listener start.");
    this->commonEventSubscriber = CreateBatteryEventSubscriber(*this);
    return EventFwk::CommonEventManager::SubscribeCommonEvent(this->commonEventSubscriber);
}

bool BatteryStatusListener::Stop()
{
    WS_HILOGD("Battery status listener stop.");
    if (this->commonEventSubscriber != nullptr) {
        bool result = EventFwk::CommonEventManager::UnSubscribeCommonEvent(this->commonEventSubscriber);
        if (result) {
            this->commonEventSubscriber = nullptr;
        }
        return result;
    }
    return true;
}

void BatteryStatusListener::OnConditionChanged(WorkCondition::Type conditionType,
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