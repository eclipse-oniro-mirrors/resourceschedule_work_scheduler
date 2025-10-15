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

#include "work_policy_manager.h"
#include "work_sched_constants.h"

#include <string>
#include <hisysevent.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>
#include "parameters.h"
#include "policy/app_data_clear_listener.h"
#include "work_scheduler_service.h"
#include "work_event_handler.h"
#include "work_sched_hilog.h"
#include "work_sched_errors.h"
#include "work_sched_utils.h"
#include "watchdog.h"
#include "work_sched_data_manager.h"
#include "work_sched_hisysevent_report.h"
#include <cinttypes>

using namespace std;
using namespace OHOS::AppExecFwk;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace WorkScheduler {

WorkPolicyManager::WorkPolicyManager(const std::shared_ptr<WorkSchedulerService>& wss) : wss_(wss)
{
    conditionReadyQueue_ = std::make_shared<WorkQueue>();
    watchdogId_ = INIT_WATCHDOG_ID;
    dumpSetMemory_ = INIT_DUMP_SET_MEMORY;
    watchdogTime_ = WATCHDOG_TIME;
    dumpSetCpu_ = INIT_DUMP_SET_CPU;
    dumpSetMaxRunningCount_ = INVALID_VALUE;
}

bool WorkPolicyManager::Init(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    WS_HILOGD("Work policy manager init.");
    if (wss_.expired()) {
        WS_HILOGE("wss_ expired");
        return false;
    }
    workConnManager_ = make_shared<WorkConnManager>();
    handler_ = wss_.lock()->GetHandler();
    if (handler_ == nullptr) {
        WS_HILOGE("failed due to handler_ is nullptr");
        return false;
    }
    watchdog_ = std::make_shared<Watchdog>(wss_.lock()->GetWorkPolicyManager(), runner);
    return true;
}

void WorkPolicyManager::AddPolicyFilter(shared_ptr<IPolicyFilter> filter)
{
    policyFilters_.emplace_back(filter);
}

void WorkPolicyManager::AddAppDataClearListener(std::shared_ptr<AppDataClearListener> listener)
{
    appDataClearListener_ = listener;
    appDataClearListener_->Start();
}


std::string WorkPolicyManager::GetConditionString(const shared_ptr<WorkStatus> workStatus)
{
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
    return conditions;
}

int32_t WorkPolicyManager::AddWork(shared_ptr<WorkStatus> workStatus, int32_t uid)
{
    WS_HILOGD("Add work");
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        if (uidQueueMap_.at(uid)->Contains(make_shared<string>(workStatus->workId_))) {
            WS_HILOGD("Workid has been added, should remove first.");
            return E_ADD_REPEAT_WORK_ERR;
        } else if (uidQueueMap_.at(uid)->GetSize() >= MAX_WORK_COUNT_PER_UID) {
            WS_HILOGE("each uid only can be added %{public}u works", MAX_WORK_COUNT_PER_UID);
            return E_WORK_EXCEED_UPPER_LIMIT;
        }
        uidQueueMap_.at(uid)->Push(workStatus);
    } else {
        WS_HILOGD("uidQueue(%{public}d) not exists, create", uid);
        uidQueueMap_.emplace(uid, make_shared<WorkQueue>());
        uidQueueMap_.at(uid)->Push(workStatus);
    }

    // Notify work add event to battery statistics
    int32_t pid = IPCSkeleton::GetCallingPid();
    string type = "Repeat";
    if (!workStatus->workInfo_->IsRepeat()) {
        type = "Not Repeat";
    }

    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        "WORK_ADD", HiSysEvent::EventType::STATISTIC, "UID", uid, "PID", pid,
        "NAME", workStatus->bundleName_, "WORKID", workStatus->workId_, "TRIGGER", GetConditionString(workStatus),
        "TYPE", type, "INTERVAL", workStatus->workInfo_->GetTimeInterval());

    WS_HILOGI("push workStatus ID: %{public}s to uidQueue(%{public}d)", workStatus->workId_.c_str(), uid);
    return ERR_OK;
}

bool WorkPolicyManager::RemoveWork(shared_ptr<WorkStatus> workStatus, int32_t uid)
{
    WS_HILOGD("Remove work.");
    bool ret = false;
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        WS_HILOGD("Remove workStatus ID: %{public}s form uidQueue(%{public}d)", workStatus->workId_.c_str(), uid);
        ret = uidQueueMap_.at(uid)->Remove(workStatus);
        if (uidQueueMap_.count(uid) <= 0) {
            uidQueueMap_.erase(uid);
        }
    }
    return ret;
}

shared_ptr<WorkStatus> WorkPolicyManager::FindWorkStatus(WorkInfo& workInfo, int32_t uid)
{
    WS_HILOGD("Find work status start.");
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        return uidQueueMap_.at(uid)->Find(WorkStatus::MakeWorkId(workInfo.GetWorkId(), uid));
    }
    return nullptr;
}

