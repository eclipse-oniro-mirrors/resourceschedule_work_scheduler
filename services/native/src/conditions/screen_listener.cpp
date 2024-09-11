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
#include "conditions/screen_listener.h"

#include <string>

#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"
#include "work_sched_hilog.h"
#include "work_sched_utils.h"
#include "work_status.h"
#include "work_event_handler.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
const int MIN_DEEP_IDLE_SCREEN_OFF_TIME_MIN = 31 * 60 * 1000;
}
ScreenEventSubscriber::ScreenEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
    ScreenListener &listener) : EventFwk::CommonEventSubscriber(subscribeInfo), listener_(listener) {}

void ScreenEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const std::string action = data.GetWant().GetAction();
    WS_HILOGI("OnReceiveEvent get action: %{public}s", action.c_str());
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED) {
        listener_.service_->GetHandler()->RemoveEvent(WorkEventHandler::CHECK_DEEPIDLE_MSG);
        listener_.OnConditionChanged(WorkCondition::Type::DEEP_IDLE,
            std::make_shared<DetectorValue>(0, 0, false, std::string()));
        int32_t ret = listener_.service_->StopDeepIdleWorks();
        if (ret != ERR_OK) {
            WS_HILOGE("stop work by condition failed, error code:%{public}d.", ret);
        } else {
            WS_HILOGI("stop work by condition successed.");
        }
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        listener_.service_->GetHandler()->RemoveEvent(WorkEventHandler::CHECK_DEEPIDLE_MSG);
        listener_.service_->GetHandler()->SendEvent(
            AppExecFwk::InnerEvent::Get(WorkEventHandler::CHECK_DEEPIDLE_MSG, 0), MIN_DEEP_IDLE_SCREEN_OFF_TIME_MIN);
    }
}

ScreenListener::ScreenListener(std::shared_ptr<WorkQueueManager> workQueueManager,
    std::shared_ptr<WorkSchedulerService> service)
{
    workQueueManager_ = workQueueManager;
    service_ = service;
}

ScreenListener::~ScreenListener()
{
    this->Stop();
}

std::shared_ptr<EventFwk::CommonEventSubscriber> CreateScreenEventSubscriber(ScreenListener &listener)
{
    EventFwk::MatchingSkills skill = EventFwk::MatchingSkills();
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventSubscribeInfo info(skill);
    return std::make_shared<ScreenEventSubscriber>(info, listener);
}

bool ScreenListener::Start()
{
    WS_HILOGD("screen listener start.");
    this->commonEventSubscriber = CreateScreenEventSubscriber(*this);
    return EventFwk::CommonEventManager::SubscribeCommonEvent(this->commonEventSubscriber);
}

bool ScreenListener::Stop()
{
    WS_HILOGD("screen listener stop.");
    if (this->commonEventSubscriber != nullptr) {
        bool result = EventFwk::CommonEventManager::UnSubscribeCommonEvent(this->commonEventSubscriber);
        if (result) {
            this->commonEventSubscriber = nullptr;
        }
        return result;
    }
    return true;
}

void ScreenListener::OnConditionChanged(WorkCondition::Type conditionType,
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