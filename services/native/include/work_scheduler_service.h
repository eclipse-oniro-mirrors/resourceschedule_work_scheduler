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
#include "system_ability_status_change_stub.h"
#include "work_sched_service_stub.h"
#include "work_status.h"
#include "work_event_handler.h"
#include "delayed_sp_singleton.h"

namespace OHOS {
namespace WorkScheduler {
class WorkQueueManager;
class WorkPolicyManager;
class WorkBundleGroupChangeCallback;
class SchedulerBgTaskSubscriber;
class WorkSchedulerService final : public SystemAbility, public WorkSchedServiceStub,
    public std::enable_shared_from_this<WorkSchedulerService> {
    DISALLOW_COPY_AND_MOVE(WorkSchedulerService);
    DECLARE_SYSTEM_ABILITY(WorkSchedulerService);
    DECLARE_DELAYED_SP_SINGLETON(WorkSchedulerService);
public:
    WorkSchedulerService(const int32_t systemAbilityId, bool runOnCreate);

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
    int32_t ObtainAllWorks(int32_t &uid, int32_t &pid,
        std::list<std::shared_ptr<WorkInfo>>& workInfos) override;
    /**
     * @brief Get the status of work.
     *
     * @param uid The uid.
     * @param workId The id of work.
     * @param workInfo The info of work.
     * @return error code, ERR_OK if success.
     */
    int32_t GetWorkStatus(int32_t &uid, int32_t &workId, std::shared_ptr<WorkInfo>& workInfo) override;
    /**
     * @brief Dump.
     *
     * @param fd The file descriptor.
     * @param args The dump args.
     * @return Status code, ERR_OK if success.
     */
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;

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
     * @brief Watchdog time out.
     *
     * @param workStatus The status of work.
     */
    void WatchdogTimeOut(std::shared_ptr<WorkStatus> workStatus);
    /**
     * @brief Init.
     *
     * @return True if success,else false.
     */
    bool Init();

    /**
     * @brief Get handler.
     *
     * @return Handler.
     */
    std::shared_ptr<WorkEventHandler> GetHandler()
    {
        return handler_;
    }

    /**
     * @brief Get work queue manager.
     *
     * @return Work queue manager.
     */
    std::shared_ptr<WorkQueueManager> GetWorkQueueManager()
    {
        return workQueueManager_;
    }

    /**
     * @brief Get work policy manager.
     *
     * @return work policy manager.
     */
    std::shared_ptr<WorkPolicyManager> GetWorkPolicyManager()
    {
        return workPolicyManager_;
    }

    /**
     * @brief add uid to the whitelist or delete uid from the whitelist.
     *
     * @param uid uid of the app.
     * @param isAdd true if add name, else delete.
     */
    void UpdateEffiResApplyInfo(int32_t uid, bool isAdd);

    /**
     * @brief init background task subscriber, subscribe self to bgtask service
     *
     * @return true seccess to init subscriber
     * @return false fail to init subscriber
     */
    bool InitBgTaskSubscriber();

    /**
     * @brief check uid has work_scheduler resources or not
     *
     * @param uid the uid of application
     * @return true uid has work_scheduler resources or not
     * @return false uid does not have work_scheduler resources or not
     */
    bool CheckEffiResApplyInfo(int32_t uid);

    /**
     * @brief Get the efficiency resources uid List object
     *
     * @return std::string string consists of uid
     */
    std::string GetEffiResApplyUid();

    /**
     * @brief Get the Efficiency Resources Infos object
     *
     * @return ErrCode ERR_OK if succeed, others if failed
     */
    ErrCode QueryResAppliedUid();
private:
    std::set<int32_t> whitelist_;
    std::mutex whitelistMutex_;
#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
    std::shared_ptr<SchedulerBgTaskSubscriber> subscriber_;
#endif
private:
    class SystemAbilityStatusChangeListener : public OHOS::SystemAbilityStatusChangeStub {
    public:
        SystemAbilityStatusChangeListener() {};
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    };

private:
    std::shared_ptr<WorkQueueManager> workQueueManager_;
    std::shared_ptr<WorkPolicyManager> workPolicyManager_;
    std::mutex mutex_;
    std::map<std::string, std::shared_ptr<WorkInfo>> persistedMap_;
    bool ready_ {false};
    std::shared_ptr<WorkEventHandler> handler_;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_;
    bool checkBundle_ {true};
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    sptr<WorkBundleGroupChangeCallback> groupObserver_;
#endif

    void WorkQueueManagerInit();
    bool WorkPolicyManagerInit();
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    void GroupObserverInit();
#endif
    void RefreshPersistedWorks();
    std::list<std::shared_ptr<WorkInfo>> ReadPersistedWorks();
    void InitPersistedWork(WorkInfo& workInfo);
    void DumpAllInfo(std::string &result);
    bool CheckWorkInfo(WorkInfo &workInfo, int32_t &uid);
    bool StopWorkInner(std::shared_ptr<WorkStatus> workStatus, int32_t uid, const bool needCancel, bool isTimeOut);
    bool CheckCondition(WorkInfo& workInfo);
    bool IsBaseAbilityReady();
    void DumpUsage(std::string &result);
    void DumpParamSet(std::string &key, std::string &value, std::string &result);
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHEDULER_SERVICES_H