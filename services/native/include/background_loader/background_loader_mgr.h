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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_MGR_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_MGR_H

#include <memory>
#include <mutex>
#include <unordered_map>
#include <string>
#include <unordered_set>
#include <atomic>

#include <nocopyable.h>

#include "work_sched_errors.h"
#include "background_loader_task_info.h"
#include "background_loader/background_loader_database.h"
#include "single_instance.h"
#include "ffrt.h"
#include "iremote_object.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace WorkScheduler {

namespace {
static constexpr int32_t BACKGROUND_LOADER_TIMEOUT_US = 30000000;
static constexpr int32_t BACKGROUND_LOADER_TIMEOUT_COUNT = 3;
}

enum class TaskStatus : int32_t {
    NOT_STARTED = 0,
    RUNNING = 1,
    FINISHED = 2,
    IN_BLOCK_LIST = 3,
};

enum StopCode : uint32_t {
    SUCCESS = 0,
    SYSTEM_ERROR = 1,
    PERCEPTIBLE_ERROR = 2,
    TIMEOUT_ERROR = 3,
    EXECUTE_ERROR = 4
};

struct TaskInfo {
    std::string bundleName_ = "";
    std::string abilityName_ = "";
    int32_t appIndex_ = -1;
    int32_t taskId_ = 0;
    TaskStatus status_ = TaskStatus::NOT_STARTED;
    int32_t uid_ = 0;
    int32_t timeoutCount_ = 0;
    int32_t pid_ = -1;
    bool isBg_ = false;
    bool hasShortTask_ = false;
    bool hasLongTask_ = false;
    bool hasAudioStream_ = false;
};

enum class BlockListReason : int32_t {
    TIMEOUT = 0,  // 预取任务超时次数达到上限
    HAS_CAMERA_BG_PERM = 1,  // 拥有 CEMERA_BACKGROUND 权限
    HAS_MIC_BG_PERM = 2,  // 拥有 MICROPHONE_BACKGROUND 权限
    HAS_LOCATION_BG_PERM = 3,  // 拥有 LOCATION_IN_BACKGROUND 权限
    AUDIO_RENDER = 4,  // 音频播放
    AUDIO_CAPTURE = 5,  // 音频录制
    USE_LOCATION = 6,  // 定位
    USE_TORCH = 7,  //闪光灯
    APP_CRASH = 8,  // 预取过程中应用崩溃
    TRANSIENT_TASK = 9,  // 短时任务
    CONTINUOUS_TASK = 10,  // 长时任务
};

enum class ProcessDiedResult : int32_t {
    NOT_REGISTERED = 0,  // 未注册，无需后续处理
    NOT_RUNNING = 1,  // 已注册但非RUNNING状态
    RUNNING_NORMAL = 2, // RUNNING状态 + 非异常退出
    RUNNING_ABNORMAL = 3,  // RUNNING状态 + 异常退出
};

