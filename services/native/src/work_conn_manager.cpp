/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "work_conn_manager.h"

#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include <system_ability_definition.h>

#include "ability_manager_client.h"
#include "work_sched_common.h"

using namespace std;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace WorkScheduler {
void WorkConnManager::AddConnInfo(string &workId, sptr<WorkSchedulerConnection> &connection)
{
    std::lock_guard<std::mutex> lock(connMapMutex_);
    connMap_.emplace(workId, connection);
}

void WorkConnManager::RemoveConnInfo(string &workId)
{
    std::lock_guard<std::mutex> lock(connMapMutex_);
    connMap_.erase(workId);
}

sptr<WorkSchedulerConnection> WorkConnManager::GetConnInfo(string &workId)
{
    std::lock_guard<std::mutex> lock(connMapMutex_);
    if (connMap_.count(workId) > 0) {
        return connMap_.at(workId);
    }
    return nullptr;
}

bool WorkConnManager::StartWork(shared_ptr<WorkStatus> workStatus)
{
    WS_HILOGD("StartWork, id: %{public}s, bundleName: %{public}s, abilityName: %{public}s",
        workStatus->workId_.c_str(), workStatus->bundleName_.c_str(), workStatus->abilityName_.c_str());
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        WS_HILOGI("WorkSchedulerExtension Failed to get system ability manager service.");
        return false;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        WS_HILOGI("WorkSchedulerExtension Failed to ability manager service.");
        return false;
    }
    sptr<AAFwk::IAbilityManager> abilityMgr_ = iface_cast<AAFwk::IAbilityManager>(remoteObject);
    if ((abilityMgr_ == nullptr) || (abilityMgr_->AsObject() == nullptr)) {
        WS_HILOGI("WorkSchedulerExtension Failed to get ability manager services object");
        return false;
    }

    WS_HILOGI("Begin to connect bundle:%{public}s, abilityName:%{public}s, userId:%{public}d",
        workStatus->bundleName_.c_str(), workStatus->abilityName_.c_str(), workStatus->userId_);
    sptr<WorkSchedulerConnection> connection(new (std::nothrow) WorkSchedulerConnection(workStatus->workInfo_));
    Want want;
    want.SetElementName(workStatus->bundleName_, workStatus->abilityName_);
    int ret = abilityMgr_->ConnectAbility(want, connection, nullptr, workStatus->userId_);
    if (ret != ERR_OK) {
        WS_HILOGE("connect failed");
        return false;
    }
    AddConnInfo(workStatus->workId_, connection);
    return true;
}

bool WorkConnManager::DisConnect(sptr<WorkSchedulerConnection> connect)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        WS_HILOGI("WorkSchedulerExtension Failed to get system ability manager service.");
        return false;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        WS_HILOGI("WorkSchedulerExtension Failed to ability manager service.");
        return false;
    }
    sptr<AAFwk::IAbilityManager> abilityMgr_ = iface_cast<AAFwk::IAbilityManager>(remoteObject);
    if ((abilityMgr_ == nullptr) || (abilityMgr_->AsObject() == nullptr)) {
        WS_HILOGI("WorkSchedulerExtension Failed to  get ability manager services object.");
        return false;
    }
    int ret = abilityMgr_->DisconnectAbility(connect);
    if (ret != ERR_OK) {
        WS_HILOGE("disconnect failed");
        return false;
    }
    return true;
}

bool WorkConnManager::StopWork(shared_ptr<WorkStatus> workStatus)
{
    bool ret = false;
    sptr<WorkSchedulerConnection> conn = GetConnInfo(workStatus->workId_);
    if (conn != nullptr) {
        conn->StopWork();
    } else {
        WS_HILOGD("connection is null");
    }
    ret = DisConnect(conn);
    RemoveConnInfo(workStatus->workId_);
    return ret;
}
} // namespace WorkScheduler
} // namespace OHOS