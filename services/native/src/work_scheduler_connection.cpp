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
#include <string>
#include "work_scheduler_connection.h"
#include "work_sched_data_manager.h"

#include "work_sched_hilog.h"
#include "work_sched_utils.h"
#include "work_scheduler_service.h"
#include "work_policy_manager.h"

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
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    std::shared_ptr<WorkStatus> workStatus = service->GetWorkPolicyManager()->FindWorkStatus(*workInfo_,
        workInfo_->GetUid());
    // idle类型任务满足触发条件后任务拉起，此时延迟任务连接回调还未完成，用户解锁屏幕任务停止失败，当延迟任务连接回调完成时需停止任务
    if (WorkSchedUtils::IsUserMode() && workInfo_->GetDeepIdle() == WorkCondition::DeepIdle::DEEP_IDLE_IN &&
        !DelayedSingleton<DataManager>::GetInstance()->GetDeepIdle() &&
        workStatus != nullptr && !workStatus->IsDebugTask()) {
        WS_HILOGE("Exited deep idle, cancel execute OnWorkStart, bundleName:%{public}s workId = %{public}d.",
            workInfo_->GetBundleName().c_str(), workInfo_->GetWorkId());
        isConnected_.store(true);
        service->StopWorkInner(workStatus, workInfo_->GetUid(), false, false);
        return;
    }
    proxy_->OnWorkStart(*workInfo_);
    WS_HILOGI("On ability connectDone, workId = %{public}d.", workInfo_->GetWorkId());
    isConnected_.store(true);
    // 调试命令拉起的其他任务重置debugTask_
    if (workStatus != nullptr && workStatus->IsDebugTask()) {
        workStatus->SetDebugTask(false);
    }
}

void WorkSchedulerConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode)
{
    WS_HILOGI("On ability disconnect done.");
    if (workInfo_ == nullptr) {
        return;
    }
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    std::shared_ptr<WorkStatus> workStatus = service->GetWorkPolicyManager()->FindWorkStatus(*workInfo_,
        workInfo_->GetUid());
    std::string workId = "u" + std::to_string(workInfo_->GetUid()) + "_" + std::to_string(workInfo_->GetWorkId());
    if (service->CheckPreinstalledWorkId(workId)) {
        service->RemovePreinstalledWorkId(workId);
        service->RemovePreinstalledBundles(workInfo_->GetBundleName());
        service->StopWorkInner(workStatus, workInfo_->GetUid(), true, false);
        if (workInfo_->IsPersisted()) {
            service->RemovePersistedMap(workId);
        }
    }
}

bool WorkSchedulerConnection::IsConnected()
{
    return isConnected_.load();
}
}  // namespace WorkScheduler
}  // namespace OHOS
