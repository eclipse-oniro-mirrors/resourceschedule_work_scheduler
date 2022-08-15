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
    /**
     * @brief The OnConditionChanged callback.
     *
     * @param type The type.
     * @param conditionVal The condition val.
     */
    std::vector<std::shared_ptr<WorkStatus>> OnConditionChanged(
        WorkCondition::Type type, std::shared_ptr<DetectorValue> conditionVal);
    /**
     * @brief Push.
     *
     * @param workStatusVector The work status vector.
     */
    void Push(std::shared_ptr<std::vector<std::shared_ptr<WorkStatus>>> workStatusVector);
    /**
     * @brief Push.
     *
     * @param workStatus The status of work.
     */
    void Push(std::shared_ptr<WorkStatus> workStatus);
    /**
     * @brief Get work to run by priority.
     *
     * @return The status of work.
     */
    std::shared_ptr<WorkStatus> GetWorkToRunByPriority();
    /**
     * @brief Remove.
     *
     * @param workStatus The status of work.
     * @return True if success,else false.
     */
    bool Remove(std::shared_ptr<WorkStatus> workStatus);
    /**
     * @brief Contains.
     *
     * @param workId The id of work.
     * @return True if success,else false.
     */
    bool Contains(std::shared_ptr<std::string> workId);
    /**
     * @brief Find.
     *
     * @param workId The id of work.
     * @return The id of work.
     */
    std::shared_ptr<WorkStatus> Find(std::string workId);
    /**
     * @brief Get size.
     *
     * @return The work list size.
     */
    uint32_t GetSize();
    /**
     * @brief Cancel work.
     *
     * @param workStatus The status of work.
     * @return True if success,else false.
     */
    bool CancelWork(std::shared_ptr<WorkStatus> workStatus);
    /**
     * @brief Get the list of work.
     *
     * @return The list of work.
     */
    std::list<std::shared_ptr<WorkStatus>> GetWorkList();
    /**
     * @brief Remove unready.
     */
    void RemoveUnReady();
    /**
     * @brief Get the count of running.
     *
     * @return The count of running.
     */
    int32_t GetRunningCount();
    /**
     * @brief Get work id str.
     *
     * @param result The result.
     */
    void GetWorkIdStr(std::string& result);
    /**
     * @brief Dump.
     *
     * @param result The result.
     */
    void Dump(std::string& result);
    /**
     * @brief Clear all.
     */
    void ClearAll();
    /**
     * @brief Set min interval by dump.
     */
    void SetMinIntervalByDump(int64_t interval);
private:
    std::list<std::shared_ptr<WorkStatus>> workList_;
};
class WorkComp {
public:
    /**
     * @brief operator.
     *
     * @param w1 The w1.
     * @param w2 The w2.
     * @return True if success,else false.
     */
    bool operator () (const std::shared_ptr<WorkStatus> w1, const std::shared_ptr<WorkStatus> w2);
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_QUEUE_H