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
#include "napi/native_api.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
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
    return objValue;
}
} // namespace WorkScheduler
} // namespace OHOS
