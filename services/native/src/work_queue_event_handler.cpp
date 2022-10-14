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

#include "work_queue_event_handler.h"

#include "work_sched_hilog.h"

using namespace std;

namespace OHOS {
namespace WorkScheduler {
WorkQueueEventHandler::WorkQueueEventHandler(const shared_ptr<AppExecFwk::EventRunner>& runner,
    shared_ptr<WorkQueueManager> manager) : AppExecFwk::EventHandler(runner)
{
    manager_ = manager;
    WS_HILOGD("instance created.");
}

void WorkQueueEventHandler::ProcessEvent([[maybe_unused]] const AppExecFwk::InnerEvent::Pointer& event)
{
    WS_HILOGD("eventId: %{public}u", event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case TIMER_TICK: {
            if (manager_ == nullptr) {
                return;
            }
            manager_->OnConditionChanged(WorkCondition::Type::TIMER, make_shared<DetectorValue>(0, 0, 0, string()));
            uint32_t time = manager_->GetTimeCycle();
            SendEvent(AppExecFwk::InnerEvent::Get(TIMER_TICK, 0), time);
            break;
        }
        case GROUP_TICK: {
            if (manager_ == nullptr) {
                return;
            }
            manager_->OnConditionChanged(WorkCondition::Type::GROUP, make_shared<DetectorValue>(-1, 0, 0, string()));
            break;
        }
        default: {
            return;
        }
    }
}
} // namespace WorkScheduler
} // namespace OHOS