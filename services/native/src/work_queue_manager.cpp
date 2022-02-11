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
#include "work_queue_manager.h"

using namespace std;

namespace OHOS {
namespace WorkScheduler {
const int TIME_CYCLE = 10 * 60 * 60; // 10min

WorkQueueManager::WorkQueueManager(const wptr<WorkSchedulerService>& wss) : wss_(wss)
{
    timeCycle_ = TIME_CYCLE;
}

bool WorkQueueManager::Init()
{
    return true;
}

bool WorkQueueManager::AddListener(WorkCondition::Type type, shared_ptr<IConditionListener> listener)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (listenerMap_.count(type) > 0) {
        return false;
    }
    listenerMap_.emplace(type, listener);
    return true;
}

bool WorkQueueManager::AddWork(shared_ptr<WorkStatus> workStatus)
{
    WS_HILOGD("WorkQueueManager::%{public}s workStatus ID: %{public}s", __func__, workStatus->workId_.c_str());
    std::lock_guard<std::mutex> lock(mutex_);
    auto map = workStatus->workInfo_->GetConditionMap();
    for (auto it : *map) {
        if (queueMap_.count(it.first) == 0) {
            queueMap_.emplace(it.first, make_shared<WorkQueue>());
            listenerMap_.at(it.first)->Start();
        }
        queueMap_.at(it.first)->Push(workStatus);
    }
    return true;
}

bool WorkQueueManager::RemoveWork(shared_ptr<WorkStatus> workStatus)
{
    std::lock_guard<std::mutex> lock(mutex_);
    WS_HILOGD("WorkQueueManager::%{public}s workStatus ID: %{public}s", __func__, workStatus->workId_.c_str());
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
    std::lock_guard<std::mutex> lock(mutex_);
    WS_HILOGD("WorkQueueManager::%{public}s workStatus ID: %{public}s", __func__, workStatus->workId_.c_str());
    for (auto it : queueMap_) {
        it.second->CancelWork(workStatus);
        if (queueMap_.count(it.first) == 0) {
            listenerMap_.at(it.first)->Stop();
        }
    }
    return true;
}

vector<shared_ptr<WorkStatus>> WorkQueueManager::GetReayQueue(WorkCondition::Type conditionType,
    shared_ptr<DetectorValue> conditionVal)
{
    vector<shared_ptr<WorkStatus>> result;
    std::lock_guard<std::mutex> lock(mutex_);
    if (queueMap_.count(conditionType) > 0) {
        shared_ptr<WorkQueue> workQueue = queueMap_.at(conditionType);
        result = workQueue->OnConditionChanged(conditionType, conditionVal);
    }
    return result;
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
    auto ws = wss_.promote();
    ws->OnConditionReady(make_shared<vector<shared_ptr<WorkStatus>>>(readyWorkVector));
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
    std::lock_guard<std::mutex> lock(mutex_);
    string conditionType[] = {"network", "charger", "battery_status", "battery_level",
        "storage", "timer", "unknown"};
    int size = sizeof(conditionType);
    for (auto it : queueMap_) {
        int type = it.first;
        if (type < size) {
            result.append(conditionType[type]);
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

void WorkQueueManager::SetTimeCycle(int time)
{
    timeCycle_ = time;
}

int WorkQueueManager::GetTimeCycle() {
    return timeCycle_;
}
} // namespace WorkScheduler
} // namespace OHOS