shared_ptr<WorkStatus> WorkPolicyManager::FindWorkStatus(int32_t uId, int32_t workId)
{
    WS_HILOGD("Find work status start.");
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uId) > 0) {
        return uidQueueMap_.at(uId)->Find(WorkStatus::MakeWorkId(workId, uId));
    }
    return nullptr;
}

shared_ptr<WorkStatus> WorkPolicyManager::FindSA(int32_t saId, int32_t uid)
{
    WS_HILOGD("Find SA, saId:%{public}d, uid:%{public}d", saId, uid);
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        return uidQueueMap_.at(uid)->FindSA(saId);
    }
    return nullptr;
}

void WorkPolicyManager::RemoveFromUidQueue(std::shared_ptr<WorkStatus> workStatus, int32_t uid)
{
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        uidQueueMap_.at(uid)->CancelWork(workStatus);
        if (uidQueueMap_.at(uid)->GetSize() <= 0) {
            uidQueueMap_.erase(uid);
        }
    }
}

void WorkPolicyManager::RemoveFromReadyQueue(std::shared_ptr<WorkStatus> workStatus)
{
    conditionReadyQueue_->RemoveUnReady();
}

bool WorkPolicyManager::StopWork(std::shared_ptr<WorkStatus> workStatus, int32_t uid,
    const bool needCancel, bool isTimeOut)
{
    WS_HILOGD("enter");
    bool hasCanceled = false;
    if (workStatus->IsRunning()) {
        workStatus->lastTimeout_ = isTimeOut;
        if (!workConnManager_->StopWork(workStatus, isTimeOut)) {
            return hasCanceled;
        }
        if (!workStatus->IsRepeating()) {
            workStatus->MarkStatus(WorkStatus::Status::REMOVED);
            RemoveFromUidQueue(workStatus, uid);
            RemoveFromReadyQueue(workStatus);
            hasCanceled = true;
        } else {
            workStatus->workStartTime_ = 0;
            workStatus->workWatchDogTime_ = 0;
            workStatus->duration_ = 0;
            workStatus->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
        }
    }

    if (!hasCanceled && needCancel) {
        RemoveFromUidQueue(workStatus, uid);
        RemoveFromReadyQueue(workStatus);
        hasCanceled = true;
    }
    if (isTimeOut && (workStatus->GetStatus() == WorkStatus::Status::REMOVED)) {
        WS_HILOGI("disconect %{public}s when timeout", workStatus->workId_.c_str());
        workStatus->lastTimeout_ = isTimeOut;
        workConnManager_->StopWork(workStatus, isTimeOut);
    }
    CheckWorkToRun();
    return hasCanceled;
}

bool WorkPolicyManager::StopAndClearWorks(int32_t uid)
{
    WS_HILOGD("enter");
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    auto iter = uidQueueMap_.find(uid);
    if (iter != uidQueueMap_.end()) {
        auto queue = iter->second;
        if (!queue) {
            WS_HILOGE("StopAndClearWorks failed, queue is nullptr");
            return false;
        }
        for (auto it : queue->GetWorkList()) {
            workConnManager_->StopWork(it, false);
            it->MarkStatus(WorkStatus::Status::REMOVED);
            RemoveFromReadyQueue(it);
        }
        queue->ClearAll();
        uidQueueMap_.erase(uid);
    }
    CheckWorkToRun();
    return true;
}

int32_t WorkPolicyManager::IsLastWorkTimeout(int32_t workId, int32_t uid, bool &result)
{
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    string workIdStr = WorkStatus::MakeWorkId(workId, uid);
    if (uidQueueMap_.count(uid) > 0) {
        auto queue = uidQueueMap_.at(uid);
        if (!queue) {
            WS_HILOGE("IsLastWorkTimeout failed, queue is nullptr");
            return E_WORK_NOT_EXIST_FAILED;
        }
        shared_ptr<WorkStatus> workStatus = queue->Find(workIdStr);
        if (workStatus != nullptr) {
            return workStatus->IsLastWorkTimeout();
        }
    }
    return E_WORK_NOT_EXIST_FAILED;
}

void WorkPolicyManager::OnConditionReady(shared_ptr<vector<shared_ptr<WorkStatus>>> workStatusVector)
{
    WS_HILOGD("enter");
    if (workStatusVector == nullptr) {
        return;
    }
    AddToReadyQueue(workStatusVector);
    CheckWorkToRun();
}

void WorkPolicyManager::AddToReadyQueue(shared_ptr<vector<shared_ptr<WorkStatus>>> workStatusVector)
{
    conditionReadyQueue_->Push(workStatusVector);
}

