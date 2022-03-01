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

#include "work_scheduler_stub.h"

namespace OHOS {
namespace WorkScheduler {
int WorkSchedulerStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    auto remoteDescriptor = data.ReadInterfaceToken();
    if (GetDescriptor() != remoteDescriptor) {
        WS_HILOGE("WorkSchedulerStub::OnRemoteRequest failed, descriptor is not matched!");
        return ERR_INVALID_STATE;
    }
    switch (code) {
        case COMMAND_ON_WORK_START: {
            WorkInfo* workInfo = data.ReadParcelable<WorkInfo>();
            OnWorkStart(*workInfo);
            return ERR_NONE;
        }
        case COMMAND_ON_WORK_STOP: {
            WorkInfo* workInfo = data.ReadParcelable<WorkInfo>();
            OnWorkStop(*workInfo);
            return ERR_NONE;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return ERR_TRANSACTION_FAILED;
}
} // namespace WorkScheduler
} // namespace OHOS
