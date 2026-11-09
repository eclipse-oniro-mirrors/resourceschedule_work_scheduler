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

#include <string_ex.h>
#include <vector>
#include <algorithm>
#include <cstdint>
#include "background_loader/ability_connect_callback.h"
#include "ability_manager_client.h"
#include "background_loader/background_loader_mgr.h"
#include "background_loader/background_loader_hisysevent_report.h"
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
#include "work_sched_constants.h"
#include "work_sched_hisysevent_report.h"
#include "app_mgr_client.h"
#include "ability_state.h"
#include "privacy_kit.h"
#include "singleton.h"
#ifdef WORKSCHEDULER_AUDIO_FRAMEWORK_ENABLE
#include "audio_system_manager.h"
#endif

extern "C" void ReportDataInProcess(uint32_t resType, int64_t value, const nlohmann::json& payload);
using namespace OHOS::ResourceSchedule;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr std::string_view TIMEOUT_MESSAGE = "timeOut";
constexpr std::string_view ON_START = "onStart";
constexpr std::string_view ON_STOP = "onStop";
constexpr int32_t DEFAULT_INVAL_VALUE = -1;
constexpr int32_t REPORT_DATA_MAX_RETRY = 3;
constexpr int32_t REPORT_DATA_RETRY_DELAY_US = 2000000;
constexpr int32_t MAX_BUNDLE_NAME_LENGTH = 256;
constexpr int32_t MAX_ABILITY_NAME_LENGTH = 256;
constexpr int32_t MAX_TASK_MAP_SIZE = 1000;
constexpr std::string_view LOCATION_STATE_START = "start";
constexpr std::string_view TORCH_STATE_ON = "ON";
constexpr std::string_view OHOS_PERMISSION_CAMERA_BACKGROUND = "ohos.permission.CAMERA_BACKGROUND";
constexpr std::string_view OHOS_PERMISSION_MICROPHONE_BACKGROUND = "ohos.permission.MICROPHONE_BACKGROUND";
constexpr std::string_view OHOS_PERMISSION_LOCATION_IN_BACKGROUND = "ohos.permission.LOCATION_IN_BACKGROUND";

const std::vector<int32_t> ABNORMAL_EXIT_REASON = {AAFwk::REASON_CPP_CRASH, AAFwk::REASON_JS_ERROR,
                                                   AAFwk::REASON_CJ_ERROR, AAFwk::REASON_APP_FREEZE};
}
IMPLEMENT_SINGLE_INSTANCE(BackgroundLoaderMgr)

void BackgroundLoaderMgr::Init(int32_t maxTimeoutCount, int32_t backgroundLoaderTimeoutUs)
{
    BL_HILOGI("[%{public}s:%{public}d] BackgroundLoaderMgr init", __FUNCTION__, __LINE__);
    maxTimeoutCount_ = maxTimeoutCount;
    backgroundLoaderTimeoutUs_ = backgroundLoaderTimeoutUs;
    ffrtQueue_ = std::make_unique<ffrt::queue>("background_loader_queue");
    if (dbQueue_ == nullptr) {
        dbQueue_ = std::make_unique<ffrt::queue>("BackgroundLoaderDbQueue",
            ffrt::queue_attr().qos(ffrt::qos_default));
    }
    InitDatabaseAsync();
}

void BackgroundLoaderMgr::InitDatabaseAsync()
{
    ffrt::submit([this]() {
        if (!database_.Init()) {
            BL_HILOGE("database init failed");
            return;
        }
        RestoreTasksFromDb();
        isReady_.store(true);
    });
}

void BackgroundLoaderMgr::RestoreTaskMapFromDb(const std::vector<TaskRow>& rows)
{
    std::vector<std::pair<std::string, int32_t>> restoreKeys;
    {
        std::lock_guard<ffrt::mutex> lock(taskLock_);
        for (const auto& row : rows) {
            if (row.status < static_cast<int32_t>(TaskStatus::NOT_STARTED) ||
                row.status > static_cast<int32_t>(TaskStatus::IN_BLOCK_LIST)) {
                BL_HILOGE("invalid status from db: %{public}d, uid=%{public}d, skip", row.status, row.uid);
                continue;
            }
            TaskInfo info;
            info.bundleName_ = row.bundleName;
            info.abilityName_ = row.abilityName;
            info.appIndex_ = row.appIndex;
            info.taskId_ = row.taskId;
            info.status_ = static_cast<TaskStatus>(row.status); // running状态的要做接续处理
            info.uid_ = row.uid;
            info.timeoutCount_ = row.timeoutCount;
            // 重启后运行状态已失效，强制恢复默认值
            info.pid_ = DEFAULT_INVAL_VALUE;
            info.isBg_ = false;
            info.hasShortTask_ = false;
            info.hasLongTask_ = false;
            info.hasAudioStream_ = false;
            taskMap_[row.uid] = info;
            if (info.status_ != TaskStatus::IN_BLOCK_LIST) {
                restoreKeys.emplace_back(info.bundleName_, info.appIndex_);
            }
        }
    }
    BL_HILOGI("restore %{public}zu tasks from db", rows.size());
    if (restoreKeys.empty()) {
        return;
    }
    retryCount_.store(0);
    ReportRestoredTasks(restoreKeys);
}

bool BackgroundLoaderMgr::IsResSchedReady()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        return false;
    }
    return samgr->GetSystemAbility(RES_SCHED_SYS_ABILITY_ID) != nullptr;
}

void BackgroundLoaderMgr::ReportRestoredTasks(const std::vector<std::pair<std::string, int32_t>>& restoreKeys)
{
    if (IsResSchedReady()) {
        for (const auto& [bundleName, appIndex] : restoreKeys) {
            nlohmann::json payload;
            payload["bundleName"] = bundleName;
            payload["appIndex"] = std::to_string(appIndex);
            ReportDataInProcess(ResType::RES_TYPE_BACKGROUND_LOADER_CHANGE_EVENT,
                ResType::BackgroundLoaderState::ADD, payload);
        }
        if (retryReportHandle_ && dbQueue_ != nullptr) {
            dbQueue_->cancel(retryReportHandle_);
            retryReportHandle_ = nullptr;
        }
        return;
    }
    if (retryCount_.fetch_add(1) >= REPORT_DATA_MAX_RETRY) {
        BL_HILOGW("res sched not ready after %{public}d retries, skip report restore events.", REPORT_DATA_MAX_RETRY);
        return;
    }
    if (dbQueue_ == nullptr) {
        BL_HILOGE("dbQueue is null, skip report restore events.");
        return;
    }
    retryReportHandle_ = dbQueue_->submit_h([this, restoreKeys]() {
        ReportRestoredTasks(restoreKeys);
    }, ffrt::task_attr().delay(REPORT_DATA_RETRY_DELAY_US));
}

void BackgroundLoaderMgr::RestoreTasksFromDb()
{
    std::vector<TaskRow> rows;
    if (!database_.QueryAllTasks(rows)) {
        BL_HILOGE("query active tasks from db failed");
        return;
    }
    RestoreBlockListFromDb();
    RestoreTaskMapFromDb(rows);
}

void BackgroundLoaderMgr::RestoreBlockListFromDb()
{
    std::unordered_set<std::string> dbKeys;
    if (!database_.QueryAllBlockList(dbKeys)) {
        BL_HILOGE("query blockLists from db failed");
        return;
    }
    std::lock_guard<ffrt::mutex> lock(blockListLock_);
    blockLists_.merge(dbKeys);
    BL_HILOGI("retore %{public}zu blockLists from db", blockLists_.size());
}

TaskRow BackgroundLoaderMgr::ToTaskRow(const TaskInfo& info)
{
    TaskRow row;
    row.uid = info.uid_;
    row.bundleName = info.bundleName_;
    row.abilityName = info.abilityName_;
    row.appIndex = info.appIndex_;
    row.taskId = info.taskId_;
    row.status = static_cast<int32_t>(info.status_);
    row.timeoutCount = info.timeoutCount_;
    row.pid = info.pid_;
    return row;
}

std::string BackgroundLoaderMgr::GenerateTaskKey(const std::string& bundleName, int32_t appIndex)
{
    return bundleName + "_" + std::to_string(appIndex);
}

