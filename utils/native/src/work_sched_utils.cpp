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
#include "work_sched_utils.h"

#include "errors.h"
#include "ohos_account_kits.h"
#include "os_account_manager.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
const int32_t INVALID_DATA = -1;
int32_t WorkSchedUtils::GetCurrentAccountId()
{
    std::vector<int32_t> osAccountIds;
    ErrCode ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(osAccountIds);
    if (ret != ERR_OK) {
        WS_HILOGE("QueryActiveOsAccountIds failed.");
        return -1;
    }

    if (osAccountIds.empty()) {
        WS_HILOGE("osAccountInfos is empty, no accounts.");
        return -1;
    }

    auto iter = std::find_if(osAccountIds.cbegin(), osAccountIds.cend(),
        [](const int32_t &accountId) { return accountId >= 0; });
    if (iter != osAccountIds.end()) {
        return *iter;
    }
    WS_HILOGE("GetCurrentAccountId failed, no osAccountIds now.");
    return -1;
}

bool WorkSchedUtils::IsIdActive(int32_t id)
{
    std::vector<int32_t> osAccountIds;
    ErrCode ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(osAccountIds);
    if (ret != ERR_OK) {
        WS_HILOGE("QueryActiveOsAccountIds failed.");
        return false;
    }

    if (osAccountIds.empty()) {
        WS_HILOGE("osAccountIds is empty, no accounts.");
        return false;
    }

    auto iter = std::find_if(osAccountIds.cbegin(), osAccountIds.cend(),
        [&id](const int32_t &accountId) { return accountId == id; });
    if (iter != osAccountIds.end()) {
        return true;
    }
    WS_HILOGE("IsIdActive failed, osAccountIds now.");
    return false;
}

int32_t WorkSchedUtils::GetUserIdByUid(int32_t uid)
{
    if (uid <= INVALID_DATA) {
        WS_HILOGE("uid is illegal: %{public}d", uid);
        return INVALID_DATA;
    }
    const int32_t baseUserRange = 200000;
    return uid / baseUserRange;
}

bool WorkSchedUtils::ConvertFullPath(const std::string& partialPath, std::string& fullPath)
{
    if (partialPath.empty() || partialPath.length() >= PATH_MAX) {
        return false;
    }
    char tmpPath[PATH_MAX] = {0};
    if (realpath(partialPath.c_str(), tmpPath) == nullptr) {
        return false;
    }
    fullPath = tmpPath;
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS