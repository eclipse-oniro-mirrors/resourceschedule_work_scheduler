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
#include "work_sched_hilog.h"

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
    if (napped && !isSleep_) {
        WS_HILOGI("device_standby state is nap, do not need process");
        return;
    }
    workQueueManager_->OnConditionChanged(WorkCondition::Type::STANDBY,
        std::make_shared<DetectorValue>(0, 0, sleeping, std::string()));
    isSleep_ = sleeping;
}

void WorkStandbyStateChangeCallback::OnAllowListChanged(int32_t uid, const std::string& name,
    uint32_t allowType, bool added)
{
    WS_HILOGD("%{public}s apply allow, added type is %{public}d", name.c_str(), added);
    if (!isSleep_) {
        WS_HILOGD("current device_standby state is not sleep");
        return;
    }
    workQueueManager_->OnConditionChanged(WorkCondition::Type::STANDBY,
        std::make_shared<DetectorValue>(0, 0, isSleep_, std::string()));
}
}  // namespace WorkScheduler
}  // namespace OHOS
#endif