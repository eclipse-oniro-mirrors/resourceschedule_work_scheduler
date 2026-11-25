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
#include <string>
#include <errors.h>

#include "common.h"
#include "work_sched_hilog.h"
#include "work_sched_errors.h"
#include "background_loader/background_loader_errors.h"
#include "background_loader_napi_helper.h"
#include "background_loader_task_info.h"
#include "workscheduler_srv_client.h"

#include "background_loader_task.h"

namespace {
constexpr uint32_t TASK_INFO_INDEX = 0;
constexpr uint32_t TASK_INFO_PARAMS = 1;

const char* ABILITY_NAME = "abilityName";
const char* TASK_ID = "taskId";
}

namespace OHOS {
namespace WorkScheduler {
using namespace BackgroundLoader;
struct GetTaskInfoAsyncContext : public AsyncWorkData {
    explicit GetTaskInfoAsyncContext(napi_env env) : AsyncWorkData(env) {}
    BackgroundLoaderTaskInfo taskInfo;
    int32_t taskId {0};
};

bool GetTaskInfo(napi_env env, napi_callback_info info, BackgroundLoaderTaskInfo& taskInfo)
{
    size_t argc = TASK_INFO_PARAMS;
    napi_value argv[TASK_INFO_PARAMS] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != TASK_INFO_PARAMS) {
        BackgroundLoaderNapiHelper::HandleBackgroundLoaderError(env, BackgroundLoader::E_PARAM_ERROR);
        return false;
    }
    if (!Common::MatchValueType(env, argv[TASK_INFO_INDEX], napi_object)) {
        BackgroundLoaderNapiHelper::HandleBackgroundLoaderError(env, BackgroundLoader::E_PARAM_ERROR);
        return false;
    }

    int32_t taskId = BackgroundLoaderNapiHelper::GetIntProperty(env, argv[TASK_INFO_INDEX], TASK_ID,
        BackgroundLoader::E_PARAM_ERROR);
    std::string abilityName = BackgroundLoaderNapiHelper::GetStringProperty(env, argv[TASK_INFO_INDEX], ABILITY_NAME,
        BackgroundLoader::E_PARAM_ERROR);
    if (abilityName == "") {
        WS_HILOGE("abilityName is invalid, failed.");
        BackgroundLoaderNapiHelper::HandleBackgroundLoaderError(env, BackgroundLoader::E_ABILITY_NAME_INCONFORMITY);
        return false;
    }
    taskInfo.SetAbilityName(abilityName);
    taskInfo.SetTaskId(taskId);
    return true;
}

napi_value RegisterTask(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Register task napi begin.");
    BackgroundLoaderTaskInfo taskInfo;
    if (!GetTaskInfo(env, info, taskInfo)) {
        return Common::NapiGetNull(env);
    }
    ErrCode errCode = WorkSchedulerSrvClient::GetInstance().RegisterTask(taskInfo);
    BackgroundLoaderNapiHelper::HandleBackgroundLoaderError(env, errCode);
    WS_HILOGD("Register task napi end.");
    return Common::NapiGetNull(env);
}

napi_value UnregisterTask(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Unregister task napi begin.");
    BackgroundLoaderTaskInfo taskInfo;
    if (!GetTaskInfo(env, info, taskInfo)) {
        return Common::NapiGetNull(env);
    }
    ErrCode errCode = WorkSchedulerSrvClient::GetInstance().UnregisterTask(taskInfo);
    BackgroundLoaderNapiHelper::HandleBackgroundLoaderError(env, errCode);
    WS_HILOGD("Unregister task napi end.");
    return Common::NapiGetNull(env);
}

napi_value FinishTask(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Finish task napi begin.");
    BackgroundLoaderTaskInfo taskInfo;
    if (!GetTaskInfo(env, info, taskInfo)) {
        return Common::NapiGetNull(env);
    }
    ErrCode errCode = WorkSchedulerSrvClient::GetInstance().FinishTask(taskInfo);
    BackgroundLoaderNapiHelper::HandleBackgroundLoaderError(env, errCode);
    WS_HILOGD("Finish task napi end.");
    return Common::NapiGetNull(env);
}

void GetTaskInfoExecute(napi_env env, void* data)
{
    auto* context = static_cast<GetTaskInfoAsyncContext*>(data);
    if (context == nullptr) {
        return;
    }
    context->errorCode = WorkSchedulerSrvClient::GetInstance().GetTaskInfo(context->taskId, context->taskInfo);
}

napi_value CreateTaskInfoResult(napi_env env, const BackgroundLoaderTaskInfo& taskInfo)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value taskIdValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, taskInfo.GetTaskId(), &taskIdValue));
    NAPI_CALL(env, napi_set_named_property(env, result, TASK_ID, taskIdValue));

    napi_value abilityNameValue = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, taskInfo.GetAbilityName().c_str(),
        NAPI_AUTO_LENGTH, &abilityNameValue));
    NAPI_CALL(env, napi_set_named_property(env, result, ABILITY_NAME, abilityNameValue));

    return result;
}

