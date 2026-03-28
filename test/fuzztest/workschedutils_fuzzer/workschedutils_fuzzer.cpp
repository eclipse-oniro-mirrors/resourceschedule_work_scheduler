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

#include "workschedutils_fuzzer.h"
#include "work_sched_utils.h"
#include "refbase.h"
#include "os_account_manager.h"
#include "tokenid_kit.h"
#include "ipc_skeleton.h"
#include "parameters.h"
#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>
#include "bundle_mgr_proxy.h"
#include <cstdint>
#include <string>

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace AccountSA {
ErrCode OsAccountManager::QueryActiveOsAccountIds(std::vector<int32_t>& osAccountIds)
{
    osAccountIds.push_back(100);
    return ERR_OK;
}
}

namespace Security::AccessToken {
bool TokenIdKit::IsSystemAppByFullTokenID(uint64_t fullTokenId)
{
    return false;
}
}

namespace system {
bool GetBoolParameter(const std::string& key, bool def)
{
    return def;
}

int32_t GetIntParameter(const std::string& key, int32_t def)
{
    return def;
}
}

uint64_t IPCSkeleton::GetCallingFullTokenID()
{
    return 0;
}

sptr<ISystemAbilityManager> SystemAbilityManagerClient::GetSystemAbilityManager()
{
    return nullptr;
}

namespace WorkScheduler {
    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size < 8) {
            return false;
        }

        // Extract different types of data from the input
        const uint8_t* ptr = data;
        size_t remaining = size;

        // Get an int32_t for IsIdActive and GetUserIdByUid
        int32_t intValue = *reinterpret_cast<const int32_t*>(ptr);
        ptr += sizeof(int32_t);
        remaining -= sizeof(int32_t);

        // Get another int32_t for CheckExtensionInfos uid
        int32_t uid = *reinterpret_cast<const int32_t*>(ptr);
        ptr += sizeof(int32_t);
        remaining -= sizeof(int32_t);

        // Split remaining data into two strings for ConvertFullPath and CheckExtensionInfos
        size_t halfRemaining = remaining / 2;
        std::string string1(reinterpret_cast<const char*>(ptr), halfRemaining);
        ptr += halfRemaining;
        remaining -= halfRemaining;

        std::string string2(reinterpret_cast<const char*>(ptr), remaining);

        // Call all the functions we want to fuzz
        WorkSchedUtils::GetCurrentAccountId();
        WorkSchedUtils::IsIdActive(intValue);
        WorkSchedUtils::GetUserIdByUid(intValue);
        
        std::string fullPath;
        WorkSchedUtils::ConvertFullPath(string1, fullPath);
        
        WorkSchedUtils::IsSystemApp();
        WorkSchedUtils::GetCurrentTimeMs();
        WorkSchedUtils::CheckExtensionInfos(string1, string2, uid);
        WorkSchedUtils::IsUserMode();

        return true;
    }
} // WorkScheduler
} // OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}