ErrCode BackgroundLoaderMgr::UpsertTaskToDb(int32_t uid, const TaskInfo& taskInfo)
{
    TaskRow row = ToTaskRow(taskInfo);
    if (dbQueue_ == nullptr) {
        BL_HILOGE("dbQueue_ is null, cannot upsert task, uid=%{public}d", uid);
        return E_INNER_ERR;
    }
    dbQueue_->submit([this, row, uid]() {
        if (!database_.UpsertTask(row)) {
            BL_HILOGE("Upsert task to db failed, uid=%{public}d", uid);
        }
    });
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::DeleteTaskFromDb(int32_t uid)
{
    if (dbQueue_ == nullptr) {
        BL_HILOGE("dbQueue_ is null, cannot delete task, uid=%{public}d", uid);
        return E_INNER_ERR;
    }
    dbQueue_->submit([this, uid]() {
        if (!database_.DeleteTask(uid)) {
            BL_HILOGE("Delete task from db failed, uid=%{public}d", uid);
        }
    });
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::DeleteBlockListFromDb(const std::string& bundleName)
{
    if (dbQueue_ == nullptr) {
        BL_HILOGE("dbQueue_ is null, cannot delete blockList, bundleName: %{public}s", bundleName.c_str());
        return E_INNER_ERR;
    }
    dbQueue_->submit([this, bundleName]() {
        if (!database_.DeleteBlockList(bundleName)) {
            BL_HILOGE("Delete blockList from db failed, bundleName: %{public}s", bundleName.c_str());
        }
    });
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::HandleExistingTask(const BackgroundLoaderTaskInfo& taskInfo, const std::string &bundleName,
                                                int32_t uid, TaskInfo &taskInfoCopy)
{
    // 存在：在黑名单里面 —— 不允许注册修改，返回错误码
    if (IsInBlockListLocked(bundleName)) {
        BL_HILOGE("bundleName: %{public}s is in block list, reject register", bundleName.c_str());
        return E_CHECK_WORKINFO_FAILED;
    }
    // 存在：不在黑名单里面 —— 校验 taskId 和 abilityName
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end()) {
        BL_HILOGE("task not found in taskMap_, uid: %{public}d", uid);
        return E_WORK_NOT_EXIST_FAILED;
    }
    const std::string &existingAbilityName = it->second.abilityName_;
    int32_t existingTaskId = it->second.taskId_;
    // abilityName 不一致 —— 参数错误
    if (existingAbilityName != taskInfo.GetAbilityName()) {
        BL_HILOGE("abilityName mismatch, esisting: %{public}s, input: %{public}s, uid: %{public}d",
                  existingAbilityName.c_str(), taskInfo.GetAbilityName().c_str(), uid);
        return E_PARAM_ERROR;
    }
    // taskId 和 abilityName 都一致 —— 已注册过，返回重复错误码
    if (existingTaskId == taskInfo.GetTaskId()) {
        BL_HILOGE("task already registered, taskId: %{public}d, abilityName: %{public}s, uid: %{public}d",
                  taskInfo.GetTaskId(), taskInfo.GetAbilityName().c_str(), uid);
        return E_ADD_REPEAT_WORK_ERR;
    }
    // 仅taskId不一致 —— 允许刷新 taskId
    it->second.taskId_ = taskInfo.GetTaskId();
    taskInfoCopy = it->second;
    BL_HILOGE("task exist, refresh taskId: %{public}d -> taskId: %{public}d, uid: %{public}d", existingTaskId,
              taskInfo.GetTaskId(), uid);
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::RegisterNewTask(const BackgroundLoaderTaskInfo& taskInfo, const std::string &bundleName,
                                             int32_t appIndex, int32_t uid)
{
    // 1. 校验黑名单
    if (IsInBlockListLocked(bundleName)) {
        BL_HILOGE("bundleName %{public}s is in block list, regect register", bundleName.c_str());
        return E_CHECK_WORKINFO_FAILED;
    }

    // 2. 校验敏感权限
    uint32_t tokenID = static_cast<uint32_t>(IPCSkeleton::GetCallingTokenID());
    if (HasPerceivableSensitivePermission(tokenID)) {
        BL_HILOGW("bundleName: %{public}s has perceivable sensitive permission, reject register", bundleName.c_str());
        AddToBlockListLocked(bundleName);
        if (dbQueue_ == nullptr) {
            return E_CHECK_WORKINFO_FAILED;
        }
        std::string blockKey = bundleName;
        dbQueue_->submit([this, blockKey]() {
            if (!database_.InsertBlockList(blockKey)) {
                BL_HILOGE("insert blocklist to db failed, key=%{public}s", blockKey.c_str());
            }
        });
        return E_CHECK_WORKINFO_FAILED;
    }

    // 3. 校验 abilityName 是否为主 ability
    if (!VerifyAbilityName(bundleName, taskInfo.GetAbilityName(), uid)) {
        BL_HILOGE("VerifyAbilityName failed, bundleName: %{public}s, abilityName: %{public}s", bundleName.c_str(),
                  taskInfo.GetAbilityName().c_str());
        return E_CHECK_WORKINFO_FAILED;
    }

    // 4. 允许注册
    TaskInfo info = {
        .bundleName_ = bundleName,
        .abilityName_ = taskInfo.GetAbilityName(),
        .appIndex_ = appIndex,
        .taskId_ = taskInfo.GetTaskId(),
        .uid_ = uid,
    };
    if (!AddNewTaskLocked(uid, info)) {
        return E_WORK_EXCEED_UPPER_LIMIT;
    }
    ReportTaskChangeEvent(bundleName, appIndex, static_cast<int32_t>(ResType::BackgroundLoaderState::ADD));
    UpsertTaskToDb(uid, info);
    BackgroundLoaderHiSysEvent::ReportRecordSysEvent(info, "Register Task", "");
    return ERR_OK;
}

void BackgroundLoaderMgr::CheckAndSendOnStop(int32_t uid)
{
    BL_HILOGI("checkAndSendOnStop, uid: %{public}d", uid);
    TaskInfo taskInfoCopy;
    bool shouldAddToBlockList = false;
    if (!IsTimeoutTriggeredLocked(uid, taskInfoCopy, shouldAddToBlockList)) {
        return;
    }
    // 先发 stop 事件，(依赖abilityMap_中的remoteObject)，保持status状态为RUNNNING
    // 避免被并发的解注册导致abilityMap_被清理
    SendOnStop(taskInfoCopy, static_cast<int32_t>(StopCode::TIMEOUT_ERROR), std::string(TIMEOUT_MESSAGE));
    // stop 事件发送完成后再刷新status, abilityMap_被清理
    MarkTimeOutStatusLocked(uid, shouldAddToBlockList, taskInfoCopy);
    if (shouldAddToBlockList) {
        AddToBlockList(taskInfoCopy.bundleName_, uid, static_cast<int32_t>(BlockListReason::TIMEOUT));
        ReportTaskChangeEvent(taskInfoCopy.bundleName_, taskInfoCopy.appIndex_,
                              static_cast<int32_t>(ResType::BackgroundLoaderState::DELETE));
    }
    ReportTaskFinishEvent(taskInfoCopy.bundleName_, taskInfoCopy.appIndex_, 1, E_INNER_ERR);
    UpsertTaskToDb(uid, taskInfoCopy);
    // 超时任务已执行完毕, 清理handle表残留引用（handle 已被ffrt消费，无需cancel)
    {
        std::lock_guard<ffrt::mutex> lock(timeoutTaskLock_);
        timeoutTaskMap_.erase(uid);
    }
}

void BackgroundLoaderMgr::PostTimeoutTask(int32_t uid)
{
    BL_HILOGI("PostTimeOutTask, uid: %{public}d", uid);
    if (ffrtQueue_ == nullptr) {
        BL_HILOGE("ffrtQueue_ is null, cannot post timeout task");
        return;
    }
    // 同 uid 重复投递时先取消旧 handle, 避免 handle 泄露与重复触发
    {
        std::lock_guard<ffrt::mutex> lock(timeoutTaskLock_);
        auto it = timeoutTaskMap_.find(uid);
        if (it != timeoutTaskMap_.end()) {
            ffrtQueue_->cancel(it->second);
            timeoutTaskMap_.erase(it);
            BL_HILOGW("PostTimeoutTask: uid %{public}d has pending timeout task, cancel first", uid);
        }
    }
    ffrt::task_handle handle =
        ffrtQueue_->submit_h([uid]() { BackgroundLoaderMgr::GetInstance().CheckAndSendOnStop(uid); },
                             ffrt::task_attr().delay(backgroundLoaderTimeoutUs_));
    {
        std::lock_guard<ffrt::mutex> lock(timeoutTaskLock_);
        timeoutTaskMap_[uid] = std::move(handle);
    }
    BL_HILOGI("post timeout task success, uid: %{public}d, delay: %{public}dus", uid, backgroundLoaderTimeoutUs_);
}

void BackgroundLoaderMgr::CancelTimeoutTask(int32_t uid)
{
    if (ffrtQueue_ == nullptr) {
        BL_HILOGD("ffrtQueue_ is null, cannot cancel timeout task");
        return;
    }
    std::lock_guard<ffrt::mutex> lock(timeoutTaskLock_);
    auto it = timeoutTaskMap_.find(uid);
    if (it == timeoutTaskMap_.end()) {
        BL_HILOGD("CancelTimeoutTask: no timeout task for uid: %{public}d", uid);
        return;
    }
    int ret = ffrtQueue_->cancel(it->second);
    BL_HILOGI("cancel timeout task, uid: %{public}d, ret: %{public}d", uid, ret);
    timeoutTaskMap_.erase(it);
}

bool BackgroundLoaderMgr::ParseBackgroundLoaderPayload(const nlohmann::json &payload, std::string &bundleName,
                                                       std::string &abilityName, int32_t &appIndex)
{
    if (!ResCommonUtil::ParseStringParameterFromJson("bundleName", bundleName, payload) ||
        !ResCommonUtil::ParseStringParameterFromJson("abilityName", abilityName, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("appIndex", appIndex, payload)) {
        BL_HILOGE("get background loader info fail");
        return false;
    }
    if (bundleName.empty() || bundleName.size() > MAX_BUNDLE_NAME_LENGTH) {
        BL_HILOGE("bundleName is empty or too long");
        return false;
    }
    if (abilityName.empty() || abilityName.size() > MAX_ABILITY_NAME_LENGTH) {
        BL_HILOGE("abilityName is empty or too long");
        return false;
    }
    return true;
}

void BackgroundLoaderMgr::HandleBackgroundLoaderTask(const nlohmann::json& payload)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderTask service is not ready");
        return;
    }
    std::string abilityName;
    std::string bundleName;
    int32_t appIndex = 0;
    if (!ParseBackgroundLoaderPayload(payload, bundleName, abilityName, appIndex)) {
        ReportTaskFinishEvent(bundleName, appIndex, 1, E_INNER_ERR);
        return;
    }
    if (IsInBlockListLocked(bundleName)) {
        BL_HILOGW("bundleName: %{public}s in block list", bundleName.c_str());
        ReportTaskFinishEvent(bundleName, appIndex, 1, E_INNER_ERR);
        return;
    }
    int32_t uid = -1;
    if (!CanBackGroundLoadLocked(bundleName, appIndex, uid)) {
        ReportTaskFinishEvent(bundleName, appIndex, 1, E_INNER_ERR);
        return;
    }
    ConnectToAbility(bundleName, abilityName, appIndex, uid);
}

void BackgroundLoaderMgr::ConnectToAbility(const std::string &bundleName, const std::string &abilityName,
                                           int32_t appIndex, int32_t uid)
{
    OHOS::sptr<IRemoteObject> remoteObject = GetRemoteObjectLocked(bundleName, abilityName, appIndex);
    if (remoteObject != nullptr) {
        BL_HILOGI("remoteObject cached, direct SendOnstart for %{public}s", bundleName.c_str());
        SendOnStart(remoteObject, bundleName, appIndex, uid);
        return;
    }

    OHOS::AAFwk::Want providerWant;
    providerWant.SetElementName(bundleName, abilityName);
    sptr<AAFwk::IAbilityConnection> connect = new(std::nothrow) AbilityConnectCallback(
        bundleName, abilityName, appIndex, uid);
    if (connect == nullptr) {
        BL_HILOGE("failed to new connection");
        ReportTaskFinishEvent(bundleName, appIndex, 1, E_INNER_ERR);
        return;
    }
    BL_HILOGI("StartAbilityByCall start for %{public}s", bundleName.c_str());
    int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByCall(providerWant, connect,
        nullptr, DEFAULT_INVAL_VALUE, false, true);
    if (ret != ERR_OK) {
        BL_HILOGE("StartAbilityByCall uid: %{public}d falied ret : %{public}d", uid, ret);
        ReportTaskFinishEvent(bundleName, appIndex, 1, E_INNER_ERR);
        return;
    }
    BL_HILOGI("StartAbilityByCall success for %{public}s", bundleName.c_str());
}

void BackgroundLoaderMgr::SendOnStart(const sptr<IRemoteObject>& remoteObject,
    const std::string& bundleName, int32_t appIndex, int32_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!data.WriteString16(Str8ToStr16(std::string(ON_START)))) {
        BL_HILOGE("SendOnStart: write function name failed");
        return;
    }
    TaskInfo taskInfoCopy;
    if (!MarkTaskRunningLocked(uid, taskInfoCopy)) {
        BL_HILOGE("SendOnStart: task not found, uid %{public}d", uid);
        return;
    }
    int32_t ret = remoteObject->SendRequest(AAFwk::IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    if (ret != ERR_OK) {
        BL_HILOGE("SendOnStart: send request failed, uid %{public}d", uid);
        // 异常结束，标志status结束， 且发送finish事件
        MarkTaskFinishedLocked(uid, taskInfoCopy);
        UpsertTaskToDb(uid, taskInfoCopy);
        ReportTaskFinishEvent(bundleName, appIndex, 1, E_INNER_ERR);
        BackgroundLoaderHiSysEvent::ReportRecordSysEvent(taskInfoCopy, "Send onStart failed", "");
        return;
    }
    BL_HILOGI("send request to %{public}s success", bundleName.c_str());
    UpsertTaskToDb(uid, taskInfoCopy);
    PostTimeoutTask(uid);
    BackgroundLoaderHiSysEvent::ReportRecordSysEvent(taskInfoCopy, "Send onStart", "");
}

void BackgroundLoaderMgr::SendOnStop(const TaskInfo& taskInfo, int32_t stopCode, const std::string& message)
{
    sptr<IRemoteObject> remoteObject =
       GetRemoteObjectLocked(taskInfo.bundleName_, taskInfo.abilityName_, taskInfo.appIndex_);
    if (remoteObject == nullptr) {
        BL_HILOGE("remoteObject for %{public}s, appIndex: %{public}d not found",
            taskInfo.bundleName_.c_str(), taskInfo.appIndex_);
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);

    if (!data.WriteString16(Str8ToStr16(std::string(ON_STOP)))) {
        BL_HILOGE("SendOnStop: write function name failed");
        return;
    }
    if (!data.WriteInt32(taskInfo.taskId_)) {
        BL_HILOGE("SendOnStop: write task id failed");
        return;
    }
    if (!data.WriteString16(Str8ToStr16(taskInfo.abilityName_))) {
        BL_HILOGE("SendOnStop: write ability name failed");
        return;
    }
    if (!data.WriteInt32(stopCode)) {
        BL_HILOGE("SendOnStop: write stopCode failed");
        return;
    }
    if (!data.WriteString16(Str8ToStr16(message))) {
        BL_HILOGE("SendOnStop: write message failed");
        return;
    }
    int32_t ret = remoteObject->SendRequest(AAFwk::IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    if (ret != ERR_OK) {
        BL_HILOGE("SendOnStop: send request for %{public}s, appIndex: %{public}d failed",
            taskInfo.bundleName_.c_str(), taskInfo.appIndex_);
        return;
    }
    BL_HILOGI("SendOnStop: send request for %{public}s, appIndex: %{public}d success",
        taskInfo.bundleName_.c_str(), taskInfo.appIndex_);
}

sptr<IRemoteObject> BackgroundLoaderMgr::GetRemoteObjectLocked(const std::string& bundleName,
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

void BackgroundLoaderMgr::RemoveRemoteObjectLocked(const std::string& bundleName, int32_t appIndex)
{
    std::string key = GenerateTaskKey(bundleName, appIndex);
    std::lock_guard<ffrt::mutex> lock(abilityMapLock_);
    auto it = abilityMap_.find(key);
    if (it != abilityMap_.end()) {
        abilityMap_.erase(it);
        BL_HILOGI("Removed object for %{public}s, appIndex: %{public}d", bundleName.c_str(), appIndex);
    } else {
        BL_HILOGD("bundleName: %{public}s, appIndex: %{public}d not found", bundleName.c_str(), appIndex);
    }
}

void BackgroundLoaderMgr::SaveRemoteObjectLocked(const std::string& bundleName,
    const std::string& abilityName, int32_t appIndex, const sptr<IRemoteObject>& remoteObject)
{
    std::string key = GenerateTaskKey(bundleName, appIndex);
    std::lock_guard<ffrt::mutex> lock(abilityMapLock_);
    abilityMap_[key] = remoteObject;
    BL_HILOGD("save remote object for %{public}s, appIndex: %{public}d success", bundleName.c_str(), appIndex);
}

void BackgroundLoaderMgr::HandleAbilityConnectDone(const std::string &bundleName, const std::string& abilityName,
                                                   int32_t appIndex, int32_t uid,
                                                   const sptr<IRemoteObject>& remoteObject)
{
    BL_HILOGI("HandleAbilityConnectDone, bundleName: %{public}s, abilityName: %{public}s, appIndex: %{public}d, uid: "
              "%{public}d",
              bundleName.c_str(), abilityName.c_str(), appIndex, uid);
    int32_t pid = -1;
    AAFwk::AbilityManagerClient::GetInstance()->GetPidByCallStub(remoteObject, pid);
    if (pid <= 0) {
        BL_HILOGE("GetPidByCallStub fail, bundleName: %{public}s, uid: %{public}d", bundleName.c_str(), uid);
        return;
    }

    TaskInfo taskInfoCopy;
    if (!UpdatePidLocked(uid, pid, taskInfoCopy)) {
        return;
    }
    UpsertTaskToDb(uid, taskInfoCopy);
    SaveRemoteObjectLocked(bundleName, abilityName, appIndex, remoteObject);
    SendOnStart(remoteObject, bundleName, appIndex, uid);
}

void BackgroundLoaderMgr::HandleAbilityDisonnectDone(const std::string &bundleName, int32_t appIndex)
{
    BL_HILOGI("HandleAbilityDisonnectDone, bundleName: %{public}s, appIndex: %{public}d",
              bundleName.c_str(), appIndex);
    RemoveRemoteObjectLocked(bundleName, appIndex);
}

void BackgroundLoaderMgr::HandleAppInstallUninstall(int64_t value, const nlohmann::json& payload)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderMgr service is not ready");
        return;
    }
    if (value != ResType::AppInstallStatus::APP_UNINSTALL &&
        value != ResType::AppInstallStatus::APP_CHANGED &&
        value != ResType::AppInstallStatus::APP_REPLACED) {
        return;
    }
    std::string bundleName = "";
    int32_t uid = DEFAULT_INVAL_VALUE;
    int32_t appIndex = DEFAULT_INVAL_VALUE;
    if (!ResCommonUtil::ParseStringParameterFromJson("bundleName", bundleName, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("appIndex", appIndex, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("uid", appIndex, payload)) {
        BL_HILOGE("HandleAppInstallUninstall parse payload fail");
        return;
    }
    BL_HILOGI("app uninstall, remove bundleNamme: %{public}s, uid: %{public}d, appIndex: %{public}d",
              bundleName.c_str(), uid, appIndex);
    ClearAllByUid(uid, bundleName, appIndex);
}

void BackgroundLoaderMgr::ClearAllByUid(int32_t uid, const std::string& bundleName, int32_t appIndex)
{
    if (RemoveFromBlockListLocked(bundleName)) {
        DeleteBlockListFromDb(bundleName);
    }
    RemoveRemoteObjectLocked(bundleName, appIndex);
    if (EraseTaskByKeyLocked(uid)) {
        DeleteTaskFromDb(uid);
        ReportTaskChangeEvent(bundleName, appIndex, static_cast<int32_t>(ResType::BackgroundLoaderState::DELETE));
        CancelTimeoutTask(uid);
    }
}

bool BackgroundLoaderMgr::GetAppIndexAndBundleNameByUid(int32_t uid, int32_t& appIndex, std::string& bundleName)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        BL_HILOGE("fail to get system ability mgr.");
        return false;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        BL_HILOGE("fail to get bundle manager proxy.");
        return false;
    }
    sptr<IBundleMgr> bundleMgr = iface_cast<IBundleMgr>(remoteObject);
    if (!bundleMgr) {
        BL_HILOGE("GetAppIndexAndBundleNameByUid fail to get bundle manager proxy.");
        return false;
    }
    ErrCode ret = bundleMgr->GetNameAndIndexForUid(uid, bundleName, appIndex);
    if (ret != ERR_OK) {
        BL_HILOGE("fail to get app index and bundleName.");
        return false;
    }
    return true;
}

bool BackgroundLoaderMgr::VerifyAbilityName(const std::string& bundleName,
    const std::string& abilityName, int32_t uid)
{
    if (bundleName.empty() || abilityName.empty()) {
        BL_HILOGE("bundleName or abilityName invaild");
        return false;
    }

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        BL_HILOGE("fail to get system ability mgr.");
        return false;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        BL_HILOGE("fail to get bundle manager remoteObject.");
        return false;
    }
    sptr<IBundleMgr> bundleMgr = iface_cast<IBundleMgr>(remoteObject);
    if (!bundleMgr) {
        BL_HILOGE("fail to get bundle manager proxy.");
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
        BL_HILOGE("QueryAbilityInfos failed for bundle: %{public}s", bundleName.c_str());
        return false;
    }

    for (const auto& ability : abilityInfos) {
        if (ability.bundleName == bundleName && ability.name == abilityName && ability.enabled) {
            BL_HILOGI("found ability %{public}s in bundle: %{public}s", abilityName.c_str(), bundleName.c_str());
            return true;
        }
    }
    BL_HILOGE("bundle: %{public}s not found or not enable", bundleName.c_str());
    return false;
}

ErrCode BackgroundLoaderMgr::RegisterTaskWithCheck(const BackgroundLoaderTaskInfo& taskInfo)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }
    std::string bundleName = "";
    int32_t appIndex = -1;
    int32_t uid = IPCSkeleton::GetCallingAppUid();
    if (!GetAppIndexAndBundleNameByUid(uid, appIndex, bundleName)) {
        return E_CHECK_WORKINFO_FAILED;
    }
    BL_HILOGI("taskId: %{public}d, bundleName: %{public}s, abilityName: %{public}s, "
              "appIndex: %{public}d, uid: %{public}d",
              taskInfo.GetTaskId(), bundleName.c_str(), taskInfo.GetAbilityName().c_str(), appIndex, uid);
    if (IsTaskExistLocked(uid)) {
        TaskInfo taskInfoCopy;
        ErrCode ret = HandleExistingTask(taskInfo, bundleName, uid, taskInfoCopy);
        if (ret == ERR_OK) {
            UpsertTaskToDb(uid, taskInfoCopy);
        }
        return ret;
    }
    return RegisterNewTask(taskInfo, bundleName, appIndex, uid);
}

