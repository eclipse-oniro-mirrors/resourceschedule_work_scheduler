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
#include "policy/power_mode_policy.h"

#include "battery_srv_client.h"
#include "power_mgr_client.h"
#include "power_mode_info.h"
#include "work_sched_hilog.h"

using namespace std;
using namespace OHOS::PowerMgr;

namespace OHOS {
namespace WorkScheduler {
const int32_t COUNT_POWER_MODE_CRUCIAL = 0;
const int32_t COUNT_POWER_MODE_NORMAL = 3;

PowerModePolicy::PowerModePolicy(shared_ptr<WorkPolicyManager> workPolicyManager)
{
    workPolicyManager_ = workPolicyManager;
}

PowerModePolicy::~PowerModePolicy()
{
}

int32_t PowerModePolicy::GetPolicyMaxRunning()
{
    int32_t res = COUNT_POWER_MODE_NORMAL;
    auto powerMode = PowerMgrClient::GetInstance().GetDeviceMode();
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    auto chargeState = BatterySrvClient::GetInstance().GetChargingStatus();
    if ((powerMode == PowerMode::POWER_SAVE_MODE || powerMode == PowerMode::EXTREME_POWER_SAVE_MODE) &&
        (chargeState == BatteryChargeState::CHARGE_STATE_NONE || chargeState == BatteryChargeState::CHARGE_STATE_DISABLE)) {
        res = COUNT_POWER_MODE_CRUCIAL; // save mode and not charge
    }
#endif
    WS_HILOGI("power mode: %{public}d, PolicyRes: %{public}d", powerMode, res);
    return res;
}

std::string PowerModePolicy::GetPolicyName()
{
    return "POWER_MODE_POLICY";
}
} // namespace WorkScheduler
} // namespace OHOS