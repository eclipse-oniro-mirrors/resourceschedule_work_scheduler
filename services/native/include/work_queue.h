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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_QUEUE_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_QUEUE_H

#include <memory>
#include <list>

#include "work_status.h"
#include "detector_value.h"

namespace OHOS {
namespace WorkScheduler {
class WorkQueue {
public:
    explicit WorkQueue() = default;
    ~WorkQueue() = default;
    std::vector<std::shared_ptr<WorkStatus>> OnConditionChanged(
        WorkCondition::Type type, std::shared_ptr<DetectorValue> conditionVal);
    void Push(std::shared_ptr<std::vector<std::shared_ptr<WorkStatus>>> workStatusVector);
    void Push(std::shared_ptr<WorkStatus> workStatus);
    std::shared_ptr<WorkStatus> GetWorkToRunByPriority();
    bool Remove(std::shared_ptr<WorkStatus> workStatus);
    bool Contains(std::shared_ptr<std::string> workId);
    std::shared_ptr<WorkStatus> Find(std::string workId);
    size_t GetSize();
    bool CancelWork(std::shared_ptr<WorkStatus> workStatus);
    std::list<std::shared_ptr<WorkStatus>> GetWorkList();
    void RemoveUnReady();
    int32_t GetRunningCount();
    void GetWorkIdStr(std::string& result);
    void Dump(std::string& result);
    void ClearAll();
private:
    std::list<std::shared_ptr<WorkStatus>> workList_;
};
class WorkComp {
public:
    bool operator () (const std::shared_ptr<WorkStatus> w1, const std::shared_ptr<WorkStatus> w2);
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_QUEUE_H