/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_COMMON_WANT
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_COMMON_WANT

#include <string>
#include <vector>

#include "napi/native_api.h"
#include "want_params.h"

namespace OHOS {
namespace WorkScheduler {
/**
 * @brief Unwrap napi value to WantParams object.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param param The napi value parameters.
 * @param wantParams Indicates the WantParams object containing the new parameters.
 *
 * @return True if success,else false.
 */
bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams &wantParams);

/**
 * @brief Unwrap napi value to WantParams object with string property name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param param The napi value parameters.
 * @param wantParams Indicates the WantParams object containing the new parameters.
 * @param strProName The property name.
 */
void InnerUnwrapJS(napi_env env, napi_value param, AAFwk::WantParams &wantParams, std::string strProName);

/**
 * @brief Unwrap string from napi value.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param param The napi value parameters.
 * @param defaultValue The default value.
 *
 * @return The string result.
 */
std::string UnwrapStringFromJS(napi_env env, napi_value param, const std::string &defaultValue);

/**
 * @brief Judge if is expect type for napi value.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param param The napi value parameters.
 * @param expectType The expect napi value type.
 *
 * @return True if is expect type for napi value, else false.
 */
bool IsTypeForNapiValue(napi_env env, napi_value param, napi_valuetype expectType);
}  // namespace WorkScheduler
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_COMMON_WANT
