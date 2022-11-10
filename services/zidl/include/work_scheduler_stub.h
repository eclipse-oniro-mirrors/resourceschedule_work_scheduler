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

#ifndef OHOS_ZIDL_WORK_SCHED_STUB_H
#define OHOS_ZIDL_WORK_SCHED_STUB_H

#include <iremote_stub.h>
#include "iwork_scheduler.h"
#include "work_scheduler_extension.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerStub : public IRemoteStub<IWorkScheduler> {
public:
    WorkSchedulerStub() {}

    virtual ~WorkSchedulerStub() {}

    /**
     * @brief The OnRemoteRequest callback.
     *
     * @param code The code.
     * @param data The data.
     * @param reply The reply.
     * @param option The option.
     * @return ERR_OK on success, others on failure.
     */
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // OHOS_ZIDL_WORK_SCHED_STUB_H

