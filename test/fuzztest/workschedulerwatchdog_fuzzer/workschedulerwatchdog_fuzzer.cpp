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

#include "workschedulerwatchdog_fuzzer.h"

#include <cstdint>
#include <cstring>

#include "securec.h"
#include <memory>

#include "watchdog.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
int32_t FuzzInt32(const uint8_t* data, size_t size, size_t offset = 0)
{
    if (data == nullptr || size < offset + sizeof(int32_t)) {
        return 0;
    }
    int32_t value = 0;
    memcpy_s(&value, sizeof(value), data + offset, sizeof(value));
    return value;
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t)) {
        return false;
    }

    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (service == nullptr) {
        return false;
    }

    auto policyMgr = std::make_shared<WorkPolicyManager>(service);
    if (policyMgr == nullptr) {
        return false;
    }

    if (service->eventRunner_ == nullptr) {
        service->eventRunner_ = AppExecFwk::EventRunner::Create("work_sched_fuzz");
    }

    auto runner = service->eventRunner_;
    if (runner == nullptr) {
        return false;
    }

    Watchdog watchdog(policyMgr, runner);

    uint32_t watchdogId1 = static_cast<uint32_t>(FuzzInt32(data, size));
    int32_t interval1 = FuzzInt32(data, size, sizeof(int32_t));
    watchdog.AddWatchdog(watchdogId1, interval1);

    uint32_t watchdogId2 = static_cast<uint32_t>(FuzzInt32(data, size, sizeof(int32_t) * 2));
    int32_t interval2 = FuzzInt32(data, size, sizeof(int32_t) * 3);
    watchdog.AddWatchdog(watchdogId2, interval2);

    watchdog.RemoveWatchdog(watchdogId1);
    watchdog.RemoveWatchdog(watchdogId2);

    uint32_t watchdogId3 = static_cast<uint32_t>(FuzzInt32(data, size, sizeof(int32_t) * 4));
    int32_t interval3 = FuzzInt32(data, size, sizeof(int32_t) * 5);
    watchdog.AddWatchdog(watchdogId3, interval3);
    watchdog.RemoveWatchdog(watchdogId3);

    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
