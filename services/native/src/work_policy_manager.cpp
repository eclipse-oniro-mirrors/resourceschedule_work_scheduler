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

#include "work_policy_manager.h"

#include <string>
#include <hisysevent.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "policy/app_data_clear_listener.h"
#include "policy/app_removed_listener.h"
#include "work_scheduler_service.h"
#include "work_event_handler.h"
#include "work_sched_hilog.h"
#include "work_sched_errors.h"
#include "watchdog.h"

using namespace std;
using namespace OHOS::AppExecFwk;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace WorkScheduler {
namespace {
const int32_t MAX_RUNNING_COUNT = 3;
const uint32_t MAX_WORK_COUNT_PER_UID = 10;
const int32_t DELAY_TIME_LONG = 30000;
const int32_t DELAY_TIME_SHORT = 5000;
const uint32_t MAX_WATCHDOG_ID = 1000;
const uint32_t INIT_WATCHDOG_ID = 1;
const int32_t INIT_DUMP_SET_MEMORY = -1;
const int32_t WATCHDOG_TIME = 2 * 60 * 1000;
const int32_t MEDIUM_WATCHDOG_TIME = 10 * 60 * 1000;
const int32_t LONG_WATCHDOG_TIME = 20 * 60 * 1000;
static int32_t g_lastWatchdogTime = WATCHDOG_TIME;
}

WorkPolicyManager::WorkPolicyManager(const wptr<WorkSchedulerService>& wss) : wss_(wss)
{
    std::lock_guard<std::mutex> lock(conditionReadyMutex_);
    conditionReadyQueue_ = std::make_shared<WorkQueue>();
    watchdogId_ = INIT_WATCHDOG_ID;
    dumpSetMemory_ = INIT_DUMP_SET_MEMORY;
    watchdogTime_ = WATCHDOG_TIME;
}

bool WorkPolicyManager::Init()
{
    WS_HILOGD("Work policy manager init.");
    workConnManager_ = make_shared<WorkConnManager>();
    auto wmsptr = wss_.promote();
    if (wmsptr == nullptr) {
        WS_HILOGE("failed due to wmsptr is nullptr");
        return false;
    }
    handler_ = wmsptr->GetHandler();
    if (handler_ == nullptr) {
        WS_HILOGE("failed due to handler_ is nullptr");
        return false;
    }
    watchdog_ = std::make_shared<Watchdog>(wmsptr->GetWorkPolicyManager());
    return true;
}

void WorkPolicyManager::AddPolicyFilter(shared_ptr<IPolicyFilter> filter)
{
    policyFilters_.emplace_back(filter);
}

void WorkPolicyManager::AddAppRemoveListener(shared_ptr<AppRemovedListener> listener)
{
    appRemovedListener_ = listener;
    appRemovedListener_->Start();
}

void WorkPolicyManager::AddAppDataClearListener(std::shared_ptr<AppDataClearListener> listener)
{
    appDataClearListener_ = listener;
    appDataClearListener_->Start();
}

int32_t WorkPolicyManager::AddWork(shared_ptr<WorkStatus> workStatus, int32_t uid)
{
    WS_HILOGD("Add work");
    std::lock_guard<std::mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        if (uidQueueMap_.at(uid)->Contains(make_shared<string>(workStatus->workId_))) {
            WS_HILOGE("Workid has been added, should remove first.");
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
    conditions.pop_back();

    string type = "Repeat";
    if (!workStatus->workInfo_->IsRepeat()) {
        type = "Not Repeat";
    }

    HiSysEvent::Write("WORKSCHEDULER", "WORK_ADD", HiSysEvent::EventType::STATISTIC, "UID", uid,
        "PID", pid, "NAME", workStatus->bundleName_, "WORKID", workStatus->workId_, "TRIGGER", conditions, "TYPE",
        type, "INTERVAL", workStatus->workInfo_->GetTimeInterval());

    WS_HILOGI("push workStatus ID: %{public}s to uidQueue(%{public}d)", workStatus->workId_.c_str(), uid);
    return ERR_OK;
}

bool WorkPolicyManager::RemoveWork(shared_ptr<WorkStatus> workStatus, int32_t uid)
{
    WS_HILOGD("Remove work.");
    bool ret = false;
    std::lock_guard<std::mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        WS_HILOGD("Remove workStatus ID: %{public}s form uidQueue(%{public}d)", workStatus->workId_.c_str(), uid);
        ret = uidQueueMap_.at(uid)->Remove(workStatus);
        if (uidQueueMap_.count(uid) <= 0) {
            uidQueueMap_.erase(uid);
        }
    }

    // Notify work remove event to battery statistics
    int32_t pid = IPCSkeleton::GetCallingPid();
    HiSysEvent::Write("WORKSCHEDULER", "WORK_REMOVE", HiSysEvent::EventType::STATISTIC, "UID", uid,
        "PID", pid, "NAME", workStatus->bundleName_, "WORKID", workStatus->workId_);

    return ret;
}

shared_ptr<WorkStatus> WorkPolicyManager::FindWorkStatus(WorkInfo& workInfo, int32_t uid)
{
    WS_HILOGD("Find work status start.");
    std::lock_guard<std::mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        return uidQueueMap_.at(uid)->Find(WorkStatus::MakeWorkId(workInfo.GetWorkId(), uid));
    }
    return nullptr;
}

