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

#include "workscheduleractivegroupconfig_fuzzer.h"

#include <algorithm>
#include <string>

#include "work_sched_config.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t MAX_FUZZ_STRING_LEN = 1000;

std::string FuzzString(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return "{}";
    }
    size_t len = std::min(size, MAX_FUZZ_STRING_LEN);
    return std::string(reinterpret_cast<const char*>(data), len);
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    auto config = DelayedSingleton<WorkSchedulerConfig>::GetInstance();
    if (config == nullptr) {
        return false;
    }
    std::string configData = FuzzString(data, size);
    config->InitActiveGroupWhitelist(configData);
    std::string bundleName = "fuzz_bundle";
    if (!configData.empty()) {
        bundleName.assign(configData.begin(), configData.begin() + std::min(configData.size(), size_t(64)));
    }
    (void)config->IsInActiveGroupWhitelist(bundleName);
    (void)config->IsInActiveGroupWhitelist("");
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
