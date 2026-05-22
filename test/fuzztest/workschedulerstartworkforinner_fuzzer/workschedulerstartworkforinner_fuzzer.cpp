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

#include "workschedulerstartworkforinner_fuzzer.h"

#include <cstdint>
#include <cstring>

#include "iwork_sched_service.h"
#include "work_scheduler_service.h"
#include "work_condition.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
const std::u16string WORK_SCHEDULER_STUB_TOKEN = u"ohos.workscheduler.iworkschedservice";
std::shared_ptr<WorkSchedulerService> g_workSchedulerService;

bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
{
    return true;
}

int32_t FuzzInt32(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t)) {
        return 0;
    }
    int32_t value = 0;
    memcpy(&value, data, sizeof(value));
    return value;
}

WorkInfo BuildFuzzWorkInfo(const uint8_t* data, size_t size)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(FuzzInt32(data, size));
    workInfo.SetElement("fuzz_bundle", "fuzz_ability");
    workInfo.SetIsInnerApply(true);
    workInfo.RequestPersisted((size > 0) && ((data[0] & 1) != 0));
    workInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY);
    workInfo.RefreshUid(FuzzInt32(data, size));
    return workInfo;
}

void InitService()
{
    g_workSchedulerService = DelayedSingleton<WorkSchedulerService>::GetInstance();
    g_workSchedulerService->OnStart();
    g_workSchedulerService->InitBgTaskSubscriber();
    if (!g_workSchedulerService->ready_) {
        g_workSchedulerService->ready_ = true;
    }
    if (g_workSchedulerService->checkBundle_) {
        g_workSchedulerService->checkBundle_ = false;
    }
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInterfaceToken(WORK_SCHEDULER_STUB_TOKEN);
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(IWorkSchedServiceIpcCode::COMMAND_START_WORK_FOR_INNER);
    WorkInfo workInfo = BuildFuzzWorkInfo(data, size);
    if (!dataMessageParcel.WriteParcelable(&workInfo)) {
        return false;
    }
    InitService();
    g_workSchedulerService->OnRemoteRequest(code, dataMessageParcel, reply, option);
    g_workSchedulerService->OnStop();
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
