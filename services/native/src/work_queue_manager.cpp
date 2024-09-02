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
#include <hisysevent.h>
#include <ipc_skeleton.h>

#include "work_queue_manager.h"
#include "work_scheduler_service.h"
#include "work_sched_hilog.h"
#include "work_sched_utils.h"

using namespace std;

namespace OHOS {
namespace WorkScheduler {
const uint32_t TIME_CYCLE = 20 * 60 * 1000; // 20min
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
            if (it.first != WorkCondition::Type::BATTERY_LEVEL && listenerMap_.count(it.first) != 0) {
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
            if (it.first == WorkCondition::Type::BATTERY_LEVEL) {
                continue;
            }
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
    auto it = result.begin();
    while (it != result.end()) {
        if ((*it)->needRetrigger_) {
            if (conditionType != WorkCondition::Type::TIMER
                    && conditionType != WorkCondition::Type::GROUP) {
                WS_HILOGI("Need retrigger, start group listener, bundleName:%{public}s, workId:%{public}s",
                    (*it)->bundleName_.c_str(), (*it)->workId_.c_str());
                SetTimeRetrigger((*it)->timeRetrigger_);
                listenerMap_.at(WorkCondition::Type::GROUP)->Start();
            }
            (*it)->needRetrigger_ = false;
            (*it)->timeRetrigger_ = INT32_MAX;
            it = result.erase(it);
        } else {
            ++it;
        }
    }
    return result;
}

void WorkQueueManager::PushWork(vector<shared_ptr<WorkStatus> &works, vector<shared_ptr<WorkStatus> &result)
{
    for (const auto &work : works) {
        auto iter = std::find_if(result.begin(), result.end(),
        [work](const shared_ptr<WorkStatus> &existingWork) {
            WS_HILOGE("WorkId:%{public}s existing, bundleName:%{public}s",
                work->workId_.c_str(), work->bundleName_.c_str());
            return existingWork->workId_ == work->workId_;
        });
        if (iter == result.end()) {
            result.push_back(work);
        }
    }
}

void WorkQueueManager::OnConditionChanged(WorkCondition::Type conditionType,
    shared_ptr<DetectorValue> conditionVal)
{
    vector<shared_ptr<WorkStatus>> readyWorkVector = GetReayQueue(conditionType, conditionVal);
    if (readyWorkVector.size() == 0) {
        return;
    }
    for (auto it : readyWorkVector) {
        it->MarkStatus(WorkStatus::Status::CONDITION_READY);
    }
    if (wss_.expired()) {
        WS_HILOGE("wss_ expired");
        return;
    }
    wss_.lock()->OnConditionReady(make_shared<vector<shared_ptr<WorkStatus>>>(readyWorkVector));
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
    uint32_t size = sizeof(conditionType);
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