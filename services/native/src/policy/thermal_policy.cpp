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
#include "policy/thermal_policy.h"

#include "thermal_mgr_client.h"
#include "work_sched_hilog.h"

using namespace std;
using namespace OHOS::PowerMgr;

namespace OHOS {
namespace WorkScheduler {
const int32_t COUNT_THERMAL_CRUCIAL = 0;
const int32_t COUNT_THERMAL_LOW = 1;
const int32_t COUNT_THERMAL_NORMAL = 3;

ThermalPolicy::ThermalPolicy(shared_ptr<WorkPolicyManager> workPolicyManager)
{
    workPolicyManager_ = workPolicyManager;
}

ThermalPolicy::~ThermalPolicy()
{
}

int32_t ThermalPolicy::GetPolicyMaxRunning()
{
    auto& thermalMgrClient = ThermalMgrClient::GetInstance();
    ThermalLevel thermalLevel = thermalMgrClient.GetThermalLevel();
    int32_t res;
    if (thermalLevel >= ThermalLevel::WARM) {
        res = COUNT_THERMAL_CRUCIAL;
    } else if (thermalLevel < ThermalLevel::WARM && thermalLevel >= ThermalLevel::NORMAL) {
        res = COUNT_THERMAL_LOW;
    } else {
        res = COUNT_THERMAL_NORMAL;
    }
    WS_HILOGI("ThermalLevel:%{public}d, PolicyRes:%{public}d", thermalLevel, res);
    return res;
}
} // namespace WorkScheduler
} // namespace OHOS