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
    explicit Watchdog(const std::shared_ptr<WorkPolicyManager>& service);
    ~Watchdog() override = default;
    /**
     * @brief Add watchdog.
     *
     * @param watchdogId The id of watchdog.
     * @param interval The interval.
     * @return True if success,else false.
     */
    bool AddWatchdog(const uint32_t watchdogId, int32_t interval);
    /**
     * @brief Remove watchdog.
     *
     * @param watchdogId The id of watchdog.
     */
    void RemoveWatchdog(uint32_t watchdogId);
    /**
     * @brief Process event.
     *
     * @param event The event.
     */
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;

private:
    std::shared_ptr<WorkPolicyManager> service_;
};
}  // namespace WorkScheduler
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WATCHDOG_H