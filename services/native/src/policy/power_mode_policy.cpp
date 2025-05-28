/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "policy/power_mode_policy.h"

#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
#include "battery_srv_client.h"
#endif
#include "power_mgr_client.h"
#include "power_mode_info.h"
#include "work_sched_hilog.h"

using namespace std;
using namespace OHOS::PowerMgr;

namespace OHOS {
namespace WorkScheduler {
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
const int32_t COUNT_POWER_MODE_CRUCIAL = 1;
#endif
const int32_t COUNT_POWER_MODE_NORMAL = 3;

PowerModePolicy::PowerModePolicy(shared_ptr<WorkPolicyManager> workPolicyManager)
{
    workPolicyManager_ = workPolicyManager;
}

PowerModePolicy::~PowerModePolicy()
{
}

int32_t PowerModePolicy::GetPolicyMaxRunning(WorkSchedSystemPolicy& systemPolicy)
{
    int32_t res = COUNT_POWER_MODE_NORMAL;
    auto mode = PowerMgrClient::GetInstance().GetDeviceMode();
    if (mode == PowerMode::NORMAL_MODE || mode == PowerMode::PERFORMANCE_MODE) {
        WS_HILOGD("power mode: %{public}d, PolicyRes: %{public}d", mode, res);
        return res;
    }
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    auto charge = BatterySrvClient::GetInstance().GetChargingStatus();
    if (charge == BatteryChargeState::CHARGE_STATE_NONE || charge == BatteryChargeState::CHARGE_STATE_DISABLE) {
        res = COUNT_POWER_MODE_CRUCIAL;
        systemPolicy.powerMode = static_cast<uint32_t>(mode);
        systemPolicy.policyName = "POWER_MODE_POLICY";
        WS_HILOGI("charge: %{public}d, power mode: %{public}d, PolicyRes: %{public}d", charge, mode, res);
    }
#endif
    WS_HILOGD("power mode: %{public}d, PolicyRes: %{public}d", mode, res);
    return res;
}
} // namespace WorkScheduler
} // namespace OHOS