/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ani_work_scheduler_extension.h"
#include <string>
#include "runtime.h"
#include "ets_runtime.h"
#include "work_scheduler_extension.h"
#include "ani_work_scheduler_extension_context.h"
#include "work_scheduler_stub_ani.h"
#include "work_sched_constants.h"
#include "work_sched_hilog.h"
#include <ani_signature_builder.h>
#include "ani_utils.h"

namespace OHOS {
namespace WorkScheduler {
using namespace arkts::ani_signature;
constexpr const char* WORKSCHEDULERABILITY_CLS =
    "@ohos.WorkSchedulerExtensionAbility.WorkSchedulerExtensionAbility";
constexpr const char* WORKSCHEDULER_WORKINFO_CLS =
    "@ohos.resourceschedule.workScheduler.workScheduler.WorkInfoInner";
constexpr const char* CLASSNAME_BOOLEAN = "std.core.Boolean";
constexpr const char* CLASSNAME_INT = "std.core.Int";

AniWorkSchedulerExtension* AniWorkSchedulerExtension::Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime)
{
    WS_HILOGI("AniWorkSchedulerExtension begin Create");
    return new (std::nothrow) AniWorkSchedulerExtension(static_cast<AbilityRuntime::ETSRuntime&>(*runtime));
}

AniWorkSchedulerExtension::AniWorkSchedulerExtension(AbilityRuntime::ETSRuntime &etsRuntime)
    : etsRuntime_(etsRuntime) {}

AniWorkSchedulerExtension::~AniWorkSchedulerExtension()
{
    WS_HILOGD("Ani WorkScheduler extension destructor.");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }
}

void AniWorkSchedulerExtension::Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord>& record,
    const std::shared_ptr<AppExecFwk::OHOSApplication>& application,
    std::shared_ptr<AppExecFwk::AbilityHandler>& handler, const sptr<IRemoteObject>& token)
{
    WS_HILOGD("Init begin");
    WorkSchedulerExtension::Init(record, application, handler, token);
    std::string srcPath = "";
    std::string moduleName = "";
    if (!GetSrcPathAndModuleName(srcPath, moduleName)) {
        return;
    }
    env_ = etsRuntime_.GetAniEnv();
    if (env_ == nullptr) {
        HILOG_ERROR("null env");
        return;
    }
    WS_HILOGI("moduleName:%{public}s, srcPath:%{private}s.", moduleName.c_str(), srcPath.c_str());
    etsObj_ = etsRuntime_.LoadModule(moduleName, srcPath, abilityInfo_->hapPath,
        abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE, false, abilityInfo_->srcEntrance);
    if (etsObj_ == nullptr) {
        WS_HILOGE("WorkSchedulerExtension Failed to get etsObj_");
        return;
    }
    BindContext(env_, record->GetWant(), application);
}

bool AniWorkSchedulerExtension::GetSrcPathAndModuleName(std::string &srcPath, std::string &moduleName)
{
    if (!Extension::abilityInfo_) {
        WS_HILOGE("abilityInfo_ is nullptr");
        return false;
    }
    if (!Extension::abilityInfo_->isStageBasedModel) {
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
    } else if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        size_t pos = srcPath.rfind('.');
        if (pos != std::string::npos) {
            srcPath.erase(pos);
            srcPath.append(".abc");
        }
    } else {
        WS_HILOGE("Failed to get srcPath");
        return false;
    }
    moduleName = Extension::abilityInfo_->moduleName;
    moduleName.append("::").append(abilityInfo_->name);
    return true;
}

