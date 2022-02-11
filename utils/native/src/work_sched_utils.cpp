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

#include <string>
#include <vector>

#include "errors.h"
#include "ohos_account_kits.h"
#include "os_account_manager.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
int WorkSchedUtils::GetCurrentAccountId()
{
    std::vector<AccountSA::OsAccountInfo> osAccountInfos;
    ErrCode ret = AccountSA::OsAccountManager::QueryAllCreatedOsAccounts(osAccountInfos);
    if (ret != ERR_OK) {
        WS_HILOGE("QueryAllCreatedOsAccounts failed.");
        return 0;
    }

    if (osAccountInfos.empty()) {
        WS_HILOGE("osAccountInfos is empty, no accounts.");
        return 0;
    }

    for (const auto& account : osAccountInfos) {
        if (account.GetIsActived()) {
            return account.GetLocalId();
        }
    }
    WS_HILOGE("GetCurrentAccountId failed, no Actived now.");
    return -1;
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
}
}