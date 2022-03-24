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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORKSCHEDDULER_SRV_CLIENT_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORKSCHEDDULER_SRV_CLIENT_H

#include <string>

#include <singleton.h>

#include "iwork_sched_service.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerSrvClient final : public DelayedRefSingleton<WorkSchedulerSrvClient> {
    DECLARE_DELAYED_REF_SINGLETON(WorkSchedulerSrvClient)
public:
    DISALLOW_COPY_AND_MOVE(WorkSchedulerSrvClient);

    /**
     * @brief Start work.
     *
     * @param workInfo The info of work.
     * @return True if success,else false
     */
    bool StartWork(WorkInfo& workInfo);
    /**
     * @brief Stop work.
     *
     * @param workInfo The info of work.
     * @return True if success,else false
     */
    bool StopWork(WorkInfo& workInfo);
    /**
     * @brief Stop and cancel work.
     *
     * @param workInfo The info of work.
     * @return True if success,else false
     */
    bool StopAndCancelWork(WorkInfo& workInfo);
    /**
     * @brief Stop and clear works.
     *
     * @return True if success,else false
     */
    bool StopAndClearWorks();
    /**
     * @brief The last work timeout.
     *
     * @param workId The workId.
     * @param result The result.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode IsLastWorkTimeout(int32_t workId, bool &result);
    /**
     * @brief Get work status.
     *
     * @param workId The workId.
     * @param workInfo The info of work.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetWorkStatus(int32_t workId, std::shared_ptr<WorkInfo> &workInfo);
    /**
     * @brief Obtain all works.
     *
     * @param workInfos The infos of work.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ObtainAllWorks(std::list<std::shared_ptr<WorkInfo>> &workInfos);
    /**
     * @brief Shell dump.
     *
     * @param dumpOption The dump option.
     * @param dumpInfo The dump info.
     * @return True if success,else false
     */
    bool ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo);

private:
    class WorkSchedulerDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        WorkSchedulerDeathRecipient() = default;
        ~WorkSchedulerDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote);
    private:
        DISALLOW_COPY_AND_MOVE(WorkSchedulerDeathRecipient);
    };

    ErrCode Connect();
    sptr<IWorkSchedService> iWorkSchedService_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
    void ResetProxy(const wptr<IRemoteObject>& remote);
    std::mutex mutex_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORKSCHEDDULER_SRV_CLIENT_H