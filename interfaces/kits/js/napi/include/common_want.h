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

#include <map>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace WorkScheduler {
bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams &wantParams);
void InnerUnwrapJS(napi_env env, napi_value param, AAFwk::WantParams &wantParams, std::string strProName);

std::string UnwrapStringFromJS(napi_env env, napi_value param, const std::string &defaultValue);

bool IsTypeForNapiValue(napi_env env, napi_value param, napi_valuetype expectType);
// bool BlackListFilter(const std::string &strProName);
}  // namespace WorkScheduler
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_NAPI_INCLUDE_COMMON_WANT
