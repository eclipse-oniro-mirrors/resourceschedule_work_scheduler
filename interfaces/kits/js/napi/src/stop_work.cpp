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
#include "work_sched_errors.h"

namespace OHOS {
namespace WorkScheduler {
const uint32_t WORK_INFO_INDEX = 0;
const uint32_t NEED_CANCEL_INDEX = 1;
const uint32_t STOP_WORK_PARAMS = 2;

napi_value StopWork(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Stop Work napi begin.");

    // Check params.
    size_t argc = STOP_WORK_PARAMS;
    napi_value argv[STOP_WORK_PARAMS] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != STOP_WORK_PARAMS) {
        Common::HandleParamErr(env, E_PARAM_NUMBER_ERR);
        return Common::NapiGetNull(env);
    }
    if (!Common::MatchValueType(env, argv[WORK_INFO_INDEX], napi_object)) {
        Common::HandleParamErr(env, E_WORK_INFO_TYPE_ERR);
        return Common::NapiGetNull(env);
    }
    if (!Common::MatchValueType(env, argv[NEED_CANCEL_INDEX], napi_boolean)) {
        Common::HandleParamErr(env, E_NEED_CANCLE_TYPE_ERR);
        return Common::NapiGetNull(env);
    }

    // get params
    bool needCancel = false;
    napi_get_value_bool(env, argv[NEED_CANCEL_INDEX], &needCancel);

    // Check workInfo and call service.
    WorkInfo workInfo = WorkInfo();
    ErrCode errCode = E_WORK_INFO_TYPE_ERR;
    if (Common::GetWorkInfo(env, argv[WORK_INFO_INDEX], workInfo)) {
        if (needCancel) {
            errCode = WorkSchedulerSrvClient::GetInstance().StopAndCancelWork(workInfo);
        } else {
            errCode = WorkSchedulerSrvClient::GetInstance().StopWork(workInfo);
        }
    }
    Common::HandleErrCode(env, errCode);
    WS_HILOGD("Stop Work napi end.");
    return Common::NapiGetNull(env);
}
} // namespace WorkScheduler
} // namespace OHOS