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
#include "conditions/storage_listener.h"

#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
StorageEventSubscriber::StorageEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
    StorageListener &listener) : EventFwk::CommonEventSubscriber(subscribeInfo), listener_(listener) {}

void StorageEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const std::string action = data.GetWant().GetAction();
    WS_HILOGD("OnReceiveEvent get action: %{public}s", action.c_str());
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_LOW) {
        WS_HILOGI("Condition changed: STORAGE_LOW");
        listener_.OnConditionChanged(WorkCondition::Type::STORAGE,
            std::make_shared<DetectorValue>(WorkCondition::STORAGE_LEVEL_LOW, 0, 0, std::string()));
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_OK) {
        WS_HILOGI("Condition changed: STORAGE_OK");
        listener_.OnConditionChanged(WorkCondition::Type::STORAGE,
            std::make_shared<DetectorValue>(WorkCondition::STORAGE_LEVEL_OKAY, 0, 0, std::string()));
    } else {
        WS_HILOGE("OnReceiveEvent action is invalid");
    }
}

std::shared_ptr<EventFwk::CommonEventSubscriber> CreateStorageEventSubscriber(StorageListener &listener)
{
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_LOW);
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_OK);
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_FULL);
    EventFwk::CommonEventSubscribeInfo info(skill);
    return std::make_shared<StorageEventSubscriber>(info, listener);
}

StorageListener::StorageListener(std::shared_ptr<WorkQueueManager> workQueueManager)
{
    workQueueManager_ = workQueueManager;
}

StorageListener::~StorageListener()
{
    this->Stop();
}

bool StorageListener::Start()
{
    WS_HILOGI("Storage listener start");
    this->commonEventSubscriber = CreateStorageEventSubscriber(*this);
    return EventFwk::CommonEventManager::SubscribeCommonEvent(this->commonEventSubscriber);
}

bool StorageListener::Stop()
{
    WS_HILOGI("Storage listener stop");
    if (this->commonEventSubscriber != nullptr) {
        bool result = EventFwk::CommonEventManager::UnSubscribeCommonEvent(this->commonEventSubscriber);
        if (result) {
            this->commonEventSubscriber = nullptr;
        }
        return result;
    }
    return true;
}

void StorageListener::OnConditionChanged(WorkCondition::Type conditionType,
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