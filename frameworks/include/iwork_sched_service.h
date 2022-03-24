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
    enum {
        START_WORK = 0,
        STOP_WORK,
        STOP_AND_CANCEL_WORK,
        STOP_AND_CLEAR_WORKS,
        IS_LAST_WORK_TIMEOUT,
        OBTAIN_ALL_WORKS,
        GET_WORK_STATUS,
        DUMP_INFO
    };

    virtual  StartWork(WorkInfo& workInfo) = 0;
    /**
     * @brief The Stop work.
     * @param workInfo The info of work.
     * @return True if success,else false
     */
    virtual bool StopWork(WorkInfo& workInfo) = 0;
    /**
     * @brief The Stop and cancel work.
     * @param workInfo The info of work.
     * @return True if success,else false
     */
    virtual bool StopAndCancelWork(WorkInfo& workInfo) = 0;
    /**
     * @brief The Stop and clear works.
     * @return True if success,else false
     */
    virtual bool StopAndClearWorks() = 0;
    /**
     * @brief The Is last work timeout.
     * @return True if success,else false
     */
    virtual bool IsLastWorkTimeout(int32_t workId) = 0;
    virtual std::list<std::shared_ptr<WorkInfo>> ObtainAllWorks(int32_t &uid, int32_t &pid) = 0;
    virtual std::shared_ptr<WorkInfo> GetWorkStatus(int32_t &uid, int32_t &workId) = 0;
    /**
     * @brief The Shell dump.
     * @param workInfo The dumpOption,dumpInfo The dumpInfo.
     * @return True if success,else false
     */
    virtual bool ShellDump(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.workscheduler.iworkschedservice");
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_IWORK_SCHED_SERVICE_H