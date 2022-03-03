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
const int INVALID_VALUE = -1;

JsWorkSchedulerExtension* JsWorkSchedulerExtension::Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime)
{
    return new JsWorkSchedulerExtension(static_cast<AbilityRuntime::JsRuntime&>(*runtime));
}

JsWorkSchedulerExtension::JsWorkSchedulerExtension(AbilityRuntime::JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsWorkSchedulerExtension::~JsWorkSchedulerExtension() = default;

void JsWorkSchedulerExtension::Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord>& record,
    const std::shared_ptr<AppExecFwk::OHOSApplication>& application,
    std::shared_ptr<AppExecFwk::AbilityHandler>& handler,
    const sptr<IRemoteObject>& token)
{
    WS_HILOGI("enter");
    WorkSchedulerExtension::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        WS_HILOGI(" JsWorkSchedulerExtension Failed to get srcPath");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    WS_HILOGI(" JsWorkSchedulerExtension::Init moduleName:%{public}s,srcPath:%{public}s.",
        moduleName.c_str(), srcPath.c_str());
    AbilityRuntime::HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();

    jsObj_ = jsRuntime_.LoadModule(moduleName, srcPath);
    if (jsObj_ == nullptr) {
        WS_HILOGI("WorkSchedulerExtension Failed to get jsObj_");
        return;
    }
    WS_HILOGI(" JsWorkSchedulerExtension::Init ConvertNativeValueTo.");
    NativeObject* obj = AbilityRuntime::ConvertNativeValueTo<NativeObject>(jsObj_->Get());
    if (obj == nullptr) {
        WS_HILOGI("WorkSchedulerExtension Failed to get JsWorkSchedulerExtension object");
        return;
    }

    auto context = GetContext();
    if (context == nullptr) {
        WS_HILOGI("WorkSchedulerExtension Failed to get context");
        return;
    }
    WS_HILOGI("WorkSchedulerExtension ::Init CreateJsWorkSchedulerExtensionContext.");
    NativeValue* contextObj = CreateJsWorkSchedulerExtensionContext(engine, context);
    auto shellContextRef = jsRuntime_.LoadSystemModule("application.CreateJsWorkSchedulerExtensionContext",
        &contextObj, 1);
    contextObj = shellContextRef->Get();
    WS_HILOGI("WorkSchedulerExtension JsWorkSchedulerExtension::Init Bind.");
    context->Bind(jsRuntime_, shellContextRef.release());
    WS_HILOGI("WorkSchedulerExtension JsWorkSchedulerExtension::SetProperty.");
    obj->SetProperty("context", contextObj);
    WS_HILOGI("WorkSchedulerExtension JsWorkSchedulerExtension::Init end.");
}

void JsWorkSchedulerExtension::OnStart(const AAFwk::Want& want)
{
    WS_HILOGI("JsWorkSchedulerExtension OnStart begin");
    AbilityRuntime::Extension::OnStart(want);
}

void JsWorkSchedulerExtension::OnStop()
{
    AbilityRuntime::Extension::OnStop();
    WS_HILOGI("WorkSchedulerExtension %{public}s end.", __func__);
}

sptr<IRemoteObject> JsWorkSchedulerExtension::OnConnect(const AAFwk::Want& want)
{
    AbilityRuntime::Extension::OnConnect(want);
    WS_HILOGI("WorkSchedulerExtension %{public}s begin.", __func__);
    sptr<WorkSchedulerStubImp> remoteObject = new (std::nothrow) WorkSchedulerStubImp(
        std::static_pointer_cast<JsWorkSchedulerExtension>(shared_from_this()));
    WS_HILOGI("WorkSchedulerExtension %{public}s end. ", __func__);
    return remoteObject->AsObject();
}

void JsWorkSchedulerExtension::OnDisconnect(const AAFwk::Want& want)
{
    WS_HILOGI("WorkSchedulerExtension %{public}s begin.", __func__);
    AbilityRuntime::Extension::OnDisconnect(want);
}

