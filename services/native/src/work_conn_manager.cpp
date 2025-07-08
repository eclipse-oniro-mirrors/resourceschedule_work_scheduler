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
#include "work_conn_manager.h"

#include <hisysevent.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include <system_ability_definition.h>

#include "ability_manager_client.h"
#include "ability_manager_proxy.h"
#include "work_sched_hilog.h"
#include "work_sched_utils.h"
#include "errors.h"
#include <cinttypes>
#include "work_sched_constants.h"
#include "work_sched_hisysevent_report.h"

#ifdef DEVICE_STANDBY_ENABLE
#include "standby_service_client.h"
#endif // DEVICE_STANDBY_ENABLE

using namespace std;
using namespace OHOS::AAFwk;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace WorkScheduler {
const std::string PARAM_APP_CLONE_INDEX_KEY = "ohos.extra.param.key.appCloneIndex";

void WorkConnManager::AddConnInfo(string &workId, sptr<WorkSchedulerConnection> &connection)
{
    std::lock_guard<ffrt::mutex> lock(connMapMutex_);
    connMap_.emplace(workId, connection);
}

void WorkConnManager::RemoveConnInfo(string &workId)
{
    std::lock_guard<ffrt::mutex> lock(connMapMutex_);
    connMap_.erase(workId);
}

sptr<WorkSchedulerConnection> WorkConnManager::GetConnInfo(string &workId)
{
    std::lock_guard<ffrt::mutex> lock(connMapMutex_);
    if (connMap_.count(workId) > 0) {
        return connMap_.at(workId);
    }
    return nullptr;
}

bool WorkConnManager::StartWork(shared_ptr<WorkStatus> workStatus)
{
    sptr<WorkSchedulerConnection> conn = GetConnInfo(workStatus->workId_);
    if (conn) {
        WS_HILOGE("Work has started with id: %{public}s, bundleName: %{public}s, abilityName: %{public}s",
            workStatus->workId_.c_str(), workStatus->bundleName_.c_str(), workStatus->abilityName_.c_str());
        WorkSchedUtil::HiSysEventException(EventErrorCode::CONNECT_ABILITY, "connect info has existed, connect failed");
        RemoveConnInfo(workStatus->workId_);
        if (conn->IsConnected()) {
            conn->StopWork();
            DisConnect(conn);
        }
    }

    if (!workStatus->workInfo_->GetExtension()) {
        WS_HILOGE("%{public}s extension's type is not workScheduler, connect failed", workStatus->bundleName_.c_str());
        WorkSchedUtil::HiSysEventException(EventErrorCode::CONNECT_ABILITY,
            "app extension's type is not workScheduler");
        return false;
    }

    WS_HILOGI("Begin to connect bundle:%{public}s, abilityName:%{public}s, workId:%{public}s",
        workStatus->bundleName_.c_str(), workStatus->abilityName_.c_str(), workStatus->workId_.c_str());
    sptr<AAFwk::IAbilityManager> abilityMgr_ = GetSystemAbilityManager(CONNECT_ABILITY);
    if (abilityMgr_ == nullptr) {
        return false;
    }
    sptr<WorkSchedulerConnection> connection(new (std::nothrow) WorkSchedulerConnection(workStatus->workInfo_));
    if (connection == nullptr) {
        WS_HILOGE("Failed to new connection.");
        WorkSchedUtil::HiSysEventException(EventErrorCode::CONNECT_ABILITY, "create connection failed");
        return false;
    }

    Want want;
    want.SetElementName(workStatus->bundleName_, workStatus->abilityName_);
    want.SetParam(PARAM_APP_CLONE_INDEX_KEY, workStatus->workInfo_->GetAppIndex());
    int32_t ret = abilityMgr_->ConnectAbility(want, connection, nullptr, workStatus->userId_);
    if (ret != ERR_OK) {
        WS_HILOGE("connect failed, ret: %{public}d", ret);
        WorkSchedUtil::HiSysEventException(EventErrorCode::CONNECT_ABILITY, "connect system ability failed");
        return false;
    }
    AddConnInfo(workStatus->workId_, connection);

    // Notify work add event to battery statistics
    WriteStartWorkEvent(workStatus);

    return true;
}

bool WorkConnManager::DisConnect(sptr<WorkSchedulerConnection> connect)
{
    sptr<AAFwk::IAbilityManager> abilityMgr_ = GetSystemAbilityManager(DISCONNECT_ABILITY);
    if (abilityMgr_ == nullptr) {
        return false;
    }
    int32_t ret = abilityMgr_->DisconnectAbility(connect);
    if (ret != ERR_OK) {
        WS_HILOGE("disconnect failed, ret: %{public}d", ret);
        WorkSchedUtil::HiSysEventException(EventErrorCode::DISCONNECT_ABILITY, "disconnect system ability failed");
        return false;
    }
    return true;
}

bool WorkConnManager::StopWork(shared_ptr<WorkStatus> workStatus, bool isTimeOut)
{
    bool ret = false;
    sptr<WorkSchedulerConnection> conn = GetConnInfo(workStatus->workId_);
    if (!conn) {
        WS_HILOGE("%{public}s %{public}d connection is null", workStatus->workId_.c_str(), isTimeOut);
        return false;
    }
    if (!conn->IsConnected()) {
        WS_HILOGE("%{public}s %{public}d is not connected, work will be stopped  by timeout",
            workStatus->workId_.c_str(), isTimeOut);
        return false;
    }
    conn->StopWork();
    ret = DisConnect(conn);

    RemoveConnInfo(workStatus->workId_);

    // Notify work remove event to battery statistics only work has started
    if (ret) {
        int32_t pid = IPCSkeleton::GetCallingPid();
        workStatus->duration_ += WorkSchedUtils::GetCurrentTimeMs() - workStatus->workStartTime_;
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER, "WORK_STOP",
            HiSysEvent::EventType::STATISTIC, "UID",
            workStatus->uid_, "PID", pid, "NAME", workStatus->bundleName_, "WORKID", workStatus->workId_,
            "REASON", isTimeOut, "DURATION", workStatus->duration_);
#ifdef DEVICE_STANDBY_ENABLE
        WS_HILOGI("OnWorkStop uid: %{public}d, duration:%{public}" PRIu64 ", startTime:%{public}" PRIu64,
            workStatus->uid_, workStatus->duration_, workStatus->workStartTime_);
        DevStandbyMgr::StandbyServiceClient::GetInstance().ReportWorkSchedulerStatus(false,
            workStatus->uid_, workStatus->bundleName_);
#endif // DEVICE_STANDBY_ENABLE
    }
    return ret;
}