void WorkPolicyManager::RemoveFromUidQueue(std::shared_ptr<WorkStatus> workStatus, int32_t uid)
{
    std::lock_guard<std::mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        uidQueueMap_.at(uid)->CancelWork(workStatus);
        if (uidQueueMap_.at(uid)->GetSize() <= 0) {
            uidQueueMap_.erase(uid);
        }
    }
}

void WorkPolicyManager::RemoveFromReadyQueue(std::shared_ptr<WorkStatus> workStatus)
{
    std::lock_guard<std::mutex> lock(conditionReadyMutex_);
    conditionReadyQueue_->RemoveUnReady();
}

bool WorkPolicyManager::StopWork(std::shared_ptr<WorkStatus> workStatus, int32_t uid,
    const bool needCancel, bool isTimeOut)
{
    WS_HILOGD("enter");
    bool hasCanceled = false;
    if (workStatus->IsRunning()) {
        workStatus->lastTimeout_ = isTimeOut;
        workConnManager_->StopWork(workStatus);
        if (!workStatus->IsRepeating()) {
            workStatus->MarkStatus(WorkStatus::Status::REMOVED);
            RemoveFromUidQueue(workStatus, uid);
            RemoveFromReadyQueue(workStatus);
            hasCanceled = true;
        } else {
            workStatus->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
        }
    }

    if (!hasCanceled && needCancel) {
        RemoveFromUidQueue(workStatus, uid);
        RemoveFromReadyQueue(workStatus);
        hasCanceled = true;
    }

    CheckWorkToRun();
    return hasCanceled;
}

