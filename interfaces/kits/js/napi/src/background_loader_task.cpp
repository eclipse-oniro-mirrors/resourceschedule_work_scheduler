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

#include "common.h"
#include "work_sched_hilog.h"
#include "work_sched_errors.h"
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
struct GetTaskInfoAsyncContext {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    bool isCallback;
    BackgroundLoaderTaskInfo taskInfo;
    int32_t taskId;
    ErrCode errCode;
};

bool GetTaskInfo(napi_env env, napi_callback_info info, BackgroundLoaderTaskInfo& taskInfo)
{
    size_t argc = TASK_INFO_PARAMS;
    napi_value argv[TASK_INFO_PARAMS] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != TASK_INFO_PARAMS) {
        Common::HandleErrCode(env, E_CHECK_WORKINFO_FAILED);
        return false;
    }
    if (!Common::MatchValueType(env, argv[TASK_INFO_INDEX], napi_object)) {
        Common::HandleErrCode(env, E_CHECK_WORKINFO_FAILED);
        return false;
    }

    int32_t taskId = Common::GetIntProperty(env, argv[TASK_INFO_INDEX], TASK_ID, E_WORKID_ERR);
    std::string abilityName = Common::GetStringProperty(env, argv[TASK_INFO_INDEX], ABILITY_NAME,
        E_BUNDLE_OR_ABILITY_NAME_ERR);
    if (abilityName == "") {
        WS_HILOGE("abilityName is invalid, failed.");
        Common::HandleErrCode(env, E_CHECK_WORKINFO_FAILED);
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
    Common::HandleErrCode(env, errCode);
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
    Common::HandleErrCode(env, errCode);
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
    Common::HandleErrCode(env, errCode);
    WS_HILOGD("Finish task napi end.");
    return Common::NapiGetNull(env);
}

GetTaskInfoAsyncContext* CreateGetTaskInfoAsyncContext(napi_env env)
{
    auto* context = new (std::nothrow) GetTaskInfoAsyncContext();
    if (context == nullptr) {
        WS_HILOGE("failed to create GetTaskInfoAsyncContext");
        return nullptr;
    }
    context->env = env;
    context->asyncWork = nullptr;
    context->deferred = nullptr;
    context->callback = nullptr;
    context->isCallback = false;
    context->errCode = ERR_OK;
    return context;
}

void DeleteGetTaskInfoAsyncContext(GetTaskInfoAsyncContext* context)
{
    if (context == nullptr) {
        return;
    }
    if (context->callback != nullptr) {
        napi_delete_reference(context->env, context->callback);
    }
    if (context->asyncWork != nullptr) {
        napi_delete_async_work(context->env, context->asyncWork);
    }
    delete context;
}

void GetTaskInfoExecute(napi_env env, void* data)
{
    auto* context = static_cast<GetTaskInfoAsyncContext*>(data);
    if (context == nullptr) {
        return;
    }
    context->errCode = WorkSchedulerSrvClient::GetInstance().GetTaskInfo(context->taskId, context->taskInfo);
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
    if (context->errCode == ERR_OK) {
        napi_resolve_deferred(env, context->deferred, result);
    } else {
        std::string errMsg = Common::FindErrMsg(env, context->errCode);
        int32_t errCodeInfo = Common::FindErrCode(env, context->errCode);
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

    napi_value result = nullptr;
    if (context->errCode == ERR_OK) {
        result = CreateTaskInfoResult(env, context->taskInfo);
    }
    HandlePromiseMode(env, context, result);
    DeleteGetTaskInfoAsyncContext(context);
}

napi_value GetTaskInfo(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Get task info begin.");
    constexpr uint32_t TASK_ID_INDEX = 0;
    constexpr uint32_t GET_TASK_INFO_MAX_PARAMS = 1;
    size_t argc = GET_TASK_INFO_MAX_PARAMS;
    napi_value argv[GET_TASK_INFO_MAX_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc != GET_TASK_INFO_MAX_PARAMS) {
        Common::HandleErrCode(env, E_CHECK_WORKINFO_FAILED);
        return Common::NapiGetNull(env);
    }
    if (!Common::MatchValueType(env, argv[TASK_ID_INDEX], napi_number)) {
        Common::HandleErrCode(env, E_CHECK_WORKINFO_FAILED);
        return Common::NapiGetNull(env);
    }

    auto* context = CreateGetTaskInfoAsyncContext(env);
    if (context == nullptr) {
        napi_throw_error(env, nullptr, "failed to create context");
        return Common::NapiGetNull(env);
    }

    int32_t taskId;
    NAPI_CALL(env, napi_get_value_int32(env, argv[TASK_ID_INDEX], &taskId));
    context->taskId = taskId;

    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &context->deferred, &promise));

    napi_value resource = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "GetTaskInfo", NAPI_AUTO_LENGTH, &resource));
    NAPI_CALL(env, napi_create_async_work(env, nullptr, resource, GetTaskInfoExecute, GetTaskInfoComplete,
        static_cast<void*>(context), &context->asyncWork));
    napi_queue_async_work(env, context->asyncWork);

    WS_HILOGD("Get task info napi end.");
    return promise;
}

}  // namespace WorkScheduler
}  // namespace OHOS