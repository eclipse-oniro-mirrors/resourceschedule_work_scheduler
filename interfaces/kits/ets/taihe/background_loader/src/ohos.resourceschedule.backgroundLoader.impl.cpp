/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include "ohos.resourceschedule.backgroundLoader.impl.hpp"
#include "taihe/runtime.hpp"
#include "workscheduler_srv_client.h"
#include "background_loader_task_info.h"
#include "work_sched_hilog.h"
#include "work_sched_errors.h"
#include "common.h"

using namespace taihe;
using namespace ohos::resourceschedule::backgroundLoader;
using namespace OHOS::WorkScheduler;
using namespace OHOS;

void RegisterTask(const ::ohos::resourceschedule::backgroundLoader::TaskInfo& taskInfo)
{
    BackgroundLoaderTaskInfo info (taskInfo.taskId, std::string(taskInfo.abilityName));

    ErrCode errCode = WorkSchedulerSrvClient::GetInstance().RegisterTask(info);
    if (errCode != ERR_OK) {
        auto errMsg = Common::FindErrMsg(errCode);
        WS_HILOGE("RegisterTask fail: %{public}s", errMsg.c_str());
        set_business_error(Common::FindErrCode(errCode), errMsg);
    }
}

void UnregisterTask(const ::ohos::resourceschedule::backgroundLoader::TaskInfo& taskInfo)
{
    BackgroundLoaderTaskInfo info (taskInfo.taskId, std::string(taskInfo.abilityName));

    ErrCode errCode = WorkSchedulerSrvClient::GetInstance().UnregisterTask(info);
    if (errCode != ERR_OK) {
        auto errMsg = Common::FindErrMsg(errCode);
        WS_HILOGE("UnregisterTask fail: %{public}s", errMsg.c_str());
        set_business_error(Common::FindErrCode(errCode), errMsg);
    }
}

void FinishTask(const ::ohos::resourceschedule::backgroundLoader::TaskInfo& taskInfo)
{
    BackgroundLoaderTaskInfo info (taskInfo.taskId, std::string(taskInfo.abilityName));

    ErrCode errCode = WorkSchedulerSrvClient::GetInstance().FinishTask(info);
    if (errCode != ERR_OK) {
        auto errMsg = Common::FindErrMsg(errCode);
        WS_HILOGE("FinishTask fail: %{public}s", errMsg.c_str());
        set_business_error(Common::FindErrCode(errCode), errMsg);
    }
}

::ohos::resourceschedule::backgroundLoader::TaskInfo GetTaskInfoPromise(int32_t taskId)
{
    BackgroundLoaderTaskInfo info;
    ErrCode errCode = WorkSchedulerSrvClient::GetInstance().GetTaskInfo(taskId, info);
    if (errCode != ERR_OK) {
        auto errMsg = Common::FindErrMsg(errCode);
        WS_HILOGE("GetTaskInfoPromise fail: %{public}s", errMsg.c_str());
        set_business_error(Common::FindErrCode(errCode), errMsg);
    }
    ::ohos::resourceschedule::backgroundLoader::TaskInfo taskInfo(info.taskId, info.abilityName);
    return taskInfo;
}

TH_EXPORT_CPP_API_RegisterTask(RegisterTask);
TH_EXPORT_CPP_API_UnregisterTask(UnregisterTask);
TH_EXPORT_CPP_API_FinishTask(FinishTask);
TH_EXPORT_CPP_API_GetTaskInfoforPromise(GetTaskInfoPromise);