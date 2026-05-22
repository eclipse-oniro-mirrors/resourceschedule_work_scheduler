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

#include "workschedulersrvclient_fuzzer.h"

#include <cstdint>
#include <cstring>

#include "securec.h"
#include <memory>
#include <string>
#include <list>

#include "workscheduler_srv_client.h"
#include "work_info.h"
#include "work_condition.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t NETWORK_TYPE_COUNT = 8;
constexpr size_t BYTE_CHECK_INDEX_1 = 1;
constexpr size_t OFFSET_UID = 2;
constexpr size_t OFFSET_WORK_ID = 3;
constexpr size_t OFFSET_QUERY_UID = 4;
constexpr size_t OFFSET_SOURCE_TYPE = 5;
constexpr size_t OFFSET_SA_ID = 6;

int32_t FuzzInt32(const uint8_t* data, size_t size, size_t offset = 0)
{
    if (data == nullptr || size < offset + sizeof(int32_t)) {
        return 0;
    }
    int32_t value = 0;
    memcpy_s(&value, sizeof(value), data + offset, sizeof(value));
    return value;
}

std::string FuzzString(const uint8_t* data, size_t size, size_t maxLen = 128)
{
    if (data == nullptr || size == 0) {
        return "fuzz_bundle";
    }
    size_t len = std::min(size, maxLen);
    return std::string(reinterpret_cast<const char*>(data), len);
}

WorkInfo BuildFuzzWorkInfo(const uint8_t* data, size_t size)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(FuzzInt32(data, size));
    workInfo.SetElement("fuzz_bundle", "fuzz_ability");
    workInfo.RequestPersisted((size > 0) && ((data[0] & 1) != 0));
    workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(
        FuzzInt32(data, size, sizeof(int32_t)) % NETWORK_TYPE_COUNT));
    workInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY);
    workInfo.RefreshUid(FuzzInt32(data, size, sizeof(int32_t) * OFFSET_UID));
    return workInfo;
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t)) {
        return false;
    }

    auto& client = WorkSchedulerSrvClient::GetInstance();

    WorkInfo startWorkInfo = BuildFuzzWorkInfo(data, size);
    (void)client.StartWork(startWorkInfo);

    WorkInfo innerWorkInfo = BuildFuzzWorkInfo(data, size);
    (void)client.StartWorkForInner(innerWorkInfo);

    WorkInfo stopWorkInfo = BuildFuzzWorkInfo(data, size);
    (void)client.StopWork(stopWorkInfo);

    WorkInfo stopInnerInfo = BuildFuzzWorkInfo(data, size);
    bool needCancel = (size > BYTE_CHECK_INDEX_1) && ((data[BYTE_CHECK_INDEX_1] & 1) != 0);
    (void)client.StopWorkForInner(stopInnerInfo, needCancel);

    WorkInfo cancelInfo = BuildFuzzWorkInfo(data, size);
    (void)client.StopAndCancelWork(cancelInfo);

    (void)client.StopAndClearWorks();

    int32_t workId = FuzzInt32(data, size, sizeof(int32_t) * OFFSET_WORK_ID);
    bool timeoutResult = false;
    (void)client.IsLastWorkTimeout(workId, timeoutResult);

    std::shared_ptr<WorkInfo> statusInfo = nullptr;
    (void)client.GetWorkStatus(workId, statusInfo);

    std::list<std::shared_ptr<WorkInfo>> allWorks;
    (void)client.ObtainAllWorks(allWorks);

    int32_t uid = FuzzInt32(data, size, sizeof(int32_t) * OFFSET_QUERY_UID);
    std::list<std::shared_ptr<WorkInfo>> innerWorks;
    (void)client.ObtainWorksByUidAndWorkIdForInner(uid, innerWorks, workId);

    std::list<std::shared_ptr<WorkInfo>> runningWorks;
    (void)client.GetAllRunningWorks(runningWorks);

    (void)client.PauseRunningWorks(uid);
    (void)client.ResumePausedWorks(uid);

    std::string configData = FuzzString(data, size);
    int32_t sourceType = FuzzInt32(data, size, sizeof(int32_t) * OFFSET_SOURCE_TYPE);
    (void)client.SetWorkSchedulerConfig(configData, sourceType);

    int32_t saId = FuzzInt32(data, size, sizeof(int32_t) * OFFSET_SA_ID);
    (void)client.StopWorkForSA(saId);

    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
