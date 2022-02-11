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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_SERVICE_PROXY_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_SERVICE_PROXY_H

#include <iremote_proxy.h>
#include <nocopyable.h>
#include <memory>

#include "iwork_sched_service.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedServiceProxy : public IRemoteProxy<IWorkSchedService> {
public:
    explicit WorkSchedServiceProxy(const sptr<IRemoteObject>& impl) :
    IRemoteProxy<IWorkSchedService>(impl) {}
    ~WorkSchedServiceProxy() = default;
    DISALLOW_COPY_AND_MOVE(WorkSchedServiceProxy);

    virtual bool StartWork(WorkInfo& workInfo) override;
    virtual bool StopWork(WorkInfo& workInfo) override;
    virtual bool StopAndCancelWork(WorkInfo& workInfo) override;
    virtual bool StopAndClearWorks() override;
    virtual bool IsLastWorkTimeout(int32_t workId) override;
    virtual std::list<std::shared_ptr<WorkInfo>> ObtainAllWorks(int32_t &uid, int32_t &pid) override;
    virtual std::shared_ptr<WorkInfo> GetWorkStatus(int32_t &uid, int32_t &workId) override;
    virtual bool ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo) override;
private:
    static inline BrokerDelegator<WorkSchedServiceProxy> delegator_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_SERVICE_PROXY_H