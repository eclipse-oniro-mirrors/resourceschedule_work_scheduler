/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifdef DEVICE_STANDBY_ENABLE
#include "work_standby_state_change_callback.h"
#include "allow_type.h"
#include "work_sched_hilog.h"
#include "work_scheduler_service.h"
#include "work_policy_manager.h"
#include "work_sched_data_manager.h"

namespace OHOS {
namespace WorkScheduler {
WorkStandbyStateChangeCallback::WorkStandbyStateChangeCallback(std::shared_ptr<WorkQueueManager>
    workQueueManager)
{
    workQueueManager_ = workQueueManager;
}

void WorkStandbyStateChangeCallback::OnDeviceIdleMode(bool napped, bool sleeping)
{
    WS_HILOGI("napped is %{public}d, sleeping is %{public}d", napped, sleeping);
    if (napped && !sleeping) {
        WS_HILOGI("Device standby state is nap, do not need process");
        return;
    }
    if (!napped && sleeping) {
        WS_HILOGI("Device standby state is sleeping");
    } else {
        // (1, 0) or (0, 0)
        WS_HILOGI("Device standby exit sleeping state");
    }
    DelayedSingleton<DataManager>::GetInstance()->SetDeviceSleep(sleeping);
    workQueueManager_->OnConditionChanged(WorkCondition::Type::STANDBY,
        std::make_shared<DetectorValue>(0, 0, sleeping, std::string()));
}

void WorkStandbyStateChangeCallback::OnAllowListChanged(int32_t uid, const std::string& name,
    uint32_t allowType, bool added)
{
    if (allowType != DevStandbyMgr::AllowType::WORK_SCHEDULER) {
        WS_HILOGE("Standby allow list changed, allowType is not WORK_SCHEDULER");
        return;
    }
    WS_HILOGI("%{public}s apply allow, added %{public}d", name.c_str(), added);
    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    dataManager->OnDeviceStandyWhitelistChanged(name, added);
    if (!dataManager->GetDeviceSleep()) {
        WS_HILOGI("current device standby state is not sleep");
        return;
    }
    auto policy = DelayedSingleton<WorkSchedulerService>::GetInstance()->GetWorkPolicyManager();
    if (!policy) {
        WS_HILOGE("Standby allow list changed callback error, WorkPolicyManager is nullptr");
        return;
    }
    if (!policy->FindWork(uid)) {
        WS_HILOGI("Standby allow list changed callback return, uid:%{public}d has no work", uid);
        return;
    }
    workQueueManager_->OnConditionChanged(WorkCondition::Type::STANDBY,
        std::make_shared<DetectorValue>(0, 0, dataManager->GetDeviceSleep(), std::string()));
}
}  // namespace WorkScheduler
}  // namespace OHOS
#endif