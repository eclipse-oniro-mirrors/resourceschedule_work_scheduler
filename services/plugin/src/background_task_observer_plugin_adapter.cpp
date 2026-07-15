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

#include "background_task_observer_plugin_adapter.h"
#include "res_type.h"
#include "res_sched_json_util.h"
#include "work_sched_hilog.h"
#include "work_sched_config.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace WorkScheduler {
IMPLEMENT_SINGLE_INSTANCE(BackgroundTaskObserverPluginAdapter)
using namespace OHOS::ResourceSchedule;
using namespace OHOS::BackgroundTaskMgr;

std::shared_ptr<ResourceCallbackInfo> BackgroundTaskObserverPluginAdapter::UnmarshallingResourceCallbackInfo(
    const nlohmann::json& payload)
{
    int32_t uid = 0;
    int32_t pid = 0;
    int32_t resourceNumber = 0;
    std::string bundleName;

    if (!ResCommonUtil::ParseStringParameterFromJson("bundleName", bundleName, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("pid", pid, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("uid", uid, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("resourceNumber", resourceNumber, payload)) {
        WS_HILOGE("BackgroundTaskObserverPluginAdapter UnmarshallingResourceCallbackInfo nullptr");
        return nullptr;
    }
    return std::make_shared<BackgroundTaskMgr::ResourceCallbackInfo>(
        uid, pid, static_cast<uint32_t>(resourceNumber), bundleName);
}

void BackgroundTaskObserverPluginAdapter::OnEfficiencyResourcesStateChanged(
    int32_t stateType, const nlohmann::json& payload)
{
    if (!backgroundStateObserver_) {
        WS_HILOGE("backgroundStateObserver_ nullptr");
        return;
    }

    auto resourceInfo = UnmarshallingResourceCallbackInfo(payload);
    if (resourceInfo == nullptr) {
        WS_HILOGE("resourceInfo nullptr");
        return;
    }

    switch (stateType) {
        case ResType::EfficiencyResourcesStatus::APP_EFFICIENCY_RESOURCES_APPLY:
            backgroundStateObserver_->OnAppEfficiencyResourcesApply(resourceInfo);
            break;
        case ResType::EfficiencyResourcesStatus::APP_EFFICIENCY_RESOURCES_RESET:
            backgroundStateObserver_->OnAppEfficiencyResourcesReset(resourceInfo);
            break;
        case ResType::EfficiencyResourcesStatus::PROC_EFFICIENCY_RESOURCES_APPLY:
            backgroundStateObserver_->OnProcEfficiencyResourcesApply(resourceInfo);
            break;
        case ResType::EfficiencyResourcesStatus::PROC_EFFICIENCY_RESOURCES_RESET:
            backgroundStateObserver_->OnProcEfficiencyResourcesReset(resourceInfo);
            break;
        default:
            WS_HILOGE("OnEfficiencyResourcesStateChanged failed, unknown stateType: %{public}d", stateType);
            break;
    }
}

void BackgroundTaskObserverPluginAdapter::HandleCloudConfigUpdateEvent(int32_t stateType,
    const nlohmann::json &payload)
{
    if (stateType == SUSPEND_MANAGER_SYSTEM_ABILITY_ID) {
        auto ret = DelayedSingleton<WorkSchedulerConfig>::GetInstance()->UpdateSusMgrCloudConfig(payload);
        if (!ret) {
            WS_HILOGE("suspend manager cloud config update fail.");
        }
    } else if (stateType == BACKGROUND_TASK_MANAGER_SERVICE_ID) {
        bool ret = DelayedSingleton<WorkSchedulerConfig>::GetInstance()->UpdateBgMgrCloudConfig(payload);
        if (!ret) {
            WS_HILOGE("background task cloud config update fail.");
        }
    }
}

void BackgroundTaskObserverPluginAdapter::Init()
{
    if (backgroundStateObserver_ != nullptr) {
        WS_HILOGE("BackgroundTaskObserverPluginAdapter has been initialized");
        return;
    }

    backgroundStateObserver_ = std::make_shared<WorkScheduler::SchedulerBgTaskSubscriber>();
    if (!backgroundStateObserver_) {
        WS_HILOGE("malloc backgroundStateObserver_ failed");
        return;
    }
    WS_HILOGD("BackgroundTaskObserverPluginAdapter init succeed");
}

void BackgroundTaskObserverPluginAdapter::UnInit()
{
    backgroundStateObserver_ = nullptr;
    WS_HILOGD("BackgroundTaskObserverPluginAdapter uninit");
}
}  // namespace WorkScheduler
}  // namespace OHOS