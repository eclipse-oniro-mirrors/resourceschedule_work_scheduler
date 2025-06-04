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
void HiSysEventDeepIdleState(bool deepIdleState)
{
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        "DEEP_IDLE_STATE", HiviewDFX::HiSysEvent::EventType::STATISTIC, "STATE", deepIdleState);
}

void HiSysEventDeviceStandbyState(bool deviceStandbyState)
{
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        "DEVICE_STANDBY_STATE", HiviewDFX::HiSysEvent::EventType::STATISTIC, "STATE", deviceStandbyState);
}

void HiSysEventSystemPolicyLimit(const WorkSchedSystemPolicy& systemPolicy)
{
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        "SYSTEM_POLICY_LIMIT", HiviewDFX::HiSysEvent::EventType::STATISTIC, "POLICY", systemPolicy.GetInfo());
}

void HiSysEventException(const std::string& moduleName, const std::string& funcName, const std::string& exceptionInfo)
{
    struct HiSysEventParam params[] = {
        {
            .name = "MODULE_NAME",
            .t = HISYSEVENT_STRING,
            .v = { .s = const_cast<char *>(moduleName.c_str()) },
            .arraySize = 0
        },
        {
            .name = "FUNC_NAME",
            .t = HISYSEVENT_STRING,
            .v = { .s = const_cast<char *>(funcName.c_str()) },
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
        "WORK_SCHEDULER_RUNTIME_EXCEPTION",
        HISYSEVENT_STATISTIC,
        params,
        sizeof(params)/sizeof(params[0])
    );
}
} // namespace WorkSchedUtil
} // namespace WorkScheduler
} // namespace OHOS