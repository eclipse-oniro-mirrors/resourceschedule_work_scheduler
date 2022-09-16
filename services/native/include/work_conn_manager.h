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

#ifndef WORK_SCHED_SERVICES_WORK_CONN_MANAGER_H
#define WORK_SCHED_SERVICES_WORK_CONN_MANAGER_H

#include <map>
#include <string>
#include <memory>

#include "work_scheduler_connection.h"
#include "work_status.h"

namespace OHOS {
using namespace Utils;
namespace WorkScheduler {
class WorkConnManager {
public:
    /**
     * @brief Start work.
     *
     * @param workStatus The status of work.
     * @return True if success,else false.
     */
    bool StartWork(std::shared_ptr<WorkStatus> workStatus);
    /**
     * @brief Stop work.
     *
     * @param workStatus The status of work.
     * @return True if success,else false.
     */
    bool StopWork(std::shared_ptr<WorkStatus> workStatus);
private:
    void RemoveConnInfo(std::string &workId);
    void AddConnInfo(std::string &workId, sptr<WorkSchedulerConnection> &connection);
    sptr<WorkSchedulerConnection> GetConnInfo(std::string &workId);
    bool DisConnect(sptr<WorkSchedulerConnection> connect);
    void WriteStartWorkEvent(std::shared_ptr<WorkStatus> workStatus);
    std::mutex connMapMutex_;
    std::map<std::string, sptr<WorkSchedulerConnection>> connMap_;
    std::map<std::string, int32_t> eventIdMap_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // WORK_SCHED_SERVICES_WORK_CONN_MANAGER_H