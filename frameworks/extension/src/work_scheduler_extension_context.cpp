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

#include "work_scheduler_extension_context.h"
#include "ability_manager_client.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
WorkSchedulerExtensionContext::WorkSchedulerExtensionContext() {}

WorkSchedulerExtensionContext::~WorkSchedulerExtensionContext() {}
ErrCode WorkSchedulerExtensionContext::StartServiceExtensionAbility(const AAFwk::Want &want, int32_t accountId) const
{
    WS_HILOGI("begin");
    ErrCode err = OHOS::AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (err != ERR_OK) {
        WS_HILOGE("StartServiceExtensionAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode WorkSchedulerExtensionContext::StopServiceExtensionAbility(const AAFwk::Want& want, int32_t accountId) const
{
    WS_HILOGI("begin");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StopExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (err != ERR_OK) {
        WS_HILOGE("StopServiceExtensionAbility is failed %{public}d", err);
    }
    return err;
}
}  // namespace WorkScheduler
}  // namespace OHOS