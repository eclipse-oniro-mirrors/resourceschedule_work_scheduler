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

#include "workschedulerobtainworksbyuidandworkidforinner_fuzzer.h"

#include <cstdint>
#include <cstring>

#include "securec.h"

#include "iwork_sched_service.h"
#include "work_scheduler_service.h"
#include "work_condition.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
{
    return true;
}

namespace {
const std::u16string WORK_SCHEDULER_STUB_TOKEN = u"ohos.workscheduler.iworkschedservice";
std::shared_ptr<WorkSchedulerService> g_workSchedulerService;

int32_t FuzzInt32(const uint8_t* data, size_t size, size_t offset = 0)
{
    if (data == nullptr || size < offset + sizeof(int32_t)) {
        return 0;
    }
    int32_t value = 0;
    memcpy_s(&value, sizeof(value), data + offset, sizeof(value));
    return value;
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
    uint32_t code =
        static_cast<uint32_t>(IWorkSchedServiceIpcCode::COMMAND_OBTAIN_WORKS_BY_UID_AND_WORK_ID_FOR_INNER);
    int32_t uid = FuzzInt32(data, size);
    int32_t workId = FuzzInt32(data, size, sizeof(int32_t));
    if (!dataMessageParcel.WriteInt32(uid)) {
        return false;
    }
    if (!dataMessageParcel.WriteInt32(workId)) {
        return false;
    }
    InitService();
    WorkInfo workInfo;
    workInfo.SetWorkId(workId);
    workInfo.SetElement("fuzz_bundle", "fuzz_ability");
    workInfo.RefreshUid(uid);
    workInfo.SetIsInnerApply(true);
    g_workSchedulerService->AddWorkInner(workInfo);
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
