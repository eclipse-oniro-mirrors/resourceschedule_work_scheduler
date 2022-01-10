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

#ifndef WORK_SCHED_SERVICES_WORK_SCHEDULER_SERVICES_H
#define WORK_SCHED_SERVICES_WORK_SCHEDULER_SERVICES_H

#include "refbase.h"
#include "singleton.h"
#include "system_ability.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerService final : public SystemAbility,
    public std::enable_shared_from_this<WorkSchedulerService>{
    DISALLOW_COPY_AND_MOVE(WorkSchedulerService);
    DECLARE_SYSTEM_ABILITY(WorkSchedulerService);
    DECLARE_DELAYED_SINGLETON(WorkSchedulerService);
public:
    WorkSchedulerService(const int32_t systemAbilityId, bool runOnCreate);
    virtual void OnStart() override;
    virtual void OnStop() override;

private:
    bool Init();
    bool ready_ {false};
};
} // namespace WorkScheduler
} // namespace OHOS

#endif // WORK_SCHED_SERVICES_WORK_SCHEDULER_SERVICES_H