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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_PLUGIN_INCLUDE_EFFICIENCY_RESOURCES_PLUGIN_ADAPTER_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_PLUGIN_INCLUDE_EFFICIENCY_RESOURCES_PLUGIN_ADAPTER_H

#include "single_instance.h"
#include "nlohmann/json.hpp"
#include "resource_callback_info.h"
#include "scheduler_bg_task_subscriber.h"

namespace OHOS {
namespace WorkScheduler {
class BackgroundTaskObserverPluginAdapter {
    DECLARE_SINGLE_INSTANCE(BackgroundTaskObserverPluginAdapter)
public:
    void Init();
    void UnInit();
    void OnEfficiencyResourcesStateChanged(int32_t stateType, const nlohmann::json& payload);
    void HandleCloudConfigUpdateEvent(int32_t stateType, const nlohmann::json &payload);
    
private:
    std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo> UnmarshallingResourceCallbackInfo(
        const nlohmann::json& payload);
    std::shared_ptr<WorkScheduler::SchedulerBgTaskSubscriber> backgroundStateObserver_ = nullptr;
};
}  // namespace WorkScheduler
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_PLUGIN_INCLUDE_EFFICIENCY_RESOURCES_PLUGIN_ADAPTER_H