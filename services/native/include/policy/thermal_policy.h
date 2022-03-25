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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_THERMAL_POLICY_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_THERMAL_POLICY_H

#include "ipolicy_filter.h"
#include "work_policy_manager.h"

namespace OHOS {
namespace WorkScheduler {
class ThermalPolicy : public IPolicyFilter {
public:
    explicit ThermalPolicy(std::shared_ptr<WorkPolicyManager> workPolicyManager);
    ~ThermalPolicy();
    /**
     * @brief Get policy max running.
     *
     * @return res.
     */
    int32_t getPolicyMaxRunning() override;
private:
    std::shared_ptr<WorkPolicyManager> workPolicyManager_;
    const int32_t COUNT_THERMAL_CRUCIAL = 0;
    const int32_t COUNT_THERMAL_LOW = 1;
    const int32_t COUNT_THERMAL_NORMAL = 3;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_THERMAL_POLICY_H