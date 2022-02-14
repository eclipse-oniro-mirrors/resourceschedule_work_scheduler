/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "work_sched_common.h"
#include "work_sched_utils.h"

using namespace std;
using namespace OHOS::PowerMgr;

namespace OHOS {
namespace WorkScheduler {
static const int ONE_SECOND = 1000;

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
        if (!workTimerCondition->boolVal) {
            timeCondition->intVal = workTimerCondition->intVal;
        }
        conditionMap_.emplace(WorkCondition::Type::TIMER, timeCondition);
    }
    this->persisted_ = workInfo.IsPersisted();
    this->priority_ = DEFAULT_PRIORITY;
    this->currentStatus_ = WAIT_CONDITION;
}

WorkStatus::~WorkStatus() {}

void WorkStatus::OnConditionChanged(WorkCondition::Type &type, shared_ptr<Condition> value)
{
    if (workInfo_->GetConditionMap()->count(type) > 0
        && type != WorkCondition::Type::TIMER) {
        if (conditionMap_.count(type) > 0) {
            conditionMap_.at(type) = value;
        } else {
            conditionMap_.emplace(type, value);
        }
    }
    if (IsReady()) {
        MarkStatus(Status::CONDITION_READY);
    }
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
        int cycleLeft = conditionMap_.at(WorkCondition::Type::TIMER)->intVal;
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
    if (WorkSchedUtils::GetCurrentAccountId() != userId_) {
        return false;
    }
    return true;
}

bool WorkStatus::IsReady()
{
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
        if (conditionMap_.count(it.first) <= 0 || workConditionMap == nullptr) {
            return false;
        }
        switch (it.first) {
            case WorkCondition::Type::NETWORK:
            case WorkCondition::Type::BATTERY_STATUS:
            case WorkCondition::Type::STORAGE: {
                if (workConditionMap->at(it.first)->enumVal != conditionMap_.at(it.first)->enumVal) {
                    return false;
                }
                break;
            }
            case WorkCondition::Type::CHARGER: {
                auto conditionSet = workConditionMap->at(it.first);
                auto conditionCurrent = conditionMap_.at(it.first);
                if (conditionSet->boolVal) {
                    if (conditionCurrent->enumVal != conditionSet->enumVal &&
                        conditionSet->enumVal !=
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
            case WorkCondition::Type::BATTERY_LEVEL: {
                if (workConditionMap->at(it.first)->intVal > conditionMap_.at(it.first)->intVal) {
                    return false;
                }
                break;
            }
            case WorkCondition::Type::TIMER: {
                uint32_t intervalTime = workConditionMap->at(WorkCondition::Type::TIMER)->uintVal;
                double del = difftime(getCurrentTime(), baseTime_) * ONE_SECOND;
                WS_HILOGD("del time:%{public}f, intervalTime:%{public}d", del, intervalTime);
                if (del < intervalTime) {
                    return false;
                }
                break;
            }
            default:
                break;
        }
    }
    return true;
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