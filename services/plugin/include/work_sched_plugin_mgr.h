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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_PLUGIN_INCLUDE_WORK_SCHED_PLUGIN_MGR_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_PLUGIN_INCLUDE_WORK_SCHED_PLUGIN_MGR_H

#include "res_data.h"
#include "single_instance.h"
#include "plugin.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedPluginMgr : public ResourceSchedule::Plugin {
    DECLARE_SINGLE_INSTANCE(WorkSchedPluginMgr)
public:
    void Init() override;
    void Disable() override;
    void DispatchResource(const std::shared_ptr<ResourceSchedule::ResData>& resData) override;

private:
    std::atomic<bool> pluginEnable_ {false};
};
}  // namespace WorkScheduler
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_PLUGIN_INCLUDE_WORK_SCHED_PLUGIN_MGR_H