ErrCode BackgroundLoaderMgr::UnregisterTaskWithCheck(const BackgroundLoaderTaskInfo& taskInfo)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }
    int32_t uid = IPCSkeleton::GetCallingAppUid();
    TaskInfo taskInfoCopy;
    ErrCode ret = RemoveTaskLocked(taskInfo, uid, taskInfoCopy);
    if (ret != ERR_OK) {
        return ret;
    }
    RemoveRemoteObjectLocked(taskInfoCopy.bundleName_, taskInfoCopy.appIndex_);
    ReportTaskChangeEvent(taskInfoCopy.bundleName_, taskInfoCopy.appIndex_,
                          static_cast<int32_t>(ResType::BackgroundLoaderState::DELETE));
    DeleteTaskFromDb(uid);
    BackgroundLoaderHiSysEvent::ReportRecordSysEvent(taskInfoCopy, "Unregister Task", "");
    BL_HILOGI("unregister success, bundleName: %{public}s, appIndex: %{public}d", taskInfoCopy.bundleName_.c_str(),
              taskInfoCopy.appIndex_);
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::FinishTaskWithCheck(const BackgroundLoaderTaskInfo& taskInfo)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }
    BL_HILOGI("FinishTaskWithCheck, taskId: %{public}d, abilityName: %{public}s", taskInfo.GetTaskId(),
              taskInfo.GetAbilityName().c_str());
    int32_t uid = IPCSkeleton::GetCallingAppUid();
    TaskInfo taskInfoCopy;
    ErrCode ret = FinishTaskLocked(taskInfo, uid, taskInfoCopy);
    if (ret != ERR_OK) {
        return ret;
    }
    ReportTaskFinishEvent(taskInfoCopy.bundleName_, taskInfoCopy.appIndex_, 0, ERR_OK);
    UpsertTaskToDb(uid, taskInfoCopy);
    BackgroundLoaderHiSysEvent::ReportRecordSysEvent(taskInfoCopy, "Finish Task", "");
    BL_HILOGI("finish success, bundleName: %{public}s, appIndex: %{public}d, status -> FINISHED",
              taskInfoCopy.bundleName_.c_str(), taskInfoCopy.appIndex_);
    CancelTimeoutTask(uid);
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::FinishTaskLocked(const BackgroundLoaderTaskInfo& taskInfo, int32_t uid,
                                              TaskInfo &taskInfoCopy)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end()) {
        BL_HILOGE("FinishTaskLocked failed, task not found, uid: %{public}d, taskInfo: %{public}d", uid,
                  taskInfo.GetTaskId());
        return E_CHECK_WORKINFO_FAILED;
    }
    if (it -> second.taskId_ != taskInfo.GetTaskId() || it->second.abilityName_ != taskInfo.GetAbilityName()) {
        BL_HILOGE("FinishTaskLocked failed, taskId/abilityName mismatch, uid: %{public}d, "
                  "taskId: %{public}d, abilityName: %{public}s",
                  uid, taskInfo.GetTaskId(), taskInfo.GetAbilityName().c_str());
        return E_WORK_NOT_EXIST_FAILED;
    }
    if (it->second.status_ != TaskStatus::RUNNING) {
        BL_HILOGE("FinishTaskLocked failed, status not running, uid: %{public}d, status: %{public}d", it->first,
                  static_cast<int32_t>(it->second.status_));
        return E_WORK_NOT_EXIST_FAILED;
    }
    it->second.status_ = TaskStatus::FINISHED;
    taskInfoCopy = it->second;
    BL_HILOGE("FinishTaskLocked success, uid: %{public}d, status -> FINISHED", it->first);
    return ERR_OK;
}

