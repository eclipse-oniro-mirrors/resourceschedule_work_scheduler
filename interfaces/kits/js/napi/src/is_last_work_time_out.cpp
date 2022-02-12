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
#include "is_last_work_time_out.h"

#include "common.h"
#include "workscheduler_srv_client.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
static const int32_t WORK_ID_INDEX = 0;
static const int32_t CALLBACK_INDEX = 1;
static const int32_t IS_LAST_WORK_TIME_OUT_MIN_PARAMS = 1;
static const int32_t IS_LAST_WORK_TIME_OUT_MAX_PARAMS = 2;

struct IsLastWorkTimeOutParamsInfo {
    int32_t workId;
    napi_ref callback = nullptr;
};

struct AsyncCallbackIsLastWorkTimeOut {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    int32_t workId;
    bool result;
    CallbackPromiseInfo info;
};

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, IsLastWorkTimeOutParamsInfo &params)
{
    size_t argc = IS_LAST_WORK_TIME_OUT_MAX_PARAMS;
    napi_value argv[IS_LAST_WORK_TIME_OUT_MAX_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    bool paramsCheck = (argc == IS_LAST_WORK_TIME_OUT_MAX_PARAMS) || (argc == IS_LAST_WORK_TIME_OUT_MIN_PARAMS);
    NAPI_ASSERT(env, paramsCheck, "Wrong number of arguments");

    // argv[0] : workId
    bool matchFlag = Common::MatchValueType(env, argv[WORK_ID_INDEX], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    napi_get_value_int32(env, argv[WORK_ID_INDEX], &params.workId);

    // argv[1]: callback
    if (argc == IS_LAST_WORK_TIME_OUT_MAX_PARAMS) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[CALLBACK_INDEX], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[CALLBACK_INDEX], 1, &params.callback);
    }
    return Common::NapiGetNull(env);
}

napi_value IsLastWorkTimeOut(napi_env env, napi_callback_info info)
{
    WS_HILOGD("IsLastWorkTimeOut napi begin.");

    // Get params.
    IsLastWorkTimeOutParamsInfo params;
    if (ParseParameters(env, info, params) == nullptr) {
        return Common::JSParaError(env, params.callback);
    }

    napi_value promise = nullptr;
    AsyncCallbackIsLastWorkTimeOut *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackIsLastWorkTimeOut {.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    asynccallbackinfo->workId = params.workId;
    WS_HILOGD("asynccallbackinfo->workId: %{public}d", asynccallbackinfo->workId);
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "IsLastWorkTimeOut", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackIsLastWorkTimeOut *asynccallbackinfo = (AsyncCallbackIsLastWorkTimeOut *)data;
            asynccallbackinfo->info.errorCode =
                WorkSchedulerSrvClient::GetInstance().IsLastWorkTimeout(asynccallbackinfo->workId,
                asynccallbackinfo->result);
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackIsLastWorkTimeOut *asynccallbackinfo = (AsyncCallbackIsLastWorkTimeOut *) data;
            if (asynccallbackinfo != nullptr) {
                napi_value result = nullptr;
                if (asynccallbackinfo->info.errorCode != ERR_OK) {
                    result = Common::NapiGetNull(env);
                } else {
                    napi_get_boolean(env, asynccallbackinfo->result, &result);
                }
                Common::ReturnCallbackPromise(env, asynccallbackinfo->info, result);
                if (asynccallbackinfo->info.callback != nullptr) {
                    napi_delete_reference(env, asynccallbackinfo->info.callback);
                }
                napi_delete_async_work(env, asynccallbackinfo->asyncWork);
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    WS_HILOGD("IsLastWorkTimeOut napi end.");
    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}
}
}