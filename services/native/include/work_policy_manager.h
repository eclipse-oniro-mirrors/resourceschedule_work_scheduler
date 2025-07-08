/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_SERVICES_POLICY_MANAGER_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_SERVICES_POLICY_MANAGER_H
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include <event_runner.h>
#include "policy_type.h"
#include "policy/ipolicy_filter.h"
#include "work_conn_manager.h"
#include "work_queue.h"
#include "work_status.h"
#include "ffrt.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerService;
class WorkEventHandler;
class AppDataClearListener;
class Watchdog;
class WorkPolicyManager {
public:
    explicit WorkPolicyManager(const std::shared_ptr<WorkSchedulerService>& wss);
    ~WorkPolicyManager() = default;
    /**
     * @brief Init.
     *
     * @return True if success,else false.
     */
    bool Init(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    /**
     * @brief Add listener.
     *
     * @return True if success,else false.
     */
    bool AddListener();
    /**
     * @brief Add policy filter.
     *
     * @param filter The filter.
     */
    void AddPolicyFilter(std::shared_ptr<IPolicyFilter> filter);
    /**
     * @brief Add work.
     *
     * @param workStatus The status of work.
     * @param uid The uid.
     * @return True if success,else false.
     */
    int32_t AddWork(std::shared_ptr<WorkStatus> workStatus, int32_t uid);
    /**
     * @brief Remove work.
     *
     * @param workStatus The status of work.
     * @param uid The uid.
     * @return True if success,else false.
     */
    bool RemoveWork(std::shared_ptr<WorkStatus> workStatus, int32_t uid);
    /**
     * @brief Stop work.
     *
     * @param workStatus The status of work.
     * @param uid The uid.
     * @param needCancel The need cancel.
     * @param isTimeOut The is time out.
     * @return True if success,else false.
     */
    bool StopWork(std::shared_ptr<WorkStatus> workStatus, int32_t uid, const bool needCancel, bool isTimeOut);
    /**
     * @brief Stop and clear works.
     *
     * @param uid The uid.
     * @return True if success,else false.
     */
    bool StopAndClearWorks(int32_t uid);
    /**
     * @brief The last work time out.
     *
     * @param workId The id of work.
     * @param uid The uid.
     * @param result True if success,else false.
     * @return The error code.
     */
    int32_t IsLastWorkTimeout(int32_t workId, int32_t uid, bool &result);
    /**
     * @brief Find work status.
     *
     * @param workInfo The info of work.
     * @param uid The uid.
     * @return The uid queue map.
     */
    std::shared_ptr<WorkStatus> FindWorkStatus(WorkInfo& workInfo, int32_t uid);
    /**
     * @brief Find work status.
     *
     * @param uId The uId.
     * @param workId The workId.
     * @return The workStatus of workId.
     */
    std::shared_ptr<WorkStatus> FindWorkStatus(int32_t uId, int32_t workId);
    /**
     * @brief Find SA.
     *
     * @param saId The id of sa.
     * @param uid The uid.
     * @return The SA of uid.
     */
    std::shared_ptr<WorkStatus> FindSA(int32_t saId, int32_t uid);
    /**
     * @brief The OnConditionReady callback.
     *
     * @param workStatusVector The work status vector.
     */
    void OnConditionReady(std::shared_ptr<std::vector<std::shared_ptr<WorkStatus>>> workStatusVector);
    /**
     * @brief The OnPolicyChanged callback.
     *
     * @param policyType The policy type.
     * @param detectorVal The detector val.
     */
    void OnPolicyChanged(PolicyType policyType, std::shared_ptr<DetectorValue> detectorVal);
    /**
     * @brief Obtain all works.
     *
     * @param uid The uid.
     * @return All works.
     */
    std::vector<WorkInfo> ObtainAllWorks(int32_t &uid);
    /**
     * @brief Get the status of work.
     *
     * @param uid The uid.
     * @param workInfo The info of work.
     * @return The status of work.
     */
    std::shared_ptr<WorkInfo> GetWorkStatus(int32_t &uid, int32_t &workInfo);
    /**
     * @brief Get all status of work.
     *
     * @param uid The uid.
     * @return All status of work.
     */
    std::list<std::shared_ptr<WorkStatus>> GetAllWorkStatus(int32_t &uid);

    /**
     * @brief Get the All Running Works object.
     *
     * @return All running works;
     */
    std::vector<WorkInfo> GetAllRunningWorks();

    /**
     * @brief The Dump.
     *
     * @param result The result.
     */
    void Dump(std::string& result);
    int32_t currentRunningCount = 0;
    /**
     * @brief Check work to run.
     */
    void CheckWorkToRun();
    /**
     * @brief Send retrigger.
     *
     * @param delayTime The delay time.
     */
    void SendRetrigger(int32_t delayTime);
    /**
     * @brief Add app data clear listener.
     *
     * @param listener The listener.
     */
    void AddAppDataClearListener(std::shared_ptr<AppDataClearListener> listener);
    /**
     * @brief Watchdog time out.
     *
     * @param watchdogId The id of watchdog.
     */
    void WatchdogTimeOut(uint32_t watchdogId);
    /**
     * @brief Set memory by dump.
     *
     * @param memory The memory.
     */
    void SetMemoryByDump(int32_t memory);
    /**
     * @brief Get dump set memory.
     *
     * @return Dump set memory.
     */
    int32_t GetDumpSetMemory();
    /**
     * @brief Set watchdog time by dump.
     *
     * @param time The time.
     */
    void SetWatchdogTimeByDump(int32_t time);
    /**
     * @brief Set watchdog time.
     *
     * @param time The time.
     */
    void SetWatchdogTime(int32_t time);
    /**
     * @brief Get the time of watchdog.
     *
     * @return The time of watchdog.
     */
    int32_t GetWatchdogTime();
    /**
     * @brief Trigger the ide work.
     */
    void TriggerIdeWork();
    /**
     * @brief Provide a command to exec onStart and onStop of the matched service extension.
     * @param bunlerName The bundleName.
     * @param abilityName The abilityName.
     */
    void DumpCheckIdeWorkToRun(const std::string &bundleName, const std::string &abilityName);
    /**
     * @brief Provide a command to exec onStart and onStop of the matched service extension.
     * @param uId The uId.
     * @param workId The workId.
     * @param result The result.
     */
    void DumpTriggerWork(int32_t uId, int32_t workId, std::string& result);
    /**
     * @brief Set cpu by dump.
     *
     * @param cpu The cpu.
     */
    void SetCpuUsageByDump(int32_t cpu);
    /**
     * @brief Get dump set cpu.
     *
     * @return Dump set cpu.
     */
    int32_t GetDumpSetCpuUsage();
    /**
     * @brief Set max running count by dump.
     *
     * @param count The count.
     */
    void SetMaxRunningCountByDump(int32_t count);
    /**
     * @brief Get dump set max running count.
     *
     * @return Dump set max running count.
     */
    int32_t GetDumpSetMaxRunningCount();

    /**
     * @brief Pause Running Works.
     *
     * @param uid The uid.
     * @return The errcode. ERR_OK on success, others on failure.
     */
    int32_t PauseRunningWorks(int32_t uid);

    /**
     * @brief Resume Paused works.
     *
     * @param uid The uid.
     * @return ErrCode ERR_OK on success, others on failure
     */
    int32_t ResumePausedWorks(int32_t uid);

    /**
     * @brief Remove watchdog From Queue.
     *
     * @param workStatus The workStatus.
     */
    void RemoveWatchDog(std::shared_ptr<WorkStatus> workStatus);

    /**
     * @brief Get All DeepIdle Works.
     *
     * @return All DeepIdle Works.
     */
    std::list<std::shared_ptr<WorkStatus>> GetDeepIdleWorks();
    bool FindWork(int32_t uid);
    bool FindWork(const int32_t userId, const std::string &bundleName);
private:
    int32_t GetMaxRunningCount(WorkSchedSystemPolicy& systemPolicy);
    int32_t GetRunningCount();
    void DumpConditionReadyQueue(std::string& result);
    void DumpUidQueueMap(std::string& result);
    void RemoveFromUidQueue(std::shared_ptr<WorkStatus> workStatus, int32_t uid);
    void RemoveFromReadyQueue(std::shared_ptr<WorkStatus> workStatus);
    void AddToReadyQueue(std::shared_ptr<std::vector<std::shared_ptr<WorkStatus>>> workStatusVector);
    void RealStartWork(std::shared_ptr<WorkStatus> workStatus);
    void RealStartSA(std::shared_ptr<WorkStatus> workStatus);
    void AddToRunningQueue(std::shared_ptr<WorkStatus> workStatus);
    void RemoveConditionUnReady();
    std::shared_ptr<WorkStatus> GetWorkToRun();
    void RemoveAllUnReady();
    uint32_t NewWatchdogId();
    void AddWatchdogForWork(std::shared_ptr<WorkStatus> workStatus);
    std::shared_ptr<WorkStatus> GetWorkFromWatchdog(uint32_t id);
    void UpdateWatchdogTime(const std::shared_ptr<WorkSchedulerService> &wmsptr,
        std::shared_ptr<WorkStatus> &topWork);
    std::list<std::shared_ptr<WorkStatus>> GetAllIdeWorkStatus(const std::string &bundleName,
        const std::string &abilityName);
    void SendIdeWorkRetriggerEvent(int32_t delaytime);
    bool IsSpecialScene(std::shared_ptr<WorkStatus> topWork, int32_t runningCount);
    std::string GetConditionString(const std::shared_ptr<WorkStatus> workStatus);
    bool HasSystemPolicyEventSend() const;
    void SetSystemPolicyEventSend(bool systemPolicyEventSend);

private:
    const std::weak_ptr<WorkSchedulerService> wss_;
    std::shared_ptr<WorkConnManager> workConnManager_;
    std::shared_ptr<WorkEventHandler> handler_;

    ffrt::recursive_mutex uidMapMutex_;
    std::map<int32_t, std::shared_ptr<WorkQueue>> uidQueueMap_;

    std::shared_ptr<WorkQueue> conditionReadyQueue_;

    std::list<std::shared_ptr<IPolicyFilter>> policyFilters_;
    std::shared_ptr<AppDataClearListener> appDataClearListener_;

    std::shared_ptr<Watchdog> watchdog_;

    ffrt::mutex watchdogIdMapMutex_;
    std::map<uint32_t, std::shared_ptr<WorkStatus>> watchdogIdMap_;

    uint32_t watchdogId_;
    int32_t dumpSetMemory_;
    int32_t watchdogTime_;
    int32_t dumpSetCpu_;
    int32_t dumpSetMaxRunningCount_;

    ffrt::recursive_mutex ideDebugListMutex_;
    std::list<std::shared_ptr<WorkStatus>> ideDebugList;
    std::atomic<bool> systemPolicyEventSend_ {false};
};
} // namespace WorkScheduler
} // namespace OHOS

#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_SERVICES_POLICY_MANAGER_H
