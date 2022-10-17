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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_IWORK_SCHED_SERVICE_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_IWORK_SCHED_SERVICE_H

#include <string>
#include <vector>

#include <iremote_broker.h>
#include <iremote_object.h>

#include "work_info.h"

namespace OHOS {
namespace WorkScheduler {
class IWorkSchedService : public IRemoteBroker {
public:
    IWorkSchedService() = default;
    ~IWorkSchedService() override = default;
    DISALLOW_COPY_AND_MOVE(IWorkSchedService);

    enum {
        START_WORK = 0,
        STOP_WORK,
        STOP_AND_CANCEL_WORK,
        STOP_AND_CLEAR_WORKS,
        IS_LAST_WORK_TIMEOUT,
        OBTAIN_ALL_WORKS,
        GET_WORK_STATUS,
    };

    /**
     * @brief Start work.
     *
     * @param workInfo The info of work.
     * @return The errcode. ERR_OK on success, others on failure.
     */
    virtual int32_t StartWork(WorkInfo& workInfo) = 0;
    /**
     * @brief Stop work.
     *
     * @param workInfo The info of work.
     * @return The errcode. ERR_OK on success, others on failure.
     */
    virtual int32_t StopWork(WorkInfo& workInfo) = 0;
    /**
     * @brief Stop and cancel work.
     *
     * @param workInfo The info of work.
     * @return The errcode. ERR_OK on success, others on failure.
     */
    virtual int32_t StopAndCancelWork(WorkInfo& workInfo) = 0;
    /**
     * @brief Stop and clear works.
     *
     * @return The errcode. ERR_OK on success, others on failure.
     */
    virtual int32_t StopAndClearWorks() = 0;
    /**
     * @brief Check whether last work executed time out.
     *
     * @param workId The id of work.
     * @param result True if the work executed time out, else false.
     * @return The errcode. ERR_OK on success, others on failure.
     */
    virtual int32_t IsLastWorkTimeout(int32_t workId, bool &result) = 0;
    /**
     * @brief Obtain all works.
     *
     * @param uid The uid.
     * @param pid The pid.
     * @param workInfos The infos of work.
     * @return The errcode. ERR_OK on success, others on failure.
     */
    virtual int32_t ObtainAllWorks(int32_t &uid, int32_t &pid,
        std::list<std::shared_ptr<WorkInfo>>& workInfos) = 0;
    /**
     * @brief Check whether last work executed time out.
     *
     * @param uid The uid.
     * @param workId The id of work.
     * @param workInfo The info of work.
     * @return The errcode. ERR_OK on success, others on failure.
     */
    virtual int32_t GetWorkStatus(int32_t &uid, int32_t &workId, std::shared_ptr<WorkInfo>& workInfo) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.workscheduler.iworkschedservice");
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_IWORK_SCHED_SERVICE_H