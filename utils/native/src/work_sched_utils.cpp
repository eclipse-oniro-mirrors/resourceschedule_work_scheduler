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
int WorkSchedUtils::GetCurrentAccountId()
{
    std::vector<int> osAccountIds;
    ErrCode ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(osAccountIds);
    if (ret != ERR_OK) {
        WS_HILOGE("QueryActiveOsAccountIds failed.");
        return -1;
    }

    if (osAccountIds.empty()) {
        WS_HILOGE("osAccountInfos is empty, no accounts.");
        return -1;
    }

    for (const auto& accountId : osAccountIds) {
        if (accountId >= 0) {
            return accountId;
        }
    }
    WS_HILOGE("GetCurrentAccountId failed, no osAccountIds now.");
    return -1;
}

bool WorkSchedUtils::IsIdActive(int id)
{
    std::vector<int> osAccountIds;
    ErrCode ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(osAccountIds);
    if (ret != ERR_OK) {
        WS_HILOGE("QueryActiveOsAccountIds failed.");
        return false;
    }

    if (osAccountIds.empty()) {
        WS_HILOGE("osAccountIds is empty, no accounts.");
        return false;
    }

    for (const auto& accountId : osAccountIds) {
        if (accountId == id) {
            return true;
        }
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
    const int BASE_USER_RANGE = 200000;
    return uid / BASE_USER_RANGE;
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
}
}