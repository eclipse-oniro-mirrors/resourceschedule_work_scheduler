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

#include "background_loader/background_loader_hisysevent_report.h"

#include <hisysevent.h>

using namespace OHOS;

namespace OHOS {
namespace WorkScheduler {
namespace BackgroundLoaderHiSysEvent {

namespace {
const std::string BACKGROUND_LOADER_RECORD_EVENT = "BACKGROUND_LOADER_RECORD";
const std::string BACKGROUND_LOADER_ANOMALOUS_EVENT = "BACKGROUND_LOADER_ANOMALOUS";
}
void ReportRecordSysEvent(const TaskInfo& taskInfo, const std::string& reason, const std::string& extReason)
{
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        BACKGROUND_LOADER_RECORD_EVENT, HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "UID", taskInfo.uid_, "PID", taskInfo.pid_, "BUNDLE_NAME", taskInfo.bundleName_, "APP_INDEX",
        taskInfo.appIndex_, "REASON", reason, "EXTEND_REASON", extReason);
}

void ReportAnomalousSysEvent(const TaskInfo& taskInfo, const std::string& reason, const std::string& extReason)
{
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        BACKGROUND_LOADER_ANOMALOUS_EVENT, HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "UID", taskInfo.uid_, "PID", taskInfo.pid_, "BUNDLE_NAME", taskInfo.bundleName_, "REASON", reason,
        "EXTEND_REASON", extReason);
}
} // namespace BackgroundLoaderHiSysEvent
} // namespace WorkScheduler
} // namespace OHOS
