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

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "work_info.h"

namespace OHOS {
namespace WorkScheduler {
struct AsyncWorkData {
    explicit AsyncWorkData(napi_env napiEnv);
    virtual ~AsyncWorkData();
    napi_env env;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
    bool isCallback = false;
    int32_t errorCode = 0;
    std::string errorMsg = "";
};

class Common {
public:
    /**
     * @brief Napi get null.
     *
     * @param env The env.
     * @return The result.
     */
    static napi_value NapiGetNull(napi_env env);
    /**
     * @brief Get the info of work.
     *
     * @param env The env.
     * @param objValue The obj value.
     * @param workInfo The info of work.
     * @return True if success,else false
     */
    static bool GetWorkInfo(napi_env env, napi_value objValue, WorkInfo &workInfo);
    /**
     * @brief Get int property.
     *
     * @param env The env.
     * @param object The object.
     * @param propertyName The name of property.
     * @param errCode Throw the errCode if failed.
     * @return Int value.
     */
    static int32_t GetIntProperty(napi_env env, napi_value object,
        const std::string &propertyName, ErrCode errCode);
    /**
     * @brief Get bool property.
     *
     * @param env The env.
     * @param object The object.
     * @param propertyName The name of property.
     * @param errCode Throw the errCode if failed.
     * @return True if success,else false
     */
    static bool GetBoolProperty(napi_env env, napi_value object,
        const std::string &propertyName, ErrCode errCode);
    /**
     * @brief Get bool to int property.
     *
     * @param env The env.
     * @param object The object.
     * @param propertyName The name of property.
     * @param errCode Throw the errCode if failed.
     * @return Bool to int property.
     */
    static int32_t GetBoolToIntProperty(napi_env env, napi_value object,
        const std::string &propertyName, ErrCode errCode);
    /**
     * @brief Get string property.
     *
     * @param env The env.
     * @param object The object.
     * @param propertyName The name of property.
     * @param errCode Throw the errCode if failed.
     * @return String property.
     */
    static std::string GetStringProperty(napi_env env, napi_value object,
        const std::string &propertyName, ErrCode errCode);
    /**
     * @brief Match value type.
     *
     * @param env The env.
     * @param value The value.
     * @param targetType The target type.
     * @return True if success,else false
     */
    static bool MatchValueType(napi_env env, napi_value value, napi_valuetype targetType);
    /**
     * @brief JS para error.
     *
     * @param env The env.
     * @param callback The callback.
     * @return Result or promise.
     */
    static napi_value JSParaError(const napi_env &env, const napi_ref &callback);
    /**
     * @brief Padding callback promise info.
     *
     * @param env The env
     * @param callback The callback.
     * @param info The info.
     * @param promise The promise.
     */
    static void PaddingAsyncWorkData(
        const napi_env &env, const napi_ref &callback, AsyncWorkData &info, napi_value &promise);
    /**
     * @brief Get the workInfo of napi.
     *
     * @param env The env.
     * @param workInfo The info of work.
     * @return Napi work info.
     */
    static napi_value GetNapiWorkInfo(napi_env env, std::shared_ptr<WorkInfo> &workInfo);
    /**
     * @brief Get callback error value.
     *
     * @param env The env.
     * @param errCode The error code.
     * @param errMsg The error message.
     * @return Callback error value.
     */
    static napi_value GetCallbackErrorValue(napi_env env, int32_t errCode, const std::string errMsg);
    /**
     * @brief Set callback.
     *
     * @param env The env.
     * @param callbackIn The callback.
     * @param errCode The errCode.
     * @param result The result.
     */
    static void SetCallback(const napi_env &env, const napi_ref &callbackIn,
        int32_t errCode, const napi_value &result);
    /**
     * @brief Set promise.
     *
     * @param env The env.
     * @param info The info.
     * @param result The result.
     * @return The result.
     */
    static napi_value SetPromise(const napi_env &env, const AsyncWorkData &info, const napi_value &result);
    /**
     * @brief Return callback promise.
     *
     * @param env The env.
     * @param info The info.
     * @param result The result.
     */
    static void ReturnCallbackPromise(const napi_env &env, const AsyncWorkData &info,
        const napi_value &result);
    /**
     * @brief Handle error code and throw error.
     *
     * @param env The env.
     * @param errCode The error code.
     */
    static void HandleErrCode(const napi_env &env, int32_t errCode);
    /**
     * @brief Handle param error code and throw param error.
     *
     * @param env The env.
     * @param errCode The error code.
     */
    static void HandleParamErr(const napi_env &env, int32_t errCode);
    /**
     * @brief Find error message by code.
     *
     * @param env The env.
     * @param errCode The error code.
     */
    static std::string FindErrMsg(const napi_env &env, int32_t errCode);
    /**
     * @brief Find the error code actually reported by code.
     *
     * @param env The env.
     * @param errCode The error code.
     */
    static int32_t FindErrCode(const napi_env &env, int32_t errCodeIn);

private:
    static bool GetBaseWorkInfo(napi_env env, napi_value objValue, WorkInfo &workInfo);
    static bool GetNetWorkInfo(napi_env env, napi_value objValue, WorkInfo &workInfo);
    static bool GetChargeInfo(napi_env env, napi_value objValue, WorkInfo &workInfo);
    static bool GetBatteryInfo(napi_env env, napi_value objValue, WorkInfo &workInfo);
    static bool GetStorageInfo(napi_env env, napi_value objValue, WorkInfo &workInfo);
    static bool GetRepeatInfo(napi_env env, napi_value objValue, WorkInfo &workInfo);
    static bool GetExtrasInfo(napi_env env, napi_value objValue, WorkInfo &workInfo);
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_COMMON