void AniWorkSchedulerExtension::BindContext(ani_env*env, std::shared_ptr<AAFwk::Want> want,
    const std::shared_ptr<AbilityRuntime::OHOSApplication> &application)
{
    if (env == nullptr || want == nullptr) {
        WS_HILOGE("env is null or want is null");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        WS_HILOGE("WorkSchedulerExtension Failed to get context");
        return;
    }
    ani_object contextObj = CreateAniWorkSchedulerExtensionContext(env, context, application);
    if (contextObj == nullptr) {
        WS_HILOGE("contextObj is null");
        return;
    }
    ani_field contextField;
    ani_class cls = nullptr;
    arkts::ani_signature::Type className =
        arkts::ani_signature::Builder::BuildClass(WORKSCHEDULERABILITY_CLS);
    if ((env->FindClass(className.Descriptor().c_str(), &cls)) != ANI_OK) {
        WS_HILOGE("FindClass failed");
        return;
    }
    auto status = env->Class_FindField(cls, "context", &contextField);
    if (status != ANI_OK) {
        WS_HILOGE("Class_GetField context failed");
        return;
    }
    ani_ref contextRef = nullptr;
    if (env->GlobalReference_Create(contextObj, &contextRef) != ANI_OK) {
        env->GlobalReference_Delete(contextRef);
        WS_HILOGE("GlobalReference_Create contextObj failed");
        return;
    }
    if (env->Object_SetField_Ref(etsObj_->aniObj, contextField, contextRef) != ANI_OK) {
        env->GlobalReference_Delete(contextRef);
        WS_HILOGE("Object_SetField_Ref contextObj failed");
        return;
    }
    env->GlobalReference_Delete(contextRef);
}

void AniWorkSchedulerExtension::OnStart(const AAFwk::Want& want)
{
    WS_HILOGD("begin");
    Extension::OnStart(want);
}

void AniWorkSchedulerExtension::OnStop()
{
    Extension::OnStop();
    WS_HILOGD("end.");
}

sptr<IRemoteObject> AniWorkSchedulerExtension::OnConnect(const AAFwk::Want& want)
{
    Extension::OnConnect(want);
    WS_HILOGI("OnConnect begin.");
    sptr<WorkSchedulerStubAni> remoteObject = new (std::nothrow) WorkSchedulerStubAni(
        std::static_pointer_cast<AniWorkSchedulerExtension>(shared_from_this()));
    if (remoteObject == nullptr) {
        WS_HILOGE("OnConnect get null");
        return remoteObject;
    }
    WS_HILOGD("end.");
    return remoteObject->AsObject();
}

void AniWorkSchedulerExtension::OnDisconnect(const AAFwk::Want& want)
{
    WS_HILOGD("begin.");
    Extension::OnDisconnect(want);
}

bool AniWorkSchedulerExtension::CallObjectMethod(const char* name, const char* signature, ...)
{
    ani_status status = ANI_OK;
    ani_method method = nullptr;
    auto env = etsRuntime_.GetAniEnv();
    if (!env) {
        WS_HILOGE("env not found StsNotificationSubscriberExtensions");
        return false;
    }
    if (etsObj_ == nullptr) {
        WS_HILOGE("etsObj_ nullptr");
        return false;
    }
    if ((status = env->Class_FindMethod(etsObj_->aniCls, name, signature, &method)) != ANI_OK) {
        WS_HILOGE("Class_FindMethod nullptr:%{public}d", status);
        return false;
    }
    if (method == nullptr) {
        return false;
    }

    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsObj_->aniObj, method, args)) != ANI_OK) {
        WS_HILOGE("status : %{public}d", status);
        return false;
    }
    va_end(args);
    return true;
}

