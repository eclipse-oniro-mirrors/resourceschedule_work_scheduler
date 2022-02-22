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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WATCHDOG_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WATCHDOG_H

#include <event_handler.h>
#include <event_runner.h>
#include <refbase.h>

namespace OHOS {
namespace WorkScheduler {
class WorkPolicyManager;
class Watchdog : public AppExecFwk::EventHandler {
public:
    Watchdog(const std::shared_ptr<WorkPolicyManager>& service);
    ~Watchdog() override = default;
    bool AddWatchdog(const int32_t watchdogId, int32_t interval);
    void RemoveWatchdog(int32_t watchdogId);
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;

private:
    std::shared_ptr<WorkPolicyManager> service_;
};
}  // namespace WorkScheduler
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WATCHDOG_H