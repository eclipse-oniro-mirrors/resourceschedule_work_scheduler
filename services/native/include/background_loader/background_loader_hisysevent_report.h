/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_HISYSEVENT_REPORT_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_HISYSEVENT_REPORT_H

#include <string>

#include "background_loader_mgr.h"

namespace OHOS {
namespace WorkScheduler {
namespace BackgroundLoaderHiSysEvent {
    /**
     * @brief Send a background loader record event to the system.
     *
     * @param taskInfo task info.
     * @param reason Reason for reporting.
     */
    void ReportRecordSysEvent(const TaskInfo& taskInfo, const std::string& reason, const std::string& extReason);

    /**
     * @brief Send a background loader anomalous event to the system.
     *
     * @param taskInfo task info.
     * @param reason Reason for reporting.
     */
    void ReportAnomalousSysEvent(const TaskInfo& taskInfo, const std::string& reason, const std::string& extReason);
} // namespace BackgroundLoaderHiSysEvent
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_HISYSEVENT_REPORT_H
