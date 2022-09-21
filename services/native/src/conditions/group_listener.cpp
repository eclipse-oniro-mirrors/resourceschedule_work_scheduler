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
#include "conditions/group_listener.h"

#include "work_queue_event_handler.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
GroupListener::GroupListener(std::shared_ptr<WorkQueueManager> workQueueManager)
{
    workQueueManager_ = workQueueManager;
}

void GroupListener::OnConditionChanged(WorkCondition::Type conditionType,
    std::shared_ptr<DetectorValue> conditionVal)
{
    workQueueManager_->OnConditionChanged(conditionType, conditionVal);
}

bool GroupListener::Start()
{
    if (!eventRunner_) {
        eventRunner_ = AppExecFwk::EventRunner::Create(GROUP_LISTENER);
        if (eventRunner_ == nullptr) {
            WS_HILOGE("Failed to create EventHandler");
            return false;
        }
    }
    if (!handler_) {
        handler_ = std::make_shared<WorkQueueEventHandler>(eventRunner_, workQueueManager_);
    }
    if (workQueueManager_ == nullptr) {
        WS_HILOGE("workQueueManager is null");
        return false;
    }
    int32_t time = workQueueManager_->GetTimeRetrigger();
    handler_->SendEvent(AppExecFwk::InnerEvent::Get(WorkQueueEventHandler::GROUP_TICK, 0), time);
    return true;
}

bool GroupListener::Stop()
{
    if (handler_ != nullptr) {
        handler_->RemoveEvent(WorkQueueEventHandler::GROUP_TICK);
    }
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS