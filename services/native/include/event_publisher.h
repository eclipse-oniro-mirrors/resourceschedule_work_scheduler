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
     * @param result The dump result.
     * @param eventType The event type.
     * @param eventValue The event value.
     */
    void PublishEvent(std::string &result, std::string &eventType, std::string &eventValue);
    /**
     * @brief Publish network event.
     *
     * @param result The dump result.
     * @param eventValue The event value.
     */
    void PublishNetworkEvent(std::string &result, std::string &eventValue);
    /**
     * @brief Publish charging event.
     *
     * @param result The dump result.
     * @param eventValue The event value.
     */
    void PublishChargingEvent(std::string &result, std::string &eventValue);
    /**
     * @brief Publish storage event.
     *
     * @param result The dump result.
     * @param eventValue The event value.
     */
    void PublishStorageEvent(std::string &result, std::string &eventValue);
    /**
     * @brief Publish battery status event.
     *
     * @param result The dump result.
     * @param eventValue The event value.
     */
    void PublishBatteryStatusEvent(std::string &result, std::string &eventValue);
    /**
     * @brief Dump event info.
     *
     * @param result The dump result.
     * @param eventType The event type.
     * @param eventValue The event value.
     */
    void Dump(std::string &result, std::string &eventType, std::string &eventValue);
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_EVENT_PUBLISHER_H