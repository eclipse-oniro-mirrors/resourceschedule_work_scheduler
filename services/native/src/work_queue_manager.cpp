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
#include <hisysevent.h>
#include <ipc_skeleton.h>

#include "work_queue_manager.h"
#include "work_scheduler_service.h"
#include "work_sched_hilog.h"
#include "work_sched_utils.h"

using namespace std;

namespace OHOS {
namespace WorkScheduler {
const uint32_t TIME_CYCLE = 10 * 60 * 1000; // 10min
static int32_t g_timeRetrigger = INT32_MAX;

WorkQueueManager::WorkQueueManager(const std::shared_ptr<WorkSchedulerService>& wss) : wss_(wss)
{
    timeCycle_ = TIME_CYCLE;
}

bool WorkQueueManager::Init()
{
    return true;
}

bool WorkQueueManager::AddListener(WorkCondition::Type type, shared_ptr<IConditionListener> listener)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (listenerMap_.count(type) > 0) {
        return false;
    }
    listenerMap_.emplace(type, listener);
    return true;
}

bool WorkQueueManager::AddWork(shared_ptr<WorkStatus> workStatus)
{
    if (!workStatus || !workStatus->workInfo_ || !workStatus->workInfo_->GetConditionMap()) {
        return false;
    }
    WS_HILOGD("workStatus ID: %{public}s", workStatus->workId_.c_str());
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto map = workStatus->workInfo_->GetConditionMap();
    for (auto it : *map) {
        if (queueMap_.count(it.first) == 0) {
            queueMap_.emplace(it.first, make_shared<WorkQueue>());
            if (listenerMap_.count(it.first) != 0) {
                listenerMap_.at(it.first)->Start();
            }
        }
        queueMap_.at(it.first)->Push(workStatus);
    }
    if (WorkSchedUtils::IsSystemApp()) {
        WS_HILOGI("Is system app, default group is active.");
        workStatus->workInfo_->SetCallBySystemApp(true);
    }
    return true;
}

bool WorkQueueManager::RemoveWork(shared_ptr<WorkStatus> workStatus)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    WS_HILOGD("workStatus ID: %{public}s", workStatus->workId_.c_str());
    auto map = workStatus->workInfo_->GetConditionMap();
    for (auto it : *map) {
        if (queueMap_.count(it.first) > 0) {
            queueMap_.at(it.first)->Remove(workStatus);
        }
        if (queueMap_.count(it.first) == 0) {
            listenerMap_.at(it.first)->Stop();
        }
    }
    return true;
}

bool WorkQueueManager::CancelWork(shared_ptr<WorkStatus> workStatus)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    WS_HILOGD("workStatus ID: %{public}s", workStatus->workId_.c_str());
    for (auto it : queueMap_) {
        it.second->CancelWork(workStatus);
        if (queueMap_.count(it.first) == 0) {
            listenerMap_.at(it.first)->Stop();
        }
    }
    // Notify work remove event to battery statistics
    int32_t pid = IPCSkeleton::GetCallingPid();
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        "WORK_REMOVE", HiviewDFX::HiSysEvent::EventType::STATISTIC, "UID", workStatus->uid_,
        "PID", pid, "NAME", workStatus->bundleName_, "WORKID", workStatus->workId_);
    return true;
}

vector<shared_ptr<WorkStatus>> WorkQueueManager::GetReayQueue(WorkCondition::Type conditionType,
    shared_ptr<DetectorValue> conditionVal)
{
    vector<shared_ptr<WorkStatus>> result;
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (conditionType != WorkCondition::Type::GROUP && queueMap_.count(conditionType) > 0) {
        shared_ptr<WorkQueue> workQueue = queueMap_.at(conditionType);
        result = workQueue->OnConditionChanged(conditionType, conditionVal);
    }
    if (conditionType == WorkCondition::Type::GROUP || conditionType == WorkCondition::Type::STANDBY) {
        for (auto it : queueMap_) {
            shared_ptr<WorkQueue> workQueue = it.second;
            auto works = workQueue->OnConditionChanged(conditionType, conditionVal);
            PushWork(works, result);
        }
    }
    bool hasStop = false;
    auto it = result.begin();
    while (it != result.end()) {
        if (!(*it)->needRetrigger_) {
            ++it;
            continue;
        }
        if (conditionType != WorkCondition::Type::TIMER
                && conditionType != WorkCondition::Type::GROUP) {
            WS_HILOGI("Need retrigger, start group listener, bundleName:%{public}s, workId:%{public}s",
                (*it)->bundleName_.c_str(), (*it)->workId_.c_str());
            SetTimeRetrigger((*it)->timeRetrigger_);
            if (!hasStop) {
                listenerMap_.at(WorkCondition::Type::GROUP)->Stop();
                hasStop = true;
            }
            listenerMap_.at(WorkCondition::Type::GROUP)->Start();
        }
        (*it)->needRetrigger_ = false;
        (*it)->timeRetrigger_ = INT32_MAX;
        it = result.erase(it);
    }
    PrintWorkStatus(conditionType);
    ClearTimeOutWorkStatus();
    return result;
}

