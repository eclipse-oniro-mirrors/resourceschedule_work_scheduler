/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "work_scheduler_extension_module_loader.h"

#include "work_scheduler_extension.h"

namespace OHOS::WorkScheduler {
WorkSchedulerExtensionModuleLoader::WorkSchedulerExtensionModuleLoader() = default;
WorkSchedulerExtensionModuleLoader::~WorkSchedulerExtensionModuleLoader() = default;

AbilityRuntime::Extension *WorkSchedulerExtensionModuleLoader::Create(
    const std::unique_ptr<AbilityRuntime::Runtime>& runtime) const
{
    return WorkSchedulerExtension::Create(runtime);
}

extern "C" __attribute__((visibility("default"))) void* OHOS_EXTENSION_GetExtensionModule()
{
    return &WorkSchedulerExtensionModuleLoader::GetInstance();
}
} // namespace OHOS::WorkScheduler