class BackgroundLoaderMgr {
DECLARE_SINGLE_INSTANCE(BackgroundLoaderMgr);
public:
    void Init(int32_t maxTimeoutCount, int32_t backgroundLoaderTimeoutUs);
    void HandleBackgroundLoaderTask(const nlohmann::json& payload);
    void HandleAbilityConnectDone(const std::string &bundleName, const std::string& abilityName,
        int32_t appIndex, int32_t uid, const sptr<IRemoteObject>& remoteObject);
    void HandleAbilityDisonnectDone(const std::string &bundleName, int32_t appIndex);
    void HandleAppInstallUninstall(int64_t value, const nlohmann::json& payload);
    void HandleAppStateChangeEvent(int64_t value, const nlohmann::json &payload);
    void HandleAppAudioRenderStateChangeEvent(int64_t value, const nlohmann::json &payload);
    void HandleAppAudioCaptureStateChangeEvent(int64_t value, const nlohmann::json &payload);
    void HandleAppLocationStateEvent(int64_t value, const nlohmann::json &payload);
    void HandleAppTorchStateEvent(int64_t value, const nlohmann::json &payload);
    void HandleTransientTaskEvent(int64_t value, const nlohmann::json &payload);
    void HandleContinuousTaskEvent(int64_t value, const nlohmann::json &payload);
    void HandleProcessDiedEvent(int64_t value, const nlohmann::json &payload);
    ErrCode RegisterTaskWithCheck(const BackgroundLoaderTaskInfo& taskInfo);
    ErrCode UnregisterTaskWithCheck(const BackgroundLoaderTaskInfo& taskInfo);
    ErrCode FinishTaskWithCheck(const BackgroundLoaderTaskInfo& taskInfo);
    ErrCode GetTaskInfoWithCheck(int32_t taskId, BackgroundLoaderTaskInfo& taskInfo);

private:
    void ConnectToAbility(const std::string &bundleName, const std::string &abilityName,
        int32_t appIndex, int32_t uid);
    void CheckAndSendOnStop(int32_t uid);
    void SendOnStop(const TaskInfo& taskInfo, int32_t stopCode, const std::string& message);
    void PostTimeoutTask(int32_t uid);
    bool ParseBackgroundLoaderPayload(const nlohmann::json &payload, std::string &bundleName,
        std::string &abilityName, int32_t &appIndex);
    ErrCode HandleExistingTask(const BackgroundLoaderTaskInfo& taskInfo,
        const std::string &bundleName, int32_t uid, TaskInfo &taskInfoCopy);
    ErrCode RegisterNewTask(const BackgroundLoaderTaskInfo& taskInfo,
        const std::string &bundleName, int32_t appIndex, int32_t uid);
    bool VerifyAbilityName(const std::string& bundleName, const std::string& abilityName, int32_t uid);
    bool GetAppIndexAndBundleNameByUid(int32_t uid, int32_t& appIndex, std::string& bundleName);
    bool HasPerceivableSensitivePermission(uint32_t tokenID);
    void ClearAllByUid(int32_t uid, const std::string& bundleName, int32_t appIndex);
    ErrCode DeleteBlockListFromDb(const std::string& bundleName);
    void AddToBlockListLocked(const std::string &bundleName);
    void AddToBlockList(const std::string &bundleName, int32_t uid, int32_t reason);
    bool IsInBlockListLocked(const std::string &bundleName);
    bool RemoveFromBlockListLocked(const std::string &bundleName);
    bool FindUidLocked(const std::string &bundleName, int32_t appIndex, int32_t &uid);
    bool IsTaskExistLocked(int32_t uid);
    bool AddNewTaskLocked(int32_t uid, const TaskInfo &taskInfo);
    ErrCode RemoveTaskLocked(const BackgroundLoaderTaskInfo &taskInfo, int32_t uid, TaskInfo &taskInfoCopy);
    bool IsTimeoutTriggeredLocked(int32_t uid, TaskInfo &taskInfoCopy, bool &shouldAddToBlockList);
    void MarkTimeOutStatusLocked(int32_t uid, bool shouldAddToBlockList, TaskInfo &taskInfoCopy);
    ErrCode FinishTaskLocked(const BackgroundLoaderTaskInfo& taskInfo, int32_t uid, TaskInfo &taskInfoCopy);
    ErrCode GetTaskInfoLocked(int32_t uid, int32_t taskId, BackgroundLoaderTaskInfo &taskInfo, std::string &bundleName);
    bool CanBackGroundLoadLocked(const std::string &bundleName, int32_t appIndex, int32_t &uid);
    bool MarkTaskRunningLocked(int32_t uid, TaskInfo &taskInfoCopy);
    bool MarkTaskFinishedLocked(int32_t uid, TaskInfo &taskInfoCopy);
    bool MarkTaskBlockListLocked(int32_t uid, TaskInfo &taskInfoCopy);
    bool EraseTaskByKeyLocked(int32_t uid);
    void UpdateIsBgLocked(int32_t uid, bool isBg);
    bool UpdatePidLocked(int32_t uid, int32_t pid, TaskInfo &taskInfoCopy);
    bool SetAudioStreamLocked(int32_t uid, bool hasAudioStream);
    bool CheckPerceivableInterceptLocked(int32_t uid, std::string &bundleName, int32_t &appIndex,
        TaskInfo &taskInfoCopy);
    ProcessDiedResult HandleProcessDiedLocked(int32_t uid, int32_t pid,
        bool isAbnormalExit, int32_t &appIndex, TaskInfo &taskInfoCopy);
    void ReportTaskChangeEvent(const std::string &bundleName, int32_t appIndex, int32_t state);
    void ReportTaskFinishEvent(const std::string &bundleName, int32_t appIndex, int64_t value, int32_t errCode);
    std::string GenerateTaskKey(const std::string& bundleName, int32_t appIndex);
    void RemoveRemoteObjectLocked(const std::string& bundleName, int32_t appIndex);
    void SaveRemoteObjectLocked(const std::string& bundleName,
        const std::string& abilityName, int32_t appIndex, const sptr<IRemoteObject>& remoteObject);
    void SendOnStart(const sptr<IRemoteObject>& remoteObject, const std::string& bundleName,
        int32_t appIndex, int32_t uid);
    sptr<IRemoteObject> GetRemoteObjectLocked(const std::string& bundleName, const std::string& abilityName,
        int32_t appIndex);
    void HandlePerceivableIntercept(int32_t uid, BlockListReason reason);
    void KillApplicationByBundleName(const std::string &bundleName, int32_t appIndex);
    void CancelTimeoutTask(int32_t uid);
    bool SetShortTaskMarkLocked(int32_t uid, bool hasTask);
    bool SetLongTaskMarkLocked(int32_t uid, bool hasTask);
    void InitDatabaseAsync();
    void RestoreTasksFromDb();
    void RestoreTaskMapFromDb(const std::vector<TaskRow>& rows);
    void RestoreBlockListFromDb();
    TaskRow ToTaskRow(const TaskInfo& info);
    ErrCode UpsertTaskToDb(int32_t uid, const TaskInfo& taskInfo);
    ErrCode DeleteTaskFromDb(int32_t uid);
    void ReportRestoredTasks(const std::vector<std::pair<std::string, int32_t>>& restoreKeys);
    bool IsResSchedReady();

    std::atomic<bool> isReady_ {false};
    ffrt::mutex taskLock_;
    std::unordered_map<int32_t, TaskInfo> taskMap_;
    ffrt::mutex abilityMapLock_;
    std::unordered_map<std::string, sptr<IRemoteObject>> abilityMap_;
    ffrt::mutex blockListLock_;
    std::unordered_set<std::string> blockLists_;
    std::unique_ptr<ffrt::queue> ffrtQueue_ {nullptr};
    ffrt::mutex timeoutTaskLock_;
    std::unordered_map<int32_t, ffrt::task_handle> timeoutTaskMap_;
    int32_t maxTimeoutCount_ = BACKGROUND_LOADER_TIMEOUT_COUNT;
    int32_t backgroundLoaderTimeoutUs_ = BACKGROUND_LOADER_TIMEOUT_US;
    BackgroundLoaderDatabase database_;
    std::unique_ptr<ffrt::queue> dbQueue_ {nullptr};
    ffrt::task_handle retryReportHandle_ {nullptr};
    std::atomic<int32_t> retryCount_ {0};
};

}  // namespace WorkScheduler
}  // namespace OHOS

#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_MGR_H