int32_t WorkPolicyManager::GetMaxRunningCount(WorkSchedSystemPolicy& systemPolicy)
{
    int32_t currentMaxRunning = GetDumpSetMaxRunningCount();
    if (currentMaxRunning > 0 && currentMaxRunning <= DUMP_SET_MAX_COUNT_LIMIT) {
        return currentMaxRunning;
    }
    currentMaxRunning = MAX_RUNNING_COUNT;
    for (auto policyFilter : policyFilters_) {
        int32_t policyMaxRunning = policyFilter->GetPolicyMaxRunning(systemPolicy);
        if (policyMaxRunning < currentMaxRunning) {
            currentMaxRunning = policyMaxRunning;
        }
    }
    return currentMaxRunning;
}

int32_t WorkPolicyManager::GetRunningCount()
{
    WS_HILOGD("enter");
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    int32_t count = 0;
    auto it = uidQueueMap_.begin();
    while (it != uidQueueMap_.end()) {
        count += it->second->GetRunningCount();
        it++;
    }
    return count;
}

void WorkPolicyManager::OnPolicyChanged(PolicyType policyType, shared_ptr<DetectorValue> detectorVal)
{
    WS_HILOGD("enter");
    if (wss_.expired()) {
        WS_HILOGE("wss_ expired");
        return;
    }
    auto service = wss_.lock();
    if (!service) {
        WS_HILOGE("service is null");
        return;
    }
    switch (policyType) {
        case PolicyType::USER_SWITCHED: {
            service->InitPreinstalledWork();
            break;
        }
        case PolicyType::APP_ADDED: {
            if (!service->IsPreinstalledBundle(detectorVal->strVal)) {
                return;
            }
            service->InitPreinstalledWork();
            break;
        }
        case PolicyType::APP_REMOVED: {
            int32_t uid = detectorVal->intVal;
            WorkStatus::ClearUidLastTimeMap(uid);
            service->StopAndClearWorksByUid(detectorVal->intVal);
            int32_t userId = WorkSchedUtils::GetUserIdByUid(uid);
            DelayedSingleton<DataManager>::GetInstance()->ClearGroup(detectorVal->strVal, userId);
            break;
        }
        default: {}
    }
    CheckWorkToRun();
}

bool WorkPolicyManager::IsSpecialScene(std::shared_ptr<WorkStatus> topWork, int32_t runningCount)
{
    if (OHOS::system::GetIntParameter("const.debuggable", 0) == 1 &&
        wss_.lock()->IsExemptionBundle(topWork->bundleName_)) {
        return true;
    }
    if (DelayedSingleton<DataManager>::GetInstance()->IsInDeviceStandyRestrictlist(topWork->bundleName_)) {
        return false;
    }
    if (DelayedSingleton<DataManager>::GetInstance()->GetDeviceSleep() &&
        runningCount < STANDBY_MAX_RUNNING_COUNT &&
        DelayedSingleton<DataManager>::GetInstance()->IsInDeviceStandyWhitelist(topWork->bundleName_)) {
        WS_HILOGI("device is in standy mode, and work %{public}s is in whitelist, allow to run",
            topWork->bundleName_.c_str());
        return true;
    }
    return false;
}

void WorkPolicyManager::CheckWorkToRun()
{
    WS_HILOGD("Check work to run.");
    if (wss_.lock() == nullptr) {
        WS_HILOGE("wss_ expired");
        return;
    }
    RemoveAllUnReady();
    if (handler_ == nullptr) {
        WS_HILOGE("handler lock() returns nullptr");
        return;
    }
    handler_->RemoveEvent(WorkEventHandler::RETRIGGER_MSG);
    shared_ptr<WorkStatus> topWork = GetWorkToRun();
    if (topWork == nullptr) {
        WS_HILOGD("no condition ready work not running, return.");
        return;
    }
    WorkSchedSystemPolicy systemPolicy;
    int32_t runningCount = GetRunningCount();
    int32_t allowRunningCount = GetMaxRunningCount(systemPolicy);
    if (HasSystemPolicyEventSend() && allowRunningCount == MAX_RUNNING_COUNT && runningCount < MAX_RUNNING_COUNT) {
        SetSystemPolicyEventSend(false);
        WorkSchedUtil::HiSysEventSystemPolicyLimit(systemPolicy);
    }
    if (runningCount < allowRunningCount || IsSpecialScene(topWork, runningCount)) {
        if (topWork->workInfo_->IsSA()) {
            RealStartSA(topWork);
        } else {
            RealStartWork(topWork);
        }
        SendRetrigger(DELAY_TIME_SHORT);
    } else {
        if (runningCount == MAX_RUNNING_COUNT) {
            systemPolicy.policyName = "OVER_LIMIT";
        }

        if (!HasSystemPolicyEventSend() && !systemPolicy.policyName.empty()) {
            topWork->delayReason_= systemPolicy.policyName;
            WS_HILOGI("trigger delay, reason:%{public}s, runningCount:%{public}d allowRunningCount:%{public}d,"
                "bundleName:%{public}s, workId:%{public}s", systemPolicy.GetInfo().c_str(), runningCount,
                allowRunningCount, topWork->bundleName_.c_str(), topWork->workId_.c_str());
            WorkSchedUtil::HiSysEventSystemPolicyLimit(systemPolicy);
            SetSystemPolicyEventSend(true);
        }
        SendRetrigger(DELAY_TIME_LONG);
    }
    WS_HILOGD("out");
}

