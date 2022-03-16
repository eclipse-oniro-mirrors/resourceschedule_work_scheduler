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
#include "stop_work.h"

#include "common.h"
#include "workscheduler_srv_client.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
const int32_t WORK_INFO_INDEX = 0;
const int32_t NEED_CANCEL_INDEX = 1;
const int32_t STOP_WORK_PARAMS = 2;

napi_value StopWork(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Stop Work napi begin.");

    // Check params.
    int32_t argc = STOP_WORK_PARAMS;
    napi_value argv[STOP_WORK_PARAMS] = {0};
    napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    NAPI_ASSERT(env, argc == STOP_WORK_PARAMS, "parameter error!");
    bool matchFlag = Common::MatchValueType(env, argv[WORK_INFO_INDEX], napi_object);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is object");
    matchFlag = Common::MatchValueType(env, argv[NEED_CANCEL_INDEX], napi_boolean);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is boolean");

    // get params
    WorkInfo workInfo = WorkInfo();
    bool getWorkRes = Common::GetWorkInfo(env, argv[WORK_INFO_INDEX], workInfo);
    bool needCancel = false;
    napi_get_value_bool(env, argv[NEED_CANCEL_INDEX], &needCancel);

    // Check workInfo and call service.
    bool result;
    if (!getWorkRes) {
        WS_HILOGD("Work info create failed.");
        result = false;
    } else {
        if (needCancel) {
            result = WorkSchedulerSrvClient::GetInstance().StopAndCancelWork(workInfo);
        } else {
            result = WorkSchedulerSrvClient::GetInstance().StopWork(workInfo);
        }
    }
    napi_value napiValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &napiValue));
    WS_HILOGD("Stop Work napi end.");
    return napiValue;
}
}
}