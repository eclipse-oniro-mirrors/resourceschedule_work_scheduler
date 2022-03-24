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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHEDULER_SERVICES_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHEDULER_SERVICES_H

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <iremote_object.h>
#include <system_ability.h>

#include "ability_manager_interface.h"
#include "delayed_sp_singleton.h"
#include "policy/app_data_clear_listener.h"
#include "policy/app_removed_listener.h"
#include "work_info.h"
#include "work_sched_service_stub.h"
#include "work_status.h"
#include "work_event_handler.h"

namespace OHOS {
namespace WorkScheduler {
class WorkQueueManager;
class WorkPolicyManager;
class WorkSchedulerService final : public SystemAbility, public WorkSchedServiceStub {
    DECLARE_SYSTEM_ABILITY(WorkSchedulerService);
    DECLARE_DELAYED_SP_SINGLETON(WorkSchedulerService);
public:
    /**
     * @brief The OnStart callback.
     */
    void OnStart() override;
    /**
     * @brief The OnStop callback.
     */
    void OnStop() override;
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
     * @brief The last work timeout.
     *
     * @param workId The workId.
     * @return True if success,else false.
     */
    bool IsLastWorkTimeout(int32_t workId) override;
    /**
     * @brief Obtain all works.
     *
     * @param uid The uid.
     * @param pid The pid.
     * @return work policy manager.
     */
    std::list<std::shared_ptr<WorkInfo>> ObtainAllWorks(int32_t &uid, int32_t &pid) override;
    /**
     * @brief Get work status.
     *
     * @param uid The uid.
     * @param workId The workId.
     * @return work policy manager.
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
    /**
     * @brief Init persisted.
     */
    void InitPersisted();
    /**
     * @brief Stop and clear works by uid.
     *
     * @param uid The uid.
     * @return True if success,else false.
     */
    bool StopAndClearWorksByUid(int32_t uid);
    /**
     * @brief Create node dir.
     *
     * @param dir The dir.
     * @return ERR_OK.
     */
    int32_t CreateNodeDir(std::string dir);
    /**
     * @brief Create node file.
     *
     * @param filePath The file path.
     * @return ERR_OK.
     */
    int32_t CreateNodeFile(std::string filePath);
    /**
     * @brief Update work before real start.
     *
     * @param work The work.
     */
    void UpdateWorkBeforeRealStart(std::shared_ptr<WorkStatus> work);
    /**
     * @brief The OnConditionReady callback.
     *
     * @param workStatusVector The work status vector.
     */
    void OnConditionReady(std::shared_ptr<std::vector<std::shared_ptr<WorkStatus>>> workStatusVector);
    /**
     * @brief Watchdog timeout.
     *
     * @param workStatus The work status.
     */
    void WatchdogTimeOut(std::shared_ptr<WorkStatus> workStatus);
    /**
     * @brief Init.
     *
     * @return True if success,else false.
     */
    bool Init();

    std::shared_ptr<WorkEventHandler> GetHandler()
    {
        return handler_;
    }

    std::shared_ptr<WorkQueueManager> GetWorkQueueManager()
    {
        return workQueueManager_;
    }

    std::shared_ptr<WorkPolicyManager> GetWorkPolicyManager()
    {
        return workPolicyManager_;
    }

private:
    const char* PERSISTED_FILE_PATH = "/data/service/el1/public/WorkScheduler/persisted_work";
    const char* PERSISTED_PATH = "/data/service/el1/public/WorkScheduler";
    const char* PERSISTED_FILE = "persisted_work";

    std::shared_ptr<WorkQueueManager> workQueueManager_;
    std::shared_ptr<WorkPolicyManager> workPolicyManager_;
    std::mutex mutex_;
    std::map<std::string, std::shared_ptr<WorkInfo>> persistedMap_;
    bool ready_ {false};
    std::shared_ptr<WorkEventHandler> handler_;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_;
    bool checkBundle_ {true};

    void WorkQueueManagerInit();
    bool WorkPolicyManagerInit();
    void RefreshPersistedWorks();
    std::list<std::shared_ptr<WorkInfo>> ReadPersistedWorks();
    void InitPersistedWork(WorkInfo& workInfo);
    void DumpAllInfo(std::vector<std::string> &dumpInfo);
    void DumpWorkQueueInfo(std::vector<std::string> &dumpInfo);
    void DumpWorkPolicyInfo(std::vector<std::string> &dumpInfo);
    bool CheckWorkInfo(WorkInfo &workInfo, int32_t &uid);
    bool StopWorkInner(std::shared_ptr<WorkStatus> workStatus, int32_t uid, const bool needCancel, bool isTimeOut);
    bool CheckCondition(WorkInfo& workInfo);
    void DumpDebugInfo(std::vector<std::string> &dumpInfo);
    bool IsBaseAbilityReady();
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHEDULER_SERVICES_H