void WorkPolicyManager::RemoveAllUnReady()
{
    conditionReadyQueue_->RemoveUnReady();
}

std::shared_ptr<WorkStatus> WorkPolicyManager::GetWorkToRun()
{
    shared_ptr<WorkStatus> topWork = conditionReadyQueue_->GetWorkToRunByPriority();
    return topWork;
}

void WorkPolicyManager::RealStartSA(std::shared_ptr<WorkStatus> topWork)
{
    WS_HILOGI("RealStartSA %{public}d workId:%{public}s", topWork->workInfo_->GetSaId(), topWork->workId_.c_str());
    if (wss_.expired()) {
        WS_HILOGE("wss_ expired");
        return;
    }
    wss_.lock()->UpdateWorkBeforeRealStart(topWork);
    RemoveFromReadyQueue(topWork);
    bool ret = wss_.lock()->LoadSa(topWork, "");
    if (ret) {
        WS_HILOGI("startSA %{public}d workId:%{public}s success",
            topWork->workInfo_->GetSaId(), topWork->workId_.c_str());
        topWork->UpdateUidLastTimeMap();
        if (!topWork->IsRepeating()) {
            topWork->MarkStatus(WorkStatus::Status::REMOVED);
            RemoveFromUidQueue(topWork, topWork->uid_);
        } else {
            topWork->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
        }
        workConnManager_->WriteStartWorkEvent(topWork);
        return;
    }
    WS_HILOGE("startSA %{public}d workId:%{public}s failed",
        topWork->workInfo_->GetSaId(), topWork->workId_.c_str());
}

void WorkPolicyManager::RealStartWork(std::shared_ptr<WorkStatus> topWork)
{
    WS_HILOGD("RealStartWork topWork ID: %{public}s", topWork->workId_.c_str());
    if (wss_.expired()) {
        WS_HILOGE("wss_ expired");
        return;
    }
    UpdateWatchdogTime(wss_.lock(), topWork);
    topWork->MarkStatus(WorkStatus::Status::RUNNING);
    RemoveFromReadyQueue(topWork);
    if (IsNeedDiscreteScheduled() && topWork->IsNeedDiscreteScheduled()) {
        WS_HILOGI("Discrete scheduled work id: %{public}s", topWork->workId_.c_str());
        DiscreteScheduled(topWork);
        return;
    }
    wss_.lock()->UpdateWorkBeforeRealStart(topWork);
    bool ret = workConnManager_->StartWork(topWork);
    if (ret) {
        AddWatchdogForWork(topWork);
        topWork->UpdateUidLastTimeMap();
    } else {
        if (!topWork->IsRepeating()) {
            topWork->MarkStatus(WorkStatus::Status::REMOVED);
            RemoveFromUidQueue(topWork, topWork->uid_);
        } else {
            topWork->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
        }
    }
}

void WorkPolicyManager::UpdateWatchdogTime(const std::shared_ptr<WorkSchedulerService> &wmsptr,
    std::shared_ptr<WorkStatus> &topWork)
{
    if (topWork->workInfo_->GetDeepIdle() == WorkCondition::DeepIdle::DEEP_IDLE_IN
        && topWork->workInfo_->GetChargerType() != WorkCondition::Charger::CHARGING_UNKNOWN
        && topWork->workInfo_->GetChargerType() != WorkCondition::Charger::CHARGING_UNPLUGGED) {
        WS_HILOGD("deep idle and charger condition, update watchdog time:%{public}d", LONG_WATCHDOG_TIME);
        SetWatchdogTime(LONG_WATCHDOG_TIME);
        return;
    }

    if (!wmsptr->CheckEffiResApplyInfo(topWork->uid_)) {
        SetWatchdogTime(g_lastWatchdogTime);
        return;
    }
    int32_t chargerStatus = 0;
    auto iter = topWork->conditionMap_.find(WorkCondition::Type::CHARGER);
    if (iter != topWork->conditionMap_.end() && iter->second) {
        chargerStatus = topWork->conditionMap_.at(WorkCondition::Type::CHARGER)->enumVal;
    } else {
        WS_HILOGD("charger is in CHARGING_UNKNOWN status");
        chargerStatus = static_cast<int32_t>(WorkCondition::Charger::CHARGING_UNKNOWN);
    }
    if (chargerStatus == static_cast<int32_t>(WorkCondition::Charger::CHARGING_UNPLUGGED)
        || chargerStatus == static_cast<int32_t>(WorkCondition::Charger::CHARGING_UNKNOWN)) {
        WS_HILOGD("charger is in CHARGING_UNKNOWN or CHARGING_UNPLUGGED status");
        SetWatchdogTime(MEDIUM_WATCHDOG_TIME);
    } else {
        WS_HILOGD("charger is in CHARGING status");
        SetWatchdogTime(LONG_WATCHDOG_TIME);
    }
}

