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

#include "workschedulerconditionchecker_fuzzer.h"

#include <cstdint>
#include <cstring>
#include <memory>

#include "conditions/condition_checker.h"
#include "work_queue_manager.h"
#include "work_scheduler_service.h"
#include "work_info.h"
#include "work_condition.h"
#include "work_status.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr int32_t DEFAULT_UID = 10008;

int32_t FuzzInt32(const uint8_t* data, size_t size, size_t offset = 0)
{
    if (data == nullptr || size < offset + sizeof(int32_t)) {
        return 0;
    }
    int32_t value = 0;
    memcpy(&value, data + offset, sizeof(value));
    return value;
}

void PopulateQueue(WorkQueueManager& queueMgr, const uint8_t* data, size_t size)
{
    for (int32_t i = 0; i < 4; ++i) {
        WorkInfo workInfo;
        workInfo.SetWorkId(FuzzInt32(data, size, sizeof(int32_t) * i));
        workInfo.SetElement("fuzz_bundle_" + std::to_string(i), "fuzz_ability_" + std::to_string(i));
        workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(i % 8));
        workInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY);
        workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
        workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_USB);
        workInfo.RefreshUid(FuzzInt32(data, size, sizeof(int32_t)));

        int32_t uid = FuzzInt32(data, size, sizeof(int32_t) * 2);
        if (uid == 0) {
            uid = DEFAULT_UID;
        }
        auto workStatus = std::make_shared<WorkStatus>(workInfo, uid);
        queueMgr.AddWork(workStatus);
    }
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

    auto queueMgr = std::make_shared<WorkQueueManager>(service);
    if (queueMgr == nullptr) {
        return false;
    }

    PopulateQueue(*queueMgr, data, size);

    ConditionChecker checker(queueMgr);
    checker.CheckAllStatus();

    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