void HandlePromiseMode(napi_env env, GetTaskInfoAsyncContext* context, napi_value result)
{
    if (context->deferred == nullptr) {
        return;
    }
    if (context->errorCode == ERR_OK) {
        napi_resolve_deferred(env, context->deferred, result);
    } else {
        int32_t errCodeInfo = 0;
        std::string errMsg = "";
        ResolveErrorInfo(context->errorCode, errCodeInfo, errMsg);
        napi_value error = nullptr;
        napi_value eCode = nullptr;
        napi_value eMsg = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, errCodeInfo, &eCode));
        NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, errMsg.c_str(), errMsg.length(), &eMsg));
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &error));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, error, "code", eCode));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, error, "message", eMsg));
        NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, context->deferred, error));
    }
}

void GetTaskInfoComplete(napi_env env, napi_status status, void* data)
{
    auto* context = static_cast<GetTaskInfoAsyncContext*>(data);
    if (context == nullptr) {
        return;
    }
    std::unique_ptr<GetTaskInfoAsyncContext> callbackPtr {context};

    napi_value result = nullptr;
    if (context->errorCode == ERR_OK) {
        result = CreateTaskInfoResult(env, context->taskInfo);
    }
    HandlePromiseMode(env, context, result);
}

napi_value GetTaskInfo(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Get task info begin.");
    constexpr uint32_t taskIdIndex = 0;
    constexpr uint32_t getTaskInfoMaxParams = 1;
    size_t argc = getTaskInfoMaxParams;
    napi_value argv[getTaskInfoMaxParams] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc != getTaskInfoMaxParams) {
        BackgroundLoaderNapiHelper::HandleBackgroundLoaderError(env, BackgroundLoader::E_PARAM_ERROR);
        return Common::NapiGetNull(env);
    }
    if (!Common::MatchValueType(env, argv[taskIdIndex], napi_number)) {
        BackgroundLoaderNapiHelper::HandleBackgroundLoaderError(env, BackgroundLoader::E_PARAM_ERROR);
        return Common::NapiGetNull(env);
    }

    int32_t taskId = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[taskIdIndex], &taskId));

    auto* context = new (std::nothrow) GetTaskInfoAsyncContext(env);
    if (context == nullptr) {
        napi_throw_error(env, nullptr, "failed to create context");
        return Common::NapiGetNull(env);
    }
    std::unique_ptr<GetTaskInfoAsyncContext> contextPtr {context};
    context->taskId = taskId;

    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &context->deferred, &promise));

    napi_value resource = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "GetTaskInfo", NAPI_AUTO_LENGTH, &resource));
    NAPI_CALL(env, napi_create_async_work(env, nullptr, resource, GetTaskInfoExecute, GetTaskInfoComplete,
        static_cast<void*>(context), &context->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, context->asyncWork));
    contextPtr.release();

    WS_HILOGD("Get task info napi end.");
    return promise;
}

}  // namespace WorkScheduler
}  // namespace OHOS