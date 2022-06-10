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
#include "watchdog.h"

#include "work_sched_hilog.h"
#include "work_policy_manager.h"

using namespace std;

namespace OHOS {
namespace WorkScheduler {
const std::string WORK_SCHEDULER_WATCHDOG = "WorkSchedulerWatchdog";

Watchdog::Watchdog(const std::shared_ptr<WorkPolicyManager>& service) : service_(service)
{
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(WORK_SCHEDULER_WATCHDOG);
    if (eventRunner.get() != nullptr) {
        SetEventRunner(eventRunner);
    }
}

bool Watchdog::AddWatchdog(uint32_t watchdogId, int32_t interval)
{
    WS_HILOGD("Add watchdog with Id:%{public}u", watchdogId);
    return SendEvent(watchdogId, 0, interval);
}

void Watchdog::RemoveWatchdog(uint32_t watchdogId)
{
    WS_HILOGD("Remove watchdog with Id:%{public}u", watchdogId);
    RemoveEvent(watchdogId);
}

void Watchdog::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    if (event == nullptr) {
        return;
    }
    uint32_t watchdogId = event->GetInnerEventId();
    if (service_ != nullptr) {
        service_->WatchdogTimeOut(watchdogId);
    } else {
        WS_HILOGE("service is null");
    }
}
} // namespace WorkScheduler
} // namespace OHOS