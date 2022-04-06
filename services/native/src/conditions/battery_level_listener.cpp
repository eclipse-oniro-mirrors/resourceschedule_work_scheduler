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
#include "conditions/battery_level_listener.h"

#include <string>

#include "battery_info.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"

namespace OHOS {
namespace WorkScheduler {
BatteryLevelEventSubscriber::BatteryLevelEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
    BatteryLevelListener &listener) : EventFwk::CommonEventSubscriber(subscribeInfo), listener_(listener) {}

void BatteryLevelEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const std::string action = data.GetWant().GetAction();

    WS_HILOGI("OnReceiveEvent get action: %{public}s", action.c_str());
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED) {
        if (data.GetCode() == PowerMgr::BatteryInfo::COMMON_EVENT_CODE_CAPACITY) {
            std::string KEY_CAPACITY = ToString(PowerMgr::BatteryInfo::COMMON_EVENT_CODE_CAPACITY);
            int defaultCapacity = -1;
            auto capacity = data.GetWant().GetIntParam(KEY_CAPACITY, defaultCapacity);
            WS_HILOGI("OnReceiveEvent  capacity %{public}d", capacity);
            if (capacity == defaultCapacity) {
                return;
            }
            listener_.OnConditionChanged(WorkCondition::Type::BATTERY_LEVEL,
                std::make_shared<DetectorValue>(capacity, 0, 0, std::string()));
        }
    } else {
        WS_HILOGI("OnReceiveEvent action is invalid");
    }
}

std::shared_ptr<EventFwk::CommonEventSubscriber> CreateBatteryEventSubscriber(BatteryLevelListener &listener)
{
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    EventFwk::CommonEventSubscribeInfo info(skill);
    return std::make_shared<BatteryLevelEventSubscriber>(info, listener);
}

BatteryLevelListener::BatteryLevelListener(std::shared_ptr<WorkQueueManager> workQueueManager)
{
    workQueueManager_ = workQueueManager;
}

BatteryLevelListener::~BatteryLevelListener()
{
    this->Stop();
}

bool BatteryLevelListener::Start()
{
    WS_HILOGI("BatteryLevelListener Start");
    this->commonEventSubscriber = CreateBatteryEventSubscriber(*this);
    return EventFwk::CommonEventManager::SubscribeCommonEvent(this->commonEventSubscriber);
}

bool BatteryLevelListener::Stop()
{
    WS_HILOGI("BatteryLevelListener Stop");
    if (this->commonEventSubscriber != nullptr) {
        bool result = EventFwk::CommonEventManager::UnSubscribeCommonEvent(this->commonEventSubscriber);
        if (result) {
            this->commonEventSubscriber = nullptr;
        }
        return result;
    }
    return true;
}

void BatteryLevelListener::OnConditionChanged(WorkCondition::Type conditionType,
    std::shared_ptr<DetectorValue> conditionVal)
{
    if (workQueueManager_ != nullptr) {
        workQueueManager_->OnConditionChanged(conditionType, conditionVal);
    } else {
        WS_HILOGD("workQueueManager_ is nullptr.");
    }
}
} // namespace WorkScheduler
} // namespace OHOS