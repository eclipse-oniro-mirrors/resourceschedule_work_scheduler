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
#include "conditions/timer_listener.h"

#include "work_queue_event_handler.h"
#include "work_sched_hilog.h"
#include "conditions/timer_info.h"

namespace OHOS {
namespace WorkScheduler {
TimerListener::TimerListener(std::shared_ptr<WorkQueueManager> workQueueManager,
    const std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    workQueueManager_ = workQueueManager;
}

void TimerListener::OnConditionChanged(WorkCondition::Type conditionType,
    std::shared_ptr<DetectorValue> conditionVal)
{
    workQueueManager_->OnConditionChanged(conditionType, conditionVal);
}

bool TimerListener::Start()
{
    if (workQueueManager_ == nullptr) {
        WS_HILOGE("workQueueManager_ is null");
        return false;
    }
    uint32_t time = workQueueManager_->GetTimeCycle();
    WS_HILOGI("TimerListener start with time = %{public}u.", time);
    auto task = [=]() {
        WS_HILOGI("begin check repeat work");
        workQueueManager_->OnConditionChanged(WorkCondition::Type::TIMER, std::make_shared<
            DetectorValue>(0, 0, 0, std::string()));
    };
    auto timerInfo = std::make_shared<TimerInfo>();
    uint8_t type = static_cast<uint8_t>(timerInfo->TIMER_TYPE_EXACT) |
        static_cast<uint8_t>(timerInfo->TIMER_TYPE_REALTIME);
    timerInfo->SetType(static_cast<int>(type));
    timerInfo->SetRepeat(true);
    timerInfo->SetInterval(time);
    timerInfo->SetCallbackInfo(task);
    timerId_ = TimeServiceClient::GetInstance()->CreateTimer(timerInfo);
    if (timerId_ == 0) {
        WS_HILOGE("TimerListener CreateTimer failed");
        return false;
    }
    bool res = TimeServiceClient::GetInstance()->StartTimer(timerId_,
        TimeServiceClient::GetInstance()->GetBootTimeMs() + time);
    WS_HILOGI("res is %{public}d, timerId = %{public}" PRIu64, res, timerId_);
    return true;
}

bool TimerListener::Stop()
{
    if (timerId_ > 0) {
        MiscServices::TimeServiceClient::GetInstance()->StopTimer(timerId_);
        MiscServices::TimeServiceClient::GetInstance()->DestroyTimer(timerId_);
        timerId_ = 0;
    }
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS