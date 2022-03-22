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
    static bool GetWorkInfo(napi_env env, napi_value objValue, WorkInfo &WorkInfo);
    static int32_t GetIntProperty(napi_env env, napi_value object, const std::string &propertyName);
    static bool GetBoolProperty(napi_env env, napi_value object, const std::string &propertyName);
    static int32_t GetBoolToIntProperty(napi_env env, napi_value object, const std::string &propertyName);
    static std::string GetStringProperty(napi_env env, napi_value object, const std::string &propertyName);
    static bool MatchValueType(napi_env env, napi_value value, napi_valuetype targetType);
    static napi_value JSParaError(const napi_env &env, const napi_ref &callback);
    static void PaddingCallbackPromiseInfo(
        const napi_env &env, const napi_ref &callback, CallbackPromiseInfo &info, napi_value &promise);
    static napi_value GetNapiWorkInfo(napi_env env, std::shared_ptr<WorkInfo> &workInfo);
    static napi_value GetCallbackErrorValue(napi_env env, int errCode);
    static void SetCallback(const napi_env &env, const napi_ref &callbackIn, const int &errorCode,
        const napi_value &result);
    static napi_value SetPromise(const napi_env &env, const CallbackPromiseInfo &info, const napi_value &result);
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