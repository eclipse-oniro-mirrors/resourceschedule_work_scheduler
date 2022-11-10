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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_SCHEDULER_BG_TASK_SUBSCRIBER_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_SCHEDULER_BG_TASK_SUBSCRIBER_H

#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
#include "background_task_subscriber.h"
#include "background_task_mgr_helper.h"

namespace OHOS {
namespace WorkScheduler {
class SchedulerBgTaskSubscriber : public BackgroundTaskMgr::BackgroundTaskSubscriber {
public:
    SchedulerBgTaskSubscriber() = default;
    void OnConnected() override;
    void OnDisconnected() override;
    void OnProcEfficiencyResourcesApply(
        const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo) override;
    void OnProcEfficiencyResourcesReset(
        const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo) override;
    void OnAppEfficiencyResourcesApply(
        const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo) override;
    void OnAppEfficiencyResourcesReset(
        const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo) override;
    void OnRemoteDied(const wptr<IRemoteObject> &object) override;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_SCHEDULER_BG_TASK_SUBSCRIBER_H