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

#include <list>
#include <memory>

#include <iremote_proxy.h>
#include <nocopyable.h>
#include "refbase.h"

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
     * @return error code, ERR_OK if success.
     */
    int32_t StartWork(WorkInfo& workInfo) override;
    /**
     * @brief Stop work.
     *
     * @param workInfo The info of work.
     * @return error code, ERR_OK if success.
     */
    int32_t StopWork(WorkInfo& workInfo) override;
    /**
     * @brief Stop and cancel work.
     *
     * @param workInfo The info of work.
     * @return error code, ERR_OK if success.
     */
    int32_t StopAndCancelWork(WorkInfo& workInfo) override;
    /**
     * @brief Stop and clear works.
     *
     * @return error code, ERR_OK if success.
     */
    int32_t StopAndClearWorks() override;
    /**
     * @brief The last work time out.
     *
     * @param workId The id of work.
     * @param result True if the work executed time out, else false.
     * @return error code, ERR_OK if success.
     */
    int32_t IsLastWorkTimeout(int32_t workId, bool &result) override;
    /**
     * @brief Obtain all works.
     *
     * @param uid The uid.
     * @param pid The pid.
     * @param workInfos The infos of work.
     * @return error code, ERR_OK if success.
     */
    int32_t ObtainAllWorks(int32_t &uid, int32_t &pid, std::list<std::shared_ptr<WorkInfo>>& workInfos) override;
    /**
     * @brief Get the status of work.
     *
     * @param uid The uid.
     * @param workId The id of work.
     * @param workInfo The info of work.
     * @return error code, ERR_OK if success.
     */
    int32_t GetWorkStatus(int32_t &uid, int32_t &workId, std::shared_ptr<WorkInfo>& workInfo) override;
private:
    static inline BrokerDelegator<WorkSchedServiceProxy> delegator_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_SERVICE_PROXY_H