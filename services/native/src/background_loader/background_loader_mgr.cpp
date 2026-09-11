/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include "background_loader/ability_connect_callback.h"
#include "ability_manager_client.h"
#include "background_loader/background_loader_mgr.h"
#include "res_sched_client.h"
#include "res_sched_json_util.h"
#include "system_ability_definition.h"
#include "want.h"
#include "work_sched_errors.h"
#include "work_sched_hilog.h"
#include "bundle_mgr_proxy.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "work_sched_utils.h"

extern "C" void ReportDataInProcess(uint32_t resType, int64_t value, const nlohmann::json& payload);
using namespace OHOS::ResourceSchedule;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr std::string_view TIMEOUT_MESSAGE = "timeOut";
constexpr std::string_view TIMEOUT_TASK_NAME = "BackgroundLoaderTimeout";
constexpr std::string_view ON_START = "onStart";
constexpr std::string_view ON_STOP = "onStop";
constexpr std::string_view BACKGROUND_LOADER_CONFIG_KEY = "background_loader_config";
constexpr std::string_view BACKGROUND_LOADER_TIMEOUT_COUNT_KEY = "maxTimeoutCount";
constexpr std::string_view BACKGROUND_LOADER_TIMEOUTMS_KEY = "backgroundLoaderTimeoutMs";
constexpr int32_t DEFAULT_INVAL_VALUE = -1;
}
IMPLEMENT_SINGLE_INSTANCE(BackgroundLoaderMgr)

void BackgroundLoaderMgr::Init(int32_t maxTimeoutCount, int32_t backgroundLoaderTimeoutMs)
{
    WS_HILOGI("[%{public}s:%{public}d] BackgroundLoaderMgr init", __FUNCTION__, __LINE__);
    maxTimeoutCount_ = maxTimeoutCount;
    backgroundLoaderTimeoutMs_ = backgroundLoaderTimeoutMs;
    isReady_.store(true);
}

std::string BackgroundLoaderMgr::GenerateTaskKey(const std::string& bundleName, int32_t appIndex)
{
    return bundleName + "_" + std::to_string(appIndex);
}

