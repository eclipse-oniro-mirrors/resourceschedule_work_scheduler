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

#include "common_want.h"

#include "bool_wrapper.h"
#include "array_wrapper.h"
#include "byte_wrapper.h"
#include "double_wrapper.h"
#include "float_wrapper.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "short_wrapper.h"
#include "string_wrapper.h"
#include "zchar_wrapper.h"
#include "remote_object_wrapper.h"
#include "want_params_wrapper.h"
#include "work_sched_hilog.h"
#include "napi_remote_object.h"
#include "securec.h"

namespace OHOS {
namespace WorkScheduler {
bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams &wantParams)
{
    WS_HILOGI("%{public}s called.", __func__);
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        return false;
    }
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;
    napi_value jsProName = nullptr;
    napi_get_property_names(env, param, &jsProNameList);
    napi_get_array_length(env, jsProNameList, &jsProCount);
    WS_HILOGI("%{public}s called. Property size=%{public}d.", __func__, jsProCount);

    for (uint32_t index = 0; index < jsProCount; index++) {
        napi_get_element(env, jsProNameList, index, &jsProName);
        std::string strProName = UnwrapStringFromJS(env, jsProName, "");
        WS_HILOGI("%{public}s called. Property name=%{public}s.", __func__, strProName.c_str());
        InnerUnwrapJS(env, param, wantParams, strProName);
    }

    return true;
}

void InnerUnwrapJS(napi_env env, napi_value param, AAFwk::WantParams &wantParams, std::string strProName)
{
    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProValue = nullptr;
    napi_get_named_property(env, param, strProName.c_str(), &jsProValue);
    napi_typeof(env, jsProValue, &jsValueType);
    switch (jsValueType) {
        case napi_string: {
            std::string natValue = UnwrapStringFromJS(env, jsProValue, "");
            wantParams.SetParam(strProName, AAFwk::String::Box(natValue));
            break;
        }
        case napi_boolean: {
            bool natValue = false;
            napi_get_value_bool(env, jsProValue, &natValue);
            wantParams.SetParam(strProName, AAFwk::Boolean::Box(natValue));
            break;
        }
        case napi_number: {
            int32_t natValue32 = 0;
            double natValueDouble = 0.0;
            bool isReadValue32 = false;
            bool isReadDouble = false;
            if (napi_get_value_int32(env, jsProValue, &natValue32) == napi_ok) {
                isReadValue32 = true;
            }
            if (napi_get_value_double(env, jsProValue, &natValueDouble) == napi_ok) {
                isReadDouble = true;
            }
            if (isReadValue32 && isReadDouble) {
                if (abs(natValueDouble - natValue32 * 1.0) > 0.0) {
                    wantParams.SetParam(strProName, AAFwk::Double::Box(natValueDouble));
                } else {
                    wantParams.SetParam(strProName, AAFwk::Integer::Box(natValue32));
                }
            } else if (isReadValue32) {
                wantParams.SetParam(strProName, AAFwk::Integer::Box(natValue32));
            } else if (isReadDouble) {
                wantParams.SetParam(strProName, AAFwk::Double::Box(natValueDouble));
            }
            break;
        }
        default:{
            WS_HILOGE("Param %{public}s is illegal. The value is only supported basic type(Number, String, Boolean).",
                strProName.c_str());
            break;
        }
    }
}

std::string UnwrapStringFromJS(napi_env env, napi_value param, const std::string &defaultValue = "")
{
    size_t size = 0;
    if (napi_get_value_string_utf8(env, param, nullptr, 0, &size) != napi_ok) {
        return defaultValue;
    }

    std::string value("");
    if (size == 0) {
        return defaultValue;
    }

    char *buf = new (std::nothrow) char[size + 1];
    if (buf == nullptr) {
        return value;
    }
    (void)memset_s(buf, size + 1, 0, size + 1);

    bool rev = napi_get_value_string_utf8(env, param, buf, size + 1, &size) == napi_ok;
    if (rev) {
        value = buf;
    } else {
        value = defaultValue;
    }

    delete[] buf;
    buf = nullptr;
    return value;
}

bool IsTypeForNapiValue(napi_env env, napi_value param, napi_valuetype expectType)
{
    napi_valuetype valueType = napi_undefined;
    if (param == nullptr) {
        return false;
    }

    if (napi_typeof(env, param, &valueType) != napi_ok) {
        return false;
    }
    return valueType == expectType;
}
}  // namespace AppExecFwk
}  // namespace OHOS
