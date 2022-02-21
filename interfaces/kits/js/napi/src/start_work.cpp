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
#include "start_work.h"

#include "common.h"
#include "workscheduler_srv_client.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
const int32_t WORK_INFO_INDEX = 0;
const int32_t START_WORK_PARAMS = 1;

napi_value StartWork(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Start work napi begin.");

    // Check params.
    size_t argc = START_WORK_PARAMS;
    napi_value argv[START_WORK_PARAMS] = {0};
    napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    NAPI_ASSERT(env, argc == START_WORK_PARAMS, "parameter error!");
    bool matchFlag = Common::MatchValueType(env, argv[WORK_INFO_INDEX], napi_object);
    NAPI_ASSERT(env, matchFlag, "Type error, it should be object");

    // Get workInfo and call service.
    WorkInfo workInfo = WorkInfo();
    bool result;
    if (!Common::GetWorkInfo(env, argv[WORK_INFO_INDEX], workInfo)) {
        result = false;
    } else {
        result = WorkSchedulerSrvClient::GetInstance().StartWork(workInfo);
    }
    napi_value napiValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &napiValue));
    WS_HILOGD("Start work napi end.");
    return napiValue;
}
}
}
