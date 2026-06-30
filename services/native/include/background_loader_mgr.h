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

#include <nocopyable.h>

#include "work_sched_errors.h"
#include "background_loader_task_info.h"
#include "single_instance.h"
#include "ffrt.h"
#include "iremote_object.h"
#include "res_data.h"

namespace OHOS {
namespace WorkScheduler {

namespace {
static constexpr int32_t BACKGROUND_LOADER_TIMEOUT_MS = 15000;
static constexpr int32_t BACKGROUND_LOADER_TIMEOUT_COUNT = 3;
}

enum class TaskStatus : int32_t {
    NOT_STARTED = 0,
    RUNNING = 1,
    FINISHED = 2
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
};

class BackgroundLoaderMgr {
DECLARE_SINGLE_INSTANCE(BackgroundLoaderMgr);
public:
    void Init(int32_t maxTimeoutCount, int32_t backgroundLoaderTimeoutMs);
    ErrCode RegisterTask(const TaskInfo& taskInfo);
    ErrCode UnregisterTask(const TaskInfo& taskInfo);
    ErrCode FinishTask(const TaskInfo& taskInfo);
    ErrCode GetTaskInfo(int32_t taskId, const std::string& bundleName, int32_t appIndex,
        BackgroundLoaderTaskInfo& taskInfo);
    int32_t GetTaskId(const std::string& bundleName, const std::string& abilityName, int32_t appIndex);
    void HandleBackgroundLoaderTask(const std::shared_ptr<ResourceSchedule::ResData>& resData);
    void SaveRemoteObject(const std::string& bundleName,
        const std::string& abilityName, int32_t appIndex, const sptr<IRemoteObject>& remoteObject);
    void CheckAndSendOnStop(const std::string& bundleName,
        const std::string& abilityName, int32_t appIndex, int32_t taskId);
    void SendOnStop(const TaskInfo& taskInfo, int32_t stopCode, const std::string& message);
    void PostTimeoutTask(const std::string& bundleName, const std::string& abilityName, int32_t appIndex,
        int32_t taskId);
    void SendOnStart(const sptr<IRemoteObject>& remoteObject, const std::string& bundleName, int32_t appIndex);
    void GetInnerTaskInfo(const std::string& bundleName, int32_t appIndex, TaskInfo& info);
    void RemoveRemoteObject(const std::string& bundleName, const std::string& abilityName, int32_t appIndex);

private:
    std::string GenerateTaskKey(const std::string& bundleName, int32_t appIndex);
    sptr<IRemoteObject> GetRemoteObject(const std::string& bundleName, const std::string& abilityName,
        int32_t appIndex);
    std::atomic<bool> isReady_ {false};
    ffrt::mutex taskLock_;
    std::unordered_map<std::string, TaskInfo> taskMap_;
    ffrt::mutex abilityMapLock_;
    std::unordered_map<std::string, sptr<IRemoteObject>> abilityMap_;
    ffrt::mutex blackListLock_;
    std::unordered_set<std::string> blackLists_;
    int32_t maxTimeoutCount_ = BACKGROUND_LOADER_TIMEOUT_COUNT;
    int32_t backgroundLoaderTimeoutMs_ = BACKGROUND_LOADER_TIMEOUT_MS;
};

}  // namespace WorkScheduler
}  // namespace OHOS

#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_MGR_H