void AniWorkSchedulerExtension::OnWorkStart(WorkInfo& workInfo)
{
    if (handler_ == nullptr) {
        WS_HILOGE("handler_ nullptr");
        return;
    }
    WS_HILOGI("begin OnWorkStart.");
    int32_t workId = workInfo.GetWorkId();
    std::string bundleName = workInfo.GetBundleName();
    std::string abilityName = workInfo.GetAbilityName();
    bool isPersisted = workInfo.IsPersisted();
    WorkCondition::Network networkType = workInfo.GetNetworkType();
    WorkCondition::Charger charger = workInfo.GetChargerType();
    int32_t batteryLevel = workInfo.GetBatteryLevel();
    WorkCondition::BatteryStatus batteryStatus = workInfo.GetBatteryStatus();
    WorkCondition::Storage storageLevel = workInfo.GetStorageLevel();
    int32_t timeInterval = static_cast<int32_t>(workInfo.GetTimeInterval());
    bool isRepeat = workInfo.IsRepeat();
    int32_t cycleCount = workInfo.GetCycleCount();
    WorkCondition::DeepIdle deepIdleValue = workInfo.GetDeepIdle();
    std::string extrasStr;
    bool getExtrasRet = GetExtrasJsonStr(workInfo, extrasStr);
    WorkSchedulerExtension::OnWorkStart(workInfo);
    auto task = [=]() {
        auto env = etsRuntime_.GetAniEnv();
        ani_object workInfoObject = WrapWorkInfo(env);
        if (workInfoObject == nullptr) {
            WS_HILOGE("WrapNotificationInfo failed");
            return;
        }
        SetCommonInfo(env, workInfoObject, workId, bundleName, abilityName);
        SetExtrasInfo(env, workInfoObject, getExtrasRet, extrasStr);
        SetPersistedInfo(env, workInfoObject, isPersisted);
        SetNetWorkInfo(env, workInfoObject, networkType);
        SetChargerTypeInfo(env, workInfoObject, charger);
        SetBatteryInfo(env, workInfoObject, batteryLevel, batteryStatus);
        SetStorageInfo(env, workInfoObject, storageLevel);
        SetDeepIdleInfo(env, workInfoObject, deepIdleValue);
        if (timeInterval > 0) {
            SetRepeatInfo(env, workInfoObject, isRepeat, timeInterval, cycleCount);
        }
        if (!CallObjectMethod("onWorkStart", nullptr, workInfoObject)) {
            return;
        }
    };
    handler_->PostTask(task);
    WS_HILOGD("OnWorkStart end.");
}

void AniWorkSchedulerExtension::OnWorkStop(WorkInfo& workInfo)
{
    if (handler_ == nullptr) {
        WS_HILOGE("handler_ nullptr");
        return;
    }
    WS_HILOGI("begin OnWorkStop.");
    int32_t workId = workInfo.GetWorkId();
    std::string bundleName = workInfo.GetBundleName();
    std::string abilityName = workInfo.GetAbilityName();
    bool isPersisted = workInfo.IsPersisted();
    WorkCondition::Network networkType = workInfo.GetNetworkType();
    WorkCondition::Charger charger = workInfo.GetChargerType();
    int32_t batteryLevel = workInfo.GetBatteryLevel();
    WorkCondition::BatteryStatus batteryStatus = workInfo.GetBatteryStatus();
    WorkCondition::Storage storageLevel = workInfo.GetStorageLevel();
    int32_t timeInterval = static_cast<int32_t>(workInfo.GetTimeInterval());
    bool isRepeat = workInfo.IsRepeat();
    int32_t cycleCount = workInfo.GetCycleCount();
    WorkCondition::DeepIdle deepIdleValue = workInfo.GetDeepIdle();
    std::string extrasStr;
    bool getExtrasRet = GetExtrasJsonStr(workInfo, extrasStr);
    WorkSchedulerExtension::OnWorkStop(workInfo);
    auto task = [=]() {
        auto env = etsRuntime_.GetAniEnv();
        ani_object workInfoObject = WrapWorkInfo(env);
        if (workInfoObject == nullptr) {
            WS_HILOGE("WrapNotificationInfo failed");
            return;
        }
        SetCommonInfo(env, workInfoObject, workId, bundleName, abilityName);
        SetExtrasInfo(env, workInfoObject, getExtrasRet, extrasStr);
        SetPersistedInfo(env, workInfoObject, isPersisted);
        SetNetWorkInfo(env, workInfoObject, networkType);
        SetChargerTypeInfo(env, workInfoObject, charger);
        SetBatteryInfo(env, workInfoObject, batteryLevel, batteryStatus);
        SetStorageInfo(env, workInfoObject, storageLevel);
        SetDeepIdleInfo(env, workInfoObject, deepIdleValue);
        if (timeInterval > 0) {
            SetRepeatInfo(env, workInfoObject, isRepeat, timeInterval, cycleCount);
        }
        if (!CallObjectMethod("onWorkStop", nullptr, workInfoObject)) {
            return;
        }
    };
    handler_->PostTask(task);
    WS_HILOGD("OnWorkStop end.");
}

