/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_BACKGROUND_LOADER_INIT
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_BACKGROUND_LOADER_INIT

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((constructor)) void RegisterBackgroundLoaderModule(void);
napi_value InitBackgroundLoaderApi(napi_env env, napi_value exports);
void InitBackgroundLoaderConstProperties(napi_env env, napi_value exports);
void InitBackgroundLoaderStopCode(napi_env env, napi_value exports);
napi_value EnumBackgroundLoaderStopCodeConstructor(napi_env env, napi_callback_info info);

#ifdef __cplusplus
}
#endif

namespace OHOS {
namespace WorkScheduler {

enum StopCode : uint32_t {
    SUCCESS = 0,
    SYSTEM_ERROR = 1,
    PERCEPTIBLE_ERROR = 2,
    TIMEOUT_ERROR = 3,
    EXECUTE_ERROR = 4
};

/*
 * Module define
 */
napi_module g_backgroundLoaderModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitBackgroundLoaderApi,
    .nm_modname = "resourceschedule.backgroundLoader",
    .nm_priv = nullptr,
    .reserved = {0}
};
}  // namespace WorkScheduler
}  // namespace OHOS

#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_BACKGROUND_LOADER_INIT