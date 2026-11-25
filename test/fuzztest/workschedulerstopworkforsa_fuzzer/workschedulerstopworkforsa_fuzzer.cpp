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

#include "workschedulerstopworkforsa_fuzzer.h"

#include <cstdint>
#include <cstring>

#include "securec.h"

#include "iwork_sched_service.h"
#include "work_scheduler_service.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr int32_t DEFAULT_UID = 10008;
}

bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
{
    (void)bundleName;
    uid = DEFAULT_UID;
    return true;
}

namespace {
constexpr int32_t DEFAULT_SA_ID = 100;
const std::u16string WORK_SCHEDULER_STUB_TOKEN = u"ohos.workscheduler.iworkschedservice";
std::shared_ptr<WorkSchedulerService> g_workSchedulerService;

int32_t FuzzInt32(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t)) {
        return DEFAULT_SA_ID;
    }
    int32_t value = 0;
    memcpy_s(&value, sizeof(value), data, sizeof(value));
    return value;
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInterfaceToken(WORK_SCHEDULER_STUB_TOKEN);
    MessageParcel reply;
    MessageOption option;
    g_workSchedulerService = DelayedSingleton<WorkSchedulerService>::GetInstance();
    uint32_t code = static_cast<uint32_t>(IWorkSchedServiceIpcCode::COMMAND_STOP_WORK_FOR_S_A);
    int32_t saId = FuzzInt32(data, size);
    if (!dataMessageParcel.WriteInt32(saId)) {
        return false;
    }
    g_workSchedulerService->OnStart();
    g_workSchedulerService->InitWorkSchedPluginMgr();
    if (!g_workSchedulerService->ready_) {
        g_workSchedulerService->ready_ = true;
    }
    if (g_workSchedulerService->checkBundle_) {
        g_workSchedulerService->checkBundle_ = false;
    }
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
