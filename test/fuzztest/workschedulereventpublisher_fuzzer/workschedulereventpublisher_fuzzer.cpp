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

#include "workschedulereventpublisher_fuzzer.h"

#include <algorithm>
#include <string>

#include "event_publisher.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t MAX_FUZZ_STRING_LEN = 256;

std::string FuzzString(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return "";
    }
    size_t len = std::min(size, MAX_FUZZ_STRING_LEN);
    return std::string(reinterpret_cast<const char*>(data), len);
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    EventPublisher publisher;
    std::string result;
    std::string eventType = FuzzString(data, size);
    std::string eventValue = FuzzString(data, size / 2 + 1);
    publisher.PublishEvent(result, eventType, eventValue);
    publisher.PublishNetworkEvent(result, eventValue);
    publisher.PublishChargingEvent(result, eventValue);
    publisher.PublishStorageEvent(result, eventValue);
    publisher.PublishBatteryStatusEvent(result, eventValue);
    publisher.Dump(result, eventType, eventValue);
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