ErrCode BackgroundLoaderMgr::GetTaskInfoWithCheck(int32_t taskId, BackgroundLoaderTaskInfo& taskInfo)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderMgr service is not ready");
        return E_SERVICE_NOT_READY;
    }
    int32_t uid = IPCSkeleton::GetCallingAppUid();
    BL_HILOGI("GetTaskInfoWithCheck taskId: %{public}d, uid: %{public}d", taskId, uid);
    std::string bundleName = "";
    ErrCode ret = GetTaskInfoLocked(uid, taskId, taskInfo, bundleName);
    if (ret != ERR_OK) {
        return ret;
    }
    if (IsInBlockListLocked(bundleName)) {
        BL_HILOGE("bundleName: %{public}s is in block lisr, get taskInfo fail", bundleName.c_str());
        return E_INNER_ERR;
    }
    return ERR_OK;
}

void BackgroundLoaderMgr::HandleAppStateChangeEvent(int64_t value, const nlohmann::json &payload)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderMgr service is not ready");
        return;
    }
    bool isBg = false;
    if (value == ResType::ProcessStatus::PROCESS_BACKGROUND) {
        isBg = true;
    } else if (value == ResType::ProcessStatus::PROCESS_FOREGROUND) {
        isBg = false;
    } else {
        return;
    }
    int32_t uid = 0;
    std::string bundleName = "";
    if (!ResCommonUtil::ParseIntParameterFromJson("uid", uid, payload) ||
        !ResCommonUtil::ParseStringParameterFromJson("bundleName", bundleName, payload)) {
        BL_HILOGE("HandleAppChangeEvent parse payload fail");
        return;
    }
    BL_HILOGD("app state change, bundleName: %{public}s, uid: %{public}d, isBg: %{public}d", bundleName.c_str(), uid,
              static_cast<int32_t>(isBg));
    UpdateIsBgLocked(uid, isBg);
}

