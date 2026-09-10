/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_GUARD_THREAD_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_GUARD_THREAD_H

#include <atomic>
#include <memory>
#include <ffrt.h>
#include "ability_manager_interface.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerService;
class WorkPolicyManager;
class WorkStatus;

class WorkGuardThread : public std::enable_shared_from_this<WorkGuardThread> {
public:
    explicit WorkGuardThread(const std::shared_ptr<WorkSchedulerService>& service);
    ~WorkGuardThread();

    /**
     * @brief Start the guard check by submitting the first delayed task.
     */
    void Start();

    /**
     * @brief Stop the guard check. Pending tasks will exit without rescheduling.
     */
    void Stop();

private:
    void ScheduleNextCheck();
    void DoGuardCheck(uint64_t gen);
    void StopGuardCheck();
    void CheckRunningExtensions(const std::vector<std::shared_ptr<WorkStatus>>& runningWorks,
        const std::vector<AppExecFwk::ExtensionRunningInfo>& extensionInfos);
    void CheckRunningWorkStatus(const std::shared_ptr<WorkPolicyManager>& policyManager,
        const std::vector<std::shared_ptr<WorkStatus>>& runningWorks,
        const std::vector<AppExecFwk::ExtensionRunningInfo>& extensionInfos);
    bool IsWorkInExtensionInfos(const std::shared_ptr<WorkStatus> workStatus,
        const std::vector<AppExecFwk::ExtensionRunningInfo>& extensionInfos);
    bool IsExtensionInRunningWorks(const AppExecFwk::ExtensionRunningInfo& extInfo,
        const std::vector<std::shared_ptr<WorkStatus>>& runningWorks);
    bool GetRunningExtensionInfos(std::vector<AppExecFwk::ExtensionRunningInfo>& extensionInfos);
    bool StopRunningExtension(const std::string& bundleName, const std::string& abilityName, int32_t uid);
    void InitAbilityManager();
    bool CheckAbilityManagerValid();

    std::weak_ptr<WorkSchedulerService> service_;
    std::atomic<bool> running_ {false};
    std::atomic<uint64_t> generation_ {0};
    sptr<AAFwk::IAbilityManager> abilityMgr_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_GUARD_THREAD_H
