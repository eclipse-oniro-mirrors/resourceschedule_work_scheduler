/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
 
#include "work_sched_config.h"
#include "res_sched_signature_validator.h"
#include "work_sched_hilog.h"
#include "nlohmann/json.hpp"
#include "work_scheduler_service.h"
 
namespace OHOS {
namespace WorkScheduler {
namespace {
const std::string CONFIG_JSON_INDEX_TOP = "params";
const std::string CONFIG_WORK_SCHEDULER = "work_scheduler";
const std::string PRINSTALLED_WORKS_KEY = "work_scheduler_preinstalled_works";
const std::string ACTIVE_GROUP_WHITELIST = "active_group_whitelist";
const std::string MIN_REPEAT_TIME_KEY = "work_scheduler_min_repeat_time";
const std::string EXEMPTION_BUNDLES_KEY = "work_scheduler_eng_exemption_bundles";
}
void WorkSchedulerConfig::InitActiveGroupWhitelist(const std::string &configData)
{
    std::lock_guard<std::mutex> lock(configMutex_);
    const nlohmann::json &jsonObj = nlohmann::json::parse(configData, nullptr, false);
    if (jsonObj.is_discarded()) {
        WS_HILOGE("jsonObj parse fail");
        return;
    }
    if (jsonObj.is_null() || jsonObj.empty()) {
        WS_HILOGE("jsonObj null");
        return;
    }
    if (!jsonObj.contains(ACTIVE_GROUP_WHITELIST) ||
        !jsonObj[ACTIVE_GROUP_WHITELIST].is_array()) {
        WS_HILOGE("no key %{public}s", ACTIVE_GROUP_WHITELIST.c_str());
        return;
    }
    nlohmann::json appArray = jsonObj[ACTIVE_GROUP_WHITELIST];
    for (const auto &app : appArray) {
        activeGroupWhitelist_.insert(app);
    }
    for (const auto &app : activeGroupWhitelist_) {
        WS_HILOGI("activeGroupWhitelist_: %{public}s.", app.c_str());
    }
}

bool WorkSchedulerConfig::CheckCloudConfigParam(const nlohmann::json &payload, nlohmann::json &workSchedulerParam)
{
    if (payload.is_null() || payload.empty()) {
        WS_HILOGE("payload null");
        return false;
    }
    if (!payload.contains(CONFIG_JSON_INDEX_TOP) ||
        !payload[CONFIG_JSON_INDEX_TOP].is_object()) {
        WS_HILOGE("no key %{public}s", CONFIG_JSON_INDEX_TOP.c_str());
        return false;
    }
    nlohmann::json params = payload[CONFIG_JSON_INDEX_TOP];
    if (!params.contains(CONFIG_WORK_SCHEDULER) ||
        !params[CONFIG_WORK_SCHEDULER].is_object()) {
        WS_HILOGE("no key %{public}s", CONFIG_WORK_SCHEDULER.c_str());
        return false;
    }
    workSchedulerParam = params[CONFIG_WORK_SCHEDULER];
    return true;
}

void WorkSchedulerConfig::UpdateCloudConfigMinRepeatTime(const nlohmann::json &root)
{
    if (!root.contains(MIN_REPEAT_TIME_KEY)) {
        WS_HILOGE("no work_scheduler_min_repeat_time key");
        return;
    }
    nlohmann::json minRepeatTimeRoot = root[MIN_REPEAT_TIME_KEY];
    if (minRepeatTimeRoot.empty() || !minRepeatTimeRoot.is_object()) {
        WS_HILOGE("work_scheduler_min_repeat_time content is empty");
        return;
    }
    if (!minRepeatTimeRoot.contains("special")) {
        WS_HILOGE("no special key");
        return;
    }
    nlohmann::json specialRoot = minRepeatTimeRoot["special"];
    if (specialRoot.empty() || !specialRoot.is_array()) {
        WS_HILOGE("special content is empty");
        return;
    }
    DelayedSingleton<WorkSchedulerService>::GetInstance()->UpdateCloudConfigMinRepeatTime(specialRoot);
}

void WorkSchedulerConfig::UpdateCloudConfigEngExemptionBundles(const nlohmann::json &root)
{
    if (!root.contains(EXEMPTION_BUNDLES_KEY)) {
        WS_HILOGE("no work_scheduler_eng_exemption_bundles key");
        return;
    }
    nlohmann::json exemptionBundlesRoot = root[EXEMPTION_BUNDLES_KEY];
    if (exemptionBundlesRoot.empty() || !exemptionBundlesRoot.is_array()) {
        WS_HILOGE("work_scheduler_eng_exemption_bundles content is empty");
        return;
    }
    DelayedSingleton<WorkSchedulerService>::GetInstance()->UpdateCloudConfigEngExemptionBundles(exemptionBundlesRoot);
}
 
void WorkSchedulerConfig::UpdateCloudConfigPrinstalledWorkKey(const nlohmann::json &root)
{
    if (!root.contains(PRINSTALLED_WORKS_KEY)) {
        WS_HILOGE("no work_scheduler_preinstalled_works key");
        return;
    }
    nlohmann::json preinstalledWorksRoot = root[PRINSTALLED_WORKS_KEY];
    if (preinstalledWorksRoot.empty() || !preinstalledWorksRoot.is_object()) {
        WS_HILOGE("work_scheduler_preinstalled_works content is empty");
        return;
    }
    DelayedSingleton<WorkSchedulerService>::GetInstance()->UpdateCloudConfigPrinstalledWorkKey(preinstalledWorksRoot);
}

bool WorkSchedulerConfig::UpdateSusMgrCloudConfig(const nlohmann::json &payload)
{
    nlohmann::json workSchedulerParam;
    if (!CheckCloudConfigParam(payload, workSchedulerParam)) {
        return false;
    }
    if (!workSchedulerParam.contains(ACTIVE_GROUP_WHITELIST) ||
        !workSchedulerParam[ACTIVE_GROUP_WHITELIST].is_array()) {
        WS_HILOGE("no key %{public}s", ACTIVE_GROUP_WHITELIST.c_str());
        return false;
    }
    // 延迟任务活跃分组
    nlohmann::json activeGroupWhiteList = workSchedulerParam[ACTIVE_GROUP_WHITELIST];
    std::lock_guard<std::mutex> lock(configMutex_);
    activeGroupWhitelist_.clear();
    for (const auto &app : activeGroupWhiteList) {
        if (!app.is_string()) {
            continue;
        }
        activeGroupWhitelist_.insert(app);
    }
    for (const auto &app : activeGroupWhitelist_) {
        WS_HILOGI("cloud config activeGroupWhitelist app name: %{public}s.", app.c_str());
    }
    return true;
}
 
bool WorkSchedulerConfig::IsInActiveGroupWhitelist(const std::string &bundleName)
{
    {
        std::lock_guard<std::mutex> lock(configMutex_);
        if (!activeGroupWhitelist_.count(bundleName)) {
            return false;
        }
    }
    return ResourceSchedule::ResSchedSignatureValidator::GetInstance().CheckSignatureByBundleName(bundleName) ==
           ResourceSchedule::SignatureCheckResult::CHECK_OK;
}

bool WorkSchedulerConfig::UpdateBgMgrCloudConfig(const nlohmann::json &payload)
{
    nlohmann::json workSchedulerParam;
    if (!CheckCloudConfigParam(payload, workSchedulerParam)) {
        return false;
    }
    // PC延迟任务刷新频率
    UpdateCloudConfigMinRepeatTime(workSchedulerParam);
    // 延迟任务超限豁免
    UpdateCloudConfigEngExemptionBundles(workSchedulerParam);
    // 延迟任务系统预置应用、延迟任务拉起SA
    UpdateCloudConfigPrinstalledWorkKey(workSchedulerParam);
    return true;
}
} // WorkScheduler
} // OHOS