void JsWorkSchedulerExtension::OnWorkStart(WorkInfo& workInfo)
{
    WorkSchedulerExtension::OnWorkStart(workInfo);
    WS_HILOGI("WorkSchedulerExtension %{public}s begin.", __func__);

    AbilityRuntime::HandleScope handleScope(jsRuntime_);
    NativeEngine& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* jworkInfoData = nativeEngine.CreateObject();
    NativeObject* workInfoData = AbilityRuntime::ConvertNativeValueTo<NativeObject>(jworkInfoData);
    workInfoData->SetProperty("workId", nativeEngine.CreateNumber(workInfo.GetWorkId()));

    std::string bundleName = workInfo.GetBundleName();
    workInfoData->SetProperty("bundleName", nativeEngine.CreateString(bundleName.c_str(), bundleName.size()));

    std::string abilityName = workInfo.GetAbilityName();
    workInfoData->SetProperty("abilityName", nativeEngine.CreateString(abilityName.c_str(), abilityName.size()));

    workInfoData->SetProperty("isPersisted", nativeEngine.CreateBoolean(workInfo.IsPersisted()));
    if (workInfo.GetNetworkType() != WorkCondition::Network::NETWORK_UNKNOWN) {
        workInfoData->SetProperty("networkType", nativeEngine.CreateNumber(workInfo.GetNetworkType()));
    }
    WorkCondition::Charger charger = workInfo.GetChargerType();
    if (charger != WorkCondition::Charger::CHARGING_UNKNOWN) {
        if (charger == WorkCondition::Charger::CHARGING_UNPLUGGED) {
            workInfoData->SetProperty("isCharging", nativeEngine.CreateBoolean(false));
        } else {
            workInfoData->SetProperty("isCharging", nativeEngine.CreateBoolean(true));
            workInfoData->SetProperty("chargerType", nativeEngine.CreateNumber(charger));
        }
    }
    if (workInfo.GetBatteryLevel() != INVALID_VALUE) {
        workInfoData->SetProperty("batteryLevel", nativeEngine.CreateNumber(workInfo.GetBatteryLevel()));
    }
    if (workInfo.GetBatteryStatus() != WorkCondition::BatteryStatus::BATTERY_UNKNOWN) {
        workInfoData->SetProperty("batteryStatus", nativeEngine.CreateNumber(workInfo.GetBatteryStatus()));
    }
    if (workInfo.GetStorageLevel() != WorkCondition::Storage::STORAGE_UNKNOWN) {
        workInfoData->SetProperty("storageRequest", nativeEngine.CreateNumber(workInfo.GetStorageLevel()));
    }

    uint32_t timeInterval = workInfo.GetTimeInterval();
    if (timeInterval > 0) {
        if (workInfo.IsRepeat()) {
            workInfoData->SetProperty("isRepeat", nativeEngine.CreateBoolean(true));
            workInfoData->SetProperty("repeatCycleTime", nativeEngine.CreateNumber(timeInterval));
        } else {
            workInfoData->SetProperty("repeatCycleTime", nativeEngine.CreateNumber(timeInterval));
            workInfoData->SetProperty("repeatCount", nativeEngine.CreateNumber(workInfo.GetCycleCount()));
        }
    }

    NativeValue* argv[] = {jworkInfoData};
    if (!jsObj_) {
        WS_HILOGI("WorkSchedulerExtension Not found js");
        return;
    }

    NativeValue* value = jsObj_->Get();
    NativeObject* obj = AbilityRuntime::ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        WS_HILOGI("WorkSchedulerExtension Failed to get WorkSchedulerExtension object");
        return;
    }

    NativeValue* method = obj->GetProperty("onWorkStart");
    if (method == nullptr) {
        WS_HILOGI("WorkSchedulerExtension Failed to get onWorkStart from WorkSchedulerExtension object");
        return;
    }
    nativeEngine.CallFunction(value, method, argv, 1);
    WS_HILOGI("WorkSchedulerExtension %{public}s end.", __func__);
}

