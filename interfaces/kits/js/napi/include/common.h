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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_COMMON
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_COMMON

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "work_info.h"

namespace OHOS {
namespace WorkScheduler {
struct CallbackPromiseInfo {
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    bool isCallback = false;
    int errorCode = 0;
};

class Common {
public:
    static napi_value NapiGetNull(napi_env env);
    /**
     * @brief Get work info.
     * @param env The env.
     * @param objValue The obj value.
     * @param workInfo The info of work.
     * @return True if success,else false
     */
    static bool GetWorkInfo(napi_env env, napi_value objValue, WorkInfo &WorkInfo);
    static int32_t GetIntProperty(napi_env env, napi_value object, const std::string &propertyName);
    /**
     * @brief Get bool property.
     * @param env The env.
     * @param object The object.
     * @param propertyName The property name.
     * @return True if success,else false
     */
    static bool GetBoolProperty(napi_env env, napi_value object, const std::string &propertyName);
    static int32_t GetBoolToIntProperty(napi_env env, napi_value object, const std::string &propertyName);
    static std::string GetStringProperty(napi_env env, napi_value object, const std::string &propertyName);
    /**
     * @brief Match value type.
     * @param env The env.
     * @param value The value.
     * @param targetType The target type.
     * @return True if success,else false
     */
    static bool MatchValueType(napi_env env, napi_value value, napi_valuetype targetType);
    static napi_value JSParaError(const napi_env &env, const napi_ref &callback);
    /**
     * @brief Padding callback promise info.
     * @param env The env
     * @param callback The callback.
     * @param info The info.
     * @param promise The promise.
     */
    static void PaddingCallbackPromiseInfo(
        const napi_env &env, const napi_ref &callback, CallbackPromiseInfo &info, napi_value &promise);
    static napi_value GetNapiWorkInfo(napi_env env, std::shared_ptr<WorkInfo> &workInfo);
    static napi_value GetCallbackErrorValue(napi_env env, int errCode);
    /**
     * @brief Set callback.
     * @param env The env.
     * @param callbackIn The callbackIn.
     * @param errorCode The errorCode.
     * @param result The result.
     */
    static void SetCallback(const napi_env &env, const napi_ref &callbackIn, const int &errorCode,
        const napi_value &result);
    static napi_value SetPromise(const napi_env &env, const CallbackPromiseInfo &info, const napi_value &result);
    /**
     * @brief Return callback promise.
     * @param env The env.
     * @param info The info.
     * @param result The result.
     */
    static void ReturnCallbackPromise(const napi_env &env, const CallbackPromiseInfo &info,
        const napi_value &result);

private:
    static bool GetBaseWorkInfo(napi_env env, napi_value objValue, WorkInfo &WorkInfo);
    static bool GetNetWorkInfo(napi_env env, napi_value objValue, WorkInfo &WorkInfo);
    static bool GetChargeInfo(napi_env env, napi_value objValue, WorkInfo &WorkInfo);
    static bool GetBatteryInfo(napi_env env, napi_value objValue, WorkInfo &WorkInfo);
    static bool GetStorageInfo(napi_env env, napi_value objValue, WorkInfo &WorkInfo);
    static bool GetRepeatInfo(napi_env env, napi_value objValue, WorkInfo &WorkInfo);
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_COMMON