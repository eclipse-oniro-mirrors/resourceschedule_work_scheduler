/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "exec_frequency.h"

#include "common.h"
#include "workscheduler_srv_client.h"
#include "work_sched_hilog.h"
#include "work_sched_errors.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr uint32_t PARAM_INDEX = 0;
constexpr uint32_t SET_PARAMS = 1;
}

napi_value SetExecFrequency(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Set exec frequency napi begin.");
    // Check params.
    size_t argc = SET_PARAMS;
    napi_value argv[SET_PARAMS] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != SET_PARAMS) {
        Common::HandleErrCode(env, E_PARAM_NUMBER_ERROR);
        return Common::NapiGetNull(env);
    }
    if (!Common::MatchValueType(env, argv[PARAM_INDEX], napi_object)) {
        Common::HandleErrCode(env, E_FREQUENCY_INFO_TYPE_ERROR);
        return Common::NapiGetNull(env);
    }

    // Get frequencyInfo and call service.
    FrequencyInfo frequencyInfo = FrequencyInfo();
    if (Common::GetFrequencyInfo(env, argv[PARAM_INDEX], frequencyInfo)) {
        ErrCode errCode = WorkSchedulerSrvClient::GetInstance().SetExecFrequency(frequencyInfo);
        Common::HandleErrCode(env, errCode);
    }
    WS_HILOGD("Set exec frequency napi end.");
    return Common::NapiGetNull(env);
}

napi_value ResetExecFrequency(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Reset exec frequency napi begin.");
    // Check params.
    size_t argc = SET_PARAMS;
    napi_value argv[SET_PARAMS] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != SET_PARAMS) {
        Common::HandleErrCode(env, E_PARAM_NUMBER_ERROR);
        return Common::NapiGetNull(env);
    }
    if (!Common::MatchValueType(env, argv[PARAM_INDEX], napi_number)) {
        Common::HandleErrCode(env, E_FREQUENCY_INFO_TYPE_ERROR);
        return Common::NapiGetNull(env);
    }
    int32_t uid = -1;
    napi_get_value_int32(env, argv[PARAM_INDEX], &uid);
    if (uid <= 0) {
        WS_HILOGE("uid is invalid, failed. uid: %{public}d", uid);
        Common::HandleErrCode(env, E_UID_ERROR);
        return Common::NapiGetNull(env);
    }
    ErrCode errCode = WorkSchedulerSrvClient::GetInstance().ResetExecFrequency(uid);
    Common::HandleErrCode(env, errCode);
    WS_HILOGD("Reset exec frequency napi end.");
    return Common::NapiGetNull(env);
}
} // namespace WorkScheduler
} // namespace OHOS