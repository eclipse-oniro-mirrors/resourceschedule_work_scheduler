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

#include <new>

#include "common.h"
#include "workscheduler_srv_client.h"
#include "work_sched_hilog.h"
#include "work_sched_errors.h"

namespace OHOS {
namespace WorkScheduler {
const uint32_t WORK_ID_INDEX = 0;
const uint32_t CALLBACK_INDEX = 1;
const uint32_t IS_LAST_WORK_TIME_OUT_MIN_PARAMS = 1;
const uint32_t IS_LAST_WORK_TIME_OUT_MAX_PARAMS = 2;

struct IsLastWorkTimeOutParamsInfo {
    int32_t workId = -1;
    napi_ref callback = nullptr;
};

struct AsyncCallbackIsLastWorkTimeOut : public AsyncWorkData {
    explicit AsyncCallbackIsLastWorkTimeOut(napi_env env) : AsyncWorkData(env) {}
    int32_t workId {-1};
    bool result {false};
};

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, IsLastWorkTimeOutParamsInfo &params)
{
    size_t argc = IS_LAST_WORK_TIME_OUT_MAX_PARAMS;
    napi_value argv[IS_LAST_WORK_TIME_OUT_MAX_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != IS_LAST_WORK_TIME_OUT_MAX_PARAMS && argc != IS_LAST_WORK_TIME_OUT_MIN_PARAMS) {
        Common::HandleParamErr(env, E_PARAM_NUMBER_ERR);
        return nullptr;
    }

    // argv[0] : workId
    if (!Common::MatchValueType(env, argv[WORK_ID_INDEX], napi_number)) {
        Common::HandleParamErr(env, E_WORKID_ERR);
        return nullptr;
    }
    napi_get_value_int32(env, argv[WORK_ID_INDEX], &params.workId);

    // argv[1]: callback
    if (argc == IS_LAST_WORK_TIME_OUT_MAX_PARAMS) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[CALLBACK_INDEX], &valuetype));
        if (valuetype != napi_function) {
            Common::HandleParamErr(env, E_CALLBACK_TYPE_ERR);
            return nullptr;
        }
        napi_create_reference(env, argv[CALLBACK_INDEX], 1, &params.callback);
    }
    return Common::NapiGetNull(env);
}

napi_value CreatAndQueueAsyncWork(napi_env env, AsyncCallbackIsLastWorkTimeOut* asyncCallbackInfo)
{
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, "IsLastWorkTimeOut", NAPI_AUTO_LENGTH, &resourceName));

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackIsLastWorkTimeOut *asyncCallbackInfo = static_cast<AsyncCallbackIsLastWorkTimeOut *>(data);
            asyncCallbackInfo->errorCode =
                WorkSchedulerSrvClient::GetInstance().IsLastWorkTimeout(asyncCallbackInfo->workId,
                asyncCallbackInfo->result);
            asyncCallbackInfo->errorMsg = Common::FindErrMsg(env, asyncCallbackInfo->errorCode);
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackIsLastWorkTimeOut *asyncCallbackInfo = static_cast<AsyncCallbackIsLastWorkTimeOut *>(data);
            std::unique_ptr<AsyncCallbackIsLastWorkTimeOut> callbackPtr {asyncCallbackInfo};
            if (asyncCallbackInfo != nullptr) {
                napi_value result = nullptr;
                if (asyncCallbackInfo->errorCode != ERR_OK) {
                    result = Common::NapiGetNull(env);
                } else {
                    napi_get_boolean(env, asyncCallbackInfo->result, &result);
                }
                Common::ReturnCallbackPromise(env, *asyncCallbackInfo, result);
            }
        },
        static_cast<AsyncCallbackIsLastWorkTimeOut *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));

    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    return nullptr;
}

napi_value IsLastWorkTimeOut(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Is LastWork TimeOut napi begin.");

    // Get params.
    IsLastWorkTimeOutParamsInfo params;
    if (ParseParameters(env, info, params) == nullptr) {
        napi_value ret = Common::JSParaError(env, params.callback);
        napi_delete_reference(env, params.callback);
        return ret;
    }

    napi_value promise = nullptr;
    AsyncCallbackIsLastWorkTimeOut *asyncCallbackInfo = new (std::nothrow) AsyncCallbackIsLastWorkTimeOut(env);
    if (!asyncCallbackInfo) {
        napi_value ret = Common::JSParaError(env, params.callback);
        napi_delete_reference(env, params.callback);
        return ret;
    }
    std::unique_ptr<AsyncCallbackIsLastWorkTimeOut> callbackPtr {asyncCallbackInfo};
    asyncCallbackInfo->workId = params.workId;
    WS_HILOGD("asyncCallbackInfo->workId: %{public}d", asyncCallbackInfo->workId);
    Common::PaddingAsyncWorkData(env, params.callback, *asyncCallbackInfo, promise);

    CreatAndQueueAsyncWork(env, asyncCallbackInfo);
    callbackPtr.release();

    WS_HILOGD("Is LastWork TimeOut napi end.");
    if (asyncCallbackInfo->isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}
} // namespace WorkScheduler
} // namespace OHOS