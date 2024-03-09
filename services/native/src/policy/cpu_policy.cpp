/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "policy/cpu_policy.h"

#include <fstream>
#include <securec.h>
#include "work_sched_hilog.h"

using namespace std;

namespace OHOS {
namespace WorkScheduler {
const int32_t CPU_CRUCIAL = 66;
const int32_t CPU_LOW = 33;
const int32_t INIT_CPU = 0;
const int32_t COUNT_CPU_CRUCIAL = 1;
const int32_t COUNT_CPU_LOW = 2;
const int32_t COUNT_CPU_NORMAL = 3;
const int32_t SUCCESS_CODE = 0;
const int32_t CPU_UPPER_LIMIT = 100;
const int32_t UNIT = 100;

CpuPolicy::CpuPolicy(shared_ptr<WorkPolicyManager> workPolicyManager)
{
    workPolicyManager_ = workPolicyManager;
}

CpuPolicy::~CpuPolicy()
{
}

int32_t CpuPolicy::GetCpuUsage()
{
    if (workPolicyManager_ != nullptr) {
        int32_t dumpSetCpu = workPolicyManager_->GetDumpSetCpuUsage();
        if (0 < dumpSetCpu && dumpSetCpu <= CPU_UPPER_LIMIT) {
            WS_HILOGD("dump set cpu: %{public}d", dumpSetCpu);
            return dumpSetCpu;
        }
    }
    int32_t cpuUsage = INIT_CPU;
    auto collector = OHOS::HiviewDFX::UCollectClient::CpuCollector::Create();
    auto collectResult = collector->GetSysCpuUsage();
    int32_t retCode = collectResult.retCode;
    WS_HILOGD("retCode of collectResult: %{public}d", retCode);
    if (retCode == SUCCESS_CODE) {
        cpuUsage = static_cast<int>(collectResult.data * UNIT);
    }
    return cpuUsage;
}

int32_t CpuPolicy::GetPolicyMaxRunning()
{
    int32_t cpuUsage = GetCpuUsage();
    WS_HILOGI("cpu_usage: %{public}d", cpuUsage);
    if (cpuUsage >= CPU_CRUCIAL) {
        return COUNT_CPU_CRUCIAL;
    }
    if (cpuUsage >= CPU_LOW) {
        return COUNT_CPU_LOW;
    }
    WS_HILOGI("cpu left normal");
    return COUNT_CPU_NORMAL;
}
} // namespace WorkScheduler
} // namespace OHOS