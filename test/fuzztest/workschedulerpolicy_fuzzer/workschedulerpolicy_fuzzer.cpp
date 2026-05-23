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

#include "workschedulerpolicy_fuzzer.h"

#include <cstdint>
#include <cstring>

#include "securec.h"
#include <memory>
#include <string>

#include "work_sched_system_policy.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"
#include "detector_value.h"
#include "policy/cpu_policy.h"
#include "policy/memory_policy.h"
#include "policy/power_mode_policy.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t STRING_LEN_MAX = 256;
constexpr size_t OFFSET_MEM_AVAILABLE = 1;
constexpr size_t OFFSET_THERMAL_LEVEL = 2;
constexpr size_t OFFSET_POWER_MODE = 3;
constexpr size_t OFFSET_ALLOW_RUNNING_COUNT = 4;
constexpr size_t OFFSET_CHAINED_POLICY_BASE = 5;
constexpr int32_t CHAINED_POLICY_COUNT = 4;


int32_t FuzzInt32(const uint8_t* data, size_t size, size_t offset = 0)
{
    if (data == nullptr || size < offset + sizeof(int32_t)) {
        return 0;
    }
    int32_t value = 0;
    memcpy_s(&value, sizeof(value), data + offset, sizeof(value));
    return value;
}

uint32_t FuzzUint32(const uint8_t* data, size_t size, size_t offset = 0)
{
    return static_cast<uint32_t>(FuzzInt32(data, size, offset));
}

std::string FuzzString(const uint8_t* data, size_t size, size_t maxLen = STRING_LEN_MAX)
{
    if (data == nullptr || size == 0) {
        return "";
    }
    size_t len = std::min(size, maxLen);
    return std::string(reinterpret_cast<const char*>(data), len);
}

void FuzzSystemPolicyStruct(const uint8_t* data, size_t size)
{
    WorkSchedSystemPolicy policy;
    policy.cpuUsage = FuzzInt32(data, size);
    policy.memAvailable = FuzzInt32(data, size, sizeof(int32_t) * OFFSET_MEM_AVAILABLE);
    policy.thermalLevel = FuzzInt32(data, size, sizeof(int32_t) * OFFSET_THERMAL_LEVEL);
    policy.powerMode = FuzzUint32(data, size, sizeof(int32_t) * OFFSET_POWER_MODE);

    // Test GetInfo with fuzzed fields
    (void)policy.GetInfo();

    // Test SetPolicyName with various policy name strings and allowRunningCount values
    std::string policyName = FuzzString(data, size);
    int32_t allowRunningCount = FuzzInt32(data, size, sizeof(int32_t) * OFFSET_ALLOW_RUNNING_COUNT);
    policy.SetPolicyName(policyName, allowRunningCount);
    (void)policy.GetInfo();

    // Test chained SetPolicyName calls to exercise the "&" concatenation path
    for (int32_t i = 0; i < CHAINED_POLICY_COUNT; ++i) {
        std::string name = "POLICY_" + std::to_string(i);
        int32_t count = FuzzInt32(data, size, sizeof(int32_t) * (OFFSET_CHAINED_POLICY_BASE + i));
        policy.SetPolicyName(name, count);
    }
    (void)policy.GetInfo();
}

void FuzzDetectorValue(const uint8_t* data, size_t size)
{
    int32_t intVal = FuzzInt32(data, size);
    int32_t timeVal = FuzzInt32(data, size, sizeof(int32_t));
    bool boolVal = (size > 0) && ((data[0] & 1) != 0);
    std::string strVal = FuzzString(data, size);

    DetectorValue dv(intVal, timeVal, boolVal, strVal);

    // Exercise access to all fields
    (void)dv.intVal;
    (void)dv.timeVal;
    (void)dv.boolVal;
    (void)dv.strVal;

    // Test copy construction
    DetectorValue dvCopy(dv);
    (void)dvCopy.intVal;
    (void)dvCopy.strVal;

    // Test with extreme values
    DetectorValue dvExtreme(INT32_MAX, INT32_MIN, true, std::string(STRING_LEN_MAX, 'A'));
    (void)dvExtreme.intVal;

    DetectorValue dvEmpty(0, 0, false, "");
    (void)dvEmpty.strVal;
}

void FuzzPolicyGetMaxRunning(const uint8_t* data, size_t size)
{
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (service == nullptr) {
        return;
    }

    auto policyMgr = std::make_shared<WorkPolicyManager>(service);
    if (policyMgr == nullptr) {
        return;
    }

    // Test CpuPolicy
    CpuPolicy cpuPolicy(policyMgr);
    WorkSchedSystemPolicy cpuSysPolicy;
    cpuSysPolicy.cpuUsage = FuzzInt32(data, size);
    (void)cpuPolicy.GetPolicyMaxRunning(cpuSysPolicy);
    (void)cpuSysPolicy.GetInfo();

    // Test MemoryPolicy
    MemoryPolicy memPolicy(policyMgr);
    WorkSchedSystemPolicy memSysPolicy;
    memSysPolicy.memAvailable = FuzzInt32(data, size, sizeof(int32_t));
    (void)memPolicy.GetPolicyMaxRunning(memSysPolicy);
    (void)memSysPolicy.GetInfo();

    // Test PowerModePolicy
    PowerModePolicy powerPolicy(policyMgr);
    WorkSchedSystemPolicy powerSysPolicy;
    powerSysPolicy.powerMode = FuzzUint32(data, size, sizeof(int32_t) * OFFSET_THERMAL_LEVEL);
    (void)powerPolicy.GetPolicyMaxRunning(powerSysPolicy);
    (void)powerSysPolicy.GetInfo();
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t)) {
        return false;
    }

    FuzzSystemPolicyStruct(data, size);
    FuzzDetectorValue(data, size);
    FuzzPolicyGetMaxRunning(data, size);
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
