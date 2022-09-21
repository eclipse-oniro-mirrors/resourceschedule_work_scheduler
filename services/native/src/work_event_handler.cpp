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

#include "work_event_handler.h"

#include "work_scheduler_service.h"
#include "work_policy_manager.h"
#include "work_sched_hilog.h"

using namespace std;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace WorkScheduler {
WorkEventHandler::WorkEventHandler(const shared_ptr<EventRunner>& runner,
    const wptr<WorkSchedulerService>& service) : EventHandler(runner), service_(service)
{
    WS_HILOGD("instance created.");
}

void WorkEventHandler::ProcessEvent([[maybe_unused]] const InnerEvent::Pointer& event)
{
    WS_HILOGD("begin");
    auto wssptr = service_.promote();
    if (wssptr == nullptr) {
        WS_HILOGE("service.promote() returns nullptr");
        return;
    }
    WS_HILOGD("eventid = %{public}u", event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case RETRIGGER_MSG: {
            wssptr->GetWorkPolicyManager()->CheckWorkToRun();
            break;
        }
        case SERVICE_INIT_MSG: {
            wssptr->Init();
            break;
        }
        default:
            return;
    }
}
} // namespace WorkScheduler
} // namespace OHOS