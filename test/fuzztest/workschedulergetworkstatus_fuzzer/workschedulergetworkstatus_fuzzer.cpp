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

#include "workschedulergetworkstatus_fuzzer.h"

#include "iwork_sched_service_ipc_interface_code.h"
#include "work_scheduler_service.h"


namespace OHOS {
namespace WorkScheduler {
    const std::u16string WORK_SCHEDULER_STUB_TOKEN = u"ohos.workscheduler.iworkschedservice";
    static std::shared_ptr<WorkSchedulerService> workSchedulerService_;

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        MessageParcel dataMessageParcel;
        dataMessageParcel.WriteInterfaceToken(WORK_SCHEDULER_STUB_TOKEN);
        dataMessageParcel.WriteBuffer(data, size);
        dataMessageParcel.RewindRead(0);
        MessageParcel reply;
        MessageOption option;
        workSchedulerService_ = DelayedSingleton<WorkSchedulerService>::GetInstance();
        uint32_t code = static_cast<int32_t>(IWorkSchedServiceInterfaceCode::GET_WORK_STATUS);
        workSchedulerService_->OnRemoteRequest(code, dataMessageParcel, reply, option);
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