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

#ifndef WORK_SCHED_SERVICES_WORK_STATUS_H
#define WORK_SCHED_SERVICES_WORK_STATUS_H

#include <memory>
#include <string>

#include "timer.h"
#include "work_info.h"

namespace OHOS {
using namespace Utils;
namespace WorkScheduler {
class WorkStatus {
public:
    enum Status {
        WAIT_CONDITION = 0,
        CONDITION_READY,
        RUNNING,
        REMOVED
    };
    const int32_t DEFAULT_PRIORITY = 100;
    WorkStatus(WorkInfo &workInfo, int32_t uid);
    ~WorkStatus();

    static std::string MakeWorkId(int32_t workId, int32_t uid);

    std::string workId_;
    std::string bundleName_;
    std::string abilityName_;
    int32_t uid_;
    int userId_;
    bool persisted_;
    int32_t priority_;
    std::map<WorkCondition::Type, std::shared_ptr<Condition>> conditionMap_;
    std::shared_ptr<WorkInfo> workInfo_;

    bool IsReady();
    bool IsReadyStatus();
    bool IsRunning();
    bool IsRemoved();
    bool IsRepeating();
    bool IsLastWorkTimeout();
    void OnConditionChanged(WorkCondition::Type &type, std::shared_ptr<Condition> value);
    void MarkRound();
    void MarkStatus(Status status);
    Status GetStatus();
    void Dump(std::string& result);
    void UpdateTimerIfNeed();
    bool NeedRemove();

    bool lastTimeout_ {false};

private:
    Status currentStatus_;
    time_t baseTime_;
    void MarkTimeout();
    bool IsSameUser();
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // WORK_SCHED_SERVICES_WORK_STATUS_H