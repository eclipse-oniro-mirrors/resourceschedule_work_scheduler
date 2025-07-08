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

#include <hisysevent_c.h>
#include <hisysevent.h>

#include "work_sched_utils.h"
#include "work_sched_hisysevent_report.h"

using namespace OHOS;

namespace OHOS {
namespace WorkScheduler {
namespace WorkSchedUtil {
void HiSysEventStateChanged(const ServiceState state)
{
    char* events[] = { const_cast<char *>(state.eventName.c_str()) };
    int32_t states[] = { state.state };
    int64_t timestamps[] = { WorkSchedUtils::GetCurrentTimeMs() };
    struct HiSysEventParam params[] = {
        {
            .name = "EVENT",
            .t = HISYSEVENT_STRING_ARRAY,
            .v = { .array = events },
            .arraySize = sizeof(events) / sizeof(events[0])
        },
        {
            .name = "STATE",
            .t = HISYSEVENT_INT32_ARRAY,
            .v = { .array = states },
            .arraySize = sizeof(states) / sizeof(states[0])
        },
        {
            .name = "TIMESTAMP",
            .t = HISYSEVENT_INT64_ARRAY,
            .v = { .array = timestamps },
            .arraySize = sizeof(timestamps) / sizeof(timestamps[0])
        }
    };

    OH_HiSysEvent_Write(
        HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        "SERVICE_STATE_CHANGE",
        HISYSEVENT_STATISTIC,
        params,
        sizeof(params) / sizeof(params[0]));
}

void HiSysEventSystemPolicyLimit(const WorkSchedSystemPolicy& systemPolicy)
{
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        "SYSTEM_POLICY_LIMIT", HiviewDFX::HiSysEvent::EventType::STATISTIC, "POLICY", systemPolicy.GetInfo());
}

void HiSysEventException(int32_t errCode, const std::string& exceptionInfo)
{
    struct HiSysEventParam params[] = {
        {
            .name = "ERR_CODE",
            .t = HISYSEVENT_INT32,
            .v = { .i32 = errCode },
            .arraySize = 0
        },
        {
            .name = "EXCEPTION_INFO",
            .t = HISYSEVENT_STRING,
            .v = { .s = const_cast<char *>(exceptionInfo.c_str()) },
            .arraySize = 0
        },
    };

    OH_HiSysEvent_Write(
        HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        "WORK_SCHEDULER_RTE",
        HISYSEVENT_STATISTIC,
        params,
        sizeof(params) / sizeof(params[0]));
}
} // namespace WorkSchedUtil
} // namespace WorkScheduler
} // namespace OHOS