void WorkPolicyManager::AddWatchdogForWork(std::shared_ptr<WorkStatus> workStatus)
{
    uint32_t watchId = NewWatchdogId();
    WS_HILOGI("AddWatchdog, watchId:%{public}u, bundleName:%{public}s, workId:%{public}s, watchdogTime:%{public}d",
        watchId, workStatus->bundleName_.c_str(), workStatus->workId_.c_str(), watchdogTime_);
    watchdog_->AddWatchdog(watchId, watchdogTime_);
    workStatus->workStartTime_ = WorkSchedUtils::GetCurrentTimeMs();
    workStatus->workWatchDogTime_ = static_cast<uint64_t>(watchdogTime_);
    std::lock_guard<ffrt::mutex> lock(watchdogIdMapMutex_);
    watchdogIdMap_.emplace(watchId, workStatus);
}

void WorkPolicyManager::SendRetrigger(int32_t delaytime)
{
    WS_HILOGD("enter");
    if (handler_ == nullptr) {
        return;
    }
    WS_HILOGD("delay = %{public}d", delaytime);
    handler_->SendEvent(InnerEvent::Get(WorkEventHandler::RETRIGGER_MSG, 0), delaytime);
}

void WorkPolicyManager::WatchdogTimeOut(uint32_t watchdogId)
{
    if (wss_.expired()) {
        WS_HILOGE("wss_ expired");
        return;
    }
    std::shared_ptr<WorkStatus> workStatus = GetWorkFromWatchdog(watchdogId);
    if (workStatus == nullptr) {
        WS_HILOGE("watchdog:%{public}u time out error, workStatus is nullptr", watchdogId);
        WorkSchedUtil::HiSysEventException(EventErrorCode::WATCHDOG_TIMEOUT, "get workstatus from watchdog is nullptr");
        return;
    }
    WS_HILOGI("WatchdogTimeOut, watchId:%{public}u, bundleName:%{public}s, workId:%{public}s",
        watchdogId, workStatus->bundleName_.c_str(), workStatus->workId_.c_str());
    wss_.lock()->WatchdogTimeOut(workStatus);
    std::lock_guard<ffrt::mutex> lock(watchdogIdMapMutex_);
    watchdogIdMap_.erase(watchdogId);
}

std::shared_ptr<WorkStatus> WorkPolicyManager::GetWorkFromWatchdog(uint32_t id)
{
    std::lock_guard<ffrt::mutex> lock(watchdogIdMapMutex_);
    return watchdogIdMap_.count(id) > 0 ? watchdogIdMap_.at(id) : nullptr;
}

vector<WorkInfo> WorkPolicyManager::ObtainAllWorks(int32_t &uid)
{
    WS_HILOGD("Wenter");
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    vector<WorkInfo> allWorks;
    if (uidQueueMap_.count(uid) > 0) {
        auto queue = uidQueueMap_.at(uid);
        if (!queue) {
            WS_HILOGE("ObtainAllWorks failed, queue is nullptr");
            return allWorks;
        }
        auto allWorkStatus = queue->GetWorkList();
        std::transform(allWorkStatus.begin(), allWorkStatus.end(), std::back_inserter(allWorks),
            [](std::shared_ptr<WorkStatus> it) { return *(it->workInfo_); });
    }
    return allWorks;
}

shared_ptr<WorkInfo> WorkPolicyManager::GetWorkStatus(int32_t &uid, int32_t &workId)
{
    WS_HILOGD("enter");
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        auto queue = uidQueueMap_.at(uid);
        if (!queue) {
            WS_HILOGE("GetWorkStatus failed, queue is nullptr");
            return nullptr;
        }
        auto workStatus = queue->Find(string("u") + to_string(uid) + "_" + to_string(workId));
        if (workStatus != nullptr) {
            return workStatus->workInfo_;
        }
    }
    return nullptr;
}

list<std::shared_ptr<WorkStatus>> WorkPolicyManager::GetAllWorkStatus(int32_t &uid)
{
    WS_HILOGD("enter");
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    list<shared_ptr<WorkStatus>> allWorks;
    if (uidQueueMap_.count(uid) > 0) {
        allWorks = uidQueueMap_.at(uid)->GetWorkList();
    }
    return allWorks;
}

std::vector<WorkInfo> WorkPolicyManager::GetAllRunningWorks()
{
    WS_HILOGD("enter");
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    vector<WorkInfo> allWorks;
    auto it = uidQueueMap_.begin();
    while (it != uidQueueMap_.end()) {
        std::vector<WorkInfo> workList = it->second->GetRunningWorks();
        allWorks.insert(allWorks.end(), workList.begin(), workList.end());
        it++;
    }
    return allWorks;
}

