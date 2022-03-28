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

    /**
     * @brief Start work.
     *
     * @param workInfo The info of work.
     * @return True if success,else false.
     */
    bool StartWork(WorkInfo& workInfo) override;
    /**
     * @brief Stop work.
     *
     * @param workInfo The info of work.
     * @return True if success,else false.
     */
    bool StopWork(WorkInfo& workInfo) override;
    /**
     * @brief Stop and cancel work.
     *
     * @param workInfo The info of work.
     * @return True if success,else false.
     */
    bool StopAndCancelWork(WorkInfo& workInfo) override;
    /**
     * @brief Stop and clear works.
     *
     * @return True if success,else false.
     */
    bool StopAndClearWorks() override;
    /**
     * @brief The last work time out.
     *
     * @param workId The id of work.
     * @return True if success,else false.
     */
    bool IsLastWorkTimeout(int32_t workId) override;
    /**
     * @brief Obtain all works.
     *
     * @param uid The uid.
     * @param pid The pid.
     * @return Infos of work.
     */
    std::list<std::shared_ptr<WorkInfo>> ObtainAllWorks(int32_t &uid, int32_t &pid) override;
    /**
     * @brief Get the status of work.
     *
     * @param uid The uid.
     * @param pid The pid.
     * @return The status of work.
     */
    std::shared_ptr<WorkInfo> GetWorkStatus(int32_t &uid, int32_t &workId) override;
    /**
     * @brief Shell dump.
     *
     * @param dumpOption The dump option.
     * @param dumpInfo The dump info.
     * @return True if success,else false.
     */
    bool ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo) override;
private:
    static inline BrokerDelegator<WorkSchedServiceProxy> delegator_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_SERVICE_PROXY_H