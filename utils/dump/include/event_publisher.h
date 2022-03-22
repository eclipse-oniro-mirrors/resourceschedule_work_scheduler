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

#ifndef WORK_SCHED_UTILS_DUMP_EVENT_PUBLISHER_H
#define WORK_SCHED_UTILS_DUMP_EVENT_PUBLISHER_H

#include <string>
#include <vector>

namespace OHOS {
namespace WorkScheduler {
class EventPublisher {
public:
    explicit EventPublisher() = default;
    ~EventPublisher() = default;
    void PublishEvent(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    void PublishNetworkEvent(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    void PublishChargingEvent(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    void PublishStorageEvent(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    void PublishbatteryStatusEvent(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
    void PublishbatteryLevelEvent(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // WORK_SCHED_UTILS_DUMP_EVENT_PUBLISHER_H