void WorkPolicyManager::DumpConditionReadyQueue(string& result)
{
    conditionReadyQueue_->Dump(result);
}

void WorkPolicyManager::DumpUidQueueMap(string& result)
{
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    for (auto it : uidQueueMap_) {
        result.append("uid: " + std::to_string(it.first) + ":\n");
        it.second->Dump(result);
    }
}

void WorkPolicyManager::Dump(string& result)
{
    WS_HILOGI("enter");
    result.append("1. workPolicyManager conditionReadyQueue:\n");
    DumpConditionReadyQueue(result);
    result.append("\n");

    result.append("2. workPolicyManager uidQueueMap:\n");
    DumpUidQueueMap(result);

    WorkSchedSystemPolicy systemPolicy;
    result.append("3. GetMaxRunningCount:");
    int32_t maxRunningCount = GetMaxRunningCount(systemPolicy);
    result.append(to_string(maxRunningCount) +
        (maxRunningCount == MAX_RUNNING_COUNT ? "" : " " + systemPolicy.GetInfo()) + "\n");
}

uint32_t WorkPolicyManager::NewWatchdogId()
{
    if (watchdogId_ == MAX_WATCHDOG_ID) {
        watchdogId_ = INIT_WATCHDOG_ID;
    }
    return watchdogId_++;
}

int32_t WorkPolicyManager::GetDumpSetMemory()
{
    return dumpSetMemory_;
}

void WorkPolicyManager::SetMemoryByDump(int32_t memory)
{
    dumpSetMemory_ = memory;
}

int32_t WorkPolicyManager::GetDumpSetCpuUsage()
{
    return dumpSetCpu_;
}

void WorkPolicyManager::SetCpuUsageByDump(int32_t cpu)
{
    dumpSetCpu_ = cpu;
}

int32_t WorkPolicyManager::GetDumpSetMaxRunningCount()
{
    return dumpSetMaxRunningCount_;
}

void WorkPolicyManager::SetMaxRunningCountByDump(int32_t count)
{
    dumpSetMaxRunningCount_ = count;
}

void WorkPolicyManager::SetWatchdogTimeByDump(int32_t time)
{
    WS_HILOGD("Set watchdog time by dump to %{public}d", time);
    watchdogTime_ = time == 0 ? WATCHDOG_TIME : time;
    g_lastWatchdogTime = watchdogTime_;
}

void WorkPolicyManager::SetWatchdogTime(int32_t time)
{
    watchdogTime_ = time;
}

int32_t WorkPolicyManager::WorkPolicyManager::GetWatchdogTime()
{
    return watchdogTime_;
}

void WorkPolicyManager::DumpCheckIdeWorkToRun(const std::string &bundleName, const std::string &abilityName)
{
    std::lock_guard<ffrt::recursive_mutex> lock(ideDebugListMutex_);
    ideDebugList = GetAllIdeWorkStatus(bundleName, abilityName);
    if (ideDebugList.empty()) {
        WS_HILOGE("ideDebugList is empty, please add one work");
        return;
    }
    SendIdeWorkRetriggerEvent(0);
}

void WorkPolicyManager::DumpTriggerWork(int32_t uId, int32_t workId, std::string& result)
{
    std::shared_ptr<WorkStatus> workStatus = FindWorkStatus(uId, workId);
    if (workStatus == nullptr) {
        result.append("the work is not exist\n");
        return;
    }
    if (workStatus->IsRunning()) {
        result.append("the work is running\n");
        return;
    }
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    bool ret = workConnManager_->StartWork(workStatus);
    if (ret) {
        result.append("the work trigger ok\n");
        AddWatchdogForWork(workStatus);
    } else {
        result.append("the work trigger error\n");
        workStatus->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    }
}

void WorkPolicyManager::TriggerIdeWork()
{
    std::lock_guard<ffrt::recursive_mutex> lock(ideDebugListMutex_);
    if (ideDebugList.empty()) {
        WS_HILOGI("ideDebugList has been empty, all the works have been done");
        return;
    }

    auto topWork = ideDebugList.front();
    ideDebugList.pop_front();
    if (topWork->IsRunning()) {
        SendIdeWorkRetriggerEvent(g_lastWatchdogTime + DELAY_TIME_SHORT);
        return;
    }
    topWork->MarkStatus(WorkStatus::Status::RUNNING);
    bool ret = workConnManager_->StartWork(topWork);
    if (ret) {
        WS_HILOGI("TriggerIdeWork ok");
        int time = watchdogTime_;
        watchdogTime_ = g_lastWatchdogTime;
        AddWatchdogForWork(topWork);
        watchdogTime_ = time;
    } else {
        WS_HILOGE("TriggerIdeWork error");
        ideDebugList.clear();
        return;
    }
    SendIdeWorkRetriggerEvent(g_lastWatchdogTime + DELAY_TIME_SHORT);
}

