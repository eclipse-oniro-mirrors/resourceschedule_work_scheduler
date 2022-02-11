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

#include "common.h"
#include "workscheduler_srv_client.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
static const int32_t CALLBACK_INDEX = 0;
static const int32_t OBTAIN_ALL_WORKS_MIN_PARAMS = 0;
static const int32_t OBTAIN_ALL_WORKS_MAX_PARAMS = 1;

struct AsyncCallbackInfoObtainAllWorks {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    std::list<std::shared_ptr<WorkInfo>> workInfoList;
    CallbackPromiseInfo info;
};

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, napi_ref &callback)
{
    size_t argc = OBTAIN_ALL_WORKS_MAX_PARAMS;
    napi_value argv[OBTAIN_ALL_WORKS_MAX_PARAMS] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    bool paramsCheck = (argc == OBTAIN_ALL_WORKS_MIN_PARAMS) || (argc == OBTAIN_ALL_WORKS_MAX_PARAMS);
    NAPI_ASSERT(env, paramsCheck, "Wrong number of arguments");

    // argv[0]: callback
    if (argc == OBTAIN_ALL_WORKS_MAX_PARAMS) {
        napi_create_reference(env, argv[CALLBACK_INDEX], 1, &callback);
    }
    return Common::NapiGetNull(env);
}

napi_value ObtainAllWorks(napi_env env, napi_callback_info info)
{
    WS_HILOGD("ObtainAllWorks napi begin.");

    // Get params.
    napi_ref callback = nullptr;
    if (ParseParameters(env, info, callback) == nullptr) {
        return Common::JSParaError(env, callback);
    }

    napi_value promise = nullptr;
    AsyncCallbackInfoObtainAllWorks *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoObtainAllWorks {.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, callback);
    }
    Common::PaddingCallbackPromiseInfo(env, callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "ObtainAllWorks", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoObtainAllWorks *asynccallbackinfo = (AsyncCallbackInfoObtainAllWorks *)data;
            asynccallbackinfo->info.errorCode =
                WorkSchedulerSrvClient::GetInstance().ObtainAllWorks(asynccallbackinfo->workInfoList);
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoObtainAllWorks *asynccallbackinfo = (AsyncCallbackInfoObtainAllWorks *)data;
            if (asynccallbackinfo != nullptr) {
                napi_value result = nullptr;
                if (asynccallbackinfo->info.errorCode != ERR_OK) {
                    result = Common::NapiGetNull(env);
                } else {
                    napi_create_array(env, &result);
                    int count = 0;
                    for (auto workInfo : asynccallbackinfo->workInfoList) {
                        napi_value napiWork = Common::GetNapiWorkInfo(env, workInfo);
                        napi_set_element(env, result, count, napiWork);
                        count++;
                    }
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
    WS_HILOGD("ObtainAllWorks napi end.");
    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}
}
}