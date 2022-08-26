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

#include "scheduler_bg_task_subscriber.h"
#include "resource_type.h"
#include "resource_callback_info.h"

namespace OHOS {
namespace WorkScheduler {

using namespace OHOS::BackgroundTaskMgr;
SchedulerBgTaskSubscriber::SchedulerBgTaskSubscriber(const std::shared_ptr<WorkSchedulerService> workSchedulerService)
{
    if (workSchedulerService != nullptr) {
        workSchedulerService_ = workSchedulerService;
    }
}

void SchedulerBgTaskSubscriber::OnConnected()
{
    WS_HILOGI("called");
}

void SchedulerBgTaskSubscriber::OnDisconnected()
{
    WS_HILOGI("called");
}

void SchedulerBgTaskSubscriber::OnEfficiencyResourcesApply(
    const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo)
{
    WS_HILOGI("OnEfficiencyResourcesApply before");
    if (resourceInfo == nullptr || (resourceInfo->GetResourceNumber() & BackgroundTaskMgr::ResourceType::WORK_SCHEDULER) == 0) {
        WS_HILOGE("called with null EfficiencyResourceCallbackInfo");
        return;
    }
    auto workSchedulerServicePtr = workSchedulerService_.lock();
    if (workSchedulerServicePtr) {
        int32_t uid = resourceInfo->GetUid();
        WS_HILOGD("called, bundleName=%{public}s, uid=%{public}d, pid=%{public}d, resourcenumber=%{public}u",
            resourceInfo->GetBundleName().c_str(), uid, resourceInfo->GetPid(), resourceInfo->GetResourceNumber());
        DelayedSingleton<WorkSchedulerService>::GetInstance()->UpdateWhiteList(uid, true);
    }
}

void SchedulerBgTaskSubscriber::OnEfficiencyResourcesReset(
    const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo)
{
    WS_HILOGI("OnEfficiencyResourcesApply before");
    if (resourceInfo == nullptr || (resourceInfo->GetResourceNumber() & BackgroundTaskMgr::ResourceType::WORK_SCHEDULER) == 0) {
        WS_HILOGE("called with null EfficiencyResourceCallbackInfo");
        return;
    }
    auto workSchedulerServicePtr = workSchedulerService_.lock();
    if (workSchedulerServicePtr) {
        int32_t uid = resourceInfo->GetUid();
        WS_HILOGD("called, bundleName=%{public}s, uid=%{public}d, pid=%{public}d, resourcenumber=%{public}u",
            resourceInfo->GetBundleName().c_str(), uid, resourceInfo->GetPid(), resourceInfo->GetResourceNumber());
        DelayedSingleton<WorkSchedulerService>::GetInstance()->UpdateWhiteList(uid, false);
    }
}

void SchedulerBgTaskSubscriber::OnAppEfficiencyResourcesApply(
    const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo)
{
    WS_HILOGI("OnAppEfficiencyResourcesApply before");
    OnEfficiencyResourcesApply(resourceInfo);
}

void SchedulerBgTaskSubscriber::OnAppEfficiencyResourcesReset(
    const std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> &resourceInfo)
{
    WS_HILOGI("OnAppEfficiencyResourcesReset before");
    OnEfficiencyResourcesReset(resourceInfo);
}

void SchedulerBgTaskSubscriber::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    WS_HILOGE("called");
}
} // namespace Memory
} // namespace OHOS
