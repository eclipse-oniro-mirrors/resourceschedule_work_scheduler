/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "work_sched_common.h"

using namespace std;

namespace OHOS {
namespace WorkScheduler {
static const int TIMER_TICK_DELAY = 600000;

WorkQueueEventHandler::WorkQueueEventHandler(const shared_ptr<AppExecFwk::EventRunner>& runner,
    shared_ptr<WorkQueueManager> manager) : AppExecFwk::EventHandler(runner)
{
    manager_ = manager;
    WS_HILOGD("WorkQueueEventHandler::WorkQueueEventHandler instance created.");
}

void WorkQueueEventHandler::ProcessEvent([[maybe_unused]] const AppExecFwk::InnerEvent::Pointer& event)
{
    WS_HILOGD("WorkQueueEventHandler::%{public}s, eventId: %{public}d", __func__, event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case TIMER_TICK: {
            manager_->OnConditionChanged(WorkCondition::Type::TIMER, make_shared<DetectorValue>(0, 0, 0, string()));
            SendEvent(AppExecFwk::InnerEvent::Get(TIMER_TICK, 0), TIMER_TICK_DELAY);
            break;
        }
        default: {
            return;
        }
    }
}
} // namespace WorkScheduler
} // namespace OHOS