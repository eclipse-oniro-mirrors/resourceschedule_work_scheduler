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

#ifndef OHOS_ZIDL_WORK_SCHED_STUB_IPC_INTERFACE_CODE_H
#define OHOS_ZIDL_WORK_SCHED_STUB_IPC_INTERFACE_CODE_H

/* SAID: 1904 */
namespace OHOS {
namespace WorkScheduler {
    enum class WorkSchedulerStubInterfaceCode {
        COMMAND_ON_WORK_START = MIN_TRANSACTION_ID,
        COMMAND_ON_WORK_STOP = MIN_TRANSACTION_ID + 1,
    };
} // namespace WorkScheduler
} // namespace OHOS
#endif // OHOS_ZIDL_WORK_SCHED_STUB_IPC_INTERFACE_CODE_H
