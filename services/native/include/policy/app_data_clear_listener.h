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
#ifndef WORK_SCHED_SERVICES_POLICY_APP_DATA_CLEAR_LISTENER_H
#define WORK_SCHED_SERVICES_POLICY_APP_DATA_CLEAR_LISTENER_H

#include "ipolicy_listener.h"
#include "common_event_subscriber.h"
#include "work_policy_manager.h"

namespace OHOS {
using namespace EventFwk;
namespace WorkScheduler {
class WorkPolicyManager;
class AppDataClearListener : public IPolicyListener {
public:
    AppDataClearListener(std::shared_ptr<WorkPolicyManager> workPolicyManager);
    ~AppDataClearListener();

    void OnPolicyChanged(PolicyType policyType, std::shared_ptr<DetectorValue> detectorVal) override;
    bool Start() override;
    bool Stop() override;
private:
    std::shared_ptr<WorkPolicyManager> workPolicyManager_;
    std::shared_ptr<CommonEventSubscriber> commonEventSubscriber = nullptr;
};

class AppDataClearSubscriber : public CommonEventSubscriber {
public:
    AppDataClearSubscriber(const CommonEventSubscribeInfo &subscribeInfo, AppDataClearListener &listener);
    ~AppDataClearSubscriber() override = default;
    void OnReceiveEvent(const CommonEventData &data) override;
private:
    AppDataClearListener &listener_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // WORK_SCHED_SERVICES_POLICY_APP_DATA_CLEAR_LISTENER_H