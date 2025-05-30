/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "work_event_handler.h"

#include "work_scheduler_service.h"
#include "work_policy_manager.h"
#include "work_sched_hilog.h"

using namespace std;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace WorkScheduler {
WorkEventHandler::WorkEventHandler(const shared_ptr<EventRunner>& runner,
    const std::shared_ptr<WorkSchedulerService>& service) : EventHandler(runner), service_(service)
{
    WS_HILOGD("instance created.");
}

void WorkEventHandler::ProcessEvent([[maybe_unused]] const InnerEvent::Pointer& event)
{
    WS_HILOGD("begin");
    if (service_.expired()) {
        WS_HILOGE("service_ expired");
        return;
    }
    auto service = service_.lock();
    if (!service) {
        WS_HILOGE("service_ null");
        return;
    }
    WS_HILOGD("eventid = %{public}u", event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case RETRIGGER_MSG: {
            service->GetWorkPolicyManager()->CheckWorkToRun();
            break;
        }
        case SERVICE_INIT_MSG: {
            service->Init(GetEventRunner());
            break;
        }
        case IDE_RETRIGGER_MSG: {
            service->GetWorkPolicyManager()->TriggerIdeWork();
            break;
        }
        case CHECK_CONDITION_MSG: {
            service->TriggerWorkIfConditionReady();
            break;
        }
        default:
            return;
    }
}
} // namespace WorkScheduler
} // namespace OHOS