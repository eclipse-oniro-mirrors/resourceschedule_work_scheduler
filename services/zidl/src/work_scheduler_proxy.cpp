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

#include "work_scheduler_proxy.h"

namespace OHOS {
namespace WorkScheduler {
static constexpr int32_t COMMAND_ON_WORK_START = MIN_TRANSACTION_ID;
static constexpr int32_t COMMAND_ON_WORK_STOP = MIN_TRANSACTION_ID + 1;
void WorkSchedulerProxy::OnWorkStart(WorkInfo& workInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WS_HILOGE("write descriptor failed!");
        return;
    }
    data.WriteParcelable(&workInfo);
    Remote()->SendRequest(COMMAND_ON_WORK_START, data, reply, option);
}

void WorkSchedulerProxy::OnWorkStop(WorkInfo& workInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WS_HILOGE("write descriptor failed!");
        return;
    }
    data.WriteParcelable(&workInfo);
    Remote()->SendRequest(COMMAND_ON_WORK_STOP, data, reply, option);
}
} // namespace WorkScheduler
} // namespace OHOS