void WorkPolicyManager::SendIdeWorkRetriggerEvent(int32_t delaytime)
{
    if (handler_ == nullptr) {
        WS_HILOGE("handle is nullptr");
        return;
    }
    handler_->SendEvent(InnerEvent::Get(WorkEventHandler::IDE_RETRIGGER_MSG, 0), delaytime);
}

std::list<std::shared_ptr<WorkStatus>> WorkPolicyManager::GetAllIdeWorkStatus(const std::string &bundleName,
    const std::string &abilityName)
{
    int32_t currentAccountId = WorkSchedUtils::GetCurrentAccountId();
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    std::list<shared_ptr<WorkStatus>> allWorks;
    auto it = uidQueueMap_.begin();
    while (it != uidQueueMap_.end()) {
        if (it->second->GetWorkList().empty()) {
            it++;
            continue;
        }
        bool isExist = false;
        for (auto work : it->second->GetWorkList()) {
            if (work->workInfo_->GetBundleName() == bundleName &&
                work->workInfo_->GetAbilityName() == abilityName &&
                (work->userId_ == 0 || work->userId_ == currentAccountId)) {
                allWorks.push_back(work);
                isExist = true;
            }
        }
        if (isExist) {
            return allWorks;
        }
        it++;
    }
    return allWorks;
}

int32_t WorkPolicyManager::PauseRunningWorks(int32_t uid)
{
    WS_HILOGI("Pause Running Work Scheduler Work, uid:%{public}d", uid);
    bool hasWorkWithUid = false;
    std::lock_guard<ffrt::mutex> lock(watchdogIdMapMutex_);
    for (auto it = watchdogIdMap_.begin(); it != watchdogIdMap_.end(); it++) {
        auto workStatus = it->second;
        if (workStatus->uid_ == uid && workStatus->IsRunning()) {
            hasWorkWithUid = true;
            if (workStatus->IsPaused()) {
                WS_HILOGE("Work has paused, bundleName:%{public}s, workId:%{public}s",
                    workStatus->bundleName_.c_str(), workStatus->workId_.c_str());
                continue;
            }
            uint64_t oldWatchdogTime = workStatus->workWatchDogTime_;
            uint64_t runningTime = WorkSchedUtils::GetCurrentTimeMs() - workStatus->workStartTime_;
            uint64_t newWatchdogTime = oldWatchdogTime - runningTime;
            if (newWatchdogTime > LONG_WATCHDOG_TIME) {
                WS_HILOGE("bundleName:%{public}s, workId:%{public}s, invalid watchdogtime: %{public}" PRIu64
                    ",oldWatchdogTime:%{public}" PRIu64 ", runningTime:%{public}" PRIu64,
                    workStatus->bundleName_.c_str(), workStatus->workId_.c_str(), newWatchdogTime, oldWatchdogTime,
                    runningTime);
                newWatchdogTime = 0;
            }
            workStatus->duration_ += runningTime;
            WS_HILOGI("PauseRunningWorks, watchId:%{public}u, bundleName:%{public}s, workId:%{public}s,"
                " oldWatchdogTime:%{public}" PRIu64 ", newWatchdogTime:%{public}" PRIu64 ", duration:%{public}" PRIu64,
                it->first, workStatus->bundleName_.c_str(), workStatus->workId_.c_str(),
                oldWatchdogTime, newWatchdogTime, workStatus->duration_);
            workStatus->paused_ = true;
            workStatus->workWatchDogTime_ = newWatchdogTime;
            watchdog_->RemoveWatchdog(it->first);
        }
    }

    if (!hasWorkWithUid) {
        WS_HILOGE("PauseRunningWorks fail, the uid:%{public}d has no matching work", uid);
        return E_UID_NO_MATCHING_WORK_ERR;
    }
    return ERR_OK;
}

int32_t WorkPolicyManager::ResumePausedWorks(int32_t uid)
{
    WS_HILOGI("Resume Paused Work Scheduler Work, uid:%{public}d", uid);
    bool hasWorkWithUid = false;
    std::lock_guard<ffrt::mutex> lock(watchdogIdMapMutex_);
    for (auto it = watchdogIdMap_.begin(); it != watchdogIdMap_.end(); it++) {
        auto workStatus = it->second;
        if (workStatus->uid_ == uid && workStatus->IsRunning()) {
            hasWorkWithUid = true;
            if (!workStatus->IsPaused()) {
                WS_HILOGE("Work has resumed, bundleName:%{public}s, workId:%{public}s",
                    workStatus->bundleName_.c_str(), workStatus->workId_.c_str());
                continue;
            }
            int32_t watchdogTime = static_cast<int32_t>(workStatus->workWatchDogTime_);
            WS_HILOGI("ResumePausedWorks, watchId:%{public}u, bundleName:%{public}s, workId:%{public}s"
                " watchdogTime:%{public}d",
                it->first, workStatus->bundleName_.c_str(), workStatus->workId_.c_str(), watchdogTime);
            workStatus->paused_ = false;
            watchdog_->AddWatchdog(it->first, watchdogTime);
            workStatus->workStartTime_ = WorkSchedUtils::GetCurrentTimeMs();
        }
    }

    if (!hasWorkWithUid) {
        WS_HILOGE("ResumePausedWorks fail, the uid:%{public}d has no matching work", uid);
        return E_UID_NO_MATCHING_WORK_ERR;
    }
    return ERR_OK;
}

