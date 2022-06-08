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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_EVENT_PUBLISHER_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_EVENT_PUBLISHER_H

#include <string>
#include <vector>

namespace OHOS {
namespace WorkScheduler {
class EventPublisher {
public:
    explicit EventPublisher() = default;
    ~EventPublisher() = default;
    /**
     * @brief Publish event.
     *
     * @param dumpOption The dump option.
     * @param dumpInfo The dump info.
     */
    void PublishEvent(std::string &result, std::string &eventType, std::string &eventValue);
    /**
     * @brief Publish network event.
     *
     * @param dumpOption The dump option.
     * @param dumpInfo The dump info.
     */
    void PublishNetworkEvent(std::string &result, std::string &eventValue);
    /**
     * @brief Publish charging event.
     *
     * @param dumpOption The dump option.
     * @param dumpInfo The dump info.
     */
    void PublishChargingEvent(std::string &result, std::string &eventValue);
    /**
     * @brief Publish storage event.
     *
     * @param dumpOption The dump option.
     * @param dumpInfo The dump info.
     */
    void PublishStorageEvent(std::string &result, std::string &eventValue);
    /**
     * @brief Publishbattery status event.
     *
     * @param dumpOption The dump option.
     * @param dumpInfo The dump info.
     */
    void PublishBatteryStatusEvent(std::string &result, std::string &eventValue);

    void Dump(std::string &result, std::string &eventType, std::string &eventValue);
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_EVENT_PUBLISHER_H