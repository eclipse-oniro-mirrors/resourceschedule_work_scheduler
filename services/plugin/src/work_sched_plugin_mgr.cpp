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

#include "background_loader_mgr.h"
#include "work_sched_plugin_mgr.h"
#include "background_task_observer_plugin_adapter.h"
#include "work_sched_hilog.h"
#include "plugin_mgr.h"
#include "res_type.h"
#include "res_data.h"
#include "res_sched_json_util.h"

namespace OHOS {
namespace WorkScheduler {
using namespace OHOS::ResourceSchedule;
namespace {
    static constexpr std::string_view LIB_NAME = "libworkschedservice.z.so";
}
IMPLEMENT_SINGLE_INSTANCE(WorkSchedPluginMgr);

void WorkSchedPluginMgr::Init()
{
    if (pluginEnable_.load()) {
        WS_HILOGI("init WorkSchedPluginMgr repeatedly");
        return;
    }
    BackgroundTaskObserverPluginAdapter::GetInstance().Init();
    PluginMgr::GetInstance().SubscribeResource(std::string(LIB_NAME),
        ResType::RES_TYPE_EFFICIENCY_RESOURCES_STATE_CHANGED);
    PluginMgr::GetInstance().SubscribeResource(std::string(LIB_NAME),
        ResType::RES_TYPE_START_BACKGROUND_LOADER_TASK);
    pluginEnable_.store(true);
    WS_HILOGI("WorkSchedPluginMgr init succeed");
}

void WorkSchedPluginMgr::Disable()
{
    BackgroundTaskObserverPluginAdapter::GetInstance().UnInit();
    PluginMgr::GetInstance().UnSubscribeResource(std::string(LIB_NAME),
        ResType::RES_TYPE_EFFICIENCY_RESOURCES_STATE_CHANGED);
    PluginMgr::GetInstance().UnSubscribeResource(std::string(LIB_NAME),
        ResType::RES_TYPE_START_BACKGROUND_LOADER_TASK);
    pluginEnable_.store(false);
}

void WorkSchedPluginMgr::DispatchResource(const std::shared_ptr<ResourceSchedule::ResData>& resData)
{
    if (!pluginEnable_.load() || resData == nullptr) {
        WS_HILOGE("WorkSchedPluginMgr not enable or data is nullptr");
        return;
    }
    switch (resData->resType) {
        case ResType::RES_TYPE_EFFICIENCY_RESOURCES_STATE_CHANGED: {
            BackgroundTaskObserverPluginAdapter::GetInstance().OnEfficiencyResourcesStateChanged(
                resData->value, resData->payload);
            break;
        }
        case ResType::RES_TYPE_START_BACKGROUND_LOADER_TASK: {
            backgroundLoaderMgr::GetInstance().HandleBackgroundLoaderTask(resData);
            break;
        }
        default: {
            return;
        }
    }
}

extern "C" bool OnPluginInit(std::string& libName)
{
    if (libName != std::string(LIB_NAME)) {
        WS_HILOGE("lib name is not match");
        return false;
    }
    WorkSchedPluginMgr::GetInstance().Init();
    WS_HILOGI("WorkSchedPluginMgr OnPluginInit success");
    return true;
}

extern "C" void OnPluginDisable()
{
    WorkSchedPluginMgr::GetInstance().Disable();
    WS_HILOGI("WorkSchedPluginMgr OnPluginDisable success");
}

extern "C" void OnDispatchResource(const std::shared_ptr<ResourceSchedule::ResData>& resData)
{
    WorkSchedPluginMgr::GetInstance().DispatchResource(resData);
}
}  // namespace WorkScheduler
}  // namespace OHOS