ErrCode BackgroundLoaderMgr::RegisterTask(const TaskInfo& taskInfo)
{
    WS_HILOGI("[%{public}s:%{public}d] taskId: %{public}d, bundleName: %{public}s, abilityName: %{public}s, "
        "appIndex: %{public}d",
        __FUNCTION__, __LINE__, taskInfo.taskId_, taskInfo.bundleName_.c_str(),
        taskInfo.abilityName_.c_str(), taskInfo.appIndex_);
    if (!isReady_.load()) {
        WS_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }
    std::string key = GenerateTaskKey(taskInfo.bundleName_, taskInfo.appIndex_);

    {
        std::lock_guard<ffrt::mutex> lock(blackListLock_);
        if (blackLists_.find(key) != blackLists_.end()) {
            WS_HILOGE("bundleName: %{public}s is in black list", taskInfo.bundleName_.c_str());
            return E_CHECK_WORKINFO_FAILED;
        }
    }

    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(key);
    if (it == taskMap_.end() || it->second.status_ == TaskStatus::UNREGISIERED) {
        nlohmann::json payload;
        payload["bundleName"] = taskInfo.bundleName_;
        payload["appIndex"] = std::to_string(taskInfo.appIndex_);
        ReportDataInProcess(ResType::RES_TYPE_BACKGROUND_LOADER_CHANGE_EVENT,
            ResType::BackgroundLoaderState::ADD, payload);
        if (it == taskMap_.end()) {
            it = taskMap_.emplace(key, taskInfo).first;
        } else {
            it->second = taskInfo;
        }
        it->second.status_ = TaskStatus::NOT_STARTED;
    } else if (it->second.taskId_ != taskInfo.taskId_) {
        // 任务存在且taskId不同的情况下仅刷新taskId
        it->second.taskId_ = taskInfo.taskId_;
    }
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::UnregisterTask(const TaskInfo& taskInfo)
{
    WS_HILOGI("[%{public}s:%{public}d] taskId: %{public}d, bundleName: %{public}s, abilityName: %{public}s, "
        "appIndex: %{public}d",
        __FUNCTION__, __LINE__, taskInfo.taskId_, taskInfo.bundleName_.c_str(),
        taskInfo.abilityName_.c_str(), taskInfo.appIndex_);
    if (!isReady_.load()) {
        WS_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }

    std::lock_guard<ffrt::mutex> lock(taskLock_);
    std::string key = GenerateTaskKey(taskInfo.bundleName_, taskInfo.appIndex_);
    auto it = taskMap_.find(key);
    if (it == taskMap_.end() || it->second.taskId_ != taskInfo.taskId_ ||
        it->second.status_ == TaskStatus::UNREGISIERED) {
        WS_HILOGE("UnregisterTask failed : task not found");
        return E_WORK_NOT_EXIST_FAILED;
    }

    nlohmann::json payload;
    payload["bundleName"] = taskInfo.bundleName_;
    payload["appIndex"] = std::to_string(taskInfo.appIndex_);
    ReportDataInProcess(ResType::RES_TYPE_BACKGROUND_LOADER_CHANGE_EVENT,
        ResType::BackgroundLoaderState::DELETE, payload);
    it->second.status_ = TaskStatus::UNREGISIERED;
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::FinishTask(const TaskInfo& taskInfo)
{
    WS_HILOGI("[%{public}s:%{public}d] taskId: %{public}d, bundleName: %{public}s, abilityName: %{public}s, "
        "appIndex: %{public}d",
        __FUNCTION__, __LINE__, taskInfo.taskId_, taskInfo.bundleName_.c_str(),
        taskInfo.abilityName_.c_str(), taskInfo.appIndex_);
    if (!isReady_.load()) {
        WS_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }

    std::lock_guard<ffrt::mutex> lock(taskLock_);
    std::string key = GenerateTaskKey(taskInfo.bundleName_, taskInfo.appIndex_);
    auto it = taskMap_.find(key);
    if (it == taskMap_.end() || it->second.taskId_ != taskInfo.taskId_ ||
        it->second.status_ != TaskStatus::RUNNING) {
        WS_HILOGE("FinishTask failed : task not found or status not correct");
        return E_WORK_NOT_EXIST_FAILED;
    }
    nlohmann::json payload;
    payload["bundleName"] = taskInfo.bundleName_;
    payload["appIndex"] = std::to_string(taskInfo.appIndex_);
    ReportDataInProcess(ResType::RES_TYPE_BACKGROUND_LOADER_TASK_FINISH, 0, payload);
    it->second.status_ = TaskStatus::FINISHED;
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::GetTaskInfo(int32_t taskId, const std::string& bundleName, int32_t appIndex,
    BackgroundLoaderTaskInfo& taskInfo)
{
    WS_HILOGI("[%{public}s:%{public}d] taskId: %{public}d, bundleName: %{public}s, appIndex: %{public}d",
        __FUNCTION__, __LINE__, taskId, bundleName.c_str(), appIndex);
    if (!isReady_.load()) {
        WS_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }

    std::lock_guard<ffrt::mutex> lock(taskLock_);
    std::string key = GenerateTaskKey(bundleName, appIndex);
    auto it = taskMap_.find(key);
    if (it != taskMap_.end() && it->second.taskId_ == taskId && it->second.status_ != TaskStatus::UNREGISIERED) {
        TaskInfo& info = it->second;
        BackgroundLoaderTaskInfo newInfo(info.taskId_, info.abilityName_);
        taskInfo = newInfo;
        return ERR_OK;
    }

    WS_HILOGE("GetTaskInfo failed : task not found");
    return E_WORK_NOT_EXIST_FAILED;
}

void BackgroundLoaderMgr::CheckAndSendOnStop(const std::string& bundleName,
    const std::string& abilityName, int32_t appIndex, int32_t taskId)
{
    TaskInfo taskInfoCopy;
    bool shouldAddToBlackList = false;
    {
        std::lock_guard<ffrt::mutex> lock(taskLock_);
        std::string key = GenerateTaskKey(bundleName, appIndex);
        auto it = taskMap_.find(key);
        if (it == taskMap_.end()) {
            WS_HILOGE("task not found for bundle %{public}s", bundleName.c_str());
            return;
        }

        TaskInfo* taskInfo = &it->second;
        if (taskInfo->status_ == TaskStatus::RUNNING) {
            WS_HILOGI("[%{public}s:%{public}d] task still running, send onstop for bundle %{public}s",
                __FUNCTION__, __LINE__, bundleName.c_str());
            taskInfoCopy = *taskInfo;
            taskInfo->timeoutCount_++;
            if (taskInfo->timeoutCount_ >= maxTimeoutCount_) {
                taskInfo->status_ = TaskStatus::UNREGISIERED;
                shouldAddToBlackList = true;
            } else {
                taskInfo->status_ = TaskStatus::FINISHED;
            }
        } else {
            WS_HILOGI("[%{public}s:%{public}d] task already finished for bundle %{public}s",
                __FUNCTION__, __LINE__, bundleName.c_str());
            return;
        }
    }

    SendOnStop(taskInfoCopy, static_cast<int32_t>(StopCode::TIMEOUT_ERROR), std::string(TIMEOUT_MESSAGE));
    nlohmann::json payload;
    payload["bundleName"] = taskInfoCopy.bundleName_;
    payload["appIndex"] = std::to_string(taskInfoCopy.appIndex_);
    ReportDataInProcess(ResType::RES_TYPE_BACKGROUND_LOADER_CHANGE_EVENT,
        ResType::BackgroundLoaderState::DELETE, payload);
    ReportDataInProcess(ResType::RES_TYPE_BACKGROUND_LOADER_TASK_FINISH, 0, payload);

    if (shouldAddToBlackList) {
        auto key = GenerateTaskKey(taskInfoCopy.bundleName_, taskInfoCopy.appIndex_);
        std::lock_guard<ffrt::mutex> lock(blackListLock_);
        blackLists_.insert(key);
    }
}

void BackgroundLoaderMgr::PostTimeoutTask(const std::string& bundleName,
    const std::string& abilityName, int32_t appIndex, int32_t taskId)
{
    WS_HILOGI("[%{public}s:%{public}d] bundleName: %{public}s, abilityName: %{public}s, appIndex: %{public}d",
        __FUNCTION__, __LINE__, bundleName.c_str(), abilityName.c_str(), appIndex);
    ffrt::submit(
        [bundleName, abilityName, appIndex, taskId] () {
            BackgroundLoaderMgr::GetInstance().CheckAndSendOnStop(bundleName, abilityName, appIndex, taskId);
        },
        ffrt::task_attr().delay(backgroundLoaderTimeoutMs_));
}

void BackgroundLoaderMgr::HandleBackgroundLoaderTask(const nlohmann::json& payload)
{
    int32_t appIndex = 0;
    int32_t  taskId = 0;
    std::string abilityName = "";
    std::string bundleName = "";
    if (!ResCommonUtil::ParseStringParameterFromJson("bundleName", bundleName, payload) ||
        !ResCommonUtil::ParseStringParameterFromJson("abilityName", abilityName, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("appIndex", appIndex, payload)) {
        WS_HILOGE("get background loader info fail");
        return;
    }
    WS_HILOGI("[%{public}s:%{public}d] bundleName: %{public}s, abilityName: %{public}s, "
        "appIndex: %{public}d, taskId: %{public}d",
        __FUNCTION__, __LINE__, bundleName.c_str(), abilityName.c_str(), appIndex, taskId);
    std::string key = GenerateTaskKey(bundleName, appIndex);
    {
        std::lock_guard<ffrt::mutex> lock(blackListLock_);
        if (blackLists_.find(key) != blackLists_.end()) {
            WS_HILOGE("bundleName: %{public}s is in black list", bundleName.c_str());
            return;
        }
    }
    OHOS::sptr<IRemoteObject> remoteObject = GetRemoteObject(bundleName, abilityName, appIndex);
    if (remoteObject != nullptr) {
        SendOnStart(remoteObject, bundleName, appIndex);
        WS_HILOGI("[%{public}s:%{public}d] send request to existing remoteObject success",
            __FUNCTION__, __LINE__);
        return;
    }

    OHOS::AAFwk::Want providerWant;
    providerWant.SetElementName(bundleName, abilityName);
    sptr<AAFwk::IAbilityConnection> connect = new(std::nothrow) AbilityConnectCallback(
        bundleName, abilityName, appIndex);
    if (connect == nullptr) {
        WS_HILOGE("failed to new connection");
        return;
    }
    int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByCall(providerWant, connect,
        nullptr, DEFAULT_INVAL_VALUE, false, true);
    if (ret != ERR_OK) {
        WS_HILOGE("StartAbilityByCall falied ret : %{public}d", ret);
        return;
    }
    WS_HILOGI("[%{public}s:%{public}d] StartAbilityByCall success for %{public}s",
        __FUNCTION__, __LINE__, bundleName.c_str());
}

sptr<IRemoteObject> BackgroundLoaderMgr::GetRemoteObject(const std::string& bundleName,
    const std::string& abilityName, int32_t appIndex)
{
    std::string key = GenerateTaskKey(bundleName, appIndex);
    std::lock_guard<ffrt::mutex> lock(abilityMapLock_);
    auto it = abilityMap_.find(key);
    if (it != abilityMap_.end()) {
        return it->second;
    }
    return nullptr;
}

void BackgroundLoaderMgr::SendOnStart(const sptr<IRemoteObject>& remoteObject,
    const std::string& bundleName, int32_t appIndex)
{
    std::string key = GenerateTaskKey(bundleName, appIndex);
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(key);
    if (it == taskMap_.end()) {
        WS_HILOGE("find %{public}s task info failed", bundleName.c_str());
        return;
    }
    auto& info = it->second;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);

    if (!data.WriteString(std::string(ON_START))) {
        WS_HILOGE("SendOnStart: write function name failed");
        return;
    }
    int32_t ret = remoteObject->SendRequest(AAFwk::IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendOnStart: send request failed");
        return;
    }
    info.status_ = TaskStatus::RUNNING;
    PostTimeoutTask(info.bundleName_, info.abilityName_, info.appIndex_, info.taskId_);
}

void BackgroundLoaderMgr::RemoveRemoteObject(const std::string& bundleName, int32_t appIndex)
{
    std::string key = GenerateTaskKey(bundleName, appIndex);
    {
        std::lock_guard<ffrt::mutex> lock(abilityMapLock_);
        auto it = abilityMap_.find(key);
        if (it != abilityMap_.end()) {
            abilityMap_.erase(it);
            WS_HILOGI("[%{public}s:%{public}d] Removed object for %{public}s, appIndex: %{public}d",
                __FUNCTION__, __LINE__, bundleName.c_str(), appIndex);
        } else {
            WS_HILOGI("[%{public}s:%{public}d] %{public}s, appIndex: %{public}d not found",
                __FUNCTION__, __LINE__, bundleName.c_str(), appIndex);
        }
    }
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(key);
    if (it != taskMap_.end()) {
        it->second.pid_ = -1;
    }
}

void BackgroundLoaderMgr::SendOnStop(const TaskInfo& taskInfo, int32_t stopCode, const std::string& message)
{
    sptr<IRemoteObject> remoteObject = GetRemoteObject(taskInfo.bundleName_,
        taskInfo.abilityName_, taskInfo.appIndex_);
    if (remoteObject == nullptr) {
        WS_HILOGE("remoteObject for %{public}s, appIndex: %{public}d not found",
            taskInfo.bundleName_.c_str(), taskInfo.appIndex_);
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);

    if (!data.WriteString(std::string(ON_STOP))) {
        WS_HILOGE("SendOnStop: write function name failed");
        return;
    }
    if (!data.WriteInt32(taskInfo.taskId_)) {
        WS_HILOGE("SendOnStop: write task id failed");
        return;
    }
    if (!data.WriteString(taskInfo.abilityName_)) {
        WS_HILOGE("SendOnStop: write ability name failed");
        return;
    }
    if (!data.WriteInt32(stopCode)) {
        WS_HILOGE("SendOnStop: write stopCode failed");
        return;
    }
    if (!data.WriteString(message)) {
        WS_HILOGE("SendOnStop: write message failed");
        return;
    }
    int32_t ret = remoteObject->SendRequest(AAFwk::IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendOnStop: send request for %{public}s, appIndex: %{public}d failed",
            taskInfo.bundleName_.c_str(), taskInfo.appIndex_);
        return;
    }
    WS_HILOGI("SendOnStop: send request for %{public}s, appIndex: %{public}d success",
        taskInfo.bundleName_.c_str(), taskInfo.appIndex_);
}

void BackgroundLoaderMgr::SaveRemoteObject(const std::string& bundleName,
    const std::string& abilityName, int32_t appIndex, const sptr<IRemoteObject>& remoteObject)
{
    std::string key = GenerateTaskKey(bundleName, appIndex);
    std::lock_guard<ffrt::mutex> lock(abilityMapLock_);
    abilityMap_[key] = remoteObject;
    WS_HILOGI("[%{public}s:%{public}d] save remote object for %{public}s, appIndex: %{public}d success",
        __FUNCTION__, __LINE__, bundleName.c_str(), appIndex);
}

void BackgroundLoaderMgr::HandleAppUninstallEvent(int64_t value, const nlohmann::json& payload)
{
    if (value != ResType::AppInstallStatus::APP_UNINSTALL) {
        return;
    }
    std::string bundleName = "";
    int32_t appIndex = 0;
    if (!ResCommonUtil::ParseStringParameterFromJson("bundleName", bundleName, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("appIndex", appIndex, payload)) {
        WS_HILOGE("get app info fail");
        return;
    }
    WS_HILOGI("[%{public}s:%{public}d] %{public}s, appIndex: %{public}d uninstall clear task info",
        __FUNCTION__, __LINE__, bundleName.c_str(), appIndex);
    RemoveRemoteObject(bundleName, appIndex);
    auto key = GenerateTaskKey(bundleName, appIndex);
    {
        std::lock_guard<ffrt::mutex> lock(blackListLock_);
        blackLists_.erase(key);
    }
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(key);
    if (it != taskMap_.end()) {
        taskMap_.erase(it);
    }
}

bool BackgroundLoaderMgr::GetAppIndexAndBundleNameByUid(int32_t uid, int32_t& appIndex, std::string& bundleName)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        WS_HILOGE("fail to get system ability mgr.");
        return false;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        WS_HILOGE("fail to get bundle manager proxy.");
        return false;
    }
    sptr<IBundleMgr> bundleMgr = iface_cast<IBundleMgr>(remoteObject);
    ErrCode ret = bundleMgr->GetNameAndIndexForUid(uid, bundleName, appIndex);
    if (ret == ERR_OK) {
        WS_HILOGD("appIndex = %{public}d", appIndex);
        return true;
    }
    WS_HILOGE("fail to get app index.");
    return false;
}

int32_t BackgroundLoaderMgr::CheckPermissionAndTaskInfo(std::string& bundleName, int32_t& appIndex, int32_t uid)
{
    if (!isReady_.load()) {
        WS_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }
    if (!GetAppIndexAndBundleNameByUid(uid, appIndex, bundleName)) {
        WS_HILOGE("Failed to get bundle for uid %{public}d", uid);
        return E_CHECK_WORKINFO_FAILED;
    }
    return ERR_OK;
}

bool BackgroundLoaderMgr::VerifyAbilityName(const std::string& bundleName,
    const std::string& abilityName, int32_t uid)
{
    if (bundleName.empty() || abilityName.empty()) {
        WS_HILOGE("bundleName or abilityName invaild");
        return false;
    }

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        WS_HILOGE("fail to get system ability mgr.");
        return false;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        WS_HILOGE("fail to get bundle manager remoteObject.");
        return false;
    }
    sptr<IBundleMgr> bundleMgr = iface_cast<IBundleMgr>(remoteObject);
    if (!bundleMgr) {
        WS_HILOGE("fail to get bundle manager proxy.");
        return false;
    }

    std::vector<AbilityInfo> abilityInfos;
    OHOS::AAFwk::Want want;
    want.SetAction(OHOS::AAFwk::Want::ACTION_HOME);
    want.AddEntity(OHOS::AAFwk::Want::ENTITY_HOME);
    ElementName elementName;
    elementName.SetBundleName(bundleName);
    want.SetElement(elementName);
    int32_t userId = WorkSchedUtils::GetUserIdByUid(uid);
    if (!bundleMgr->QueryAbilityInfos(want, 0, userId, abilityInfos)) {
        WS_HILOGE("QueryAbilityInfos failed for bundle: %{public}s", bundleName.c_str());
        return false;
    }

    for (const auto& ability : abilityInfos) {
        if (ability.bundleName == bundleName && ability.name == abilityName && ability.enabled) {
            WS_HILOGI("[%{public}s:%{public}d] found ability %{public}s in bundle: %{public}s",
                __FUNCTION__, __LINE__, abilityName.c_str(), bundleName.c_str());
            return true;
        }
    }
    WS_HILOGE("bundle: %{public}s not found or not enable", bundleName.c_str());
    return false;
}

