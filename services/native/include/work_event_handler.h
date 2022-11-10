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

#ifndef WORK_SCHED_SERVICES_WORK_EVENT_HANDLER_H
#define WORK_SCHED_SERVICES_WORK_EVENT_HANDLER_H

#include <event_handler.h>
#include <refbase.h>
#include <memory>

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerService;
class WorkEventHandler : public AppExecFwk::EventHandler {
public:
    enum {
        RETRIGGER_MSG = 0,
        SERVICE_INIT_MSG
    };
    WorkEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
        const wptr<WorkSchedulerService>& service);
    ~WorkEventHandler() override = default;
    /**
     * @brief Process event.
     *
     * @param event The event.
     */
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;

private:
    wptr<WorkSchedulerService> service_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // WORK_SCHED_SERVICES_WORK_EVENT_HANDLER_H