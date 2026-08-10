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
constexpr uint32_t FREQUENCY_INFO_INDEX = 0;
constexpr uint32_t SET_EXEX_FREQUENCY_PAEAMS = 1;
}

napi_value SetExecFrequency(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Set exec frequency napi begin");

    // Check params.
    size_t argc = SET_EXEX_FREQUENCY_PAEAMS;
    napi_value argv[SET_EXEX_FREQUENCY_PAEAMS] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != SET_EXEX_FREQUENCY_PAEAMS) {
        Common::HandleErrCode(env, E_PARAM_NUMBER_ERR);
        return Common::NapiGetNull(env);
    }
    if (Common::MatchValueType(env, argv[FREQUENCY_INFO_INDEX], napi_object)) {
        Common::HandleErrCode(env, E_FERQUENCY_INFO_ERR);
        return Common::NapiGetNull(env);
    }

    // Get frequencyInfo anc call service.
    FrequencyInfo frequencyInfo = FrequencyInfo();
    if (Common::GetFrequencyInfo(env, argv[FREQUENCY_INFO_INDEX], frequencyInfo)) {
        ErrCode errCode = WorkSchedulerSrvClient::GetInstance().SetExecFrequency(frequencyInfo);
        Common::HandleErrCode(env, errCode);
    }
    WS_HILOGD("Set exec frequency napi end");
    return Common::NapiGetNull(env);
}

napi_value ResetExecFrequency(napi_env env, napi_callback_info info)
{
    WS_HILOGD("Reset exec frequency napi begin");

    // Check params.
    size_t argc = SET_EXEX_FREQUENCY_PAEAMS;
    napi_value argv[SET_EXEX_FREQUENCY_PAEAMS] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != SET_EXEX_FREQUENCY_PAEAMS) {
        Common::HandleErrCode(env, E_PARAM_NUMBER_ERR);
        return Common::NapiGetNull(env);
    }
    if (Common::MatchValueType(env, argv[FREQUENCY_INFO_INDEX], napi_object)) {
        Common::HandleErrCode(env, E_FERQUENCY_INFO_ERR);
        return Common::NapiGetNull(env);
    }

    // Get frequencyInfo anc call service.
    FrequencyInfo frequencyInfo = FrequencyInfo();
    if (Common::GetFrequencyInfo(env, argv[FREQUENCY_INFO_INDEX], frequencyInfo)) {
        ErrCode errCode = WorkSchedulerSrvClient::GetInstance().ResetExecFrequency(frequencyInfo);
        Common::HandleErrCode(env, errCode);
    }
    WS_HILOGD("Reset exec frequency napi end");
    return Common::NapiGetNull(env);
}
} // WorkScheduler
} // OHOS