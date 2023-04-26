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
    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
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
    auto& engine = jsRuntime_.GetNativeEngine();

    jsObj_ = jsRuntime_.LoadModule(moduleName, srcPath, abilityInfo_->hapPath);
    if (jsObj_ == nullptr) {
        WS_HILOGE("WorkSchedulerExtension Failed to get jsObj_");
        return;
    }
    NativeObject* obj = AbilityRuntime::ConvertNativeValueTo<NativeObject>(jsObj_->Get());
    if (obj == nullptr) {
        WS_HILOGE("WorkSchedulerExtension Failed to get JsWorkSchedulerExtension object");
        return;
    }

    auto context = GetContext();
    if (context == nullptr) {
        WS_HILOGE("WorkSchedulerExtension Failed to get context");
        return;
    }
    NativeValue* contextObj = CreateJsWorkSchedulerExtensionContext(engine, context);
    shellContextRef_ = jsRuntime_.LoadSystemModule("WorkSchedulerExtensionContext",
        &contextObj, 1);
    contextObj = shellContextRef_->Get();
    context->Bind(jsRuntime_, shellContextRef_.get());
    obj->SetProperty("context", contextObj);
    WS_HILOGD("end.");
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
        NativeEngine& nativeEngine = jsRuntime_.GetNativeEngine();

        NativeValue* jworkInfoData = nativeEngine.CreateObject();
        NativeObject* workInfoData = AbilityRuntime::ConvertNativeValueTo<NativeObject>(jworkInfoData);
        workInfoData->SetProperty("workId", nativeEngine.CreateNumber(workId));

        workInfoData->SetProperty("bundleName", nativeEngine.CreateString(bundleName.c_str(), bundleName.size()));

        workInfoData->SetProperty("abilityName", nativeEngine.CreateString(abilityName.c_str(), abilityName.size()));

        if (getExtrasRet) {
            workInfoData->SetProperty("parameters", nativeEngine.CreateString(extrasStr.c_str(), extrasStr.size()));
        }

        workInfoData->SetProperty("isPersisted", nativeEngine.CreateBoolean(isPersisted));
        if (networkType != WorkCondition::Network::NETWORK_UNKNOWN) {
            workInfoData->SetProperty("networkType", nativeEngine.CreateNumber(networkType));
        }
        if (charger != WorkCondition::Charger::CHARGING_UNKNOWN) {
            if (charger == WorkCondition::Charger::CHARGING_UNPLUGGED) {
                workInfoData->SetProperty("isCharging", nativeEngine.CreateBoolean(false));
            } else {
                workInfoData->SetProperty("isCharging", nativeEngine.CreateBoolean(true));
                workInfoData->SetProperty("chargerType", nativeEngine.CreateNumber(charger));
            }
        }
        if (batteryLevel != INVALID_VALUE) {
            workInfoData->SetProperty("batteryLevel", nativeEngine.CreateNumber(batteryLevel));
        }
        if (batteryStatus != WorkCondition::BatteryStatus::BATTERY_UNKNOWN) {
            workInfoData->SetProperty("batteryStatus", nativeEngine.CreateNumber(batteryStatus));
        }
        if (storageLevel != WorkCondition::Storage::STORAGE_UNKNOWN) {
            workInfoData->SetProperty("storageRequest", nativeEngine.CreateNumber(storageLevel));
        }

        if (timeInterval > 0) {
            if (isRepeat) {
                workInfoData->SetProperty("isRepeat", nativeEngine.CreateBoolean(true));
                workInfoData->SetProperty("repeatCycleTime", nativeEngine.CreateNumber(timeInterval));
            } else {
                workInfoData->SetProperty("repeatCycleTime", nativeEngine.CreateNumber(timeInterval));
                workInfoData->SetProperty("repeatCount", nativeEngine.CreateNumber(cycleCount));
            }
        }

        NativeValue* argv[] = {jworkInfoData};
        if (!jsObj_) {
            WS_HILOGE("WorkSchedulerExtension Not found js");
            return;
        }

        NativeValue* value = jsObj_->Get();
        NativeObject* obj = AbilityRuntime::ConvertNativeValueTo<NativeObject>(value);
        if (obj == nullptr) {
            WS_HILOGE("WorkSchedulerExtension Failed to get WorkSchedulerExtension object");
            return;
        }

        NativeValue* method = obj->GetProperty("onWorkStart");
        if (method == nullptr) {
            WS_HILOGE("WorkSchedulerExtension Failed to get onWorkStart from WorkSchedulerExtension object");
            return;
        }
        nativeEngine.CallFunction(value, method, argv, 1);
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
        NativeEngine& nativeEngine = jsRuntime_.GetNativeEngine();

        NativeValue* jworkInfoData = nativeEngine.CreateObject();
        NativeObject* workInfoData = AbilityRuntime::ConvertNativeValueTo<NativeObject>(jworkInfoData);
        workInfoData->SetProperty("workId", nativeEngine.CreateNumber(workId));

        workInfoData->SetProperty("bundleName", nativeEngine.CreateString(bundleName.c_str(), bundleName.size()));

        workInfoData->SetProperty("abilityName", nativeEngine.CreateString(abilityName.c_str(), abilityName.size()));

        if (getExtrasRet) {
            workInfoData->SetProperty("parameters", nativeEngine.CreateString(extrasStr.c_str(), extrasStr.size()));
        }

        workInfoData->SetProperty("isPersisted", nativeEngine.CreateBoolean(isPersisted));
        if (networkType != WorkCondition::Network::NETWORK_UNKNOWN) {
            workInfoData->SetProperty("networkType", nativeEngine.CreateNumber(networkType));
        }
        if (charger != WorkCondition::Charger::CHARGING_UNKNOWN) {
            if (charger == WorkCondition::Charger::CHARGING_UNPLUGGED) {
                workInfoData->SetProperty("isCharging", nativeEngine.CreateBoolean(false));
            } else {
                workInfoData->SetProperty("isCharging", nativeEngine.CreateBoolean(true));
                workInfoData->SetProperty("chargerType", nativeEngine.CreateNumber(charger));
            }
        }
        if (batteryLevel != INVALID_VALUE) {
            workInfoData->SetProperty("batteryLevel", nativeEngine.CreateNumber(batteryLevel));
        }
        if (batteryStatus != WorkCondition::BatteryStatus::BATTERY_UNKNOWN) {
            workInfoData->SetProperty("batteryStatus", nativeEngine.CreateNumber(batteryStatus));
        }
        if (storageLevel != WorkCondition::Storage::STORAGE_UNKNOWN) {
            workInfoData->SetProperty("storageRequest", nativeEngine.CreateNumber(storageLevel));
        }

        if (timeInterval > 0) {
            if (isRepeat) {
                workInfoData->SetProperty("isRepeat", nativeEngine.CreateBoolean(true));
                workInfoData->SetProperty("repeatCycleTime", nativeEngine.CreateNumber(timeInterval));
            } else {
                workInfoData->SetProperty("repeatCycleTime", nativeEngine.CreateNumber(timeInterval));
                workInfoData->SetProperty("repeatCount", nativeEngine.CreateNumber(cycleCount));
            }
        }

        NativeValue* argv[] = {jworkInfoData};
        if (!jsObj_) {
            WS_HILOGE("WorkSchedulerExtension Not found js");
            return;
        }

        NativeValue* value = jsObj_->Get();
        NativeObject* obj = AbilityRuntime::ConvertNativeValueTo<NativeObject>(value);
        if (obj == nullptr) {
            WS_HILOGE("WorkSchedulerExtension Failed to get object");
            return;
        }

        NativeValue* method = obj->GetProperty("onWorkStop");
        if (method == nullptr) {
            WS_HILOGE("WorkSchedulerExtension Failed to get onWorkStop from object");
            return;
        }
        nativeEngine.CallFunction(value, method, argv, 1);
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
