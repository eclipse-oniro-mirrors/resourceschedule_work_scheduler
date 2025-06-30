/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "WorkSchedulerExtensionContext.proj.hpp"
#include "WorkSchedulerExtensionContext.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

namespace {
// To be implemented.
class WorkSchedulerExtensionContext_innerImpl {
public:
    WorkSchedulerExtensionContext_innerImpl()
    {
        // Don't forget to implement the constructor.
    }

    void StartServiceExtensionAbilitySync(uintptr_t want)
    {
        TH_THROW(std::runtime_error, "StartServiceExtensionAbilitySync not implemented");
    }

    void StopServiceExtensionAbilitySync(uintptr_t want)
    {
        TH_THROW(std::runtime_error, "StopServiceExtensionAbilitySync not implemented");
    }
};

::WorkSchedulerExtensionContext::WorkSchedulerExtensionContext_inner Create()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return taihe::make_holder<WorkSchedulerExtensionContext_innerImpl,
        ::WorkSchedulerExtensionContext::WorkSchedulerExtensionContext_inner>();
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_Create(Create);
// NOLINTEND