void WorkPolicyManager::RemoveWatchDog(std::shared_ptr<WorkStatus> workStatus)
{
    if (!workStatus || workStatus->workId_.empty()) {
        WS_HILOGE("remove watchdog error, workStatus or workId is null");
        return;
    }

    std::lock_guard<ffrt::mutex> lock(watchdogIdMapMutex_);
    uint32_t watchdogId = UINT32_MAX;
    for (auto it = watchdogIdMap_.begin(); it != watchdogIdMap_.end(); it++) {
        if (workStatus->workId_ == it->second->workId_) {
            watchdog_->RemoveWatchdog(it->first);
            watchdogId = it->first;
            break;
        }
    }
    if (watchdogId != UINT32_MAX) {
        watchdogIdMap_.erase(watchdogId);
    }
}

std::list<std::shared_ptr<WorkStatus>> WorkPolicyManager::GetDeepIdleWorks()
{
    std::list<shared_ptr<WorkStatus>> deepIdleWorkds;
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    auto it = uidQueueMap_.begin();
    while (it != uidQueueMap_.end()) {
        std::list<std::shared_ptr<WorkStatus>> workList = it->second->GetDeepIdleWorks();
        if (workList.size() != 0) {
            deepIdleWorkds.insert(deepIdleWorkds.end(), workList.begin(), workList.end());
        }
        it++;
    }
    return deepIdleWorkds;
}

bool WorkPolicyManager::FindWork(int32_t uid)
{
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    auto iter = uidQueueMap_.find(uid);
    return iter != uidQueueMap_.end() && iter->second->GetSize() > 0;
}

bool WorkPolicyManager::FindWork(const int32_t userId, const std::string &bundleName)
{
    std::lock_guard<ffrt::recursive_mutex> lock(uidMapMutex_);
    for (auto list : uidQueueMap_) {
        if (list.second && list.second->Find(userId, bundleName)) {
            return true;
        }
    }
    return false;
}

bool WorkPolicyManager::HasSystemPolicyEventSend() const
{
    return systemPolicyEventSend_.load();
}

void WorkPolicyManager::SetSystemPolicyEventSend(bool systemPolicyEventSend)
{
    systemPolicyEventSend_.store(systemPolicyEventSend);
}

bool WorkPolicyManager::IsNeedDiscreteScheduled()
{
    const std::vector<int32_t> mins{ 0, 29, 30, 59 };
    constexpr int32_t MIN_HOUR = 6;
    constexpr int32_t MAX_HOUR = 9;
    time_t t;
    (void)time(&t);
    struct tm nowTime;
    (void)localtime_r(&t, &nowTime);
    if (nowTime.tm_hour < MIN_HOUR || nowTime.tm_hour > MAX_HOUR) {
        return false;
    }
    auto iter = std::find(mins.cbegin(), mins.cend(), nowTime.tm_min);
    if (iter == mins.end()) {
        return false;
    }
    return true;
}

void WorkPolicyManager::DiscreteScheduled(std::shared_ptr<WorkStatus> topWork)
{
    auto service = wss_.lock();
    if (!service) {
        WS_HILOGE("service is null");
        return;
    }
    auto task = [this, topWork]() {
        wss_.lock()->UpdateWorkBeforeRealStart(topWork);
        bool ret = workConnManager_->StartWork(topWork);
        if (ret) {
            AddWatchdogForWork(topWork);
            topWork->UpdateUidLastTimeMap();
        } else {
            if (!topWork->IsRepeating()) {
                topWork->MarkStatus(WorkStatus::Status::REMOVED);
                RemoveFromUidQueue(topWork, topWork->uid_);
            } else {
                topWork->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
            }
        }
    };
    auto handler = service->GetHandler();
    if (!handler) {
        WS_HILOGE("handler is null");
        return;
    }
    constexpr int32_t MAX_DELAY_SECOND = 120;
    constexpr int32_t MILLISECOND = 1000;
    int32_t seed = (unsigned)time(NULL);
    srand(seed);
    int32_t delay = rand() % MAX_DELAY_SECOND + 1;
    handler->PostTask(task, delay * MILLISECOND);
}
} // namespace WorkScheduler
} // namespace OHOS