ani_object AniWorkSchedulerExtension::WrapWorkInfo(ani_env* env)
{
    WS_HILOGD("WrapWorkInfo call");
    if (env == nullptr) {
        WS_HILOGE("env nullptr");
        return nullptr;
    }
    ani_object workInfoObject = nullptr;
    ani_class infoCls = nullptr;
    if (!CreateClassObjByClassName(env,
        WORKSCHEDULER_WORKINFO_CLS, infoCls, workInfoObject)) {
        WS_HILOGE("WrapWorkInfo : CreateClassObjByClassName failed");
        return nullptr;
    }
    return workInfoObject;
}

bool AniWorkSchedulerExtension::CreateClassObjByClassName(
    ani_env *env, const char *className, ani_class &cls, ani_object &outAniObj)
{
    if (env == nullptr || className == nullptr) {
        WS_HILOGE("CreateClassObjByClassName fail, has nullptr");
        return false;
    }
    if (env->FindClass(className, &cls) != ANI_OK) {
        WS_HILOGE("FindClass fail");
        return false;
    }
    ani_method ctor;
    if (env->Class_FindMethod(cls, "<ctor>", ":", &ctor) != ANI_OK) {
        WS_HILOGE("FindMethod fail");
        return false;
    }
    outAniObj = {};
    if (env->Object_New(cls, ctor, &outAniObj) != ANI_OK) {
        WS_HILOGE("Object_New fail");
        return false;
    }
    return true;
}

void AniWorkSchedulerExtension::SetCommonInfo(ani_env* env, ani_object& workInfoObject, int32_t workId,
    const std::string& bundleName, const std::string& abilityName)
{
    if (!SetPropertyOptionalByInt(env, workInfoObject, "workId", workId)) {
        WS_HILOGE("Set workId failed");
    }
    if (!SetPropertyOptionalByString(env, workInfoObject, "bundleName", bundleName)) {
        WS_HILOGE("Set bundleName failed");
    }
    if (!SetPropertyOptionalByString(env, workInfoObject, "abilityName", abilityName)) {
        WS_HILOGE("Set abilityName failed");
    }
}

void AniWorkSchedulerExtension::SetPersistedInfo(ani_env* env, ani_object& workInfoObject, bool isPersisted)
{
    if (!SetPropertyOptionalByBoolean(env, workInfoObject, "isPersisted", isPersisted)) {
        WS_HILOGE("Set isPersisted failed");
    }
}

void AniWorkSchedulerExtension::SetNetWorkInfo(
    ani_env* env, ani_object& workInfoObject, WorkCondition::Network networkType)
{
    if (networkType != WorkCondition::Network::NETWORK_UNKNOWN) {
        if (!SetPropertyOptionalByEnum(env, workInfoObject, "networkType", networkType)) {
            WS_HILOGE("Set netWorkInfo failed");
            return;
        }
    }
}

void AniWorkSchedulerExtension::SetChargerTypeInfo(
    ani_env* env, ani_object& workInfoObject, WorkCondition::Charger charger)
{
    if (charger != WorkCondition::Charger::CHARGING_UNKNOWN) {
        if (charger == WorkCondition::Charger::CHARGING_UNPLUGGED) {
            SetPropertyOptionalByBoolean(env, workInfoObject, "isCharging", false);
        } else {
            SetPropertyOptionalByBoolean(env, workInfoObject, "isCharging", true);
            SetPropertyOptionalByEnum(env, workInfoObject, "chargerType", charger);
        }
    }
}

void AniWorkSchedulerExtension::SetBatteryInfo(ani_env* env, ani_object& workInfoObject, int32_t batteryLevel,
    WorkCondition::BatteryStatus batteryStatus)
{
    if (batteryLevel != INVALID_VALUE) {
        SetPropertyOptionalByInt(env, workInfoObject, "batteryLevel", batteryLevel);
    }
    if (batteryStatus != WorkCondition::BatteryStatus::BATTERY_UNKNOWN) {
        SetPropertyOptionalByEnum(env, workInfoObject, "batteryStatus", batteryStatus);
    }
}

void AniWorkSchedulerExtension::SetStorageInfo(
    ani_env* env, ani_object& workInfoObject, WorkCondition::Storage storageLevel)
{
    if (storageLevel != WorkCondition::Storage::STORAGE_UNKNOWN) {
        SetPropertyOptionalByEnum(env, workInfoObject, "storageRequest", storageLevel);
    }
}

