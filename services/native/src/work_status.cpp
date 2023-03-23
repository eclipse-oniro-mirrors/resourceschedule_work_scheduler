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

#include "work_status.h"

#include "battery_srv_client.h"
#include "work_sched_errors.h"
#include "work_sched_utils.h"
#include "work_scheduler_service.h"
#include "work_sched_hilog.h"
#include "work_sched_errors.h"
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
#include "bundle_active_client.h"
#include "bundle_active_group_map.h"
#endif

using namespace std;
using namespace OHOS::PowerMgr;

namespace OHOS {
namespace WorkScheduler {
static const double ONE_SECOND = 1000.0;
static bool debugMode = false;
static const int64_t MIN_INTERVAL_DEFAULT = 2 * 60 * 60 * 1000;
std::map<int32_t, time_t> WorkStatus::s_uid_last_time_map;
std::mutex WorkStatus::s_uid_last_time_mutex;
const int32_t DEFAULT_PRIORITY = 100;

time_t getCurrentTime()
{
    time_t result;
    time(&result);
    return result;
}

WorkStatus::WorkStatus(WorkInfo &workInfo, int32_t uid)
{
    this->workInfo_ = make_shared<WorkInfo>(workInfo);
    this->workId_ = MakeWorkId(workInfo.GetWorkId(), uid);
    this->bundleName_ = workInfo.GetBundleName();
    this->abilityName_ = workInfo.GetAbilityName();
    this->baseTime_ = getCurrentTime();
    this->uid_ = uid;
    this->userId_ = WorkSchedUtils::GetUserIdByUid(uid);
    if (workInfo.GetConditionMap()->count(WorkCondition::Type::TIMER) > 0) {
        auto workTimerCondition = workInfo.GetConditionMap()->at(WorkCondition::Type::TIMER);
        shared_ptr<Condition> timeCondition = make_shared<Condition>();
        timeCondition->uintVal = workTimerCondition->uintVal;
        timeCondition->boolVal = workTimerCondition->boolVal;
        if (!workTimerCondition->boolVal) {
            timeCondition->intVal = workTimerCondition->intVal;
        }
        conditionMap_.emplace(WorkCondition::Type::TIMER, timeCondition);
    }
    this->persisted_ = workInfo.IsPersisted();
    this->priority_ = DEFAULT_PRIORITY;
    this->currentStatus_ = WAIT_CONDITION;
    this->minInterval_ = MIN_INTERVAL_DEFAULT;
    this->callbackFlag_ = false;
}

WorkStatus::~WorkStatus() {}

int32_t WorkStatus::OnConditionChanged(WorkCondition::Type &type, shared_ptr<Condition> value)
{
    WS_HILOGD("Work status condition changed.");
    if (workInfo_->GetConditionMap()->count(type) > 0
        && type != WorkCondition::Type::TIMER
        && type != WorkCondition::Type::GROUP) {
        if (conditionMap_.count(type) > 0) {
            conditionMap_.at(type) = value;
        } else {
            conditionMap_.emplace(type, value);
        }
    }
    callbackFlag_ = false;
    if (type == WorkCondition::Type::GROUP && value && value->boolVal) {
        WS_HILOGD("Group changed, bundleName: %{public}s.", value->strVal.c_str());
        callbackFlag_ = true;
        if (value->intVal == userId_ && value->strVal == bundleName_) {
            SetMinIntervalByGroup(value->enumVal);
        } else {
            return E_GROUP_CHANGE_NOT_MATCH_HAP;
        }
    }
    if (IsReady()) {
        MarkStatus(Status::CONDITION_READY);
    }
    return ERR_OK;
}

string WorkStatus::MakeWorkId(int32_t workId, int32_t uid)
{
    return string("u") + to_string(uid) + "_" + to_string(workId);
}

void WorkStatus::MarkTimeout()
{
    lastTimeout_ = true;
}

void WorkStatus::MarkStatus(Status status)
{
    currentStatus_ = status;
}

void WorkStatus::MarkRound() {}

void WorkStatus::UpdateTimerIfNeed()
{
    if (conditionMap_.count(WorkCondition::Type::TIMER) > 0) {
        baseTime_ = getCurrentTime();
        if (conditionMap_.at(WorkCondition::Type::TIMER)->boolVal) {
            return;
        }
        int32_t cycleLeft = conditionMap_.at(WorkCondition::Type::TIMER)->intVal;
        conditionMap_.at(WorkCondition::Type::TIMER)->intVal = cycleLeft - 1;
    }
}

bool WorkStatus::NeedRemove()
{
    if (conditionMap_.count(WorkCondition::Type::TIMER) <= 0) {
        return true;
    }
    if (conditionMap_.at(WorkCondition::Type::TIMER)->boolVal) {
        return false;
    }
    if (conditionMap_.at(WorkCondition::Type::TIMER)->intVal <= 0) {
        return true;
    }
    return false;
}

bool WorkStatus::IsSameUser()
{
    if (userId_ > 0 && !WorkSchedUtils::IsIdActive(userId_)) {
        return false;
    }
    return true;
}

bool WorkStatus::IsReady()
{
    WS_HILOGD("IsReady");
    if (!IsSameUser()) {
        WS_HILOGD("Not same user. WorkId:%{public}s", workId_.c_str());
        return false;
    }
    if (IsRunning()) {
        WS_HILOGD("Work is running");
        return false;
    }
    auto workConditionMap = workInfo_->GetConditionMap();
    for (auto it : *workConditionMap) {
        if (conditionMap_.count(it.first) <= 0) {
            return false;
        }
        if (!IsBatteryAndNetworkReady(it.first) || !IsStorageAndChargerAndTimerReady(it.first)) {
            return false;
        }
    }
    if (DelayedSpSingleton<WorkSchedulerService>::GetInstance()->CheckEffiResApplyInfo(uid_)) {
        return true;
    }
    if (!debugMode && ((!callbackFlag_ && !SetMinInterval()) || minInterval_ == -1)) {
        WS_HILOGE("Work can't ready due to false group, forbidden group or unused group.");
        return false;
    }
    std::lock_guard<std::mutex> lock(s_uid_last_time_mutex);
    auto itMap = s_uid_last_time_map.find(uid_);
    if (itMap == s_uid_last_time_map.end()) {
        return true;
    }
    time_t lastTime = s_uid_last_time_map[uid_];
    double del = difftime(getCurrentTime(), lastTime) * ONE_SECOND;
    WS_HILOGD("CallbackFlag: %{public}d, minInterval = %{public}" PRId64 ", del = %{public}f",
        callbackFlag_, minInterval_, del);

    if (del < minInterval_) {
        needRetrigger_ = true;
        timeRetrigger_ = int(minInterval_ - del + ONE_SECOND);
        return false;
    }
    return true;
}

bool WorkStatus::IsBatteryAndNetworkReady(WorkCondition::Type type)
{
    auto workConditionMap = workInfo_->GetConditionMap();
    switch (type) {
        case WorkCondition::Type::NETWORK: {
            if (conditionMap_.at(type)->enumVal == WorkCondition::Network::NETWORK_UNKNOWN) {
                return false;
            }
            if (workConditionMap->at(type)->enumVal != WorkCondition::Network::NETWORK_TYPE_ANY &&
                workConditionMap->at(type)->enumVal != conditionMap_.at(type)->enumVal) {
                return false;
            }
            break;
        }
        case WorkCondition::Type::BATTERY_STATUS: {
            int32_t batteryReq = workConditionMap->at(type)->enumVal;
            if (batteryReq != WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY &&
                batteryReq != conditionMap_.at(type)->enumVal) {
                return false;
            }
            break;
        }
        case WorkCondition::Type::BATTERY_LEVEL: {
            if (workConditionMap->at(type)->intVal > conditionMap_.at(type)->intVal) {
                return false;
            }
            break;
        }
        default:
            break;
    }
    return true;
}

bool WorkStatus::IsStorageAndChargerAndTimerReady(WorkCondition::Type type)
{
    auto workConditionMap = workInfo_->GetConditionMap();
    switch (type) {
        case WorkCondition::Type::STORAGE: {
            if (workConditionMap->at(type)->enumVal != WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY &&
                workConditionMap->at(type)->enumVal != conditionMap_.at(type)->enumVal) {
                return false;
            }
            break;
        }
        case WorkCondition::Type::CHARGER: {
            auto conditionSet = workConditionMap->at(type);
            auto conditionCurrent = conditionMap_.at(type);
            if (conditionSet->boolVal) {
                if (conditionCurrent->enumVal != conditionSet->enumVal && conditionSet->enumVal !=
                    static_cast<int32_t>(WorkCondition::Charger::CHARGING_PLUGGED_ANY)) {
                    return false;
                }
            } else {
                if (conditionCurrent->enumVal !=
                    static_cast<int32_t>(WorkCondition::Charger::CHARGING_UNPLUGGED)) {
                    return false;
                }
            }
            break;
        }
        case WorkCondition::Type::TIMER: {
            uint32_t intervalTime = workConditionMap->at(WorkCondition::Type::TIMER)->uintVal;
            double del = difftime(getCurrentTime(), baseTime_) * ONE_SECOND;
            WS_HILOGD("del time:%{public}lf, intervalTime:%{public}u", del, intervalTime);
            if (del < intervalTime) {
                return false;
            }
            break;
        }
        default:
            break;
    }
    return true;
}

bool WorkStatus::SetMinInterval()
{
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    int32_t group = 0;
    int32_t errCode = DeviceUsageStats::BundleActiveClient::GetInstance().QueryAppGroup(group, bundleName_, userId_);
    if (errCode != ERR_OK) {
        WS_HILOGE("Query package group failed. userId = %{public}d, bundleName = %{public}s",
            userId_, bundleName_.c_str());
        return false;
    }
#else
    int32_t group = 10;
#endif
    return SetMinIntervalByGroup(group);
}

bool WorkStatus::SetMinIntervalByGroup(int32_t group)
{
    callbackFlag_ = true;
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    auto itMap = DeviceUsageStats::DeviceUsageStatsGroupMap::groupIntervalMap_.find(group);
    if (itMap != DeviceUsageStats::DeviceUsageStatsGroupMap::groupIntervalMap_.end()) {
        minInterval_ = DeviceUsageStats::DeviceUsageStatsGroupMap::groupIntervalMap_[group];
    } else {
        minInterval_ = -1;
    }
#else
    minInterval_ = MIN_INTERVAL_DEFAULT;
#endif
    WS_HILOGD("Set min interval to %{public}" PRId64 " by group %{public}d", minInterval_, group);
    return true;
}

void WorkStatus::SetMinIntervalByDump(int64_t interval)
{
    WS_HILOGD("Set min interval by dump to %{public}" PRId64 "", interval);
    debugMode = interval == 0 ? false : true;
    minInterval_ = interval == 0 ? minInterval_ : interval;
}

int64_t WorkStatus::GetMinInterval()
{
    return minInterval_;
}

void WorkStatus::UpdateUidLastTimeMap()
{
    std::lock_guard<std::mutex> lock(s_uid_last_time_mutex);
    time_t lastTime = getCurrentTime();
    s_uid_last_time_map[uid_] = lastTime;
}

void WorkStatus::ClearUidLastTimeMap(int32_t uid)
{
    std::lock_guard<std::mutex> lock(s_uid_last_time_mutex);
    s_uid_last_time_map.erase(uid);
}

bool WorkStatus::IsRunning()
{
    return currentStatus_ == RUNNING;
}

bool WorkStatus::IsReadyStatus()
{
    return currentStatus_ == CONDITION_READY;
}

bool WorkStatus::IsRemoved()
{
    return currentStatus_ == REMOVED;
}

bool WorkStatus::IsLastWorkTimeout()
{
    return lastTimeout_;
}

bool WorkStatus::IsRepeating()
{
    if (conditionMap_.count(WorkCondition::Type::TIMER) <= 0) {
        return false;
    }
    if (conditionMap_.at(WorkCondition::Type::TIMER)->boolVal) {
        return true;
    } else {
        return conditionMap_.at(WorkCondition::Type::TIMER)->intVal > 0;
    }
}

WorkStatus::Status WorkStatus::GetStatus()
{
    return currentStatus_;
}

void WorkStatus::Dump(string& result)
{
    result.append("{\n");
    result.append(string("\"workId\":") + workId_ + ",\n");
    result.append(string("\"bundleName\":") + bundleName_ + ",\n");
    result.append(string("\"status\":") + to_string(currentStatus_) + ",\n");
    result.append(string("\"conditionMap\":{\n"));
    if (conditionMap_.count(WorkCondition::Type::NETWORK) > 0) {
        result.append(string("\"networkType\":") +
            to_string(conditionMap_.at(WorkCondition::Type::NETWORK)->enumVal) + ",\n");
    }
    if (conditionMap_.count(WorkCondition::Type::CHARGER) > 0) {
        result.append(string("\"isCharging\":") +
            (conditionMap_.at(WorkCondition::Type::CHARGER)->boolVal ? "true" : "false") + ",\n");
        result.append(string("\"chargerType\":") +
            to_string(conditionMap_.at(WorkCondition::Type::CHARGER)->enumVal) + ",\n");
    }
    if (conditionMap_.count(WorkCondition::Type::BATTERY_LEVEL) > 0) {
        result.append(string("\"batteryLevel\":") +
            to_string(conditionMap_.at(WorkCondition::Type::BATTERY_LEVEL)->intVal) + ",\n");
    }
    if (conditionMap_.count(WorkCondition::Type::BATTERY_STATUS) > 0) {
        result.append(string("\"batteryStatus\":") +
            to_string(conditionMap_.at(WorkCondition::Type::BATTERY_STATUS)->enumVal) + ",\n");
    }
    if (conditionMap_.count(WorkCondition::Type::STORAGE) > 0) {
        result.append(string("\"storageLevel\":") +
            to_string(conditionMap_.at(WorkCondition::Type::STORAGE)->enumVal) + ",\n");
    }
    if (conditionMap_.count(WorkCondition::Type::TIMER) > 0) {
        result.append(string("\"baseTime\":") + to_string(baseTime_) + ",\n");
        if (conditionMap_.at(WorkCondition::Type::TIMER)->boolVal) {
            result.append(string("\"isRepeat\": true,\n"));
        } else {
            result.append(string("\"cycleLeft\":") +
                to_string(conditionMap_.at(WorkCondition::Type::TIMER)->intVal) + ",\n");
        }
    }
    result.append("},\n\"workInfo\":\n");
    workInfo_->Dump(result);
    result.append("}\n");
    result.append("\n");
    WS_HILOGD("%s", result.c_str());
}
} // namespace WorkScheduler
} // namespace OHOS