void JsWorkSchedulerExtension::OnWorkStop(WorkInfo& workInfo)
{
    WorkSchedulerExtension::OnWorkStop(workInfo);
    WS_HILOGI("WorkSchedulerExtension %{public}s begin.", __func__);

    AbilityRuntime::HandleScope handleScope(jsRuntime_);
    NativeEngine& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* jworkInfoData = nativeEngine.CreateObject();
    NativeObject* workInfoData = AbilityRuntime::ConvertNativeValueTo<NativeObject>(jworkInfoData);
    workInfoData->SetProperty("workId", nativeEngine.CreateNumber(workInfo.GetWorkId()));

    std::string bundleName = workInfo.GetBundleName();
    workInfoData->SetProperty("bundleName", nativeEngine.CreateString(bundleName.c_str(), bundleName.size()));

    std::string abilityName = workInfo.GetAbilityName();
    workInfoData->SetProperty("abilityName", nativeEngine.CreateString(abilityName.c_str(), abilityName.size()));

    workInfoData->SetProperty("isPersisted", nativeEngine.CreateBoolean(workInfo.IsPersisted()));
    if (workInfo.GetNetworkType() != WorkCondition::Network::NETWORK_UNKNOWN) {
        workInfoData->SetProperty("networkType", nativeEngine.CreateNumber(workInfo.GetNetworkType()));
    }
    WorkCondition::Charger charger = workInfo.GetChargerType();
    if (charger != WorkCondition::Charger::CHARGING_UNKNOWN) {
        if (charger == WorkCondition::Charger::CHARGING_UNPLUGGED) {
            workInfoData->SetProperty("isCharging", nativeEngine.CreateBoolean(false));
        } else {
            workInfoData->SetProperty("isCharging", nativeEngine.CreateBoolean(true));
            workInfoData->SetProperty("chargerType", nativeEngine.CreateNumber(charger));
        }
    }
    if (workInfo.GetBatteryLevel() != INVALID_VALUE) {
        workInfoData->SetProperty("batteryLevel", nativeEngine.CreateNumber(workInfo.GetBatteryLevel()));
    }
    if (workInfo.GetBatteryStatus() != WorkCondition::BatteryStatus::BATTERY_UNKNOWN) {
        workInfoData->SetProperty("batteryStatus", nativeEngine.CreateNumber(workInfo.GetBatteryStatus()));
    }
    if (workInfo.GetStorageLevel() != WorkCondition::Storage::STORAGE_UNKNOWN) {
        workInfoData->SetProperty("storageRequest", nativeEngine.CreateNumber(workInfo.GetStorageLevel()));
    }

    uint32_t timeInterval = workInfo.GetTimeInterval();
    if (timeInterval > 0) {
        if (workInfo.IsRepeat()) {
            workInfoData->SetProperty("isRepeat", nativeEngine.CreateBoolean(true));
            workInfoData->SetProperty("repeatCycleTime", nativeEngine.CreateNumber(timeInterval));
        } else {
            workInfoData->SetProperty("repeatCycleTime", nativeEngine.CreateNumber(timeInterval));
            workInfoData->SetProperty("repeatCount", nativeEngine.CreateNumber(workInfo.GetCycleCount()));
        }
    }

    NativeValue* argv[] = {jworkInfoData};
    if (!jsObj_) {
        WS_HILOGI("WorkSchedulerExtension Not found js");
        return;
    }

    NativeValue* value = jsObj_->Get();
    NativeObject* obj = AbilityRuntime::ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        WS_HILOGI("WorkSchedulerExtension Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty("onWorkStop");
    if (method == nullptr) {
        WS_HILOGI("WorkSchedulerExtension Failed to get onWorkStop from object");
        return;
    }
    nativeEngine.CallFunction(value, method, argv, 1);
    WS_HILOGI("WorkSchedulerExtension %{public}s end.", __func__);
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
} // namespace AbilityRuntime
} // namespace OHOS