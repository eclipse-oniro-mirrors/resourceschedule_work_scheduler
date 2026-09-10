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

#include "work_guard_thread.h"

#include <set>
#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "work_sched_hilog.h"
#include "work_sched_constants.h"
#include "work_sched_utils.h"
#include "work_scheduler_service.h"
#include "work_policy_manager.h"
#include "work_status.h"

namespace OHOS {
namespace WorkScheduler {

WorkGuardThread::WorkGuardThread(const std::shared_ptr<WorkSchedulerService>& service) : service_(service) {}

WorkGuardThread::~WorkGuardThread() = default;

void WorkGuardThread::StopGuardCheck()
{
    generation_.fetch_add(1);
    running_.store(false);
}

void WorkGuardThread::Start()
{
    if (running_.load()) {
        WS_HILOGI("Guard thread is already running.");
        return;
    }
    running_.store(true);
    generation_.fetch_add(1);
    ScheduleNextCheck();
    WS_HILOGI("Guard thread started.");
}

void WorkGuardThread::Stop()
{
    if (!running_.load()) {
        return;
    }
    StopGuardCheck();
    WS_HILOGI("Guard thread stopped.");
}

void WorkGuardThread::ScheduleNextCheck()
{
    uint64_t gen = generation_.load();
    auto self = shared_from_this();
    ffrt::submit(
        [self, gen]() {
            self->DoGuardCheck(gen);
        },
        ffrt::task_attr().delay(GUARD_THREAD_INTERVAL_US));
}

void WorkGuardThread::DoGuardCheck(uint64_t gen)
{
    if (!running_.load() || gen != generation_.load()) {
        WS_HILOGI("Guard check skipped, running:%{public}d, gen mismatch:%{public}d",
            running_.load(), gen != generation_.load());
        return;
    }
    auto service = service_.lock();
    if (service == nullptr) {
        WS_HILOGE("Service destroyed, stop guard check.");
        StopGuardCheck();
        return;
    }
    if (!service->IsReady()) {
        WS_HILOGD("Service not ready, skip guard check.");
        return;
    }
    auto policyManager = service->GetWorkPolicyManager();
    if (policyManager == nullptr) {
        WS_HILOGE("PolicyManager is null, skip guard check.");
        return;
    }
    std::vector<std::shared_ptr<WorkStatus>> runningWorks = policyManager->GetAllRunningWorkStatus();
    std::vector<AppExecFwk::ExtensionRunningInfo> extensionInfos;
    if (!GetRunningExtensionInfos(extensionInfos)) {
        WS_HILOGE("Failed to get running extension infos, skip guard check.");
    } else {
        WS_HILOGI("Guard thread begin periodic check.");
        CheckRunningExtensions(runningWorks, extensionInfos);
        CheckRunningWorkStatus(policyManager, runningWorks, extensionInfos);
        WS_HILOGI("Guard thread periodic check done.");
    }
    if (!running_.load() || gen != generation_.load()) {
        WS_HILOGE("Guard check not rescheduled, running:%{public}d, gen mismatch:%{public}d",
            running_.load(), gen != generation_.load());
        return;
    }
    ScheduleNextCheck();
}

void WorkGuardThread::CheckRunningWorkStatus(const std::shared_ptr<WorkPolicyManager>& policyManager,
    const std::vector<std::shared_ptr<WorkStatus>>& runningWorks,
    const std::vector<AppExecFwk::ExtensionRunningInfo>& extensionInfos)
{
    for (const auto &workStatus : runningWorks) {
        if (workStatus == nullptr) {
            continue;
        }
        if (workStatus->workInfo_ && workStatus->workInfo_->IsSA()) {
            continue;
        }
        if (IsWorkInExtensionInfos(workStatus, extensionInfos)) {
            if (!workStatus->HasTimeout()) {
                continue;
            }
            WS_HILOGI("Guard thread: work %{public}s timeout, stopping it", workStatus->workId_.c_str());
            if (StopRunningExtension(workStatus->bundleName_, workStatus->abilityName_, workStatus->uid_)) {
                policyManager->CleanOrphanWork(workStatus);
            }
        } else {
            WS_HILOGI("Orphan task found: workId=%{public}s is RUNNING but no connection, cleaning state.",
                workStatus->workId_.c_str());
            policyManager->CleanOrphanWork(workStatus);
        }
    }
}

void WorkGuardThread::CheckRunningExtensions(const std::vector<std::shared_ptr<WorkStatus>>& runningWorks,
    const std::vector<AppExecFwk::ExtensionRunningInfo>& extensionInfos)
{
    for (auto& extInfo : extensionInfos) {
        if (IsExtensionInRunningWorks(extInfo, runningWorks)) {
            continue;
        }
        bool ret = StopRunningExtension(extInfo.extension.GetBundleName(), extInfo.extension.GetAbilityName(),
            extInfo.uid);
        WS_HILOGI("Guard thread: extension bundleName=%{public}s uid=%{public}d not in running works,"
            " stopping it, ret: %{public}d", extInfo.extension.GetBundleName().c_str(), extInfo.uid, ret);
    }
}

bool WorkGuardThread::IsWorkInExtensionInfos(const std::shared_ptr<WorkStatus> workStatus,
    const std::vector<AppExecFwk::ExtensionRunningInfo>& extensionInfos)
{
    if (workStatus == nullptr) {
        return false;
    }
    for (auto& extInfo : extensionInfos) {
        if (extInfo.extension.GetBundleName() == workStatus->bundleName_ &&
            extInfo.extension.GetAbilityName() == workStatus->abilityName_ &&
            extInfo.uid == workStatus->uid_) {
            return true;
        }
    }
    return false;
}

bool WorkGuardThread::IsExtensionInRunningWorks(const AppExecFwk::ExtensionRunningInfo& extInfo,
    const std::vector<std::shared_ptr<WorkStatus>>& runningWorks)
{
    for (auto& workStatus : runningWorks) {
        if (workStatus != nullptr &&
            extInfo.extension.GetBundleName() == workStatus->bundleName_ &&
            extInfo.extension.GetAbilityName() == workStatus->abilityName_ &&
            extInfo.uid == workStatus->uid_) {
            return true;
        }
    }
    return false;
}

bool WorkGuardThread::GetRunningExtensionInfos(std::vector<AppExecFwk::ExtensionRunningInfo>& extensionInfos)
{
    if (!CheckAbilityManagerValid()) {
        WS_HILOGE("Failed to get ability manager");
        return false;
    }
    std::vector<AppExecFwk::ExtensionRunningInfo> allExtensions;
    int32_t ret = abilityMgr_->GetExtensionRunningInfos(UPPER_LIMIT, allExtensions);
    if (ret != ERR_OK) {
        WS_HILOGE("GetExtensionRunningInfos failed, ret=%{public}d", ret);
        return false;
    }

    for (auto& info : allExtensions) {
        if (info.type == AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER) {
            WS_HILOGI("GetExtensionRunningInfos, uid: %{public}d, bundleName: %{public}s, abilityName: %{public}s",
                info.uid, info.extension.GetBundleName().c_str(), info.extension.GetAbilityName().c_str());
            extensionInfos.push_back(info);
        }
    }
    return true;
}

bool WorkGuardThread::StopRunningExtension(const std::string& bundleName, const std::string& abilityName, int32_t uid)
{
    if (!CheckAbilityManagerValid()) {
        return false;
    }
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);
    int32_t userId = WorkSchedUtils::GetUserIdByUid(uid);
    int32_t ret = abilityMgr_->StopExtensionAbility(want, nullptr, userId,
        AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER);
    return ret == ERR_OK;
}

void WorkGuardThread::InitAbilityManager()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        WS_HILOGE("Failed to get system ability manager");
        return;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        WS_HILOGE("Failed to get ability manager service");
        return;
    }
    abilityMgr_ = iface_cast<AAFwk::IAbilityManager>(remoteObject);
    if (abilityMgr_ == nullptr || abilityMgr_->AsObject() == nullptr) {
        WS_HILOGE("Failed to cast ability manager");
        abilityMgr_ = nullptr;
        return;
    }
}

bool WorkGuardThread::CheckAbilityManagerValid()
{
    if (abilityMgr_ == nullptr || abilityMgr_->AsObject() == nullptr) {
        InitAbilityManager();
    }
    return abilityMgr_ != nullptr;
}
} // namespace WorkScheduler
} // namespace OHOS