void AniWorkSchedulerExtension::SetRepeatInfo(ani_env* env, ani_object& workInfoObject, bool isRepeat,
    int32_t timeInterval, int32_t cycleCount)
{
    if (isRepeat) {
        SetPropertyOptionalByBoolean(env, workInfoObject, "isRepeat", true);
        SetPropertyOptionalByInt(env, workInfoObject, "repeatCycleTime", timeInterval);
    } else {
        SetPropertyOptionalByInt(env, workInfoObject, "repeatCycleTime", timeInterval);
        SetPropertyOptionalByInt(env, workInfoObject, "repeatCount", cycleCount);
    }
}

void AniWorkSchedulerExtension::SetDeepIdleInfo(
    ani_env* env, ani_object& workInfoObject, WorkCondition::DeepIdle deepIdleValue)
{
    if (deepIdleValue == WorkCondition::DeepIdle::DEEP_IDLE_UNKNOWN) {
        return;
    }
    SetPropertyOptionalByBoolean(env, workInfoObject, "isDeepIdle",
        deepIdleValue == WorkCondition::DeepIdle::DEEP_IDLE_IN);
}

void AniWorkSchedulerExtension::SetExtrasInfo(
    ani_env* env, ani_object& workInfoObject, bool getExtrasRet, const std::string& extrasStr)
{
    if (getExtrasRet) {
        SetPropertyOptionalByString(env, workInfoObject, "parameters", extrasStr);
    }
}

bool AniWorkSchedulerExtension::GetExtrasJsonStr(const WorkInfo& workInfo, std::string& extrasStr)
{
    std::shared_ptr<AAFwk::WantParams> extras = workInfo.GetExtras();
    nlohmann::json extrasJson;
    if (!extras) {
        WS_HILOGD("parameter is null.");
        return false;
    }
    auto extrasMap = extras->GetParams();
    int typeId = INVALID_VALUE;
    bool hasConditionKey = false;
    for (const auto& it : extrasMap) {
        typeId = AAFwk::WantParams::GetDataType(it.second);
        if (typeId != INVALID_VALUE) {
            std::string value = AAFwk::WantParams::GetStringByType(it.second, typeId);
            extrasJson[it.first] = value;
        } else {
            WS_HILOGE("parameters type not supported.");
        }
        if (it.first == WORK_SCHEDULER_CONDITION) {
            hasConditionKey = true;
        }
    }
    int32_t triggerType = workInfo.GetTriggerType();
    if (!hasConditionKey && triggerType != static_cast<int32_t>(WorkCondition::Type::UNKNOWN)) {
        extrasJson[WORK_SCHEDULER_CONDITION] = triggerType;
        WS_HILOGI("extras parameters triggerType:%{public}d.", triggerType);
    }
    extrasStr = extrasJson.dump(JSON_INDENT_WIDTH);
    return true;
}

bool AniWorkSchedulerExtension::SetPropertyOptionalByInt(
    ani_env *env, ani_object &object, const char *name, int32_t value)
{
    WS_HILOGI("enter SetPropertyOptionalByInt");
    if (env == nullptr || object == nullptr || name == nullptr) {
        WS_HILOGE("The parameter is invalid.");
        return false;
    }
    if ((env->Object_SetPropertyByName_Int(object, name, value)) != ANI_OK) {
        WS_HILOGE("set '%{public}s' faild", name);
        return false;
    }
    return true;
}

bool AniWorkSchedulerExtension::SetPropertyOptionalByString(
    ani_env *env, ani_object &object, const char *name, const std::string &value)
{
    WS_HILOGI("enter SetPropertyOptionalByString");
    if (env == nullptr || object == nullptr || name == nullptr) {
        WS_HILOGE("The parameter is invalid.");
        return false;
    }
    ani_string stringObj;
    ani_status status = ANI_OK;
    if ((status = GetAniStringByString(env, value, stringObj)) != ANI_OK) {
        WS_HILOGE("GetAniStringByString faild. status %{public}d", status);
        return false;
    }
    if (stringObj == nullptr) {
        WS_HILOGE("CreateString faild");
        return false;
    }
    return SetPropertyByRef(env, object, name, static_cast<ani_ref>(stringObj));
}

