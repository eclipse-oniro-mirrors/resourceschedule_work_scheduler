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
#include "background_loader_napi_helper.h"

#include <errors.h>
#include <string>

#include "common.h"
#include "background_loader/background_loader_errors.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
namespace BackgroundLoader {
const int32_t UNSET_INT_PARAM = -1;
const std::string UNSET_STRING_PARAM = "";
constexpr int32_t NAME_MAXIMUM_LIMIT = 128;

void BackgroundLoaderNapiHelper::HandleBackgroundLoaderError(const napi_env &env, int32_t errCode)
{
    if (errCode == ERR_OK) {
        return;
    }
    int32_t code = errCode;
    std::string errMsg;
    ResolveErrorInfo(errCode, code, errMsg);
    BL_HILOGE("Background loader error, code: %{public}d, msg: %{public}s", code, errMsg.c_str());
    napi_throw_business_error(env, code, errMsg.c_str());
}

int32_t BackgroundLoaderNapiHelper::GetIntProperty(napi_env env, napi_value object,
    const std::string &propertyName, ErrCode errCode)
{
    int32_t intValue = UNSET_INT_PARAM;
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus != napi_ok) {
        return intValue;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_undefined) {
        BL_HILOGD("Unset %{public}s.", propertyName.c_str());
        return intValue;
    } else if (valueType != napi_number) {
        BL_HILOGE("%{public}s type error, number expect.", propertyName.c_str());
        HandleBackgroundLoaderError(env, errCode);
        return intValue;
    }
    napi_get_value_int32(env, value, &intValue);
    return intValue;
}

std::string BackgroundLoaderNapiHelper::GetStringProperty(napi_env env, napi_value object,
    const std::string &propertyName, ErrCode errCode)
{
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus != napi_ok) {
        return UNSET_STRING_PARAM;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_undefined) {
        BL_HILOGD("Unset %{public}s.", propertyName.c_str());
        return UNSET_STRING_PARAM;
    } else if (valueType != napi_string) {
        BL_HILOGE("%{public}s type error, string expect.", propertyName.c_str());
        HandleBackgroundLoaderError(env, errCode);
        return UNSET_STRING_PARAM;
    }
    char chars[NAME_MAXIMUM_LIMIT] = {0};
    size_t charLength = 0;
    napi_status getStringStatus = napi_get_value_string_utf8(env, value, chars, NAME_MAXIMUM_LIMIT, &charLength);
    if (getStringStatus == napi_ok && charLength > 0) {
        return std::string(chars, charLength);
    }
    return UNSET_STRING_PARAM;
}
} // namespace BackgroundLoader
} // namespace WorkScheduler
} // namespace OHOS
