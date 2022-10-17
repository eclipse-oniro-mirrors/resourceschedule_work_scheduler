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
#include "stop_and_clear_works.h"

#include "workscheduler_srv_client.h"
#include "work_sched_hilog.h"
#include "common.h"

namespace OHOS {
namespace WorkScheduler {
napi_value StopAndClearWorks(napi_env env, napi_callback_info info)
{
    WS_HILOGD("StopAndClearWorks napi begin");
    ErrCode errCode = WorkSchedulerSrvClient::GetInstance().StopAndClearWorks();
    Common::HandleErrCode(env, errCode);
    WS_HILOGD("StopAndClearWorks napi end. result: %{public}s", errCode == ERR_OK ? "true" : "false");
    return Common::NapiGetNull(env);
}
} // namespace WorkScheduler
} // namespace OHOS