void BackgroundLoaderMgr::HandlePerceivableIntercept(int32_t uid, BlockListReason reason)
{
    std::string bundleName = "";
    int32_t appIndex = 0;
    TaskInfo taskInfoCopy;
    bool shouldIntercept = CheckPerceivableInterceptLocked(uid, bundleName, appIndex, taskInfoCopy);
    if (!shouldIntercept) {
        BL_HILOGD("shouldIntercept is false, skip intercept, uid: %{public}d", uid);
        return;
    }
    int32_t reasonCode = static_cast<int32_t>(reason);
    BL_HILOGI("Perceivable Intercept: %{public}s, appIndex: %{public}d, reason: %{public}d", bundleName.c_str(),
              appIndex, reasonCode);
    AddToBlockList(bundleName, uid, reasonCode);
#ifdef WORKSCHEDULER_AUDIO_FRAMEWORK_ENABLE
    // 仅音频播放场景需要在查杀前静音， 避免进程退出过程中音频继续播放
    if (reason == BlockListReason::AUDIO_RENDER) {
        int32_t muteRet = OHOS::AudioStandard::AudioSystemManager::GetInstance()->SetAppVolumeMuted(uid, true);
        if (muteRet != ERR_OK) {
            BL_HILOGE("SetAppVolumeMuted faild, uid: %{public}d, ret: %{public}d", uid, muteRet);
        } else {
            BL_HILOGE("SetAppVolumeMuted success, uid: %{public}d", uid);
        }
    }
#endif
    KillApplicationByBundleName(bundleName, appIndex);
    ReportTaskChangeEvent(bundleName, appIndex, static_cast<int32_t>(ResType::BackgroundLoaderState::DELETE));
    ReportTaskFinishEvent(bundleName, appIndex, 1, E_CHECK_WORKINFO_FAILED);
}

void BackgroundLoaderMgr::KillApplicationByBundleName(const std::string &bundleName, int32_t appIndex)
{
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        BL_HILOGE("appMgrClient is null, cannot kill application");
        return;
    }
    int32_t ret = appMgrClient->KillApplication(bundleName, false, appIndex);
    if (ret != 0) {
        BL_HILOGE("KillApplication failed for %{public}s, ret: %{public}d", bundleName.c_str(), ret);
        return;
    }
    BL_HILOGE("KillApplication success for %{public}s, appIndex: %{public}d", bundleName.c_str(), appIndex);
}

bool BackgroundLoaderMgr::HasPerceivableSensitivePermission(uint32_t tokenID)
{
    if (Security::AccessToken::PrivacyKit::IsAllowedUsingPermission(
        tokenID, std::string(OHOS_PERMISSION_CAMERA_BACKGROUND))) {
        BL_HILOGW("app has CEMERA_BACKGROUND permission");
        return true;
    }
    if (Security::AccessToken::PrivacyKit::IsAllowedUsingPermission(
        tokenID, std::string(OHOS_PERMISSION_MICROPHONE_BACKGROUND))) {
        BL_HILOGW("app has MICROPHONE_BACKGROUND permission");
        return true;
    }
    if (Security::AccessToken::PrivacyKit::IsAllowedUsingPermission(
        tokenID, std::string(OHOS_PERMISSION_LOCATION_IN_BACKGROUND))) {
        BL_HILOGW("app has LOACTION_IN_BACKGROUND permission");
        return true;
    }
    return false;
}

void BackgroundLoaderMgr::AddToBlockListLocked(const std::string &bundleName)
{
    std::lock_guard<ffrt::mutex> lock(blockListLock_);
    blockLists_.insert(bundleName);
}

void BackgroundLoaderMgr::AddToBlockList(const std::string &bundleName, int32_t uid, int32_t reason)
{
    BL_HILOGI("add to blocklist: %{public}s, reason: %{public}d, uid: %{public}d", bundleName.c_str(), reason, uid);
    AddToBlockListLocked(bundleName);
    TaskInfo taskInfoCopy;
    if (!MarkTaskBlockListLocked(uid, taskInfoCopy)) {
        return;
    }
    UpsertTaskToDb(uid, taskInfoCopy);
    BackgroundLoaderHiSysEvent::ReportAnomalousSysEvent(taskInfoCopy, std::to_string(reason), "");
    if (dbQueue_ == nullptr) {
        return;
    }
    std::string blockKey = bundleName;
    dbQueue_->submit([this, blockKey]() {
        if (!database_.InsertBlockList(blockKey)) {
            BL_HILOGE("insert blocklist to db failed, key=%{public}s", blockKey.c_str());
        }
    });
}

