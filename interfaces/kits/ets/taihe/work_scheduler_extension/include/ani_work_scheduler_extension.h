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
 
#ifndef ANI_WORKSCHED_EXTENSION_H
#define ANI_WORKSCHED_EXTENSION_H
 
#include "work_scheduler_extension.h"
#include "ets_runtime.h"
#include "work_info.h"
#include "work_sched_hilog.h"
#include "ets_native_reference.h"
 
namespace OHOS {
namespace WorkScheduler {
class AniWorkSchedulerExtension : public WorkSchedulerExtension {
public:
    explicit AniWorkSchedulerExtension(AbilityRuntime::ETSRuntime& stsRuntime);
    ~AniWorkSchedulerExtension() override;
 
    /**
     * @brief Create AniWorkSchedulerExtension.
     *
     * @param runtime The runtime.
     * @return The AniWorkSchedulerExtension instance.
     */
    static AniWorkSchedulerExtension* Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime);
 
    /**
     * @brief The Init.
     *
     * @param record The record info.
     * @param application The application info.
     * @param handler The handler.
     * @param token The remote object token.
     */
    void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
              const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
              std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
              const sptr<IRemoteObject> &token) override;
 
    /**
     * @brief The OnStart callback.
     *
     * @param want The want.
     */
    void OnStart(const AAFwk::Want &want) override;
 
    /**
     * @brief The OnConnect callback.
     *
     * @param want The want.
     * @return Remote object.
     */
    sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;
 
    /**
     * @brief The OnDisconnect callback.
     *
     * @param want The want.
     */
    void OnDisconnect(const AAFwk::Want &want) override;
 
    /**
     * @brief The OnStop callback.
     */
    void OnStop() override;
 
    /**
     * @brief The OnWorkStart callback.
     *
     * @param workInfo The info of work.
     */
    void OnWorkStart(WorkInfo& workInfo) override;
 
    /**
     * @brief The OnWorkStop callback.
     *
     * @param workInfo The info of work.
     */
    void OnWorkStop(WorkInfo& workInfo) override;
 
private:
    bool CallObjectMethod(const char* name, const char* signature, ...);
    void BindContext(ani_env* env, std::shared_ptr<AAFwk::Want> want,
        const std::shared_ptr<AbilityRuntime::OHOSApplication> &application);
    bool GetSrcPathAndModuleName(std::string &srcPath, std::string &moduleName);
    bool GetExtrasJsonStr(const WorkInfo& workInfo, std::string& extrasStr);
    ani_object WrapWorkInfo(ani_env* env);
    bool CreateClassObjByClassName(ani_env *env, const char *className, ani_class &cls, ani_object &outAniObj);
    void SetCommonInfo(ani_env *env, ani_object &workInfoObject, int32_t workId,
        const std::string& bundleName, const std::string& abilityName);
    void SetPersistedInfo(ani_env *env, ani_object &workInfoObject, bool isPersisted);
    void SetPerformanceSensitiveInfo(ani_env *env, ani_object &workInfoObject, bool isPerformanceSensitive);
    void SetNetWorkInfo(ani_env *env, ani_object &workInfoObject, WorkCondition::Network networkType);
    void SetChargerTypeInfo(ani_env *env, ani_object &workInfoObject, WorkCondition::Charger charger);
    void SetBatteryInfo(ani_env *env, ani_object &workInfoObject, int32_t batteryLevel,
        WorkCondition::BatteryStatus batteryStatus);
    void SetStorageInfo(ani_env *env, ani_object &workInfoObject, WorkCondition::Storage storageLevel);
    void SetDeepIdleInfo(ani_env *env, ani_object &workInfoObject, WorkCondition::DeepIdle deepIdleValue);
    void SetExtrasInfo(ani_env *env, ani_object &workInfoObject, bool getExtrasRet, const std::string& extrasStr);
    void SetRepeatInfo(ani_env *env, ani_object &workInfoObject, bool isRepeat,
        int32_t timeInterval, int32_t cycleCount);
    bool SetPropertyOptionalByBoolean(ani_env *env, ani_object &object, const char *name, bool value);
    ani_object CreateBoolean(ani_env *env, bool value);
    bool SetPropertyOptionalByString(
        ani_env *env, ani_object &object, const char *name, const std::string &value);
    ani_status GetAniStringByString(ani_env* env, const std::string& str, ani_string& aniStr);
    bool SetPropertyOptionalByInt(ani_env *env, ani_object &object, const char *name, int32_t value);
    ani_object CreateInt(ani_env *env, int32_t value);
    bool SetPropertyOptionalByEnum(ani_env *env, ani_object &object, const char *name, int32_t value);
    bool SetPropertyByRef(ani_env *env, ani_object &object, const char *name, ani_ref value);
 
    std::unique_ptr<AppExecFwk::ETSNativeReference> etsObj_;
    AbilityRuntime::ETSRuntime& etsRuntime_;
    ani_env* env_ = nullptr;
};
}  // namespace WorkScheduler
}  // namespace OHOS
#endif  // ANI_WORKSCHED_EXTENSION_H