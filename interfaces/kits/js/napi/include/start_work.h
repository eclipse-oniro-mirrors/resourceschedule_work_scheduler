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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_START_WORK
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_START_WORK

#include "napi/native_api.h"

namespace OHOS {
namespace WorkScheduler {
    /**
     * @brief Start work and throw error.
     * @return void.
     */
    napi_value StartWork(napi_env env, napi_callback_info info);
    /**
     * @brief Start work.
     * @return True if success,else false.
     */
    napi_value StartWorkWithRet(napi_env env, napi_callback_info info);
}
}
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_START_WORK