ErrCode BackgroundLoaderMgr::RegisterTaskWithCheck(const BackgroundLoaderTaskInfo& taskInfo)
{
    std::string bundleName = "";
    int32_t appIndex = -1;
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
    auto ret = CheckPermissionAndTaskInfo(bundleName, appIndex, uid);
    if (ret != ERR_OK) {
        return ret;
    }
    if (!VerifyAbilityName(bundleName, taskInfo.GetAbilityName(), uid)) {
        return E_CHECK_WORKINFO_FAILED;
    }
    TaskInfo info = {
        .taskId_ = taskInfo.GetTaskId(),
        .bundleName_ = bundleName,
        .appIndex_ = appIndex,
        .abilityName_ = taskInfo.GetAbilityName(),
        .pid_ = pid
    };
    return RegisterTask(info);
}

ErrCode BackgroundLoaderMgr::UnregisterTaskWithCheck(const BackgroundLoaderTaskInfo& taskInfo)
{
    std::string bundleName = "";
    int32_t appIndex = -1;
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
    auto ret = CheckPermissionAndTaskInfo(bundleName, appIndex, uid);
    if (ret != ERR_OK) {
        return ret;
    }
    if (!VerifyAbilityName(bundleName, taskInfo.GetAbilityName(), uid)) {
        return E_CHECK_WORKINFO_FAILED;
    }
    TaskInfo info = {
        .taskId_ = taskInfo.GetTaskId(),
        .bundleName_ = bundleName,
        .appIndex_ = appIndex,
        .abilityName_ = taskInfo.GetAbilityName(),
        .pid_ = pid
    };
    return UnregisterTask(info);
}