bool WorkPolicyManager::StopAndClearWorks(int32_t uid)
{
    WS_HILOGD("enter");
    std::lock_guard<std::mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        auto queue = uidQueueMap_.at(uid);
        for (auto it : queue->GetWorkList()) {
            workConnManager_->StopWork(it);
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
    std::lock_guard<std::mutex> lock(uidMapMutex_);
    string workIdStr = WorkStatus::MakeWorkId(workId, uid);
    if (uidQueueMap_.count(uid) > 0) {
        shared_ptr<WorkStatus> workStatus = uidQueueMap_.at(uid)->Find(workIdStr);
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
    std::lock_guard<std::mutex> lock(conditionReadyMutex_);
    conditionReadyQueue_->Push(workStatusVector);
}

int32_t WorkPolicyManager::GetMaxRunningCount()
{
    int32_t currentMaxRunning = MAX_RUNNING_COUNT;
    for (auto policyFilter : policyFilters_) {
        int32_t policyMaxRunning = policyFilter->GetPolicyMaxRunning();
        if (policyMaxRunning < currentMaxRunning) {
            currentMaxRunning = policyMaxRunning;
        }
    }
    return currentMaxRunning;
}

int32_t WorkPolicyManager::GetRunningCount()
{
    WS_HILOGD("enter");
    std::lock_guard<std::mutex> lock(uidMapMutex_);
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
    switch (policyType) {
        case PolicyType::APP_REMOVED: {
            int32_t uid = detectorVal->intVal;
            WorkStatus::ClearUidLastTimeMap(uid);
            [[fallthrough]];
        }
        case PolicyType::APP_DATA_CLEAR: {
            auto ws = wss_.promote();
            ws->StopAndClearWorksByUid(detectorVal->intVal);
            break;
        }
        default: {}
    }
    CheckWorkToRun();
}

void WorkPolicyManager::CheckWorkToRun()
{
    WS_HILOGD("Check work to run.");
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
    if (GetRunningCount() < GetMaxRunningCount()) {
        WS_HILOGD("running count < max running count");
        RealStartWork(topWork);
        SendRetrigger(DELAY_TIME_SHORT);
    } else {
        WS_HILOGD("trigger delay: %{public}d", DELAY_TIME_LONG);
        SendRetrigger(DELAY_TIME_LONG);
    }
    WS_HILOGD("out");
}

void WorkPolicyManager::RemoveAllUnReady()
{
    std::lock_guard<std::mutex> lock(conditionReadyMutex_);
    conditionReadyQueue_->RemoveUnReady();
}

std::shared_ptr<WorkStatus> WorkPolicyManager::GetWorkToRun()
{
    std::lock_guard<std::mutex> lock(conditionReadyMutex_);
    shared_ptr<WorkStatus> topWork = conditionReadyQueue_->GetWorkToRunByPriority();
    return topWork;
}

void WorkPolicyManager::RealStartWork(std::shared_ptr<WorkStatus> topWork)
{
    WS_HILOGD("RealStartWork topWork ID: %{public}s", topWork->workId_.c_str());
    auto wmsptr = wss_.promote();
    if (wmsptr == nullptr) {
        WS_HILOGE("Workscheduler service is null");
        return;
    }
    UpdateWatchdogTime(wmsptr, topWork);
    topWork->MarkStatus(WorkStatus::Status::RUNNING);
    wmsptr->UpdateWorkBeforeRealStart(topWork);
    RemoveFromReadyQueue(topWork);
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

void WorkPolicyManager::UpdateWatchdogTime(const wptr<WorkSchedulerService> &wmsptr,
    std::shared_ptr<WorkStatus> &topWork)
{
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
    watchdog_->AddWatchdog(watchId, watchdogTime_);
    std::lock_guard<std::mutex> lock(watchdogIdMapMutex_);
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
    WS_HILOGD("WatchdogTimeOut.");
    std::shared_ptr<WorkStatus> workStatus = GetWorkFromWatchdog(watchdogId);
    auto wmsptr = wss_.promote();
    if (wmsptr == nullptr) {
        WS_HILOGE("Workscheduler service is null");
        return;
    }
    wmsptr->WatchdogTimeOut(workStatus);
}

std::shared_ptr<WorkStatus> WorkPolicyManager::GetWorkFromWatchdog(uint32_t id)
{
    std::lock_guard<std::mutex> lock(watchdogIdMapMutex_);
    return watchdogIdMap_.at(id);
}

list<shared_ptr<WorkInfo>> WorkPolicyManager::ObtainAllWorks(int32_t &uid)
{
    WS_HILOGD("Wenter");
    std::lock_guard<std::mutex> lock(uidMapMutex_);
    list<shared_ptr<WorkInfo>> allWorks;
    if (uidQueueMap_.count(uid) > 0) {
        auto queue = uidQueueMap_.at(uid);
        auto allWorkStatus = queue->GetWorkList();
        std::transform(allWorkStatus.begin(), allWorkStatus.end(), std::back_inserter(allWorks),
            [](std::shared_ptr<WorkStatus> it) { return it->workInfo_; });
    }
    return allWorks;
}

shared_ptr<WorkInfo> WorkPolicyManager::GetWorkStatus(int32_t &uid, int32_t &workId)
{
    WS_HILOGD("enter");
    std::lock_guard<std::mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        auto queue = uidQueueMap_.at(uid);
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
    std::lock_guard<std::mutex> lock(uidMapMutex_);
    list<shared_ptr<WorkStatus>> allWorks;
    if (uidQueueMap_.count(uid) > 0) {
        allWorks = uidQueueMap_.at(uid)->GetWorkList();
    }
    return allWorks;
}

void WorkPolicyManager::DumpConditionReadyQueue(string& result)
{
    std::lock_guard<std::mutex> lock(conditionReadyMutex_);
    conditionReadyQueue_->Dump(result);
}

void WorkPolicyManager::DumpUidQueueMap(string& result)
{
    std::lock_guard<std::mutex> lock(uidMapMutex_);
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

    result.append("3. GetMaxRunningCount:");
    result.append(to_string(GetMaxRunningCount()) + "\n");
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
} // namespace WorkScheduler
} // namespace OHOS
