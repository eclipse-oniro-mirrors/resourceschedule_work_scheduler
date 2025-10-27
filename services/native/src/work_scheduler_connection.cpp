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

#include <unistd.h>
#include "work_scheduler_connection.h"
#include "work_sched_data_manager.h"

#include "work_sched_hilog.h"
#include "work_sched_utils.h"

namespace OHOS {
namespace WorkScheduler {
WorkSchedulerConnection::WorkSchedulerConnection(std::shared_ptr<WorkInfo> workInfo)
{
    this->workInfo_ = workInfo;
}

void WorkSchedulerConnection::StopWork()
{
    if (proxy_ == nullptr) {
        WS_HILOGE("proxy is null");
        return;
    }
    proxy_->OnWorkStop(*workInfo_);
}

void WorkSchedulerConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    proxy_ = (new (std::nothrow) WorkSchedulerProxy(remoteObject));
    if (proxy_ == nullptr) {
        WS_HILOGE("proxy is null");
        return;
    }
    sleep(1);
    if (WorkSchedUtils::IsUserMode() && workInfo_->GetDeepIdle() == WorkCondition::DeepIdle::DEEP_IDLE_IN &&
        !DelayedSingleton<DataManager>::GetInstance()->GetDeepIdle()) {
        WS_HILOGE("Exited deep idle, cancel execute OnWorkStart, bundleName:%{public}s workId = %{public}d.",
            workInfo_->GetBundleName().c_str(), workInfo_->GetWorkId());
        isConnected_.store(true);
        return;
    }
    proxy_->OnWorkStart(*workInfo_);
    WS_HILOGI("On ability connectDone, workId = %{public}d.", workInfo_->GetWorkId());
    isConnected_.store(true);
}

void WorkSchedulerConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode)
{
    WS_HILOGI("On ability disconnect done.");
}

bool WorkSchedulerConnection::IsConnected()
{
    return isConnected_.load();
}
}  // namespace WorkScheduler
}  // namespace OHOS
