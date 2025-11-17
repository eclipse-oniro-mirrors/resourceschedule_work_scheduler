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

#include "js_work_scheduler_extension_context.h"

#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_error_utils.h"
#include "napi/native_api.h"
#include "napi_common_want.h"
#include "napi_common_util.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
using namespace OHOS::AbilityRuntime;
constexpr size_t ARGC_ONE = 1;
constexpr size_t INDEX_ZERO = 0;
class JsWorkSchedulerExtensionContext final {
public:
    explicit JsWorkSchedulerExtensionContext(const std::shared_ptr<WorkSchedulerExtensionContext>& context)
        : context_(context) {}
    ~JsWorkSchedulerExtensionContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        std::unique_ptr<JsWorkSchedulerExtensionContext>(
            static_cast<JsWorkSchedulerExtensionContext*>(data));
    }

    static napi_value StartServiceExtensionAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsWorkSchedulerExtensionContext, OnStartExtensionAbility);
    }

    static napi_value StopServiceExtensionAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsWorkSchedulerExtensionContext, OnStopExtensionAbility);
    }

    napi_value OnStartExtensionAbility(napi_env env, NapiCallbackInfo& info)
    {
        WS_HILOGI("called");
        if (info.argc < ARGC_ONE) {
            WS_HILOGE("invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        AAFwk::Want want;
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
            ThrowInvalidParamError(env, "Parse param want failed, must be a Want.");
            return CreateJsUndefined(env);
        }
        WS_HILOGI("%{public}s", want.ToString().c_str());
        napi_value lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [weak = context_, want, env, task = napiAsyncTask.get()]() {
            auto context = weak.lock();
            if (!context) {
                WS_HILOGE("context released");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto innerErrorCode = context->StartServiceExtensionAbility(want);
            if (innerErrorCode == 0) {
                WS_HILOGI("OK");
                task->Resolve(env, CreateJsUndefined(env));
            } else {
                WS_HILOGE("failed");
                task->Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_immediate, "OnStartExtensionAbility")) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(
                env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnStopExtensionAbility(napi_env env, NapiCallbackInfo& info)
    {
        WS_HILOGI("called");
        if (info.argc < ARGC_ONE) {
            WS_HILOGE("invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        AAFwk::Want want;
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
            ThrowInvalidParamError(env, "Parse param want failed, must be a Want.");
            return CreateJsUndefined(env);
        }
        WS_HILOGI("%{public}s", want.ToString().c_str());
        napi_value lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [weak = context_, want, env, task = napiAsyncTask.get()]() {
            auto context = weak.lock();
            if (!context) {
                WS_HILOGE("context released");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto innerErrorCode = context->StopServiceExtensionAbility(want);
            if (innerErrorCode == 0) {
                task->Resolve(env, CreateJsUndefined(env));
            } else {
                task->Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_immediate, "OnStopExtensionAbility")) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(
                env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

private:
    std::weak_ptr<WorkSchedulerExtensionContext> context_;
};
} // namespace

napi_value CreateJsWorkSchedulerExtensionContext(napi_env env,
    std::shared_ptr<WorkSchedulerExtensionContext> context)
{
    napi_value objValue = AbilityRuntime::CreateJsExtensionContext(env, context);

    std::unique_ptr<JsWorkSchedulerExtensionContext> jsContext =
        std::make_unique<JsWorkSchedulerExtensionContext>(context);
    napi_status status = napi_wrap(env, objValue, jsContext.release(), JsWorkSchedulerExtensionContext::Finalizer,
        nullptr, nullptr);
    if (status != napi_ok) {
        WS_HILOGE("JsWorkSchedulerExtensionContext failed to wrap the object");
        return nullptr;
    }
    const char *moduleName = "JsWorkSchedulerExtensionContext";
    BindNativeFunction(env, objValue, "startServiceExtensionAbility", moduleName,
        JsWorkSchedulerExtensionContext::StartServiceExtensionAbility);
    BindNativeFunction(env, objValue, "stopServiceExtensionAbility", moduleName,
        JsWorkSchedulerExtensionContext::StopServiceExtensionAbility);
    return objValue;
}
} // namespace WorkScheduler
} // namespace OHOS