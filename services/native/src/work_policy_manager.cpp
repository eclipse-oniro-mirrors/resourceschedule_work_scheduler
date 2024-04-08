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
#include "parameters.h"
#include "policy/app_data_clear_listener.h"
#include "work_scheduler_service.h"
#include "work_event_handler.h"
#include "work_sched_hilog.h"
#include "work_sched_errors.h"
#include "work_sched_utils.h"
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
const int32_t INIT_DUMP_SET_CPU = 0;
static int32_t g_lastWatchdogTime = WATCHDOG_TIME;
}

WorkPolicyManager::WorkPolicyManager(const std::shared_ptr<WorkSchedulerService>& wss) : wss_(wss)
{
    conditionReadyQueue_ = std::make_shared<WorkQueue>();
    watchdogId_ = INIT_WATCHDOG_ID;
    dumpSetMemory_ = INIT_DUMP_SET_MEMORY;
    watchdogTime_ = WATCHDOG_TIME;
    dumpSetCpu_ = INIT_DUMP_SET_CPU;
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

int32_t WorkPolicyManager::AddWork(shared_ptr<WorkStatus> workStatus, int32_t uid)
{
    WS_HILOGD("Add work");
    std::lock_guard<std::recursive_mutex> lock(uidMapMutex_);
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

    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        "WORK_ADD", HiSysEvent::EventType::STATISTIC, "UID", uid,
        "PID", pid, "NAME", workStatus->bundleName_, "WORKID", workStatus->workId_, "TRIGGER", conditions, "TYPE",
        type, "INTERVAL", workStatus->workInfo_->GetTimeInterval());

    WS_HILOGI("push workStatus ID: %{public}s to uidQueue(%{public}d)", workStatus->workId_.c_str(), uid);
    return ERR_OK;
}

bool WorkPolicyManager::RemoveWork(shared_ptr<WorkStatus> workStatus, int32_t uid)
{
    WS_HILOGD("Remove work.");
    bool ret = false;
    std::lock_guard<std::recursive_mutex> lock(uidMapMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        return uidQueueMap_.at(uid)->Find(WorkStatus::MakeWorkId(workInfo.GetWorkId(), uid));
    }
    return nullptr;
}

void WorkPolicyManager::RemoveFromUidQueue(std::shared_ptr<WorkStatus> workStatus, int32_t uid)
{
    std::lock_guard<std::recursive_mutex> lock(uidMapMutex_);
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
        workConnManager_->StopWork(workStatus, isTimeOut);
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
    std::lock_guard<std::recursive_mutex> lock(uidMapMutex_);
    if (uidQueueMap_.count(uid) > 0) {
        auto queue = uidQueueMap_.at(uid);
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
    std::lock_guard<std::recursive_mutex> lock(uidMapMutex_);
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
    conditionReadyQueue_->Push(workStatusVector);
}

int32_t WorkPolicyManager::GetMaxRunningCount(std::string& policyName)
{
    int32_t currentMaxRunning = MAX_RUNNING_COUNT;
    for (auto policyFilter : policyFilters_) {
        int32_t policyMaxRunning = policyFilter->GetPolicyMaxRunning();
        if (policyMaxRunning < currentMaxRunning) {
            currentMaxRunning = policyMaxRunning;
            policyName = policyFilter->GetPolicyName();
        }
    }
    return currentMaxRunning;
}

int32_t WorkPolicyManager::GetRunningCount()
{
    WS_HILOGD("enter");
    std::lock_guard<std::recursive_mutex> lock(uidMapMutex_);
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
        case PolicyType::APP_ADDED: {
            service->InitPreinstalledWork();
            break;
        }
        case PolicyType::APP_REMOVED: {
            int32_t uid = detectorVal->intVal;
            WorkStatus::ClearUidLastTimeMap(uid);
            service->StopAndClearWorksByUid(detectorVal->intVal);
            break;
        }
        case PolicyType::APP_DATA_CLEAR: {
            service->StopAndClearWorksByUid(detectorVal->intVal);
            break;
        }
        case PolicyType::APP_CHANGED: {
            int32_t uid = detectorVal->intVal;
            WorkStatus::ClearUidLastTimeMap(uid);
            service->StopAndClearWorksByUid(detectorVal->intVal);
            service->InitPreinstalledWork();
            break;
        }
        default: {}
    }
    CheckWorkToRun();
}

