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

#include "workschedutils_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <string>
#include <vector>

#include "securec.h"
#include "work_sched_utils.h"
#include "refbase.h"
#include "os_account_manager.h"
#include "account_error_no.h"
#include "tokenid_kit.h"
#include "ipc_skeleton.h"
#include "parameters.h"
#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>
#include "bundle_mgr_proxy.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace {
// Per-iteration values consumed by the interposed stubs below, so every stub is fuzz-input driven.
struct FuzzStubContext {
    bool inFuzzIteration = false;
    uint8_t accountMode = 0;
    int32_t activeAccountId = 0;
    bool isSystemApp = false;
    uint64_t callingTokenId = 0;
    bool secureMode = false;
    int32_t debuggable = 0;
};
FuzzStubContext g_fuzzStubContext;

constexpr uint8_t ACCOUNT_MODE_COUNT = 5;
constexpr uint8_t MOD_TWO = 2;
constexpr int32_t DEFAULT_ACCOUNT_ID = 100;
constexpr int32_t INVALID_ACCOUNT_ID = -1;
constexpr size_t MIN_DATA_SIZE = 8;
} // namespace

namespace AccountSA {
ErrCode OsAccountManager::QueryActiveOsAccountIds(std::vector<int32_t>& osAccountIds)
{
    if (!g_fuzzStubContext.inFuzzIteration) {
        osAccountIds.push_back(DEFAULT_ACCOUNT_ID);
        return ERR_OK;
    }
    switch (g_fuzzStubContext.accountMode % ACCOUNT_MODE_COUNT) {
        case 0:
            osAccountIds.push_back(DEFAULT_ACCOUNT_ID);
            break;
        case 1:
            osAccountIds.push_back(DEFAULT_ACCOUNT_ID);
            osAccountIds.push_back(g_fuzzStubContext.activeAccountId);
            break;
        case 2:
            osAccountIds.push_back(INVALID_ACCOUNT_ID);
            break;
        case 3:
            break;
        default:
            return ERR_ACCOUNT_COMMON_NOT_INIT_ERROR;
    }
    return ERR_OK;
}
} // namespace AccountSA

namespace Security::AccessToken {
bool TokenIdKit::IsSystemAppByFullTokenID(uint64_t fullTokenId)
{
    (void)fullTokenId;
    return g_fuzzStubContext.inFuzzIteration && g_fuzzStubContext.isSystemApp;
}
} // namespace Security::AccessToken

namespace system {
bool GetBoolParameter(const std::string& key, bool def)
{
    (void)key;
    (void)def;
    return g_fuzzStubContext.inFuzzIteration && g_fuzzStubContext.secureMode;
}

int32_t GetIntParameter(const std::string& key, int32_t def)
{
    (void)key;
    (void)def;
    return g_fuzzStubContext.inFuzzIteration ? g_fuzzStubContext.debuggable : 0;
}
} // namespace system

uint64_t IPCSkeleton::GetCallingFullTokenID()
{
    return g_fuzzStubContext.callingTokenId;
}

sptr<ISystemAbilityManager> SystemAbilityManagerClient::GetSystemAbilityManager()
{
    // No real SAMgr runs in the fuzz process. A non-null manager would need a hand-written
    // implementation of dozens of ISystemAbilityManager pure virtuals plus IBundleMgr IPC
    // emulation, so CheckExtensionInfos' SAM-null branch is fuzzed here and the IPC branch is
    // bounded by this stub.
    return nullptr;
}

namespace WorkScheduler {
static int32_t GetInt32FromData(const uint8_t* data, size_t size, size_t& offset)
{
    int32_t val = 0;
    if (offset + sizeof(int32_t) <= size &&
        memcpy_s(&val, sizeof(val), data + offset, sizeof(int32_t)) == EOK) {
        offset += sizeof(int32_t);
    }
    return val;
}

static uint64_t GetUint64FromData(const uint8_t* data, size_t size, size_t& offset)
{
    uint64_t val = 0;
    if (offset + sizeof(uint64_t) <= size &&
        memcpy_s(&val, sizeof(val), data + offset, sizeof(uint64_t)) == EOK) {
        offset += sizeof(uint64_t);
    }
    return val;
}

static uint8_t GetByteFromData(const uint8_t* data, size_t size, size_t& offset)
{
    uint8_t val = 0;
    if (offset < size) {
        val = data[offset];
        ++offset;
    }
    return val;
}

// corpus/init keeps the OAT-required '#' copyright header; skip leading comment/blank lines so
// the seed payload placed after the header is fed to the fuzz logic unchanged.
const uint8_t* SkipCorpusCommentHeader(const uint8_t* data, size_t size, size_t& payloadSize)
{
    size_t index = 0;
    while (index < size) {
        size_t lineEnd = index;
        while (lineEnd < size && data[lineEnd] != '\n') {
            ++lineEnd;
        }
        size_t first = index;
        while (first < lineEnd && (data[first] == ' ' || data[first] == '\t')) {
            ++first;
        }
        bool isCommentOrBlank = first == lineEnd || data[first] == '#';
        if (!isCommentOrBlank) {
            break;
        }
        index = (lineEnd < size) ? lineEnd + 1 : lineEnd;
    }
    payloadSize = size - index;
    return data + index;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < MIN_DATA_SIZE) {
        return false;
    }

    size_t offset = 0;
    int32_t intValue = GetInt32FromData(data, size, offset);
    int32_t uid = GetInt32FromData(data, size, offset);

    g_fuzzStubContext.inFuzzIteration = true;
    g_fuzzStubContext.accountMode = GetByteFromData(data, size, offset);
    g_fuzzStubContext.activeAccountId = intValue;
    g_fuzzStubContext.isSystemApp = (GetByteFromData(data, size, offset) % MOD_TWO) == 1;
    g_fuzzStubContext.callingTokenId = GetUint64FromData(data, size, offset);
    g_fuzzStubContext.secureMode = (GetByteFromData(data, size, offset) % MOD_TWO) == 1;
    g_fuzzStubContext.debuggable = static_cast<int32_t>(GetByteFromData(data, size, offset) % MOD_TWO);

    size_t remaining = size - offset;
    size_t halfRemaining = remaining / 2;
    std::string bundleName(reinterpret_cast<const char*>(data + offset), halfRemaining);
    std::string abilityName(reinterpret_cast<const char*>(data + offset + halfRemaining),
        remaining - halfRemaining);

    WorkSchedUtils::GetCurrentAccountId();
    WorkSchedUtils::IsIdActive(intValue);
    WorkSchedUtils::GetUserIdByUid(intValue);

    std::string fullPath;
    WorkSchedUtils::ConvertFullPath(bundleName, fullPath);

    WorkSchedUtils::IsSystemApp();
    WorkSchedUtils::GetCurrentTimeMs();
    WorkSchedUtils::CheckExtensionInfos(bundleName, abilityName, uid);
    WorkSchedUtils::IsUserMode();

    g_fuzzStubContext.inFuzzIteration = false;
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    size_t payloadSize = 0;
    const uint8_t* payload = OHOS::WorkScheduler::SkipCorpusCommentHeader(data, size, payloadSize);
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(payload, payloadSize);
    return 0;
}
