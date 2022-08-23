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

#include "workscheduleservice_fuzzer.h"

#include "work_event_handler.h"
#define private public
#include "work_scheduler_service.h"
#include "work_sched_service_stub.h"

namespace OHOS {
namespace WorkScheduler {
    constexpr int32_t MIN_LEN = 4;
    constexpr int32_t MAX_CODE_TEST = 15; // current max code is 7
    static bool isInited = false;

    bool DoInit()
    {
        auto instance = DelayedSpSingleton<WorkSchedulerService>::GetInstance();
        instance->OnStart();
        if (!instance->eventRunner_ || !instance->handler_) {
            return false;
        }
        instance->checkBundle_ = false;
        return true;
    }

    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data)
    {
        MessageParcel reply;
        MessageOption option;
        int32_t ret = DelayedSpSingleton<WorkSchedulerService>::GetInstance()->OnRemoteRequest(code,
            data, reply, option);
        return ret;
    }

    void IpcFuzzTest(const uint8_t* data, size_t size)
    {
        if (size <= MIN_LEN) {
            return;
        }

        MessageParcel dataMessageParcel;
        if (!dataMessageParcel.WriteInterfaceToken(WorkSchedServiceStub::GetDescriptor())) {
            return;
        }

        uint32_t code = *(reinterpret_cast<const uint32_t*>(data));
        if (code > MAX_CODE_TEST) {
            return;
        }
        size -= sizeof(uint32_t);

        dataMessageParcel.WriteBuffer(data + sizeof(uint32_t), size);
        dataMessageParcel.RewindRead(0);

        if (!isInited) {
            isInited = DoInit();
        }
        if (isInited) {
            OnRemoteRequest(code, dataMessageParcel);
        }
    }
} // namespace WorkScheduler
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::WorkScheduler::IpcFuzzTest(data, size);
    return 0;
}