bool WorkPolicyManager::IsSpecialScene(std::shared_ptr<WorkStatus> topWork)
{
    return (OHOS::system::GetIntParameter("const.debuggable", 0) == 1) &&
        (topWork->bundleName_ == "com.huawei.hmos.hiviewx");
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
    std::string policyName;
    int32_t runningCount = GetRunningCount();
    if (runningCount < GetMaxRunningCount(policyName) || IsSpecialScene(topWork)) {
        WS_HILOGD("running count < max running count");
        RealStartWork(topWork);
        SendRetrigger(DELAY_TIME_SHORT);
    } else {
        WS_HILOGD("trigger delay: %{public}d", DELAY_TIME_LONG);
        if (runningCount == MAX_RUNNING_COUNT) {
            topWork->delayReason_ = "OVER_LIMIT";
        }

        if (!policyName.empty()) {
            topWork->delayReason_= policyName;
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

void WorkPolicyManager::RealStartWork(std::shared_ptr<WorkStatus> topWork)
{
    WS_HILOGD("RealStartWork topWork ID: %{public}s", topWork->workId_.c_str());
    if (wss_.expired()) {
        WS_HILOGE("wss_ expired");
        return;
    }
    UpdateWatchdogTime(wss_.lock(), topWork);
    topWork->MarkStatus(WorkStatus::Status::RUNNING);
    wss_.lock()->UpdateWorkBeforeRealStart(topWork);
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

void WorkPolicyManager::UpdateWatchdogTime(const std::shared_ptr<WorkSchedulerService> &wmsptr,
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
    WS_HILOGI("AddWatchdog, watchId:%{public}u, bundleName:%{public}s, workId:%{public}s, watchdogTime:%{public}d",
        watchId, workStatus->bundleName_.c_str(), workStatus->workId_.c_str(), watchdogTime_);
    watchdog_->AddWatchdog(watchId, watchdogTime_);
    workStatus->workStartTime_ = WorkSchedUtils::GetCurrentTimeMs();
    workStatus->workWatchDogTime_ = static_cast<long long>(watchdogTime_);
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
    if (wss_.expired()) {
        WS_HILOGE("wss_ expired");
        return;
    }
    std::shared_ptr<WorkStatus> workStatus = GetWorkFromWatchdog(watchdogId);
    WS_HILOGI("WatchdogTimeOut, watchId:%{public}u, bundleName:%{public}s, workId:%{public}s",
        watchdogId, workStatus->bundleName_.c_str(), workStatus->workId_.c_str());
    wss_.lock()->WatchdogTimeOut(workStatus);
}

std::shared_ptr<WorkStatus> WorkPolicyManager::GetWorkFromWatchdog(uint32_t id)
{
    std::lock_guard<std::mutex> lock(watchdogIdMapMutex_);
    return watchdogIdMap_.at(id);
}

list<shared_ptr<WorkInfo>> WorkPolicyManager::ObtainAllWorks(int32_t &uid)
{
    WS_HILOGD("Wenter");
    std::lock_guard<std::recursive_mutex> lock(uidMapMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(uidMapMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(uidMapMutex_);
    list<shared_ptr<WorkStatus>> allWorks;
    if (uidQueueMap_.count(uid) > 0) {
        allWorks = uidQueueMap_.at(uid)->GetWorkList();
    }
    return allWorks;
}

std::list<std::shared_ptr<WorkInfo>> WorkPolicyManager::GetAllRunningWorks()
{
    WS_HILOGD("enter");
    std::lock_guard<std::recursive_mutex> lock(uidMapMutex_);
    list<shared_ptr<WorkInfo>> allWorks;
    auto it = uidQueueMap_.begin();
    while (it != uidQueueMap_.end()) {
        std::list<std::shared_ptr<WorkInfo>> workList = it->second->GetRunningWorks();
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
    std::lock_guard<std::recursive_mutex> lock(uidMapMutex_);
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

    std::string policyName;
    result.append("3. GetMaxRunningCount:");
    std::string reason = policyName.empty() ? "" : " reason:" + policyName;
    result.append(to_string(GetMaxRunningCount(policyName)) + reason + "\n");
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
    std::lock_guard<std::recursive_mutex> lock(ideDebugListMutex_);
    ideDebugList = GetAllIdeWorkStatus(bundleName, abilityName);
    if (ideDebugList.empty()) {
        WS_HILOGE("ideDebugList is empty, please add one work");
        return;
    }
    SendIdeWorkRetriggerEvent(0);
}

void WorkPolicyManager::TriggerIdeWork()
{
    std::lock_guard<std::recursive_mutex> lock(ideDebugListMutex_);
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
    std::lock_guard<std::recursive_mutex> lock(uidMapMutex_);
    std::list<shared_ptr<WorkStatus>> allWorks;
    auto it = uidQueueMap_.begin();
    while (it != uidQueueMap_.end()) {
        if (it->second->GetWorkList().empty()) {
            it++;
            continue;
        }
        auto work = it->second->GetWorkList().front();
        if (work->workInfo_->GetBundleName() != bundleName || work->workInfo_->GetAbilityName() != abilityName) {
            it++;
            continue;
        }
        allWorks = uidQueueMap_.at(work->uid_)->GetWorkList();
        return allWorks;
    }
    return allWorks;
}

int32_t WorkPolicyManager::PauseRunningWorks(int32_t uid)
{
    WS_HILOGI("Pause Running Work Scheduler Work, uid:%{public}d", uid);
    bool hasWorkWithUid = false;
    std::lock_guard<std::mutex> lock(watchdogIdMapMutex_);
    for (auto it = watchdogIdMap_.begin(); it != watchdogIdMap_.end(); it++) {
        auto workStatus = it->second;
        if (workStatus->uid_ == uid && workStatus->IsRunning()) {
            hasWorkWithUid = true;
            long long oldWatchdogTime = workStatus->workWatchDogTime_;
            long long currTime = WorkSchedUtils::GetCurrentTimeMs();
            long long newWatchdogTime = oldWatchdogTime - (currTime - workStatus->workStartTime_);
            if (newWatchdogTime > LONG_WATCHDOG_TIME) {
                WS_HILOGE("invalid watchdogtime: %{public}lld", newWatchdogTime);
                newWatchdogTime = WATCHDOG_TIME;
            }
            WS_HILOGI("PauseRunningWorks, watchId:%{public}u, bundleName:%{public}s, workId:%{public}s"
                " oldWatchdogTime:%{public}lld, newWatchdogTime:%{public}lld",
                it->first, workStatus->bundleName_.c_str(), workStatus->workId_.c_str(),
                oldWatchdogTime, newWatchdogTime);
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
    std::lock_guard<std::mutex> lock(watchdogIdMapMutex_);
    for (auto it = watchdogIdMap_.begin(); it != watchdogIdMap_.end(); it++) {
        auto workStatus = it->second;
        if (workStatus->uid_ == uid && workStatus->IsRunning() && workStatus->IsPaused()) {
            hasWorkWithUid = true;
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
} // namespace WorkScheduler
} // namespace OHOS