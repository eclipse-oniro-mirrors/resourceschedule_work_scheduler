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

#include "work_scheduler_service.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {

namespace {
const std::string WORKSCHEDULER_SERVICE_NAME = "WorkSchedulerService";
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<WorkSchedulerService>::GetInstance().get());
}

WorkSchedulerService::WorkSchedulerService() : SystemAbility(WORK_SCHEDULE_SERVICE_ID, true) {}

WorkSchedulerService::~WorkSchedulerService() {}

void WorkSchedulerService::OnStart()
{
    WS_HILOGI(MODULE_SERVICE, "OnStart enter.");
    if (ready_) {
        WS_HILOGI(MODULE_SERVICE, "OnStart is ready, nothing to do.");
        return;
    }

    ready_ = true;
    WS_HILOGI(MODULE_SERVICE, "OnStart and add system ability success.");
}

void WorkSchedulerService::OnStop()
{
    WS_HILOGI(MODULE_SERVICE, "stop service.");
}
} // namespace WorkScheduler
} // namespace OHOS
