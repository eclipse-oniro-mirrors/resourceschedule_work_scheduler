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

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
#include "work_bundle_group_change_callback.h"
#include "work_sched_hilog.h"
#include "work_sched_data_manager.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"

namespace OHOS {
namespace WorkScheduler {
WorkBundleGroupChangeCallback::WorkBundleGroupChangeCallback(std::shared_ptr<WorkQueueManager>
    workQueueManager)
{
    workQueueManager_ = workQueueManager;
}

void WorkBundleGroupChangeCallback::OnAppGroupChanged(
    const DeviceUsageStats::AppGroupCallbackInfo &appGroupCallbackInfo)
{
    int32_t newGroup = appGroupCallbackInfo.GetNewGroup();
    int32_t oldGroup = appGroupCallbackInfo.GetOldGroup();
    int32_t userId = appGroupCallbackInfo.GetUserId();
    std::string bundleName = appGroupCallbackInfo.GetBundleName();
    WS_HILOGI("Bundle group changed, from %{public}d to %{public}d with userId = %{public}d, bundleName = %{public}s",
        oldGroup, newGroup, userId, bundleName.c_str());
    DelayedSingleton<DataManager>::GetInstance()->AddGroup(bundleName, userId, newGroup);
    auto policy = DelayedSingleton<WorkSchedulerService>::GetInstance()->GetWorkPolicyManager();
    if (!policy) {
        WS_HILOGE("OnAppGroupChanged callback error, WorkPolicyManager is nullptr");
        return;
    }
    if (!policy->FindWork(userId, bundleName)) {
        WS_HILOGE("OnAppGroupChanged no work found, bundleName = %{public}s", bundleName.c_str());
        return;
    }
    if (newGroup < oldGroup) {
        if (!workQueueManager_) {
            WS_HILOGE("WorkBundleGroupChangeCallback::OnAppGroupChanged workQueueManger_ is nullptr");
            return;
        }
        workQueueManager_->OnConditionChanged(WorkCondition::Type::GROUP,
            std::make_shared<DetectorValue>(newGroup, userId, true, bundleName));
    }
}
}  // namespace WorkScheduler
}  // namespace OHOS
#endif