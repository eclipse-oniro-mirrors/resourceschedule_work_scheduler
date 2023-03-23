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

#ifndef WORK_SCHED_SERVICES_WORK_STATUS_H
#define WORK_SCHED_SERVICES_WORK_STATUS_H

#include <memory>
#include <string>
#include <map>
#include <mutex>

#include "timer.h"
#include "work_info.h"

namespace OHOS {
namespace WorkScheduler {
class WorkStatus {
public:
    enum Status {
        WAIT_CONDITION = 0,
        CONDITION_READY,
        RUNNING,
        REMOVED
    };
    WorkStatus(WorkInfo &workInfo, int32_t uid);
    ~WorkStatus();

    /**
     * @brief Make work id.
     *
     * @param workId The id of work.
     * @param uid The uid.
     * @return Workid and uid.
     */
    static std::string MakeWorkId(int32_t workId, int32_t uid);

    std::string workId_;
    std::string bundleName_;
    std::string abilityName_;
    int32_t uid_;
    int32_t userId_;
    bool persisted_;
    int32_t priority_;
    bool needRetrigger_ {false};
    int32_t timeRetrigger_ {INT32_MAX};
    std::map<WorkCondition::Type, std::shared_ptr<Condition>> conditionMap_;
    std::shared_ptr<WorkInfo> workInfo_;

    /**
     * @brief Judge state whether is ready.
     *
     * @return True if success,else false.
     */
    bool IsReady();
    /**
     * @brief Judge state whether is ready status.
     *
     * @return True if success,else false.
     */
    bool IsReadyStatus();
    /**
     * @brief Judge state whether is running.
     *
     * @return True if success,else false.
     */
    bool IsRunning();
    /**
     * @brief Judge state whether is removed.
     *
     * @return True if success,else false.
     */
    bool IsRemoved();
    /**
     * @brief Judge state whether is repeating.
     *
     * @return True if success,else false.
     */
    bool IsRepeating();
    /**
     * @brief Judge state whether is last work timeout.
     *
     * @return True if success,else false.
     */
    bool IsLastWorkTimeout();
    /**
     * @brief The OnConditionChanged callback.
     *
     * @param type The type.
     * @param value The value.
     */
    int32_t OnConditionChanged(WorkCondition::Type &type, std::shared_ptr<Condition> value);
    /**
     * @brief Mark round.
     */
    void MarkRound();
    /**
     * @brief Mark status.
     */
    void MarkStatus(Status status);
    /**
     * @brief Get status.
     *
     * @return The current status.
     */
    Status GetStatus();
    /**
     * @brief Dump.
     *
     * @param result The result.
     */
    void Dump(std::string& result);
    /**
     * @brief Update timer if need.
     */
    void UpdateTimerIfNeed();
    /**
     * @brief Need remove.
     *
     * @return True if success,else false.
     */
    bool NeedRemove();

    bool lastTimeout_ {false};
    /**
     * @brief Set min interval by group.
     *
     * @param group The new group.
     * @return True if success,else false.
     */
    bool SetMinIntervalByGroup(int32_t group);
    /**
     * @brief Update map<uid, lastTime>.
     */
    void UpdateUidLastTimeMap();
    /**
     * @brief clear uidLastTimeMap by uid.
     */
    static void ClearUidLastTimeMap(int32_t uid);
    /**
     * @brief Set min interval by dump.
     */
    void SetMinIntervalByDump(int64_t interval);
    /**
     * @brief get min interval.
     */
    int64_t GetMinInterval();
private:
    Status currentStatus_;
    time_t baseTime_;
    int64_t minInterval_;
    bool callbackFlag_;
    static std::mutex s_uid_last_time_mutex;
    static std::map<int32_t, time_t> s_uid_last_time_map;
    void MarkTimeout();
    bool IsSameUser();
    bool SetMinInterval();
    bool IsBatteryAndNetworkReady(WorkCondition::Type type);
    bool IsStorageAndChargerAndTimerReady(WorkCondition::Type type);
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // WORK_SCHED_SERVICES_WORK_STATUS_H