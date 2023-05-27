/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_STANDBY_STATE_CHANGE_CALLBACK_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_STANDBY_STATE_CHANGE_CALLBACK_H
#ifdef  DEVICE_STANDBY_ENABLE
#include "standby_service_subscriber_stub.h"
#include "work_queue_manager.h"

namespace OHOS {
namespace WorkScheduler {
class WorkStandbyStateChangeCallback : public DevStandbyMgr::StandbyServiceSubscriberStub {
public:
    explicit WorkStandbyStateChangeCallback(std::shared_ptr<WorkQueueManager> workQueueManager);
    ~WorkStandbyStateChangeCallback() override = default;

    /*
    * @brief callback when device_standby state change.
    *
    * @param napped current state is nap or not.
    * @param sleeping current state is sleep or not.
    */
    void OnDeviceIdleMode(bool napped, bool sleeping) override;

   /*
    * @brief callback when device_standby allowList change.
    *
    * @param uid of application who apply allowList.
    * @param name bundleName of application who apply allowList.
    * @param allowType allowType mean strategy type, such as WORK_SCHEDULER.
    * @param added true mean starting allow, false mean ending allow.
    */
    void OnAllowListChanged(int32_t uid, const std::string& name, uint32_t allowType, bool added) override;
private:
    std::shared_ptr<WorkQueueManager> workQueueManager_;
    bool isSleep_ {false};
};
}  // namespace WorkScheduler
}  // namespace OHOS
#endif
#endif  // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_STANDBY_STATE_CHANGE_CALLBACK_H