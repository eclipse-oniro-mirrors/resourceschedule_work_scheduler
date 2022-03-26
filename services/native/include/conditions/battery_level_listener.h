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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_CONDITIONS_BATTERY_LEVEL_LISTENER_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_CONDITIONS_BATTERY_LEVEL_LISTENER_H

#include <memory>

#include "common_event_subscriber.h"
#include "icondition_listener.h"
#include "work_queue_manager.h"

namespace OHOS {
namespace WorkScheduler {
class WorkQueueManager;
class BatteryLevelListener : public IConditionListener {
public:
    explicit BatteryLevelListener(std::shared_ptr<WorkQueueManager> workQueueManager);
    ~BatteryLevelListener();

    /**
     * @brief The OnConditionChanged callback.
     *
     * @param conditionType The condition type.
     * @param conditionVal The condition val.
     */
    void OnConditionChanged(WorkCondition::Type conditionType,
        std::shared_ptr<DetectorValue> conditionVal) override;
    /**
     * @brief Start.
     *
     * @return ERR_OK on success, others on failure.
     */
    bool Start() override;
    /**
     * @brief Stop.
     *
     * @return ERR_OK on success, others on failure.
     */
    bool Stop() override;
private:
    std::shared_ptr<WorkQueueManager> workQueueManager_;
    std::shared_ptr<EventFwk::CommonEventSubscriber> commonEventSubscriber = nullptr;
};

class BatteryLevelEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    BatteryLevelEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
        BatteryLevelListener &listener);
    ~BatteryLevelEventSubscriber() override = default;
    /**
     * @brief The OnReceiveEvent callback.
     *
     * @param data The dump data.
     */
    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
private:
    BatteryLevelListener &listener_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_CONDITIONS_BATTERY_LEVEL_LISTENER_H