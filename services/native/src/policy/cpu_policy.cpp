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
#include "work_sched_hilog.h"
#include "cpu_collector_client.h"

using namespace std;

namespace OHOS {
namespace WorkScheduler {
const int32_t CPU_HIGH = 60;
const int32_t CPU_NORMAL = 50;
const int32_t CPU_LOW = 30;
const int32_t INIT_CPU = 0;
const int32_t COUNT_CPU_MAX = 0;
const int32_t COUNT_CPU_HIGH = 1;
const int32_t COUNT_CPU_NORMAL = 2;
const int32_t COUNT_CPU_LOW = 3;
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
            WS_HILOGI("dump set cpu: %{public}d", dumpSetCpu);
            return dumpSetCpu;
        }
    }
    int32_t cpuUsage = INIT_CPU;
    auto collector = OHOS::HiviewDFX::UCollectClient::CpuCollector::Create();
    auto collectResult = collector->GetSysCpuUsage();
    int32_t retCode = collectResult.retCode;
    WS_HILOGD("retCode of collectResult: %{public}d", retCode);
    if (retCode == OHOS::HiviewDFX::UCollect::UcError::SUCCESS) {
        cpuUsage = static_cast<int>(collectResult.data * UNIT);
    }
    return cpuUsage;
}

int32_t CpuPolicy::GetPolicyMaxRunning()
{
    int32_t cpuUsage = GetCpuUsage();
    int32_t policyRes;
    if (cpuUsage < CPU_LOW) {
        policyRes = COUNT_CPU_LOW;
    } else if (cpuUsage >= CPU_LOW && cpuUsage < CPU_NORMAL) {
        policyRes = COUNT_CPU_NORMAL;
    } else if (cpuUsage >= CPU_NORMAL && cpuUsage < CPU_HIGH) {
        policyRes = COUNT_CPU_HIGH;
    } else {
        policyRes = COUNT_CPU_MAX;
    }
    WS_HILOGD("cpu_usage: %{public}d, policyRes: %{public}d", cpuUsage, policyRes);
    return policyRes;
}

std::string CpuPolicy::GetPolicyName()
{
    return "CPU_POLICY";
}
} // namespace WorkScheduler
} // namespace OHOS