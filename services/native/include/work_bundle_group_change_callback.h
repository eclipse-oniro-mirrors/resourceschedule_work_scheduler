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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_BUNDLE_GROUP_CHANGE_CALLBACK_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_BUNDLE_GROUP_CHANGE_CALLBACK_H
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
#include "bundle_active_group_callback_stub.h"
#include "bundle_active_group_callback_info.h"
#include "work_queue_manager.h"

namespace OHOS {
namespace WorkScheduler {
class WorkQueueManager;
class WorkBundleGroupChangeCallback : public DeviceUsageStats::BundleActiveGroupCallbackStub {
public:
    explicit WorkBundleGroupChangeCallback(std::shared_ptr<WorkQueueManager> workQueueManager);
    virtual ~WorkBundleGroupChangeCallback() = default;
    /*
    * function: OnBundleGroupChanged, bundleGroupChanged callback.
    * parameters: bundleActiveGroupCallbackInfo
    * return: void.
    */
    virtual void OnBundleGroupChanged(const DeviceUsageStats::BundleActiveGroupCallbackInfo
        &bundleActiveGroupCallbackInfo) override;
private:
    std::shared_ptr<WorkQueueManager> workQueueManager_;
};
}  // namespace WorkScheduler
}  // namespace OHOS
#endif
#endif  // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_BUNDLE_GROUP_CHANGE_CALLBACK_H