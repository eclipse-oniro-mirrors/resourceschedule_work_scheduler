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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_HISYSEVENT_REPORT_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_HISYSEVENT_REPORT_H

#include "work_sched_system_policy.h"

namespace OHOS {
namespace WorkScheduler {
namespace WorkSchedUtil {
    struct ServiceState {
        std::string eventName;
        int32_t state;
    };

    /**
     * @brief Send a state changed event to the system.
     *
     * @param state Whether the system state changed.
     */
    void HiSysEventStateChanged(const ServiceState state);

    /**
     * @brief Send a system policy limit event to the system.
     *
     * @param systemPolicy The system policy limit.
     */
    void HiSysEventSystemPolicyLimit(const WorkSchedSystemPolicy& systemPolicy);

    /**
     * @brief Send a exception event to the system.
     *
     * @param errCode the code of the exception.
     * @param exceptionInfo the exception info.
     */
    void HiSysEventException(int32_t errCode, const std::string& exceptionInfo);
} // namespace WorkSchedUtil
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_HISYSEVENT_REPORT_H