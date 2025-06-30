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

#include "ohos.WorkSchedulerExtensionAbility.proj.hpp"
#include "ohos.WorkSchedulerExtensionAbility.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

namespace {
// To be implemented.

class WorkSchedulerExtensionAbilityImpl {
public:
    WorkSchedulerExtensionAbilityImpl()
    {
        // Don't forget to implement the constructor.
    }

    uintptr_t getContext()
    {
        TH_THROW(std::runtime_error, "getContext not implemented");
    }

    void setContext(uintptr_t param)
    {
        TH_THROW(std::runtime_error, "setContext not implemented");
    }

    void onWorkStart(::ohos::resourceschedule::workScheduler::WorkInfo const &work)
    {
        TH_THROW(std::runtime_error, "onWorkStart not implemented");
    }

    void onWorkStop(::ohos::resourceschedule::workScheduler::WorkInfo const &work)
    {
        TH_THROW(std::runtime_error, "onWorkStop not implemented");
    }
};
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
// NOLINTEND