ani_status AniWorkSchedulerExtension::GetAniStringByString(
    ani_env* env, const std::string str, ani_string& aniStr)
{
    if (env == nullptr) {
        WS_HILOGE("GetAniStringByString fail, env is nullptr");
        return ANI_INVALID_ARGS;
    }
    ani_status status = env->String_NewUTF8(str.c_str(), str.size(), &aniStr);
    if (status != ANI_OK) {
        WS_HILOGE("String_NewUTF8 failed %{public}d", status);
        return status;
    }
    return status;
}

bool AniWorkSchedulerExtension::SetPropertyOptionalByEnum(
    ani_env *env, ani_object &object, const char *name, int32_t value)
{
    WS_HILOGI("enter SetPropertyOptionalByEnum");
    if (env == nullptr || object == nullptr || name == nullptr) {
        WS_HILOGE("The parameter is invalid.");
        return false;
    }
    ani_ref IntObj = CreateInt(env, value);
    if (IntObj == nullptr) {
        WS_HILOGE("CreateInt faild");
        return false;
    }
    return SetPropertyByRef(env, object, name, IntObj);
}

ani_object AniWorkSchedulerExtension::CreateInt(ani_env *env, int32_t value)
{
    ani_class cls;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(CLASSNAME_INT, &cls)) != ANI_OK) {
        WS_HILOGE("FindClass '%{public}s' faild. status %{public}d", CLASSNAME_INT, status);
        return nullptr;
    }
    ani_method ctor;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "i:", &ctor)) != ANI_OK) {
        WS_HILOGE("Class_FindMethod '%{public}s' faild. status %{public}d", CLASSNAME_INT, status);
        return nullptr;
    }
    ani_object outObj;
    if ((status = env->Object_New(cls, ctor, &outObj, value)) != ANI_OK) {
        WS_HILOGE("Object_New '%{public}s' faild. status %{public}d", CLASSNAME_INT, status);
        return nullptr;
    }
    return outObj;
}

bool AniWorkSchedulerExtension::SetPropertyOptionalByBoolean(
    ani_env *env, ani_object &object, const char *name, bool value)
{
    WS_HILOGI("enter SetPropertyOptionalByBoolean");
    if (env == nullptr || object == nullptr || name == nullptr) {
        WS_HILOGE("The parameter is invalid.");
        return false;
    }
    ani_ref boolObj = CreateBoolean(env, value);
    if (boolObj == nullptr) {
        WS_HILOGE("CreateBoolean faild");
        return false;
    }
    return SetPropertyByRef(env, object, name, boolObj);
}

ani_object AniWorkSchedulerExtension::CreateBoolean(ani_env *env, bool value)
{
    ani_class cls;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(CLASSNAME_BOOLEAN, &cls)) != ANI_OK) {
        WS_HILOGE("status : %{public}d", status);
        return nullptr;
    }
    ani_method personInfoCtor;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "z:", &personInfoCtor)) != ANI_OK) {
        WS_HILOGE("Class_FindMethod fail, status : %{public}d", status);
        return nullptr;
    }
    ani_object obj;
    if ((status = env->Object_New(cls, personInfoCtor, &obj, value ? ANI_TRUE : ANI_FALSE))
        != ANI_OK) {
        WS_HILOGE("status : %{public}d", status);
        return nullptr;
    }
    return obj;
}

bool AniWorkSchedulerExtension::SetPropertyByRef(ani_env *env, ani_object &object, const char *name, ani_ref value)
{
    WS_HILOGI("enter SetPropertyByRef");
    ani_status status = ANI_OK;
    if (env == nullptr || object == nullptr || name == nullptr || value == nullptr) {
        WS_HILOGE("The parameter is invalid.");
        return false;
    }
    if ((status = env->Object_SetPropertyByName_Ref(object, name, value)) != ANI_OK) {
        WS_HILOGE("set '%{public}s' faild. status %{public}d", name, status);
        return false;
    }
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS