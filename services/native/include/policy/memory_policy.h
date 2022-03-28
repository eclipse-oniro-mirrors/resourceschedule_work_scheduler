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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_MEMORY_POLICY_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_MEMORY_POLICY_H

#include <memory>

#include "ipolicy_filter.h"
#include "work_policy_manager.h"

namespace OHOS {
namespace WorkScheduler {
class MemoryPolicy : public IPolicyFilter {
public:
    explicit MemoryPolicy(std::shared_ptr<WorkPolicyManager> workPolicyManager);
    ~MemoryPolicy();
    /**
     * @brief Get policy max running.
     *
     * @return Res.
     */
    int32_t getPolicyMaxRunning() override;

    const int32_t INVALID_MEM = -1;
    const int32_t MEM_CRUCIAL = 1 * 1024 * 1024;
    const int32_t MEM_LOW = 2 * 1024 * 1024;
    const int32_t COUNT_MEMORY_CRUCIAL = 1;
    const int32_t COUNT_MEMORY_LOW = 2;
    const int32_t COUNT_MEMORY_NORMAL = 3;
private:
    int32_t GetMemAvailable();
    std::shared_ptr<WorkPolicyManager> workPolicyManager_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_MEMORY_POLICY_H