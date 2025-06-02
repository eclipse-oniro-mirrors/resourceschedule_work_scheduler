/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

WorkSchedulerSrvClient::~WorkSchedulerSrvClient() {}

ErrCode WorkSchedulerSrvClient::Connect()
{
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
    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new WorkSchedulerDeathRecipient(*this));
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

void WorkSchedulerSrvClient::ResetProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (iWorkSchedService_ != nullptr && (iWorkSchedService_->AsObject() != nullptr)) {
        iWorkSchedService_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    iWorkSchedService_ = nullptr;
}

WorkSchedulerSrvClient::WorkSchedulerDeathRecipient::WorkSchedulerDeathRecipient(
    WorkSchedulerSrvClient &workSchedulerSrvClient) : workSchedulerSrvClient_(workSchedulerSrvClient) {}

void WorkSchedulerSrvClient::WorkSchedulerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    WS_HILOGD("Work Scheduler Death Recipient Recv death notice.");
    workSchedulerSrvClient_.ResetProxy();
}

ErrCode WorkSchedulerSrvClient::StartWork(WorkInfo& workInfo)
{
    WS_HILOGD("Start Work");
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode ret = Connect();
    if (ret != ERR_OK) {
        WS_HILOGE("Connect() failed, errno: %{public}d", ret);
        return ret;
    }
    return iWorkSchedService_->StartWork(workInfo);
}

ErrCode WorkSchedulerSrvClient::StartWorkForInner(WorkInfo& workInfo)
{
    WS_HILOGD("start work for inner");
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode ret = Connect();
    if (ret != ERR_OK) {
        WS_HILOGE("Connect() failed, errno: %{public}d", ret);
        return ret;
    }
    return iWorkSchedService_->StartWorkForInner(workInfo);
}

ErrCode WorkSchedulerSrvClient::StopWork(WorkInfo& workInfo)
{
    WS_HILOGD("Stop Work");
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode ret = Connect();
    if (ret != ERR_OK) {
        WS_HILOGE("Connect() failed, errno: %{public}d", ret);
        return ret;
    }
    return iWorkSchedService_->StopWork(workInfo);
}

ErrCode WorkSchedulerSrvClient::StopWorkForInner(WorkInfo& workInfo, bool needCancel)
{
    WS_HILOGD("stop work for inner");
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode ret = Connect();
    if (ret != ERR_OK) {
        WS_HILOGE("Connect() failed, errno: %{public}d", ret);
        return ret;
    }
    return iWorkSchedService_->StopWorkForInner(workInfo, needCancel);
}

ErrCode WorkSchedulerSrvClient::StopAndCancelWork(WorkInfo& workInfo)
{
    WS_HILOGD("Stop And Cancel Work");
    std::lock_guard<std::mutex> lock(mutex_);
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
    std::lock_guard<std::mutex> lock(mutex_);
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
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }
    errCode = iWorkSchedService_->IsLastWorkTimeout(workId, result);
    return errCode;
}

ErrCode WorkSchedulerSrvClient::ObtainAllWorks(std::list<std::shared_ptr<WorkInfo>> &workInfos)
{
    WS_HILOGD("Obtain All Works");
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }
    std::vector<WorkInfo> vectorWorkInfos;
    ErrCode ret = iWorkSchedService_->ObtainAllWorks(vectorWorkInfos);
    for (const auto& workInfo : vectorWorkInfos) {
        workInfos.push_back(std::make_shared<WorkInfo>(workInfo));
    }
    return ret;
}

ErrCode WorkSchedulerSrvClient::ObtainWorksByUidAndWorkIdForInner(int32_t uid,
    std::list<std::shared_ptr<WorkInfo>> &workInfos, int32_t workId)
{
    WS_HILOGD("Obtain Works By uid and workId for inner");
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode errCode = Connect();
    if (errCode != ERR_OK) {
        return errCode;
    }
    if (uid < 0) {
        WS_HILOGE("param uid: %{public}d invaild.", uid);
        return E_PARAM_INVAILD_UID;
    }
    std::vector<WorkInfo> vectorWorkInfos;
    ErrCode ret = iWorkSchedService_->ObtainWorksByUidAndWorkIdForInner(uid, vectorWorkInfos, workId);
    for (const auto& workInfo : vectorWorkInfos) {
        workInfos.push_back(std::make_shared<WorkInfo>(workInfo));
    }
    return ret;
}

ErrCode WorkSchedulerSrvClient::GetWorkStatus(int32_t workId, std::shared_ptr<WorkInfo> &workInfo)
{
    WS_HILOGD("Get Work Status");
    if (workId <= 0) {
        return E_WORKID_ERR;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode code = Connect();
    if (code != ERR_OK) {
        return code;
    }
    WorkInfo workInfoTemp;
    ErrCode ret = iWorkSchedService_->GetWorkStatus(workId, workInfoTemp);
    if (ret == ERR_OK) {
        workInfo = std::make_shared<WorkInfo>(workInfoTemp);
    }
    return ret;
}

ErrCode WorkSchedulerSrvClient::GetAllRunningWorks(std::list<std::shared_ptr<WorkInfo>>& workInfos)
{
    WS_HILOGD("Get Running Work Scheduler Work");
    if (!workInfos.empty()) {
        return E_PARAM_ERROR;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode code = Connect();
    if (code != ERR_OK) {
        return code;
    }
    std::vector<WorkInfo> vectorWorkInfos;
    ErrCode ret = iWorkSchedService_->GetAllRunningWorks(vectorWorkInfos);
    for (const auto& workInfo : vectorWorkInfos) {
        workInfos.push_back(std::make_shared<WorkInfo>(workInfo));
    }
    return ret;
}

ErrCode WorkSchedulerSrvClient::PauseRunningWorks(int32_t uid)
{
    WS_HILOGD("Pause Running Work Scheduler Work, uid:%{public}d", uid);
    if (uid < 0) {
        return E_PARAM_ERROR;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode code = Connect();
    if (code != ERR_OK) {
        return code;
    }
    return iWorkSchedService_->PauseRunningWorks(uid);
}

ErrCode WorkSchedulerSrvClient::ResumePausedWorks(int32_t uid)
{
    WS_HILOGD("Resume Paused Work Scheduler Work, uid:%{public}d", uid);
    if (uid < 0) {
        return E_PARAM_ERROR;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode code = Connect();
    if (code != ERR_OK) {
        return code;
    }
    return iWorkSchedService_->ResumePausedWorks(uid);
}

ErrCode WorkSchedulerSrvClient::SetWorkSchedulerConfig(const std::string &configData, int32_t sourceType)
{
    WS_HILOGD("Set work scheduler config");
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode code = Connect();
    if (code != ERR_OK) {
        return code;
    }
    return iWorkSchedService_->SetWorkSchedulerConfig(configData, sourceType);
}

ErrCode WorkSchedulerSrvClient::StopWorkForSA(int32_t saId)
{
    WS_HILOGD("Stop SA");
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode code = Connect();
    if (code != ERR_OK) {
        return code;
    }
    return iWorkSchedService_->StopWorkForSA(saId);
}
} // namespace WorkScheduler
} // namespace OHOS