void WorkConnManager::WriteStartWorkEvent(shared_ptr<WorkStatus> workStatus)
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    string conditions = "";
    if (workStatus->workInfo_->GetConditionMap()->count(WorkCondition::Type::NETWORK) > 0) {
        conditions.append("NETWORK-");
    }

    if (workStatus->workInfo_->GetConditionMap()->count(WorkCondition::Type::CHARGER) > 0) {
        conditions.append("CHARGER-");
    }

    if (workStatus->workInfo_->GetConditionMap()->count(WorkCondition::Type::BATTERY_STATUS) > 0) {
        conditions.append("BATTERY_STATUS-");
    }

    if (workStatus->workInfo_->GetConditionMap()->count(WorkCondition::Type::BATTERY_LEVEL) > 0) {
        conditions.append("BATTERY_LEVEL-");
    }

    if (workStatus->workInfo_->GetConditionMap()->count(WorkCondition::Type::STORAGE) > 0) {
        conditions.append("STORAGE-");
    }

    if (workStatus->workInfo_->GetConditionMap()->count(WorkCondition::Type::TIMER) > 0) {
        conditions.append("TIMER-");
    }

    if (workStatus->workInfo_->GetConditionMap()->count(WorkCondition::Type::DEEP_IDLE) > 0) {
        conditions.append("DEEP_IDLE-");
    }
    conditions.pop_back();

    string type = "Repeat";
    if (!workStatus->workInfo_->IsRepeat()) {
        type = "Not Repeat";
    }
    if (!workStatus->workInfo_->GetIsInnerApply()) {
        type = "inner apply" + type;
    }

    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER, "WORK_START",
        HiSysEvent::EventType::STATISTIC, "UID",
        workStatus->uid_, "PID", pid, "NAME", workStatus->bundleName_, "WORKID", workStatus->workId_, "TRIGGER",
        conditions, "TYPE", type, "INTERVAL", workStatus->workInfo_->GetTimeInterval(),
        "DELAY_REASON", workStatus->delayReason_);
#ifdef DEVICE_STANDBY_ENABLE
    WS_HILOGI("OnWorkStart uid: %{public}d", workStatus->uid_);
    DevStandbyMgr::StandbyServiceClient::GetInstance().ReportWorkSchedulerStatus(true,
        workStatus->uid_, workStatus->bundleName_);
#endif // DEVICE_STANDBY_ENABLE
}

sptr<AAFwk::IAbilityManager> WorkConnManager::GetSystemAbilityManager(int32_t errCode)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        WS_HILOGE("Failed to get system ability manager service.");
        WorkSchedUtil::HiSysEventException(errCode, "get system ability manager failed");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        WS_HILOGE("Failed to get system ability.");
        WorkSchedUtil::HiSysEventException(errCode, "get system ability failed");
        return nullptr;
    }
    sptr<AAFwk::IAbilityManager> abilityMgr_ = iface_cast<AAFwk::IAbilityManager>(remoteObject);
    if ((abilityMgr_ == nullptr) || (abilityMgr_->AsObject() == nullptr)) {
        WS_HILOGE("Failed to get ability manager services object");
        WorkSchedUtil::HiSysEventException(errCode, "cast system ability failed");
        return nullptr;
    }
    return abilityMgr_;
}
} // namespace WorkScheduler
} // namespace OHOS
