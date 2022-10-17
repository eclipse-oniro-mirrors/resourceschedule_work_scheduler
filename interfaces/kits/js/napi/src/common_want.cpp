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

#include "common_want.h"
#include "common.h"

#include "work_sched_errors.h"
#include "bool_wrapper.h"
#include "double_wrapper.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"
#include "work_sched_hilog.h"
#include "napi/native_node_api.h"
#include "securec.h"

namespace OHOS {
namespace WorkScheduler {
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

bool InnerUnwrapJS(napi_env env, napi_value param, AAFwk::WantParams &wantParams, std::string strProName)
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
        default: {
            WS_HILOGE("Param %{public}s is illegal. The value is only supported basic type(Number, String, Boolean).",
                strProName.c_str());
            Common::HandleParamErr(env, E_PARAMETERS_TYPE_ERR);
            return false;
        }
    }
    return true;
}

bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams &wantParams)
{
    if (param == nullptr) {
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, param, &valueType) != napi_ok) {
        return false;
    }
    if (valueType == napi_undefined) {
        WS_HILOGI("parameters not set.");
        return true;
    } else if (valueType != napi_object) {
        WS_HILOGE("parameters should be {[key: string]: value} format.");
        Common::HandleParamErr(env, E_PARAMETERS_TYPE_ERR);
        return false;
    }

    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;
    napi_value jsProName = nullptr;
    napi_get_property_names(env, param, &jsProNameList);
    napi_get_array_length(env, jsProNameList, &jsProCount);
    WS_HILOGI("Property size=%{public}d.", jsProCount);

    for (uint32_t index = 0; index < jsProCount; index++) {
        napi_get_element(env, jsProNameList, index, &jsProName);
        std::string strProName = UnwrapStringFromJS(env, jsProName, "");
        WS_HILOGI("Property name=%{public}s.", strProName.c_str());
        if (!InnerUnwrapJS(env, param, wantParams, strProName)) {
            return false;
        }
    }
    return true;
}

bool InnerWrapWantParamsString(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IString *ao = AAFwk::IString::Query(value);
    if (ao == nullptr) {
        return false;
    }

    std::string natValue = AAFwk::String::Unbox(ao);
    napi_value jsValue;
    napi_create_string_utf8(env, natValue.c_str(), NAPI_AUTO_LENGTH, &jsValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsBool(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IBoolean *bo = AAFwk::IBoolean::Query(value);
    if (bo == nullptr) {
        return false;
    }

    bool natValue = AAFwk::Boolean::Unbox(bo);
    napi_value jsValue;
    napi_get_boolean(env, natValue, &jsValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsInt(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IInteger *ao = AAFwk::IInteger::Query(value);
    if (ao == nullptr) {
        return false;
    }

    int natValue = AAFwk::Integer::Unbox(ao);
    napi_value jsValue;
    napi_create_int32(env, natValue, &jsValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsDouble(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IDouble *ao = AAFwk::IDouble::Query(value);
    if (ao == nullptr) {
        return false;
    }

    double natValue = AAFwk::Double::Unbox(ao);
    napi_value jsValue;
    napi_create_double(env, natValue, &jsValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

napi_value WrapWantParams(napi_env env, const AAFwk::WantParams &wantParams)
{
    WS_HILOGI("Start wrap wantParams.");
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    const std::map<std::string, sptr<AAFwk::IInterface>> paramList = wantParams.GetParams();
    for (auto iter = paramList.begin(); iter != paramList.end(); iter++) {
        if (AAFwk::IString::Query(iter->second) != nullptr) {
            InnerWrapWantParamsString(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IBoolean::Query(iter->second) != nullptr) {
            InnerWrapWantParamsBool(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IInteger::Query(iter->second) != nullptr) {
            InnerWrapWantParamsInt(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IDouble::Query(iter->second) != nullptr) {
            InnerWrapWantParamsDouble(env, jsObject, iter->first, wantParams);
        }
    }
    return jsObject;
}
}  // namespace WorkScheduler
}  // namespace OHOS
