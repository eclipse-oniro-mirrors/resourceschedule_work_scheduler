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
#include "obtain_all_works.h"

#include <list>
#include <new>

#include "common.h"
#include "workscheduler_srv_client.h"
#include "work_sched_hilog.h"
#include "work_sched_errors.h"

namespace OHOS {
namespace WorkScheduler {
const uint32_t CALLBACK_INDEX = 0;
const uint32_t OBTAIN_ALL_WORKS_MIN_PARAMS = 0;
const uint32_t OBTAIN_ALL_WORKS_MAX_PARAMS = 1;

struct AsyncCallbackInfoObtainAllWorks : public AsyncWorkData {
    explicit AsyncCallbackInfoObtainAllWorks(napi_env env) : AsyncWorkData(env) {}
    std::list<std::shared_ptr<WorkInfo>> workInfoList;
};

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, napi_ref &callback)
{
    size_t argc = OBTAIN_ALL_WORKS_MAX_PARAMS;
    napi_value argv[OBTAIN_ALL_WORKS_MAX_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    if (argc != OBTAIN_ALL_WORKS_MIN_PARAMS && argc != OBTAIN_ALL_WORKS_MAX_PARAMS) {
        Common::HandleParamErr(env, E_PARAM_NUMBER_ERR);
        return nullptr;
    }

    // argv[0]: callback
    if (argc == OBTAIN_ALL_WORKS_MAX_PARAMS) {
        napi_create_reference(env, argv[CALLBACK_INDEX], 1, &callback);
    }
    return Common::NapiGetNull(env);
}

napi_value ObtainAllWorks(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Obtain All Works napi begin.");

    // Get params.
    napi_ref callback = nullptr;
    if (ParseParameters(env, info, callback) == nullptr) {
        return Common::JSParaError(env, callback);
    }

    napi_value promise = nullptr;
    AsyncCallbackInfoObtainAllWorks *asyncCallbackInfo =
        new (std::nothrow) AsyncCallbackInfoObtainAllWorks(env);
    if (!asyncCallbackInfo) {
        return Common::JSParaError(env, callback);
    }
    std::unique_ptr<AsyncCallbackInfoObtainAllWorks> callbackPtr {asyncCallbackInfo};
    Common::PaddingAsyncWorkData(env, callback, *asyncCallbackInfo, promise);

    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, "ObtainAllWorks", NAPI_AUTO_LENGTH, &resourceName));

    NAPI_CALL(env, napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoObtainAllWorks *asyncCallbackInfo = static_cast<AsyncCallbackInfoObtainAllWorks *>(data);
            asyncCallbackInfo->errorCode =
                WorkSchedulerSrvClient::GetInstance().ObtainAllWorks(asyncCallbackInfo->workInfoList);
            asyncCallbackInfo->errorMsg = Common::FindErrMsg(env, asyncCallbackInfo->errorCode);
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoObtainAllWorks *asyncCallbackInfo = static_cast<AsyncCallbackInfoObtainAllWorks *>(data);
            std::unique_ptr<AsyncCallbackInfoObtainAllWorks> callbackPtr {asyncCallbackInfo};
            if (asyncCallbackInfo != nullptr) {
                napi_value result = nullptr;
                if (asyncCallbackInfo->errorCode != ERR_OK) {
                    result = Common::NapiGetNull(env);
                } else {
                    napi_create_array(env, &result);
                    uint32_t count = 0;
                    for (auto workInfo : asyncCallbackInfo->workInfoList) {
                        napi_value napiWork = Common::GetNapiWorkInfo(env, workInfo);
                        napi_set_element(env, result, count, napiWork);
                        count++;
                    }
                }
                Common::ReturnCallbackPromise(env, *asyncCallbackInfo, result);
            }
        },
        static_cast<AsyncCallbackInfoObtainAllWorks *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));

    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    callbackPtr.release();
    WS_HILOGD("Obtain All Works napi end.");
    if (asyncCallbackInfo->isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}
} // namespace WorkScheduler
} // namespace OHOS