bool BackgroundLoaderMgr::IsTaskExistLocked(int32_t uid)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    return taskMap_.find(uid) != taskMap_.end();
}

bool BackgroundLoaderMgr::AddNewTaskLocked(int32_t uid, const TaskInfo &taskInfo)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    if (taskMap_.size() >= MAX_TASK_MAP_SIZE) {
        BL_HILOGE("taskMap_ size reaches upper limit %{public}d, reject register", MAX_TASK_MAP_SIZE);
        return false;
    }
    auto [it, inserted] = taskMap_.emplace(uid, taskInfo);
    BL_HILOGI("register success, uid: %{public}d, taskId: %{public}d", uid, taskInfo.taskId_);
    return true;
}

ErrCode BackgroundLoaderMgr::RemoveTaskLocked(const BackgroundLoaderTaskInfo &taskInfo, int32_t uid,
                                              TaskInfo &taskInfoCopy)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end() || it->second.taskId_ != taskInfo.GetTaskId() ||
        it->second.abilityName_ != taskInfo.GetAbilityName()) {
        BL_HILOGE("RemoveTaskLocked failed, task not found, uid: %{public}d, taskId: %{public}d", uid,
                  taskInfo.GetTaskId());
        return E_WORK_NOT_EXIST_FAILED;
    }
    if (it->second.status_ == TaskStatus::RUNNING) {
        BL_HILOGE("RemoveTaskLocked failed, task is running, uid: %{public}d", uid);
        return E_INNER_ERR;
    } else if (it->second.status_ == TaskStatus::IN_BLOCK_LIST) {
        BL_HILOGE("RemoveTaskLocked failed, task in block list, uid: %{public}d", uid);
        return E_INNER_ERR;
    }
    taskInfoCopy = it->second;
    taskMap_.erase(it);
    BL_HILOGI("RemoveTaskLocked success, uid: %{public}d, bundleName: %{public}s",
              uid, taskInfoCopy.bundleName_.c_str());
    return ERR_OK;
}

bool BackgroundLoaderMgr::IsTimeoutTriggeredLocked(int32_t uid, TaskInfo &taskInfoCopy, bool &shouldAddToBlockList)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end()) {
        BL_HILOGE("IsTimeoutTriggeredLocked: task not found, uid: %{public}d", uid);
        return false;
    }
    if (it->second.status_ != TaskStatus::RUNNING) {
        BL_HILOGE("IsTimeoutTriggeredLocked: task already finish, uid: %{public}d, status: %{public}d", uid,
                  static_cast<int32_t>(it->second.status_));
        return false;
    }
    it->second.timeoutCount_++;
    shouldAddToBlockList = (it->second.timeoutCount_ >= maxTimeoutCount_);
    taskInfoCopy = it->second;
    BL_HILOGE("uid: %{public}d, timeoutCount: %{public}d, max: %{public}d, shouldAddToBlockList: %{public}d",
              uid, it->second.timeoutCount_, maxTimeoutCount_, static_cast<int32_t>(shouldAddToBlockList));
    return true;
}

void BackgroundLoaderMgr::MarkTimeOutStatusLocked(int32_t uid, bool shouldAddToBlockList, TaskInfo &taskInfoCopy)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end()) {
        BL_HILOGE("MarkTimeOutStatusLocked: task not found, uid: %{public}d", uid);
        return;
    }
    // 防止异常场景提前加黑后被刷为finish
    if (it->second.status_ == TaskStatus::IN_BLOCK_LIST) {
        taskInfoCopy = it->second;
        return;
    }
    if (shouldAddToBlockList) {
        it->second.status_ = TaskStatus::IN_BLOCK_LIST;
        BL_HILOGI("status -> IN_BLOCK_LIST, uid: %{public}d", uid);
    } else {
        it->second.status_ = TaskStatus::FINISHED;
        BL_HILOGI("status -> FINISHED, uid: %{public}d", uid);
    }
    taskInfoCopy = it->second;
}

bool BackgroundLoaderMgr::MarkTaskRunningLocked(int32_t uid, TaskInfo &taskInfoCopy)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end()) {
        BL_HILOGE("MarkTaskRunningLocked: task not found, uid: %{public}d", uid);
        return false;
    }
    it->second.status_ = TaskStatus::RUNNING;
    taskInfoCopy = it->second;
    BL_HILOGI("MarkTaskRunningLocked: status -> RUNNING, uid: %{public}d", uid);
    return true;
}

bool BackgroundLoaderMgr::MarkTaskFinishedLocked(int32_t uid, TaskInfo &taskInfoCopy)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end() || it->second.status_ != TaskStatus::RUNNING) {
        BL_HILOGE("MarkTaskFinishedLocked: task not found or not running, uid: %{public}d", uid);
        return false;
    }
    it->second.status_ = TaskStatus::FINISHED;
    taskInfoCopy = it->second;
    BL_HILOGI("MarkTaskRunningLocked: status -> FINISHED, uid: %{public}d", uid);
    return true;
}

bool BackgroundLoaderMgr::MarkTaskBlockListLocked(int32_t uid, TaskInfo &taskInfoCopy)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end()) {
        BL_HILOGE("MarkTaskRunningLocked: task not found, uid: %{public}d", uid);
        return false;
    }
    it->second.status_ = TaskStatus::IN_BLOCK_LIST;
    taskInfoCopy = it->second;
    BL_HILOGI("MarkTaskRunningLocked: status -> IN_BLOCK_LIST, uid: %{public}d", uid);
    return true;
}

bool BackgroundLoaderMgr::EraseTaskByKeyLocked(int32_t uid)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end()) {
        BL_HILOGE("EraseTaskByKeyLocked: task not found, uid: %{public}d", uid);
        return false;
    }
    taskMap_.erase(it);
    BL_HILOGI("EraseTaskByKeyLocked success, uid: %{public}d", uid);
    return true;
}

void BackgroundLoaderMgr::UpdateIsBgLocked(int32_t uid, bool isBg)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it != taskMap_.end()) {
        it->second.isBg_ = isBg;
        BL_HILOGD("UpdateIsBgLocked: isBg: %{public}d, uid: %{public}d", static_cast<int32_t>(isBg), uid);
    }
}

bool BackgroundLoaderMgr::UpdatePidLocked(int32_t uid, int32_t pid, TaskInfo &taskInfoCopy)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end()) {
        BL_HILOGD("UpdatePidLocked fail: task not found, uid: %{public}d", uid);
        return false;
    }
    it->second.pid_ = pid;
    taskInfoCopy = it->second;
    BL_HILOGI("Update pid: %{public}d, uid: %{public}d", pid, uid);
    return true;
}

bool BackgroundLoaderMgr::SetAudioStreamLocked(int32_t uid, bool hasAudioStream)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end()) {
        BL_HILOGD("SetAudioStreamLocked: task not found, uid: %{public}d", uid);
        return false;
    }
    it->second.hasAudioStream_ = hasAudioStream;
    BL_HILOGD("SetAudioStreamLocked: hasAudioStream: %{public}d, uid: %{public}d",
              static_cast<int32_t>(hasAudioStream), uid);
    return true;
}

bool BackgroundLoaderMgr::SetShortTaskMarkLocked(int32_t uid, bool hasTask)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it != taskMap_.end()) {
        it->second.hasShortTask_ = hasTask;
        BL_HILOGD("SetLongTaskMarkLocked: hasLongTask: %{public}d, uid: %{public}d",
                  static_cast<int32_t>(hasTask), uid);
        return true;
    }
    return false;
}

bool BackgroundLoaderMgr::SetLongTaskMarkLocked(int32_t uid, bool hasTask)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it != taskMap_.end()) {
        it->second.hasLongTask_ = hasTask;
        BL_HILOGD("SetLongTaskMarkLocked: hasShortTask: %{public}d, uid: %{public}d",
                  static_cast<int32_t>(hasTask), uid);
        return true;
    }
    return false;
}

