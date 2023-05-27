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

#include <list>
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
     * @return ERR_OK on success, others on failure.
     */
    ErrCode StartWork(WorkInfo& workInfo);
    /**
     * @brief Stop work.
     *
     * @param workInfo The info of work.
     * @return ERR_OK on success, others on failure.
     */
    ErrCode StopWork(WorkInfo& workInfo);
    /**
     * @brief Stop and cancel work.
     *
     * @param workInfo The info of work.
     * @return ERR_OK on success, others on failure.
     */
    ErrCode StopAndCancelWork(WorkInfo& workInfo);
    /**
     * @brief Stop and clear works.
     *
     * @return ERR_OK on success, others on failure.
     */
    ErrCode StopAndClearWorks();
    /**
     * @brief The last work time out.
     *
     * @param workId The id of work.
     * @param result True if the work executed time out, else false.
     * @return ERR_OK on success, others on failure.
     */
    ErrCode IsLastWorkTimeout(int32_t workId, bool &result);
    /**
     * @brief Get the status of work.
     *
     * @param workId The id of work.
     * @param workInfo The info of work.
     * @return ERR_OK on success, others on failure.
     */
    ErrCode GetWorkStatus(int32_t workId, std::shared_ptr<WorkInfo> &workInfo);
    /**
     * @brief Obtain all works.
     *
     * @param workInfos The infos of work.
     * @return ERR_OK on success, others on failure.
     */
    ErrCode ObtainAllWorks(std::list<std::shared_ptr<WorkInfo>> &workInfos);

private:
    class WorkSchedulerDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit WorkSchedulerDeathRecipient(WorkSchedulerSrvClient &workSchedulerSrvClient);
        ~WorkSchedulerDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    private:
        WorkSchedulerSrvClient &workSchedulerSrvClient_;
    };

    ErrCode Connect();
    sptr<IWorkSchedService> iWorkSchedService_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
    void ResetProxy();
    std::mutex mutex_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORKSCHEDDULER_SRV_CLIENT_H