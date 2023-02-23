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

#include "work_scheduler_extension.h"

#include "ability_loader.h"
#include "js_work_scheduler_extension.h"

namespace OHOS {
namespace WorkScheduler {
WorkSchedulerExtension* WorkSchedulerExtension::Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime)
{
    if (!runtime) {
        return (new (std::nothrow) WorkSchedulerExtension());
    }
    switch (runtime->GetLanguage()) {
        case AbilityRuntime::Runtime::Language::JS:
            return JsWorkSchedulerExtension::Create(runtime);
        default:
            return (new (std::nothrow) WorkSchedulerExtension());
    }
}

void WorkSchedulerExtension::Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord>& record,
    const std::shared_ptr<AppExecFwk::OHOSApplication>& application,
    std::shared_ptr<AppExecFwk::AbilityHandler>& handler,
    const sptr<IRemoteObject>& token)
{
    ExtensionBase<WorkSchedulerExtensionContext>::Init(record, application, handler, token);
}

std::shared_ptr<WorkSchedulerExtensionContext> WorkSchedulerExtension::CreateAndInitContext(
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord>& record,
    const std::shared_ptr<AppExecFwk::OHOSApplication>& application,
    std::shared_ptr<AppExecFwk::AbilityHandler>& handler,
    const sptr<IRemoteObject>& token)
{
    std::shared_ptr<WorkSchedulerExtensionContext> context =
        ExtensionBase<WorkSchedulerExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (record == nullptr) {
        return context;
    }
    return context;
}

void WorkSchedulerExtension::OnWorkStart(WorkInfo& workInfo) {
}

void WorkSchedulerExtension::OnWorkStop(WorkInfo& workInfo) {
}
} // namespace WorkScheduler
} // namespace OHOS