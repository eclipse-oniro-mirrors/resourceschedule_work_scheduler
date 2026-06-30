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

#include "ability_connect_callback.h"
#include "ability_manager_client.h"
#include "background_loader_mgr.h"
#include "res_sched_client.h"
#include "res_sched_json_util.h"
#include "system_ability_definition.h"
#include "want.h"
#include "work_sched_errors.h"
#include "work_sched_hilog.h"


extern "C" void ReportDataInProcess(uint32_t resType, int64_t value, const nlohmann::json& payload);
using namespace OHOS::ResourceSchedule;
namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr std::string_view TIMEOUT_MESSAGE = "timeOut";
constexpr std::string_view TIMEOUT_TASK_NAME = "BackgroundLoaderTimeout";
constexpr std::string_view ON_START = "onStart";
constexpr std::string_view ON_STOP = "onStop";
}
IMPLEMENT_SINGLE_INSTANCE(BackgroundLoaderMgr)

void BackgroundLoaderMgr::Init(int32_t maxTimeoutCount, int32_t backgroundLoaderTimeoutMs)
{
    WS_HILOGI("BackgroundLoaderMgr init");
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
    WS_HILOGI("taskId: %{public}d, bundleName: %{public}s, abilityName: %{public}s, appIndex: %{public}d",
        taskInfo.taskId_, taskInfo.bundleName_.c_str(), taskInfo.abilityName_.c_str(), taskInfo.appIndex_);
    if (!isReady_.load()) {
        WS_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }
    std::string key = GenerateTaskKey(taskInfo.bundleName_, taskInfo.appIndex_);

    {
        std::lock_guard<ffrt::mutex> lock(blackListLock_);
        if (blackLists_.find(key) != blackLists_end()) {
            WS_HILOGE("bundleName: %{public}s is in black list", taskInfo.bundleName_.c_str());
            return E_CHECK_WORKINFO_FAILED;
        }
    }

    std::lock_guard<ffrt::mutex> lock(taskLock_);
    if (taskMap_.find(key) == taskMap_.end()) {
        nlohmann::json payload;
        payload["bundleName"] = taskInfo.bundleName_;
        payload["appIndex"] = std::to_string(taskInfo.appIndex_);
        ReportDataInProcess(ResType::RES_TYPE_BACKGROUND_LOADER_CHANGE_EVENT,
            ResType::BackgroundLoaderState::ADD, payload);
    }
    taskMap_[key] = taskInfo;
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::UnregisterTask(const TaskInfo& taskInfo)
{
    WS_HILOGI("taskId: %{public}d, bundleName: %{public}s, abilityName: %{public}s, appIndex: %{public}d",
        taskInfo.taskId_, taskInfo.bundleName_.c_str(), taskInfo.abilityName_.c_str(), taskInfo.appIndex_);
    if (!isReady_.load()) {
        WS_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }

    std::lock_guard<ffrt::mutex> lock(taskLock_);
    std::string key = GenerateTaskKey(taskInfo.bundleName_, taskInfo.appIndex_);
    auto it = taskMap_.find(key);
    if (it == taskMap_.end() || it->second.taskId_ != taskInfo.taskId_) {
        WS_HILOGE("UnregisterTask failed : task not found");
        return E_WORK_NOT_EXIST_FAILED;
    }

    nlohmann::json payload;
    payload["bundleName"] = taskInfo.bundleName_;
    payload["appIndex"] = std::to_string(taskInfo.appIndex_);
    ReportDataInProcess(ResType::RES_TYPE_BACKGROUND_LOADER_CHANGE_EVENT,
        ResType::BackgroundLoaderState::DELETE, payload);
    taskMap_.erase(it);
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::FinishTask(const TaskInfo& taskInfo)
{
    WS_HILOGI("taskId: %{public}d, bundleName: %{public}s, abilityName: %{public}s, appIndex: %{public}d",
        taskInfo.taskId_, taskInfo.bundleName_.c_str(), taskInfo.abilityName_.c_str(), taskInfo.appIndex_);
    if (!isReady_.load()) {
        WS_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }

    std::lock_guard<ffrt::mutex> lock(taskLock_);
    std::string key = GenerateTaskKey(taskInfo.bundleName_, taskInfo.appIndex_);
    auto it = taskMap_.find(key);
    if (it == taskMap_.end() || it->second.taskId_ != taskInfo.taskId_) {
        WS_HILOGE("FinishTask failed : task not found");
        return E_WORK_NOT_EXIST_FAILED;
    }
    it->second.status_ = TaskStatus::FINISHED;
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::GetTaskInfo(int32_t taskId, const std::string& bundleName, int32_t appIndex,
    BackgroundLoaderTaskInfo& taskInfo)
{
    WS_HILOGI("taskId: %{public}d, bundleName: %{public}s, appIndex: %{public}d",
        taskId, bundleName.c_str(), appIndex);
    if (!isReady_.load()) {
        WS_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }

    std::lock_guard<ffrt::mutex> lock(taskLock_);
    std::string key = GenerateTaskKey(bundleName, appIndex);
    auto it = taskMap_.find(key);
    if (it != taskMap_.end()) {
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
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    TaskInfo taskInfo;
    if (GetInnerTaskInfo(bundleName, appindex, taskInfo) != ERR_OK) {
        WS_HILOGE("task not found for bundle %{public}s", bundleName.c_str());
        return;
    }

    if (taskInfo.status_ == TaskStatus::RUNNING) {
        WS_HILOGI("task still running, send onstop for bundle %{public}s", bundleName.c_str());
        SendOnStop(taskInfo, static_cast<int32_t>(StopCode::TIMEOUT_ERROR), std::string(TIMEOUR_MESSAGE));
        nlohmann::json payload;
        payload["bundleName"] = taskInfo.bundleName_;
        payload["appIndex"] = std::to_string(taskInfo.appIndex_);
        ReportDataInProcess(ResType::RES_TYPE_BACKGROUND_LOADER_CHANGE_EVENT,
            ResType::BackgroundLoaderState::DELETE, payload);
        taskInfo.timeoutCount_++;
        if (taskInfo.timeoutCount_ >= maxTimeoutCount_) {
            auto key = GenerateTaskKey(taskInfo.bundleName_, taskInfo.appIndex_);
            std::lock_guard<ffrt::mutex> lock(blackListLock_);
            blackLists_.insert(key);
        }
    } else {
        WS_HILOGI("task already finished for bundle %{public}s", bundleName.c_str());
    }
}

void BackgroundLoaderMgr::PostTimeoutTask(const std::string& bundleName,
    const std::string& abilityName, int32_t appIndex, int32_t taskId)
{
    WS_HILOGI("bundleName: %{public}s, abilityName: %{public}s, appIndex: %{public}d",
        bundleName.c_str(), abilityName.c_str(), appIndex);
    ffrt::submit(
        [bundleName, abilityName, appIndex, taskId] () {
            BackgroundLoaderMgr::GetInstance()::CheckAndSendOnStop(bundleName, abilityName, appIndex, taskId);
        },
        ffrt::task_attr().delay(backgroundLoaderTimeoutMs_));
}

void BackgroundLoaderMgr::HandleBackgroundLoaderTask(const std::shared_ptr<ResourceSchedule::ResData>& resData)
{
    int32_t appIndex = 0;
    int32_t  taskId = 0;
    std::string abilityName = "";
    std::string bundleName = "";
    if (!ResCommonUtil::ParseStringParameterFromJson("bundleName", bundleName, resData->payload) ||
        !ResCommonUtil::ParseStringParameterFromJson("abilityName", abilityName, resData->payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("appIndex", appIndex, resData->payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("taskId", taskId, resData->payload)) {
        WS_HILOGE("get background loader info fail");
        return;
    }
    WS_HILOGI("bundleName: %{public}s, abilityName: %{public}s, appIndex: %{public}d, taskId: %{public}d",
        bundleName.c_str(), abilityName.c_str(), appIndex, taskId);
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
        WS_HILOGI("send request to existing remoteObject success");
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
    int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByCall(providerWant, connect);
    if (ret != ERR_OK) {
        WS_HILOGE("StartAbilityByCall falied ret : %{public}d", ret);
        return;
    }
    WS_HILOGI("StartAbilityByCall success for %{public}s", bundleName.c_str());
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
        WS_HILOGE("find %{public}d task info failed", ret);
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
    int32_t ret = remoteObject->sendRequest(AAFwk::IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendOnStart: send request failed");
        return;
    }
    info.status_ = TaskStatus::RUNNING;
    PostTimeoutTask(info.bundleName_, info.abilityName, info.appIndex_, info.taskId);
}

bool BackgroundLoaderMgr::GetInnerTaskInfo(const std::string& bundleName, int32_t appIndex, TaskInfo& info)
{
    std::string key = GenerateTaskKey(bundleName, appIndex);
    auto it = taskMap_.find(key);
    if (it != taskMap_.end()) {
       info = it->second;
       return true;
    }
    return false;
}

void BackgroundLoaderMgr::RemoveRemoteObject(const std::string& bundleName,
    const std::string& abilityName, int32_t appIndex)
{
    std::string key = GenerateTaskKey(bundleName, appIndex);
    std::lock_guard<ffrt::mutex> lock(abilityMapLock_);
    auto it = abilityMap_.find(key);
    if (it != abilityMap_.end()) {
        abilityMap_.erase(it);
        WS_HILOGI("Removed object for %{public}s, appIndex: %{public}d", bundleName.c_str(), appIndex);
    } else {
        WS_HILOGI("%{public}s, appIndex: %{public}d not found", bundleName.c_str(), appIndex);
    }
}

void BackgroundLoaderMgr::SendOnStop(const TaskInfo& taskI
    nfo, int32_t stopCode, const std::string& message)
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
    int32_t ret = remoteObject->sendRequest(AAFwk::IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    if (ret != ERR_OK) {
        WS_HILOGE("SendOnStop: send request for %{public}s, appIndex: %{public}d failed",
            taskInfo.bundleName_.c_str(), taskInfo.appIndex_);
        return;
    }
    WS_HILOGI("SendOnStop: send request for %{public}s, appIndex: %{public}d success",
        taskInfo.bundleName_.c_str(), taskInfo.appIndex_);
}

void BackgrounderMgr::SaveRemoteObject(const std::string& bundleName,
    const std::string& abilityName, int32_t appIndex, const sptr<IRemoteObject>& remoteObject)
{
    std::string key = GenerateTaskKey(bundleName, appIndex);
    std::lock_guard<ffrt::mutex> lock(abilityMapLock_);
    abilityMap_[key] = remoteObject;
    WS_HILOGI("save remote object for %{public}s, appIndex: %{public}d success", bundleName.c_str(), appIndex);
}

}  // namespace WorkScheduler
}  // namespace OHOS