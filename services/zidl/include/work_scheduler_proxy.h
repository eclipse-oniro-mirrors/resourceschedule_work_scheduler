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

#ifndef OHOS_ZIDL_WORK_SCHED_PROXY_H
#define OHOS_ZIDL_WORK_SCHED_PROXY_H

#include <iremote_proxy.h>
#include "iwork_scheduler.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerProxy : public IRemoteProxy<IWorkScheduler> {
public:
    explicit WorkSchedulerProxy(const sptr<IRemoteObject>& remote)
        : IRemoteProxy<IWorkScheduler>(remote) {}

    virtual ~WorkSchedulerProxy() {}

    /**
     * @brief The OnWorkStart callback.
     *
     * @param workInfo The info of work.
     */
    void OnWorkStart(WorkInfo& workInfo) override;

    /**
     * @brief The OnWorkStop callback.
     *
     * @param workInfo The info of work.
     */
    void OnWorkStop(WorkInfo& workInfo) override;

private:
    static inline BrokerDelegator<WorkSchedulerProxy> delegator_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // OHOS_ZIDL_WORK_SCHED_PROXY_H

