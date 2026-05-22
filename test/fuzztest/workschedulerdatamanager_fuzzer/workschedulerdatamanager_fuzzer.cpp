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

#include "workschedulerdatamanager_fuzzer.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "securec.h"
#include <list>
#include <string>

#include "work_sched_data_manager.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t MAX_FUZZ_STRING_LEN = 256;

std::string FuzzString(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return "fuzz_bundle";
    }
    size_t len = std::min(size, MAX_FUZZ_STRING_LEN);
    return std::string(reinterpret_cast<const char*>(data), len);
}

int32_t FuzzInt32(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t)) {
        return 0;
    }
    int32_t value = 0;
    memcpy_s(&value, sizeof(value), data, sizeof(value));
    return value;
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    if (dataManager == nullptr) {
        return false;
    }
    bool sleepFlag = (size > 0) && ((data[0] & 1) != 0);
    bool deepIdleFlag = (size > 1) && ((data[1] & 1) != 0);
    dataManager->SetDeviceSleep(sleepFlag);
    (void)dataManager->GetDeviceSleep();
    dataManager->SetDeepIdle(deepIdleFlag);
    (void)dataManager->GetDeepIdle();

    std::string bundleName = FuzzString(data, size);
    bool addFlag = (size > 2) && ((data[2] & 1) != 0);
    dataManager->OnDeviceStandyWhitelistChanged(bundleName, addFlag);
    (void)dataManager->IsInDeviceStandyWhitelist(bundleName);
    std::list<std::string> whitelist = { bundleName, "fuzz_bundle2" };
    dataManager->AddDeviceStandyWhitelist(whitelist);
    (void)dataManager->IsDeviceStandyWhitelistEmpty();
    dataManager->ClearDeviceStandyWhitelist();

    dataManager->OnDeviceStandyRestrictlistChanged(bundleName, addFlag);
    (void)dataManager->IsInDeviceStandyRestrictlist(bundleName);
    dataManager->AddDeviceStandyRestrictlist(whitelist);
    dataManager->ClearDeviceStandyRestrictlist();

    int32_t userId = FuzzInt32(data, size);
    int32_t appGroup = FuzzInt32(data, size) % 10;
    dataManager->AddGroup(bundleName, userId, appGroup);
    int32_t foundGroup = 0;
    (void)dataManager->FindGroup(bundleName, userId, foundGroup);
    dataManager->ClearGroup(bundleName, userId);
    dataManager->ClearAllGroup();
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
