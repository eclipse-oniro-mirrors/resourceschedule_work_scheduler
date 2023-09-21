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

#include "js_work_scheduler_extension.h"

#include <string>

#include "runtime.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "work_scheduler_extension.h"
#include "js_work_scheduler_extension_context.h"
#include "work_scheduler_stub_imp.h"

namespace OHOS {
namespace WorkScheduler {
const int32_t INVALID_VALUE = -1;

JsWorkSchedulerExtension* JsWorkSchedulerExtension::Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime)
{
    return new JsWorkSchedulerExtension(static_cast<AbilityRuntime::JsRuntime&>(*runtime));
}

JsWorkSchedulerExtension::JsWorkSchedulerExtension(AbilityRuntime::JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsWorkSchedulerExtension::~JsWorkSchedulerExtension()
{
    WS_HILOGD("Js WorkScheduler extension destructor.");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}

inline void *DetachCallbackFunc(napi_env env, void *value, void *)
{
    return value;
}

napi_value AttachWorkSchedulerExtensionContext(napi_env env, void *value, void *)
{
    WS_HILOGI("AttachWorkSchedulerExtensionContext");
    if (value == nullptr) {
        WS_HILOGE("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<WorkSchedulerExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        WS_HILOGE("invalid context.");
        return nullptr;
    }
    napi_value object = CreateJsWorkSchedulerExtensionContext(env, ptr);
    napi_value contextObj = AbilityRuntime::JsRuntime::LoadSystemModuleByEngine(env,
        "application.WorkSchedulerExtensionContext", &object, 1)->GetNapiValue();
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc,
        AttachWorkSchedulerExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<WorkSchedulerExtensionContext>(ptr);
    napi_status status = napi_wrap(env, contextObj, workContext,
        [](napi_env env, void *data, void *) {
            WS_HILOGI("Finalizer for weak_ptr WorkSchedulerExtensionContext is called");
            delete static_cast<std::weak_ptr<WorkSchedulerExtensionContext> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok) {
        WS_HILOGE("WorkSchedulerExtension failed to wrap the context");
        return nullptr;
    }
    
    return contextObj;
}

void JsWorkSchedulerExtension::Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord>& record,
    const std::shared_ptr<AppExecFwk::OHOSApplication>& application,
    std::shared_ptr<AppExecFwk::AbilityHandler>& handler,
    const sptr<IRemoteObject>& token)
{
    WS_HILOGD("enter");
    WorkSchedulerExtension::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        WS_HILOGE("JsWorkSchedulerExtension Failed to get srcPath");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    WS_HILOGD("moduleName:%{public}s, srcPath:%{public}s.", moduleName.c_str(), srcPath.c_str());
    AbilityRuntime::HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(moduleName, srcPath, abilityInfo_->hapPath,
        abilityInfo_->compileMode == AbilityRuntime::CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        WS_HILOGE("WorkSchedulerExtension Failed to get jsObj_");
        return;
    }
    napi_value obj = jsObj_->GetNapiValue();
    if (obj == nullptr) {
        WS_HILOGE("WorkSchedulerExtension Failed to get JsWorkSchedulerExtension object");
        return;
    }
    BindContext(env, obj);
    
    WS_HILOGD("end.");
}

void JsWorkSchedulerExtension::BindContext(napi_env env, napi_value obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        WS_HILOGE("WorkSchedulerExtension Failed to get context");
        return;
    }
    napi_value contextObj = CreateJsWorkSchedulerExtensionContext(env, context);
    shellContextRef_ = jsRuntime_.LoadSystemModule("application.WorkSchedulerExtensionContext",
        &contextObj, 1);
    contextObj = shellContextRef_->GetNapiValue();

    auto workContext = new (std::nothrow) std::weak_ptr<WorkSchedulerExtensionContext>(context);
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc,
        AttachWorkSchedulerExtensionContext, workContext, nullptr);
    WS_HILOGI("JsWorkSchedulerExtension init bind and set property.");
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    WS_HILOGI("Set JsWorkSchedulerExtension context pointer is nullptr or not:%{public}d",
        context.get() == nullptr);

    napi_status status = napi_wrap(env, contextObj, workContext,
        [](napi_env env, void* data, void*) {
            WS_HILOGI("Finalizer for weak_ptr WorkSchedulerExtensionContext is called");
            delete static_cast<std::weak_ptr<WorkSchedulerExtensionContext> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok) {
        WS_HILOGE("WorkSchedulerExtension failed to wrap the context");
    }
}

void JsWorkSchedulerExtension::OnStart(const AAFwk::Want& want)
{
    WS_HILOGD("begin");
    AbilityRuntime::Extension::OnStart(want);
}

void JsWorkSchedulerExtension::OnStop()
{
    AbilityRuntime::Extension::OnStop();
    WS_HILOGD("end.");
}

sptr<IRemoteObject> JsWorkSchedulerExtension::OnConnect(const AAFwk::Want& want)
{
    AbilityRuntime::Extension::OnConnect(want);
    WS_HILOGD("begin.");
    sptr<WorkSchedulerStubImp> remoteObject = new (std::nothrow) WorkSchedulerStubImp(
        std::static_pointer_cast<JsWorkSchedulerExtension>(shared_from_this()));
    if (remoteObject == nullptr) {
        WS_HILOGE("OnConnect get null");
        return remoteObject;
    }
    WS_HILOGD("end.");
    return remoteObject->AsObject();
}

void JsWorkSchedulerExtension::OnDisconnect(const AAFwk::Want& want)
{
    WS_HILOGD("begin.");
    AbilityRuntime::Extension::OnDisconnect(want);
}

void JsWorkSchedulerExtension::OnWorkStart(WorkInfo& workInfo)
{
    if (handler_ == nullptr) {
        return;
    }
    WS_HILOGD("begin.");
    int32_t workId = workInfo.GetWorkId();
    std::string bundleName = workInfo.GetBundleName();
    std::string abilityName = workInfo.GetAbilityName();
    bool isPersisted = workInfo.IsPersisted();
    WorkCondition::Network networkType = workInfo.GetNetworkType();
    WorkCondition::Charger charger = workInfo.GetChargerType();
    int32_t batteryLevel = workInfo.GetBatteryLevel();
    WorkCondition::BatteryStatus batteryStatus = workInfo.GetBatteryStatus();
    WorkCondition::Storage storageLevel = workInfo.GetStorageLevel();
    uint32_t timeInterval = workInfo.GetTimeInterval();
    bool isRepeat = workInfo.IsRepeat();
    int32_t cycleCount = workInfo.GetCycleCount();
    std::string extrasStr;
    bool getExtrasRet = GetExtrasJsonStr(workInfo, extrasStr);
    WorkSchedulerExtension::OnWorkStart(workInfo);
    auto task = [=]() {
        AbilityRuntime::HandleScope handleScope(jsRuntime_);
        napi_env env = jsRuntime_.GetNapiEnv();

        napi_value workInfoData;
        if (napi_create_object(env, &workInfoData) != napi_ok) {
            WS_HILOGE("WorkSchedulerExtension failed to create workInfoData OnWorkStart");
            return;
        }
        napi_value workIdValue;
        napi_create_int32(env, workId, &workIdValue);
        napi_set_named_property(env, workInfoData, "workId", workIdValue);

        napi_value bundleNameValue;
        napi_create_string_utf8(env, bundleName.c_str(), bundleName.size(), &bundleNameValue);
        napi_set_named_property(env, workInfoData, "bundleName", bundleNameValue);

        napi_value abilityNameValue;
        napi_create_string_utf8(env, abilityName.c_str(), abilityName.size(), &abilityNameValue);
        napi_set_named_property(env, workInfoData, "abilityName", abilityNameValue);

        if (getExtrasRet) {
            napi_value parametersValue;
            napi_create_string_utf8(env, extrasStr.c_str(), extrasStr.size(), &parametersValue);
            napi_set_named_property(env, workInfoData, "parameters", parametersValue);
        }

        napi_value isPersistedValue;
        napi_get_boolean(env, isPersisted, &isPersistedValue);
        napi_set_named_property(env, workInfoData, "isPersisted", isPersistedValue);
        if (networkType != WorkCondition::Network::NETWORK_UNKNOWN) {
            napi_value networkTypeValue;
            napi_create_int32(env, networkType, &networkTypeValue);
            napi_set_named_property(env, workInfoData, "networkType", networkTypeValue);
        }
        if (charger != WorkCondition::Charger::CHARGING_UNKNOWN) {
            if (charger == WorkCondition::Charger::CHARGING_UNPLUGGED) {
                napi_value isChargingValue;
                napi_get_boolean(env, false, &isChargingValue);
                napi_set_named_property(env, workInfoData, "isCharging", isChargingValue);
            } else {
                napi_value isChargingValue;
                napi_get_boolean(env, true, &isChargingValue);
                napi_set_named_property(env, workInfoData, "isCharging", isChargingValue);

                napi_value chargerTypeValue;
                napi_create_int32(env, charger, &chargerTypeValue);
                napi_set_named_property(env, workInfoData, "chargerType", chargerTypeValue);
            }
        }
        if (batteryLevel != INVALID_VALUE) {
            napi_value batteryLevelValue;
            napi_create_int32(env, batteryLevel, &batteryLevelValue);
            napi_set_named_property(env, workInfoData, "batteryLevel", batteryLevelValue);
        }
        if (batteryStatus != WorkCondition::BatteryStatus::BATTERY_UNKNOWN) {
            napi_value batteryStatusValue;
            napi_create_int32(env, batteryStatus, &batteryStatusValue);
            napi_set_named_property(env, workInfoData, "batteryStatus", batteryStatusValue);
        }
        if (storageLevel != WorkCondition::Storage::STORAGE_UNKNOWN) {
            napi_value storageLevelValue;
            napi_create_int32(env, storageLevel, &storageLevelValue);
            napi_set_named_property(env, workInfoData, "storageRequest", storageLevelValue);
        }

        if (timeInterval > 0) {
            if (isRepeat) {
                napi_value isRepeatValue;
                napi_get_boolean(env, true, &isRepeatValue);
                napi_set_named_property(env, workInfoData, "isRepeat", isRepeatValue);

                napi_value repeatCycleTimeValue;
                napi_create_uint32(env, timeInterval, &repeatCycleTimeValue);
                napi_set_named_property(env, workInfoData, "repeatCycleTime", repeatCycleTimeValue);
            } else {
                napi_value repeatCycleTimeValue;
                napi_create_uint32(env, timeInterval, &repeatCycleTimeValue);
                napi_set_named_property(env, workInfoData, "repeatCycleTime", repeatCycleTimeValue);

                napi_value repeatCountValue;
                napi_create_int32(env, cycleCount, &repeatCountValue);
                napi_set_named_property(env, workInfoData, "repeatCount", repeatCountValue);
            }
        }

        napi_value argv[] = {workInfoData};
        if (!jsObj_) {
            WS_HILOGE("WorkSchedulerExtension Not found js");
            return;
        }

        napi_value value = jsObj_->GetNapiValue();
        if (value == nullptr) {
            WS_HILOGE("WorkSchedulerExtension Failed to get WorkSchedulerExtension object");
            return;
        }

        napi_value method;
        napi_get_named_property(env, value, "onWorkStart", &method);
        if (method == nullptr) {
            WS_HILOGE("WorkSchedulerExtension Failed to get onWorkStart from WorkSchedulerExtension object");
            return;
        }

        napi_value callFunctionResult;
        if (napi_call_function(env, value, method, 1, argv, &callFunctionResult) != napi_ok) {
            WS_HILOGE("WorkSchedulerExtension call funcation onWorkStart error");
            return;
        }
    };
    handler_->PostTask(task);
}

void JsWorkSchedulerExtension::OnWorkStop(WorkInfo& workInfo)
{
    if (handler_ == nullptr) {
        return;
    }
    WS_HILOGD("begin.");
    int32_t workId = workInfo.GetWorkId();
    std::string bundleName = workInfo.GetBundleName();
    std::string abilityName = workInfo.GetAbilityName();
    bool isPersisted = workInfo.IsPersisted();
    WorkCondition::Network networkType = workInfo.GetNetworkType();
    WorkCondition::Charger charger = workInfo.GetChargerType();
    int32_t batteryLevel = workInfo.GetBatteryLevel();
    WorkCondition::BatteryStatus batteryStatus = workInfo.GetBatteryStatus();
    WorkCondition::Storage storageLevel = workInfo.GetStorageLevel();
    uint32_t timeInterval = workInfo.GetTimeInterval();
    bool isRepeat = workInfo.IsRepeat();
    int32_t cycleCount = workInfo.GetCycleCount();
    std::string extrasStr;
    bool getExtrasRet = GetExtrasJsonStr(workInfo, extrasStr);
    WorkSchedulerExtension::OnWorkStop(workInfo);
    auto task = [=]() {
        AbilityRuntime::HandleScope handleScope(jsRuntime_);
        napi_env env = jsRuntime_.GetNapiEnv();

        napi_value workInfoData;
        if (napi_create_object(env, &workInfoData) != napi_ok) {
            WS_HILOGE("WorkSchedulerExtension failed to create workInfoData OnWorkStop");
            return;
        }

        napi_value workIdValue;
        napi_create_int32(env, workId, &workIdValue);
        napi_set_named_property(env, workInfoData, "workId", workIdValue);

        napi_value bundleNameValue;
        napi_create_string_utf8(env, bundleName.c_str(), bundleName.size(), &bundleNameValue);
        napi_set_named_property(env, workInfoData, "bundleName", bundleNameValue);

        napi_value abilityNameValue;
        napi_create_string_utf8(env, abilityName.c_str(), abilityName.size(), &abilityNameValue);
        napi_set_named_property(env, workInfoData, "abilityName", abilityNameValue);

        if (getExtrasRet) {
            napi_value parametersValue;
            napi_create_string_utf8(env, extrasStr.c_str(), extrasStr.size(), &parametersValue);
            napi_set_named_property(env, workInfoData, "parameters", parametersValue);
        }

        napi_value isPersistedValue;
        napi_get_boolean(env, isPersisted, &isPersistedValue);
        napi_set_named_property(env, workInfoData, "isPersisted", isPersistedValue);
        if (networkType != WorkCondition::Network::NETWORK_UNKNOWN) {
            napi_value networkTypeValue;
            napi_create_int32(env, networkType, &networkTypeValue);
            napi_set_named_property(env, workInfoData, "networkType", networkTypeValue);
        }
        if (charger != WorkCondition::Charger::CHARGING_UNKNOWN) {
            if (charger == WorkCondition::Charger::CHARGING_UNPLUGGED) {
                napi_value isChargingValue;
                napi_get_boolean(env, false, &isChargingValue);
                napi_set_named_property(env, workInfoData, "isCharging", isChargingValue);
            } else {
                napi_value isChargingValue;
                napi_get_boolean(env, true, &isChargingValue);
                napi_set_named_property(env, workInfoData, "isCharging", isChargingValue);

                napi_value chargerTypeValue;
                napi_create_int32(env, charger, &chargerTypeValue);
                napi_set_named_property(env, workInfoData, "chargerType", chargerTypeValue);
            }
        }
        if (batteryLevel != INVALID_VALUE) {
            napi_value batteryLevelValue;
            napi_create_int32(env, batteryLevel, &batteryLevelValue);
            napi_set_named_property(env, workInfoData, "batteryLevel", batteryLevelValue);
        }
        if (batteryStatus != WorkCondition::BatteryStatus::BATTERY_UNKNOWN) {
            napi_value batteryStatusValue;
            napi_create_int32(env, batteryStatus, &batteryStatusValue);
            napi_set_named_property(env, workInfoData, "batteryStatus", batteryStatusValue);
        }
        if (storageLevel != WorkCondition::Storage::STORAGE_UNKNOWN) {
            napi_value storageLevelValue;
            napi_create_int32(env, storageLevel, &storageLevelValue);
            napi_set_named_property(env, workInfoData, "storageRequest", storageLevelValue);
        }

        if (timeInterval > 0) {
            if (isRepeat) {
                napi_value isRepeatValue;
                napi_get_boolean(env, true, &isRepeatValue);
                napi_set_named_property(env, workInfoData, "isRepeat", isRepeatValue);

                napi_value repeatCycleTimeValue;
                napi_create_uint32(env, timeInterval, &repeatCycleTimeValue);
                napi_set_named_property(env, workInfoData, "repeatCycleTime", repeatCycleTimeValue);
            } else {
                napi_value repeatCycleTimeValue;
                napi_create_uint32(env, timeInterval, &repeatCycleTimeValue);
                napi_set_named_property(env, workInfoData, "repeatCycleTime", repeatCycleTimeValue);

                napi_value repeatCountValue;
                napi_create_int32(env, cycleCount, &repeatCountValue);
                napi_set_named_property(env, workInfoData, "repeatCount", repeatCountValue);
            }
        }

        napi_value argv[] = {workInfoData};
        if (!jsObj_) {
            WS_HILOGE("WorkSchedulerExtension Not found js");
            return;
        }

        napi_value value = jsObj_->GetNapiValue();
        if (value == nullptr) {
            WS_HILOGE("WorkSchedulerExtension Failed to get object");
            return;
        }

        napi_value method;
        napi_get_named_property(env, value, "onWorkStop", &method);
        if (method == nullptr) {
            WS_HILOGE("WorkSchedulerExtension Failed to get onWorkStop from object");
            return;
        }

        napi_value callFunctionResult;
        if (napi_call_function(env, value, method, 1, argv, &callFunctionResult) != napi_ok) {
            WS_HILOGE("WorkSchedulerExtensioncall funcation onWorkStop error");
            return;
        }
    };
    handler_->PostTask(task);
}

void JsWorkSchedulerExtension::GetSrcPath(std::string &srcPath)
{
    if (!Extension::abilityInfo_->isStageBasedModel) {
        /* temporary compatibility api8 + config.json */
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
        return;
    }

    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

bool JsWorkSchedulerExtension::GetExtrasJsonStr(const WorkInfo& workInfo, std::string& extrasStr)
{
    std::shared_ptr<AAFwk::WantParams> extras = workInfo.GetExtras();
    Json::Value extrasJson;
    if (!extras) {
        WS_HILOGI("parameter is null.");
        return false;
    }
    auto extrasMap = extras->GetParams();
    int typeId = INVALID_VALUE;
    for (auto it : extrasMap) {
        typeId = AAFwk::WantParams::GetDataType(it.second);
        if (typeId != INVALID_VALUE) {
            std::string value = AAFwk::WantParams::GetStringByType(it.second, typeId);
            extrasJson[it.first] = value;
        } else {
            WS_HILOGE("parameters type not supported.");
        }
    }
    Json::StreamWriterBuilder builder;
    extrasStr = Json::writeString(builder, extrasJson);
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS
