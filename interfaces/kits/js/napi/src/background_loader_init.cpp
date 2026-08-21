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

#include "background_loader_init.h"
#include "background_loader_task.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
#ifdef __cplusplus
extern "C" {
#endif

napi_value InitBackgroundLoaderApi(napi_env env, napi_value exports)
{
    WS_HILOGD("Background loader napi init");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("registerTask", RegisterTask),
        DECLARE_NAPI_FUNCTION("unregisterTask", UnregisterTask),
        DECLARE_NAPI_FUNCTION("finishTask", FinishTask),
        DECLARE_NAPI_FUNCTION("getTaskInfo", GetTaskInfo),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    // Init constant value.
    InitBackgroundLoaderConstProperties(env, exports);
    InitBackgroundLoaderStopCode(env, exports);

    return exports;
}

void InitBackgroundLoaderConstProperties(napi_env env, napi_value exports)
{
    napi_value onStart = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "onStart", NAPI_AUTO_LENGTH, &onStart));
    napi_value onStop = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "onStop", NAPI_AUTO_LENGTH, &onStop));

    const napi_property_descriptor properties[] = {
        DECLARE_NAPI_PROPERTY("ON_START", onStart),
        DECLARE_NAPI_PROPERTY("ON_STOP", onStop),
    };
    NAPI_CALL_RETURN_VOID(env, napi_define_properties(env, exports,
        sizeof(properties) / sizeof(properties[0]), properties));
}

void InitBackgroundLoaderStopCode(napi_env env, napi_value exports)
{
    napi_value success = nullptr;
    napi_value systemError = nullptr;
    napi_value perceptibleError = nullptr;
    napi_value timeoutError = nullptr;
    napi_value executeError = nullptr;

    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, static_cast<uint32_t>(StopCode::SUCCESS), &success));
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, static_cast<uint32_t>(StopCode::SYSTEM_ERROR), &systemError));
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env,
        static_cast<uint32_t>(StopCode::PERCEPTIBLE_ERROR), &perceptibleError));
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, static_cast<uint32_t>(StopCode::TIMEOUT_ERROR), &timeoutError));
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, static_cast<uint32_t>(StopCode::EXECUTE_ERROR), &executeError));

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("SUCCESS", success),
        DECLARE_NAPI_STATIC_PROPERTY("SYSTEM_ERROR", systemError),
        DECLARE_NAPI_STATIC_PROPERTY("PERCEPTIBLE_ERROR", perceptibleError),
        DECLARE_NAPI_STATIC_PROPERTY("TIMEOUT_ERROR", timeoutError),
        DECLARE_NAPI_STATIC_PROPERTY("EXECUTE_ERROR", executeError),
    };

    napi_value result = nullptr;
    napi_define_class(env, "StopCode", NAPI_AUTO_LENGTH, EnumBackgroundLoaderStopCodeConstructor,
        nullptr, sizeof(desc) / sizeof(desc[0]), desc, &result);
    napi_set_named_property(env, exports, "StopCode", result);
}

napi_value EnumBackgroundLoaderStopCodeConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value args[1] = {nullptr};
    napi_value res = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &res, &data);
    if (status != napi_ok) {
        return nullptr;
    }

    return res;
}

/*
 * Module register function
 */
__attribute__((constructor)) void RegisterBackgroundLoaderModule(void)
{
    napi_module_register(&g_backgroundLoaderModule);
}
#ifdef __cplusplus
}
#endif
}  // namespace WorkScheduler
}  // namespace OHOS