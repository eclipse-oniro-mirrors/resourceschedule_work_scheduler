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

WorkGuardThread::~WorkGuardThread()
{
    Stop();
}

void WorkGuardThread::Start()
{
    if (running_.load()) {
        WS_HILOGI("Guard thread is already running.");
        return;
    }
    running_.store(true);
    thread_ = std::make_unique<ffrt::thread>(&WorkGuardThread::Run, this);
    WS_HILOGI("Guard thread started.");
}

void WorkGuardThread::Stop()
{
    if (!running_.load()) {
        return;
    }
    running_.store(false);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cv_.notify_all();
    }
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
    thread_.reset();
    WS_HILOGI("Guard thread stopped.");
}

void WorkGuardThread::Run()
{
    WS_HILOGI("Guard thread run loop started, interval:%{public}dms", GUARD_THREAD_INTERVAL_MS);
    while (running_.load()) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, std::chrono::milliseconds(GUARD_THREAD_INTERVAL_MS),
            [this]() { return !running_.load(); });
        if (!running_.load()) {
            break;
        }
        lock.unlock();

        auto service = service_.lock();
        if (service == nullptr || !service->IsReady()) {
            WS_HILOGD("Service not ready, skip guard check.");
            continue;
        }
        auto policyManager = service->GetWorkPolicyManager();
        if (policyManager == nullptr) {
            WS_HILOGE("PolicyManager is null, skip guard check.");
            continue;
        }
        WS_HILOGI("Guard thread begin periodic check.");
        CheckRunningExtensions();
        CheckRunningWorkStatus();
        WS_HILOGI("Guard thread periodic check done.");
    }
    WS_HILOGI("Guard thread run loop exited.");
}

void WorkGuardThread::CheckRunningWorkStatus()
{
    auto service = service_.lock();
    if (service == nullptr) {
        return;
    }
    auto policyManager = service->GetWorkPolicyManager();
    if (policyManager == nullptr) {
        return;
    }
    std::vector<std::shared_ptr<WorkStatus>> runningWorks = policyManager->GetAllRunningWorkStatus();
    if (runningWorks.empty()) {
        return;
    }
    std::vector<AppExecFwk::ExtensionRunningInfo> extensionInfos;
    if (!GetRunningExtensionInfos(extensionInfos)) {
        WS_HILOGE("Failed to get running extension infos");
        return;
    }

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

void WorkGuardThread::CheckRunningExtensions()
{
    auto service = service_.lock();
    if (service == nullptr) {
        return;
    }
    auto policyManager = service->GetWorkPolicyManager();
    if (policyManager == nullptr) {
        WS_HILOGE("CheckRunningExtensions: policyManager is null.");
        return;
    }

    std::vector<std::shared_ptr<WorkStatus>> runningWorks = policyManager->GetAllRunningWorkStatus();
    std::vector<AppExecFwk::ExtensionRunningInfo> extensionInfos;
    if (!GetRunningExtensionInfos(extensionInfos)) {
        WS_HILOGE("Failed to get running extension infos");
        return;
    }

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
    auto abilityMgr = GetAbilityManager();
    if (abilityMgr == nullptr) {
        WS_HILOGE("Failed to get ability manager");
        return false;
    }
    std::vector<AppExecFwk::ExtensionRunningInfo> allExtensions;
    int32_t ret = abilityMgr->GetExtensionRunningInfos(UPPER_LIMIT, allExtensions);
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
    auto abilityMgr = GetAbilityManager();
    if (abilityMgr == nullptr) {
        return false;
    }
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);
    int32_t userId = WorkSchedUtils::GetUserIdByUid(uid);
    int32_t ret = abilityMgr->StopExtensionAbility(want, nullptr, userId,
        AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER);
    return ret == ERR_OK;
}

sptr<AAFwk::IAbilityManager> WorkGuardThread::GetAbilityManager()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        WS_HILOGE("Failed to get system ability manager");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        WS_HILOGE("Failed to get ability manager service");
        return nullptr;
    }
    sptr<AAFwk::IAbilityManager> abilityMgr = iface_cast<AAFwk::IAbilityManager>(remoteObject);
    if (abilityMgr == nullptr || abilityMgr->AsObject() == nullptr) {
        WS_HILOGE("Failed to cast ability manager");
        return nullptr;
    }
    return abilityMgr;
}
} // namespace WorkScheduler
} // namespace OHOS
