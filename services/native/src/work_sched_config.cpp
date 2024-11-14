/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "work_sched_hilog.h"
#include "nlohmann/json.hpp"
 
namespace OHOS {
namespace WorkScheduler {
namespace {
const std::string ACTIVE_GROUP_WHITELIST = "active_group_whitelist";
 
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
 
bool WorkSchedulerConfig::IsInActiveGroupWhitelist(const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(configMutex_);
    return activeGroupWhitelist_.count(bundleName) > 0;
}
} // WorkScheduler
} // OHOS