ErrCode BackgroundLoaderMgr::FinishTaskWithCheck(const BackgroundLoaderTaskInfo& taskInfo)
{
    std::string bundleName = "";
    int32_t appIndex = -1;
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
    auto ret = CheckPermissionAndTaskInfo(bundleName, appIndex, uid);
    if (ret != ERR_OK) {
        return ret;
    }
    if (!VerifyAbilityName(bundleName, taskInfo.GetAbilityName(), uid)) {
        return E_CHECK_WORKINFO_FAILED;
    }
    TaskInfo info = {
        .taskId_ = taskInfo.GetTaskId(),
        .bundleName_ = bundleName,
        .appIndex_ = appIndex,
        .abilityName_ = taskInfo.GetAbilityName(),
        .pid_ = pid
    };
    return FinishTask(info);
}

ErrCode BackgroundLoaderMgr::GetTaskInfoWithCheck(int32_t taskId, BackgroundLoaderTaskInfo& taskInfo)
{
    std::string bundleName = "";
    int32_t appIndex = -1;
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto ret = CheckPermissionAndTaskInfo(bundleName, appIndex, uid);
    if (ret != ERR_OK) {
        return ret;
    }
    return GetTaskInfo(taskId, bundleName, appIndex, taskInfo);
}

}  // namespace WorkScheduler
}  // namespace OHOS