void WorkQueueManager::ClearTimeOutWorkStatus()
{
    std::set<std::string> allWorkIds;
    for (auto it : queueMap_) {
        shared_ptr<WorkQueue> workQueue = it.second;
        auto workList = workQueue->GetWorkList();
        for (auto work : workList) {
            if (!work->IsTimeout()) {
                continue;
            }
            if (allWorkIds.count(work->workId_) != 0) {
                continue;
            }
            allWorkIds.insert(work->workId_);
            WS_HILOGE("work timed out and will be ended, bundleName:%{public}s, workId:%{public}s",
                work->bundleName_.c_str(), work->workId_.c_str());
            if (wss_.expired()) {
                WS_HILOGE("wss_ expired");
                return;
            }
            wss_.lock()->StopWorkInner(work, work->uid_, false, false);
            work->SetTimeout(false);
        }
    }
}

void WorkQueueManager::PrintWorkStatus(WorkCondition::Type conditionType)
{
    if (conditionType == WorkCondition::Type::GROUP || conditionType == WorkCondition::Type::STANDBY) {
        PrintAllWorkStatus(conditionType);
        return;
    }
    if (queueMap_.count(conditionType) > 0) {
        shared_ptr<WorkQueue> workQueue = queueMap_.at(conditionType);
        auto workList = workQueue->GetWorkList();
        for (auto work : workList) {
            work->ToString(conditionType);
        }
    }
}

void WorkQueueManager::PrintAllWorkStatus(WorkCondition::Type conditionType)
{
    std::set<std::string> allWorkIds;
    for (auto it : queueMap_) {
        shared_ptr<WorkQueue> workQueue = it.second;
        auto workList = workQueue->GetWorkList();
        for (auto work : workList) {
            if (allWorkIds.count(work->workId_) != 0) {
                continue;
            }
            allWorkIds.insert(work->workId_);
            work->ToString(conditionType);
        }
    }
}

void WorkQueueManager::PushWork(vector<shared_ptr<WorkStatus>> &works, vector<shared_ptr<WorkStatus>> &result)
{
    for (const auto &work : works) {
        auto iter = std::find_if(result.begin(), result.end(),
        [&](const auto &existingWork) {
            return existingWork->workId_ == work->workId_;
        });
        if (iter != result.end()) {
            WS_HILOGE("WorkId:%{public}s existing, bundleName:%{public}s",
                work->workId_.c_str(), work->bundleName_.c_str());
            continue;
        }
        result.push_back(work);
    }
}

void WorkQueueManager::OnConditionChanged(WorkCondition::Type conditionType,
    shared_ptr<DetectorValue> conditionVal)
{
    auto service = wss_.lock();
    if (!service) {
        WS_HILOGE("service is null");
        return;
    }
    auto task = [weak = weak_from_this(), service, conditionType, conditionVal]() {
        auto strong = weak.lock();
        if (!strong) {
            WS_HILOGE("strong is null");
            return;
        }
        vector<shared_ptr<WorkStatus>> readyWorkVector = strong->GetReayQueue(conditionType, conditionVal);
        if (readyWorkVector.size() == 0) {
            return;
        }
        for (auto it : readyWorkVector) {
            it->MarkStatus(WorkStatus::Status::CONDITION_READY);
        }
        service->OnConditionReady(make_shared<vector<shared_ptr<WorkStatus>>>(readyWorkVector));
    };
    auto handler = service->GetHandler();
    if (!handler) {
        WS_HILOGE("handler is null");
        return;
    }
    handler->PostTask(task);
}

bool WorkQueueManager::StopAndClearWorks(list<shared_ptr<WorkStatus>> workList)
{
    for (auto &it : workList) {
        CancelWork(it);
    }
    return true;
}

void WorkQueueManager::Dump(string& result)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    string conditionType[] = {"network", "charger", "battery_status", "battery_level",
        "storage", "timer", "group", "deepIdle", "standby", "unknown"};
    uint32_t size = sizeof(conditionType) / sizeof(conditionType[0]);
    for (auto it : queueMap_) {
        if (it.first < size) {
            result.append(conditionType[it.first]);
        } else {
            result.append(conditionType[size - 1]);
        }
        result.append(" : ");
        result.append("[");
        string workIdStr;
        it.second->GetWorkIdStr(workIdStr);
        result.append(workIdStr);
        result.append("]\n");
    }
}

void WorkQueueManager::SetTimeCycle(uint32_t time)
{
    timeCycle_ = time;
    listenerMap_.at(WorkCondition::Type::TIMER)->Stop();
    listenerMap_.at(WorkCondition::Type::TIMER)->Start();
}

uint32_t WorkQueueManager::GetTimeCycle()
{
    return timeCycle_;
}

void WorkQueueManager::SetTimeRetrigger(int32_t time)
{
    g_timeRetrigger = time;
}

int32_t WorkQueueManager::GetTimeRetrigger()
{
    return g_timeRetrigger;
}

void WorkQueueManager::SetMinIntervalByDump(int64_t interval)
{
    for (auto it : queueMap_) {
        it.second->SetMinIntervalByDump(interval);
    }
}
} // namespace WorkScheduler
} // namespace OHOS