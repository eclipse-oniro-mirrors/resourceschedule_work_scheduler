/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include <cinttypes>

#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"
#include "work_sched_hilog.h"
#include "work_sched_utils.h"
#include "work_status.h"
#include "work_event_handler.h"
#include "work_scheduler_service.h"
#include "work_sched_constants.h"
#include "conditions/timer_info.h"
#include "work_sched_hisysevent_report.h"
#include "work_sched_data_manager.h"

namespace OHOS {
namespace WorkScheduler {
ScreenEventSubscriber::ScreenEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
    ScreenListener &listener) : EventFwk::CommonEventSubscriber(subscribeInfo), listener_(listener) {}

void ScreenEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const std::string action = data.GetWant().GetAction();
    WS_HILOGI("OnReceiveEvent get action: %{public}s", action.c_str());
    auto eventHandler = listener_.service_->GetHandler();
    if (!eventHandler) {
        WS_HILOGE("event handler is null");
        return;
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED) {
        if (DelayedSingleton<DataManager>::GetInstance()->GetDeepIdle()) {
            DelayedSingleton<DataManager>::GetInstance()->SetDeepIdle(false);
            WorkSchedUtil::HiSysEventStateChanged({"DEEP_IDLE", 0});
        }
        listener_.StopTimer();
        listener_.OnConditionChanged(WorkCondition::Type::DEEP_IDLE,
            std::make_shared<DetectorValue>(0, 0, false, std::string()));
        auto task = [weak = weak_from_this()]() {
            auto strong = weak.lock();
            if (!strong) {
                WS_HILOGE("ScreenEventSubscriber::OnReceiveEvent strong is null");
                return;
            }
            int32_t ret = strong->listener_.service_->StopDeepIdleWorks();
            if (ret != ERR_OK) {
                WS_HILOGE("stop work after unlocking failed, error code:%{public}d.", ret);
            } else {
                WS_HILOGI("stop work after unlocking successed.");
            }
        };
        eventHandler->PostTask(task);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        if (listener_.timerId_ != 0) {
            WS_HILOGW("has send check deep idle msg, ignore");
            return;
        }
        listener_.StartTimer();
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

void ScreenListener::StartTimer()
{
    WS_HILOGI("deep idle timer start with time = %{public}d.", MIN_DEEP_IDLE_SCREEN_OFF_TIME_MIN);
    auto task = [=]() {
        WS_HILOGI("Into deep idle mode");
        DelayedSingleton<DataManager>::GetInstance()->SetDeepIdle(true);
        service_->HandleDeepIdleMsg();
    };
    auto timerInfo = std::make_shared<TimerInfo>();
    uint8_t type = static_cast<uint8_t>(timerInfo->TIMER_TYPE_EXACT) |
        static_cast<uint8_t>(timerInfo->TIMER_TYPE_REALTIME);
    timerInfo->SetType(static_cast<int>(type));
    timerInfo->SetRepeat(false);
    timerInfo->SetInterval(MIN_DEEP_IDLE_SCREEN_OFF_TIME_MIN);
    timerInfo->SetCallbackInfo(task);
    timerId_ = TimeServiceClient::GetInstance()->CreateTimer(timerInfo);
    if (timerId_ == 0) {
        WS_HILOGE("TimerListener CreateTimer failed");
        return;
    }
    bool res = TimeServiceClient::GetInstance()->StartTimer(timerId_,
        TimeServiceClient::GetInstance()->GetBootTimeMs() + MIN_DEEP_IDLE_SCREEN_OFF_TIME_MIN);
    WS_HILOGI("deep idle timer start, res is %{public}d, timerId %{public}" PRIu64, res, timerId_);
}

void ScreenListener::StopTimer()
{
    if (timerId_ > 0) {
        MiscServices::TimeServiceClient::GetInstance()->StopTimer(timerId_);
        MiscServices::TimeServiceClient::GetInstance()->DestroyTimer(timerId_);
        timerId_ = 0;
        WS_HILOGI("deep idle timer stop success");
    }
    WS_HILOGI("deep idle timer stop end");
}
} // namespace WorkScheduler
} // namespace OHOS