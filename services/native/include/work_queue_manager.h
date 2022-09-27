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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_QUEUE_MANAGER_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_QUEUE_MANAGER_H

#include <memory>
#include <vector>
#include <map>

#include "conditions/icondition_listener.h"
#include "work_queue.h"
#include "work_status.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerService;
class WorkQueueManager : std::enable_shared_from_this<WorkQueueManager> {
public:
    explicit WorkQueueManager(const wptr<WorkSchedulerService>& wss);
    ~WorkQueueManager() = default;
    /**
     * @brief Init.
     *
     * @return True if success,else false.
     */
    bool Init();
    /**
     * @brief Add listener.
     *
     * @param type The type.
     * @param listener The listener.
     * @return True if success,else false.
     */
    bool AddListener(WorkCondition::Type type, std::shared_ptr<IConditionListener> listener);
    /**
     * @brief Add work.
     *
     * @param workStatus The status of work.
     * @return True if success,else false.
     */
    bool AddWork(std::shared_ptr<WorkStatus> workStatus);
    /**
     * @brief Remove work.
     *
     * @param workStatus The status of work.
     * @return True if success,else false.
     */
    bool RemoveWork(std::shared_ptr<WorkStatus> workStatus);
    /**
     * @brief Cancel work.
     *
     * @param workStatus The status of work.
     * @return True if success,else false.
     */
    bool CancelWork(std::shared_ptr<WorkStatus> workStatus);

    /**
     * @brief The OnConditionChanged callback.
     *
     * @param conditionType The condition type.
     * @param conditionVal The condition val.
     */
    void OnConditionChanged(WorkCondition::Type conditionType,
        std::shared_ptr<DetectorValue> conditionVal);
    /**
     * @brief Stop and clear works.
     *
     * @param workList The list of work.
     * @return True if success,else false.
     */
    bool StopAndClearWorks(std::list<std::shared_ptr<WorkStatus>> workList);
    /**
     * @brief Set time cycle.
     *
     * @param time The time.
     */
    void SetTimeCycle(uint32_t time);
    /**
     * @brief Get the cycle of time.
     *
     * @return The cycle of time.
     */
    uint32_t GetTimeCycle();
    /**
     * @brief Set time retrigger.
     *
     * @param time The time.
     */
    void SetTimeRetrigger(int32_t time);
    /**
     * @brief Get time retrigger.
     *
     * @param time The time.
     */
    int32_t GetTimeRetrigger();
    /**
     * @brief Dump.
     *
     * @param result The result.
     */
    void Dump(std::string& result);
    /**
     * @brief Set min interval by dump.
     */
    void SetMinIntervalByDump(int64_t interval);
private:
    std::vector<std::shared_ptr<WorkStatus>> GetReayQueue(WorkCondition::Type conditionType,
        std::shared_ptr<DetectorValue> conditionVal);
    std::mutex mutex_;
    const wptr<WorkSchedulerService> wss_;
    std::map<WorkCondition::Type, std::shared_ptr<WorkQueue>> queueMap_;
    std::map<WorkCondition::Type, std::shared_ptr<IConditionListener>> listenerMap_;

    uint32_t timeCycle_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_QUEUE_MANAGER_H