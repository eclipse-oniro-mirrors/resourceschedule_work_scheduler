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
#include "conditions/network_listener.h"

#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"

namespace OHOS {
namespace WorkScheduler {
NetworkEventSubscriber::NetworkEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
    NetworkListener &listener) : CommonEventSubscriber(subscribeInfo), listener_(listener) {}

void NetworkEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const std::string action = data.GetWant().GetAction();
    WS_HILOGI("OnReceiveEvent get action: %{public}s", action.c_str());

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE) {
        WS_HILOGI("Condition changed: WIFI_CONN_STATE");
        listener_.OnConditionChanged(WorkCondition::Type::NETWORK,
            std::make_shared<DetectorValue>(WorkCondition::NETWORK_TYPE_WIFI, 0, 0, std::string()));
    } else {
        WS_HILOGI("OnReceiveEvent action is invalid");
    }
}

std::shared_ptr<EventFwk::CommonEventSubscriber> CreateNetworkEventSubscriber(NetworkListener &listener)
{
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    return std::make_shared<NetworkEventSubscriber>(info, listener);
}

NetworkListener::NetworkListener(std::shared_ptr<WorkQueueManager> workQueueManager)
{
    workQueueManager_ = workQueueManager;
}

NetworkListener::~NetworkListener()
{
    this->Stop();
}

bool NetworkListener::Start()
{
    WS_HILOGI("NetworkListener start");
    this->commonEventSubscriber = CreateNetworkEventSubscriber(*this);
    return EventFwk::CommonEventManager::SubscribeCommonEvent(this->commonEventSubscriber);
}

bool NetworkListener::Stop()
{
    WS_HILOGI("NetworkListener stop");
    if (this->commonEventSubscriber != nullptr) {
        bool result = EventFwk::CommonEventManager::UnSubscribeCommonEvent(this->commonEventSubscriber);
        if (result) {
            this->commonEventSubscriber = nullptr;
        }
        return result;
    }
    return true;
}

void NetworkListener::OnConditionChanged(WorkCondition::Type conditionType,
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