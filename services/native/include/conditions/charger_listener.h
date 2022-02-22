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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_CONDITIONS_CHARGER_LISTENER_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_CONDITIONS_CHARGER_LISTENER_H

#include <memory>

#include "common_event_subscriber.h"
#include "icondition_listener.h"
#include "work_queue_manager.h"

namespace OHOS {
namespace WorkScheduler {
class WorkQueueManager;
class ChargerListener : public IConditionListener {
public:
    ChargerListener(std::shared_ptr<WorkQueueManager> workQueueManager);
    ~ChargerListener();
    
    void OnConditionChanged(WorkCondition::Type conditionType,
        std::shared_ptr<DetectorValue> conditionVal) override;
    bool Start() override;
    bool Stop() override;
private:
    std::shared_ptr<WorkQueueManager> workQueueManager_;
    std::shared_ptr<EventFwk::CommonEventSubscriber> commonEventSubscriber = nullptr;
};

class ChargerEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    ChargerEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
        ChargerListener &listener);
    ~ChargerEventSubscriber() override = default;
    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
private:
    ChargerListener &listener_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_CONDITIONS_CHARGER_LISTENER_H