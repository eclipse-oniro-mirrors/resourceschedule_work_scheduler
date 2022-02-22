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

#include "delayed_sp_singleton.h"
#include "work_queue.h"
#include "work_status.h"
#include "work_sched_common.h"
#include "conditions/icondition_listener.h"
#include "work_scheduler_service.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerService;
class WorkQueueManager : std::enable_shared_from_this<WorkQueueManager> {
public:
    explicit WorkQueueManager(const wptr<WorkSchedulerService>& wss);
    ~WorkQueueManager() = default;
    bool Init();
    bool AddListener(WorkCondition::Type type, std::shared_ptr<IConditionListener> listener);
    bool AddWork(std::shared_ptr<WorkStatus> workStatus);
    bool RemoveWork(std::shared_ptr<WorkStatus> workStatus);
    bool CancelWork(std::shared_ptr<WorkStatus> workStatus);

    void OnConditionChanged(WorkCondition::Type conditionType,
        std::shared_ptr<DetectorValue> conditionVal);
    bool StopAndClearWorks(std::list<std::shared_ptr<WorkStatus>> workList);
    void SetTimeCycle(uint32_t time);
    int GetTimeCycle();
    void Dump(std::string& result);

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