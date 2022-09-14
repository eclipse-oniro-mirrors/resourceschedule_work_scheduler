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
#include "workscheduler_srv_client.h"

#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "work_sched_errors.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
WorkSchedulerSrvClient::WorkSchedulerSrvClient() {}

WorkSchedulerSrvClient::~WorkSchedulerSrvClient()
{
    if (iWorkSchedService_ != nullptr) {
        auto remoteObject = iWorkSchedService_->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

ErrCode WorkSchedulerSrvClient::Connect()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (iWorkSchedService_ != nullptr) {
        return ERR_OK;
    }
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        WS_HILOGE("GetSystemAbilityManager failed!");
        return E_CLIENT_CONNECT_SERVICE_FAILED;
    }
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(WORK_SCHEDULE_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        WS_HILOGE("GetSystemAbility failed!");
        return E_CLIENT_CONNECT_SERVICE_FAILED;
    }
    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new WorkSchedulerDeathRecipient());
    if (deathRecipient_ == nullptr) {
        WS_HILOGE("Failed to create WorkScheduelrDeathRecipient!");
        return E_CLIENT_CONNECT_SERVICE_FAILED;
    }
    if ((remoteObject_->IsProxyObject()) && (!remoteObject_->AddDeathRecipient(deathRecipient_))) {
        WS_HILOGE("Add death recipient to WorkSchedulerService failed!");
        return E_CLIENT_CONNECT_SERVICE_FAILED;
    }
    iWorkSchedService_ = iface_cast<IWorkSchedService>(remoteObject_);
    WS_HILOGD("Connecting WorkSchedService success.");
    return ERR_OK;
}

void WorkSchedulerSrvClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (iWorkSchedService_ == nullptr) {
        return;
    }
    auto serviceRemote = iWorkSchedService_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        iWorkSchedService_ = nullptr;
    }
}

void WorkSchedulerSrvClient::WorkSchedulerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        WS_HILOGE("failed, remote is nullptr");
        return;
    }
    WorkSchedulerSrvClient::GetInstance().ResetProxy(remote);
    WS_HILOGD("Work Scheduler Death Recipient Recv death notice.");
}

ErrCode WorkSchedulerSrvClient::StartWork(WorkInfo& workInfo)
{
    WS_HILOGD("Start Work");
    ErrCode ret = Connect();
    if (ret != ERR_OK) {
        WS_HILOGE("Connect() failed, errno: %{public}d", ret);
        return ret;
    }
    return iWorkSchedService_->StartWork(workInfo);
}

ErrCode WorkSchedulerSrvClient::StopWork(WorkInfo& workInfo)
{
    WS_HILOGD("Stop Work");
    ErrCode ret = Connect();
    if (ret != ERR_OK) {
        WS_HILOGE("Connect() failed, errno: %{public}d", ret);
        return ret;
    }
    return iWorkSchedService_->StopWork(workInfo);
}

ErrCode WorkSchedulerSrvClient::StopAndCancelWork(WorkInfo& workInfo)
{
    WS_HILOGD("Stop And Cancel Work");
    ErrCode ret = Connect();
    if (ret != ERR_OK) {
        WS_HILOGE("Connect() failed, errno: %{public}d", ret);
        return ret;
    }
    return iWorkSchedService_->StopAndCancelWork(workInfo);
}

ErrCode WorkSchedulerSrvClient::StopAndClearWorks()
{
    WS_HILOGD("Stop And Clear Works");
    ErrCode ret = Connect();
    if (ret != ERR_OK) {
        WS_HILOGE("Connect() failed, errno: %{public}d", ret);
        return ret;
    }
    return iWorkSchedService_->StopAndClearWorks();
}

ErrCode WorkSchedulerSrvClient::IsLastWorkTimeout(int32_t workId, bool &result)
{
    WS_HILOGD("Is LastWork Timeout");
    ErrCode errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }
    errCode = iWorkSchedService_->IsLastWorkTimeout(workId);
    if (errCode == 1) {
        result = true;
    } else {
        result = false;
    }
    return errCode;
}

ErrCode WorkSchedulerSrvClient::ObtainAllWorks(std::list<std::shared_ptr<WorkInfo>> &workInfos)
{
    WS_HILOGD("Obtain All Works");
    ErrCode errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
    workInfos = iWorkSchedService_->ObtainAllWorks(uid, pid);
    return ERR_OK;
}

ErrCode WorkSchedulerSrvClient::GetWorkStatus(int32_t workId, std::shared_ptr<WorkInfo> &workInfo)
{
    WS_HILOGD("Get Work Status");
    if (workId <= 0) {
        return E_WORK_ID_INVALID;
    }
    ErrCode code = Connect();
    if (code != ERR_OK) {
        return code;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    workInfo = iWorkSchedService_->GetWorkStatus(uid, workId);
    if (workInfo == nullptr) {
        return E_GET_WORK_STATUS_ERROR;
    }
    return ERR_OK;
}
} // namespace WorkScheduler
} // namespace OHOS