ErrCode BackgroundLoaderMgr::GetTaskInfoLocked(int32_t uid, int32_t taskId,
                                               BackgroundLoaderTaskInfo &taskInfo, std::string &bundleName)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end() || it->second.taskId_ != taskInfo.GetTaskId()) {
        BL_HILOGE("GetTaskInfoLocked failed, task not found, uid: %{public}d", uid);
        return E_WORK_NOT_EXIST_FAILED;
    }
    // 黑名单中的应用， 获取信息时需要返回错误码
    if (it->second.status_ == TaskStatus::IN_BLOCK_LIST) {
        BL_HILOGE("GetTaskInfoLocked failed, task is in block list, uid: %{public}d", uid);
        return E_INNER_ERR;
    }
    TaskInfo info = it->second;
    bundleName = info.bundleName_;
    BackgroundLoaderTaskInfo newInfo(info.taskId_, info.abilityName_);
    taskInfo = newInfo;
    BL_HILOGI("GetTaskInfoLocked success, taskId: %{public}d, abilityName: %{public}s, status: %{public}d",
              info.taskId_, info.abilityName_.c_str(), static_cast<int32_t>(info.status_));
    return ERR_OK;
}

bool BackgroundLoaderMgr::FindUidLocked(const std::string &bundleName, int32_t appIndex, int32_t &uid)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    for (auto &item : taskMap_) {
        if (item.second.bundleName_ == bundleName && item.second.appIndex_ == appIndex) {
            uid = item.first;
            return true;
        }
    }
    return false;
}

bool BackgroundLoaderMgr::CanBackGroundLoadLocked(const std::string &bundleName, int32_t appIndex, int32_t &uid)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    for (auto &item : taskMap_) {
        if (item.second.bundleName_ != bundleName || item.second.appIndex_ != appIndex) {
            continue;
        }
        // 仅当任务状态为NOT_STARTED 或 FINISHED时才允许触发后台加载任务，不重复触发/加黑不触发
        if (item.second.status_ != TaskStatus::NOT_STARTED && item.second.status_ != TaskStatus::FINISHED) {
            BL_HILOGI("cannot start: status: %{public}d, skip, uid: %{public}d",
                      static_cast<int32_t>(item.second.status_), item.first);
            return false;
        }
        if (!item.second.isBg_) {
            BL_HILOGI("cannot start: in foreground, skip, bundleName: %{public}s, appIndex: %{public}d",
                      bundleName.c_str(), appIndex);
            return false;
        }
        if (!item.second.hasShortTask_) {
            BL_HILOGI("cannot start: has short task, skip, bundleName: %{public}s, appIndex: %{public}d",
                      bundleName.c_str(), appIndex);
            return false;
        }
        if (!item.second.hasLongTask_) {
            BL_HILOGI("cannot start: has long task, skip, bundleName: %{public}s, appIndex: %{public}d",
                      bundleName.c_str(), appIndex);
            return false;
        }
        if (!item.second.hasAudioStream_) {
            BL_HILOGI("cannot start: has audio stream, skip, bundleName: %{public}s, appIndex: %{public}d",
                      bundleName.c_str(), appIndex);
            return false;
        }
        uid = item.first;
        return true;
    }
    BL_HILOGI("cannot start: task not found, bundleName: %{public}s, appIndex: %{public}d",
              bundleName.c_str(), appIndex);
    return false;
}

bool BackgroundLoaderMgr::CheckPerceivableInterceptLocked(int32_t uid, std::string &bundleName, int32_t &appIndex,
                                                          TaskInfo &taskInfoCopy)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end()) {
        BL_HILOGD("CheckPerceivableInterceptLocked: task not found, uid: %{public}d", uid);
        return false;
    }
    if (it->second.status_ != TaskStatus::RUNNING) {
        BL_HILOGD("CheckPerceivableInterceptLocked: task not runnning, uid: %{public}d", uid);
        return false;
    }
    if (!it->second.isBg_) {
        BL_HILOGD("CheckPerceivableInterceptLocked: in foreground, uid: %{public}d", uid);
        return false;
    }
    it->second.status_ = TaskStatus::IN_BLOCK_LIST;
    bundleName = it->second.bundleName_;
    appIndex = it->second.appIndex_;
    taskInfoCopy = it->second;
    return true;
}

ProcessDiedResult BackgroundLoaderMgr::HandleProcessDiedLocked(int32_t uid, int32_t pid, bool isAbnormalExit,
                                                               int32_t &appIndex, TaskInfo &taskInfoCopy)
{
    std::lock_guard<ffrt::mutex> lock(taskLock_);
    auto it = taskMap_.find(uid);
    if (it == taskMap_.end()) {
        BL_HILOGD("HandleProcessDiedLocked: task not found, uid: %{public}d", uid);
        return ProcessDiedResult::NOT_REGISTERED;
    }
    if (pid == DEFAULT_INVAL_VALUE || it->second.pid_ != pid) {
        BL_HILOGD("HandleProcessDiedLocked: pid mismatch(%{public}d, %{public}d), uid: %{public}d",
                  pid, it->second.pid_, uid);
        return ProcessDiedResult::NOT_REGISTERED;
    }
    // 进程死亡无条件刷新 pid 及运行期状态，标记为默认值
    it->second.pid_ = DEFAULT_INVAL_VALUE;
    it->second.isBg_ = true;
    it->second.hasShortTask_ = false;
    it->second.hasLongTask_ = false;
    it->second.hasAudioStream_ = false;
    appIndex = it->second.appIndex_;
    if (it->second.status_ != TaskStatus::RUNNING) {
        taskInfoCopy = it->second;
        BL_HILOGD("HandleProcessDiedLocked: status: %{public}d, uid: %{public}d",
                  static_cast<int32_t>(it->second.status_), uid);
        return ProcessDiedResult::NOT_RUNNING;
    }
    // running状态下异常死亡，加黑处理
    if (isAbnormalExit) {
        it->second.status_ = TaskStatus::IN_BLOCK_LIST;
        taskInfoCopy = it->second;
        BL_HILOGD("HandleProcessDiedLocked: abnormal exit, add to block list, uid: %{public}d", uid);
        return ProcessDiedResult::RUNNING_ABNORMAL;
    }
    // 非异常死亡，状态刷回NOT_STARTED;
    it->second.status_ = TaskStatus::NOT_STARTED;
    taskInfoCopy = it->second;
    BL_HILOGI("HandleProcessDiedLocked: normal exit, reset to NOT_STARTED, uid: %{public}d", uid);
    return ProcessDiedResult::RUNNING_NORMAL;
}

bool BackgroundLoaderMgr::IsInBlockListLocked(const std::string &bundleName)
{
    std::lock_guard<ffrt::mutex> lock(blockListLock_);
    return blockLists_.find(bundleName) != blockLists_.end();
}

bool BackgroundLoaderMgr::RemoveFromBlockListLocked(const std::string &bundleName)
{
    std::lock_guard<ffrt::mutex> lock(blockListLock_);
    auto it = blockLists_.find(bundleName);
    if (it == blockLists_.end()) {
        BL_HILOGD("RemoveFromBlockListLocked: not in blocklist: %{public}s", bundleName.c_str());
        return false;
    }
    blockLists_.erase(it);
    BL_HILOGI("remove from blocklist: %{public}s", bundleName.c_str());
    return true;
}

void BackgroundLoaderMgr::HandleTransientTaskEvent(int64_t value, const nlohmann::json &payload)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderMgr service is not ready");
        return;
    }
    bool isStart = false;
    if (value == ResType::TransientTaskStatus::TRANSIENT_TASK_START ||
        value == ResType::TransientTaskStatus::APP_TRANSIENT_TASK_START) {
        isStart = true;
    } else if (value == ResType::TransientTaskStatus::TRANSIENT_TASK_END ||
               value == ResType::TransientTaskStatus::APP_TRANSIENT_TASK_END) {
        isStart = false;
    } else {
        return;
    }
    int32_t uid = 0;
    std::string bundleName = "";
    if (!ResCommonUtil::ParseIntParameterFromJson("uid", uid, payload) ||
        !ResCommonUtil::ParseStringParameterFromJson("bundleName", bundleName, payload)) {
        BL_HILOGE("HandleTransientTaskEvent parse payload fail");
        return;
    }
    BL_HILOGD("transient task event, bundleName: %{public}s, uid: %{public}d, value: %{public}lld",
              bundleName.c_str(), uid, static_cast<long long>(value));
    SetShortTaskMarkLocked(uid, isStart);
    if (isStart) {
        HandlePerceivableIntercept(uid, BlockListReason::TRANSIENT_TASK);
    }
}

