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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_SYSTEM_POLICY_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_SYSTEM_POLICY_H

#include <string>

namespace OHOS {
namespace WorkScheduler {
struct WorkSchedSystemPolicy {
    std::string policyName;
    int32_t cpuUsage;
    int32_t memAvailable;
    // interfaces/inner_api/native/include/thermal_level_info.h
    int32_t thermalLevel;
    // interfaces/inner_api/native/include/power_mode_info.h
    uint32_t powerMode;

    inline std::string GetInfo() const;
};

std::string WorkSchedSystemPolicy::GetInfo() const
{
    return "policyName: " + policyName + ", cpuUsage: " + std::to_string(cpuUsage) +
           ", memAvailable: " + std::to_string(memAvailable) + ", thermalLevel: " + std::to_string(thermalLevel) +
           ", powerMode: " + std::to_string(powerMode);
}
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_SYSTEM_POLICY_H