void BackgroundLoaderMgr::HandleContinuousTaskEvent(int64_t value, const nlohmann::json &payload)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderMgr service is not ready");
        return;
    }
    bool isStart = false;
    if (value == ResType::ContinuousTaskStatus::CONTINUOUS_TASK_START) {
        isStart = true;
    } else if (value == ResType::ContinuousTaskStatus::CONTINUOUS_TASK_END) {
        isStart = false;
    } else {
        return;
    }
    int32_t uid = 0;
    std::string bundleName = "";
    if (!ResCommonUtil::ParseIntParameterFromJson("uid", uid, payload) ||
        !ResCommonUtil::ParseStringParameterFromJson("bundleName", bundleName, payload)) {
        BL_HILOGE("HandleContinuousTaskEvent parse payload fail");
        return;
    }
    BL_HILOGD("continuous task event, bundleName: %{public}s, uid: %{public}d, value: %{public}lld",
              bundleName.c_str(), uid, static_cast<long long>(value));
    SetShortTaskMarkLocked(uid, isStart);
    if (isStart) {
        HandlePerceivableIntercept(uid, BlockListReason::CONTINUOUS_TASK);
    }
}

void BackgroundLoaderMgr::HandleAppAudioRenderStateChangeEvent(int64_t value, const nlohmann::json &payload)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderMgr service is not ready");
        return;
    }
    if (value != ResType::AudioStatus::RENDERER_RUNNING &&
        value != ResType::AudioStatus::RENDERER_STOPPED &&
        value != ResType::AudioStatus::RENDERER_RELEASED &&
        value != ResType::AudioStatus::RENDERER_PAUSED) {
        return;
    }
    int32_t uid = 0;
    if (!ResCommonUtil::ParseIntParameterFromJson("uid", uid, payload)) {
        BL_HILOGE("HandleAppAudioRenderStateChangeEvent parse payload fail");
        return;
    }
    BL_HILOGD("audio render event, uid: %{public}d, value: %{public}lld", uid, static_cast<long long>(value));
    // RENDERER_RUNNING: 视为有音频流(互斥不做预取)， 若后台预取过程中运行则强制加黑查杀
    // RENDERER_STOPPED/RENDERER_RELEASED/RENDERER_PAUSED: 视为无音频流(可以做预取)， 后台预取过程不做处理
    bool hasAudioStream = (value == ResType::AudioStatus::RENDERER_RUNNING);
    if (!SetAudioStreamLocked(uid, hasAudioStream)) {
        BL_HILOGD("HandleAppAudioRenderStateChangeEvent: task not registered, uid: %{public}d", uid);
        return;
    }
    if (hasAudioStream) {
        HandlePerceivableIntercept(uid, BlockListReason::AUDIO_RENDER);
    }
}

void BackgroundLoaderMgr::HandleAppAudioCaptureStateChangeEvent(int64_t value, const nlohmann::json &payload)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderMgr service is not ready");
        return;
    }
    if (value != ResType::AudioCaptureState::AUDIO_CAPTURE_BEGIN) {
        return;
    }
    int32_t uid = 0;
    if (!ResCommonUtil::ParseIntParameterFromJson("uid", uid, payload)) {
        BL_HILOGE("HandleAppAudioCaptureStateChangeEvent parse payload fail");
        return;
    }
    HandlePerceivableIntercept(uid, BlockListReason::AUDIO_CAPTURE);
}

void BackgroundLoaderMgr::HandleAppLocationStateEvent(int64_t value, const nlohmann::json &payload)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderMgr service is not ready");
        return;
    }
    if (value != ResType::LocationStatus::APP_LOCATION_STATUE_CHANGE) {
        return;
    }
    std::string state = "";
    if (!ResCommonUtil::ParseStringParameterFromJson("state", state, payload)) {
        BL_HILOGE("HandleAppLocationStateEvent parse state fail");
        return;
    }
    if (state != std::string(LOCATION_STATE_START)) {
        return;
    }
    int32_t uid = 0;
    if (!ResCommonUtil::ParseIntParameterFromJson("uid", uid, payload)) {
        BL_HILOGE("HandleAppLocationStateEvent parse payload fail");
        return;
    }
    HandlePerceivableIntercept(uid, BlockListReason::USE_LOCATION);
}

void BackgroundLoaderMgr::HandleAppTorchStateEvent(int64_t value, const nlohmann::json &payload)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderMgr service is not ready");
        return;
    }
    if (value != ResType::TorchState::TORCH_ON) {
        return;
    }
    int32_t uid = 0;
    if (!ResCommonUtil::ParseIntParameterFromJson("uid", uid, payload)) {
        BL_HILOGE("HandleAppLocationStateEvent parse payload fail");
        return;
    }
    HandlePerceivableIntercept(uid, BlockListReason::USE_TORCH);
}

void BackgroundLoaderMgr::HandleProcessDiedEvent(int64_t value, const nlohmann::json &payload)
{
    if (!isReady_.load()) {
        BL_HILOGE("BackgroundLoaderMgr service is not ready");
        return;
    }
    if (value != ResType::ProcessStatus::PROCESS_DIED) {
        return;
    }
    int32_t uid = DEFAULT_INVAL_VALUE;
    int32_t pid = DEFAULT_INVAL_VALUE;
    std::string bundleName = "";
    int32_t exitReason = 0;
    if (!ResCommonUtil::ParseIntParameterFromJson("uid", uid, payload) ||
        !ResCommonUtil::ParseStringParameterFromJson("bundleName", bundleName, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("pid", uid, payload) ||
        !ResCommonUtil::ParseIntParameterFromJson("exitReason", exitReason, payload)) {
        BL_HILOGE("HandleProcessDiedEvent parse payload fail");
        return;
    }
    bool isAbnormalExit = std::find(ABNORMAL_EXIT_REASON.begin(), ABNORMAL_EXIT_REASON.end(), exitReason) !=
                          ABNORMAL_EXIT_REASON.end();
    int32_t appIndex = DEFAULT_INVAL_VALUE;
    TaskInfo taskInfoCopy;
    ProcessDiedResult result = HandleProcessDiedLocked(uid, pid, isAbnormalExit, appIndex, taskInfoCopy);
    // 未注册到TaskMap_ 则无需后续处理
    if (result == ProcessDiedResult::NOT_REGISTERED) {
        return;
    }
    // 进程死亡不管是否running，都移除abilityMap_中对应条目
    RemoveRemoteObjectLocked(bundleName, appIndex);
    // pid及运行期标记已刷新，同步到数据库
    UpsertTaskToDb(uid, taskInfoCopy);
    if (result == ProcessDiedResult::NOT_RUNNING) {
        return;
    }
    // 非RUNNING 状态下进程死亡： 发finish 事件 + 取消延迟超时任务
    ReportTaskFinishEvent(bundleName, appIndex, 1, E_INNER_ERR);
    CancelTimeoutTask(uid);
    // 异常原因死亡： 加黑 + 发 delete事件
    if (result == ProcessDiedResult::RUNNING_ABNORMAL) {
        AddToBlockList(bundleName, uid, static_cast<int32_t>(BlockListReason::APP_CRASH));
        ReportTaskChangeEvent(bundleName, appIndex,
                              static_cast<int32_t>(ResType::BackgroundLoaderState::DELETE));
    }
}

void BackgroundLoaderMgr::ReportTaskChangeEvent(const std::string &bundleName, int32_t appIndex,
                                                int32_t state)
{
    nlohmann::json payload;
    payload["bundleName"] = bundleName;
    payload["appIndex"] = std::to_string(appIndex);
    ReportDataInProcess(ResType::RES_TYPE_BACKGROUND_LOADER_CHANGE_EVENT, state, payload);
}

void BackgroundLoaderMgr::ReportTaskFinishEvent(const std::string &bundleName, int32_t appIndex,
                                                int64_t value, int32_t errCode)
{
    nlohmann::json payload;
    payload["bundleName"] = bundleName;
    payload["appIndex"] = std::to_string(appIndex);
    payload["errCode"] = std::to_string(errCode);
    ReportDataInProcess(ResType::RES_TYPE_BACKGROUND_LOADER_TASK_FINISH, value, payload);
}

}  // namespace WorkScheduler
}  // namespace OHOS