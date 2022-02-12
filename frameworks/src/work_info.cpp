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
#include "work_info.h"

#include "work_sched_common.h"

namespace OHOS {
namespace WorkScheduler {
WorkInfo::WorkInfo() {}

WorkInfo::~WorkInfo() {}

void WorkInfo::SetWorkId(int32_t workId)
{
    workId_ = workId;
}

void WorkInfo::SetElement(std::string bundleName, std::string abilityName)
{
    bundleName_ = bundleName;
    abilityName_ = abilityName;
}

void WorkInfo::RequestPersisted(bool persisted)
{
    persisted_ = persisted;
}

void WorkInfo::RequestNetworkType(WorkCondition::Network condition)
{
    std::shared_ptr<Condition> networkCondition = std::make_shared<Condition>();
    networkCondition->enumVal = static_cast<int>(condition);
    conditionMap_.emplace(WorkCondition::Type::NETWORK, networkCondition);
}

void WorkInfo::RequestChargerType(bool isCharging, WorkCondition::Charger condition)
{
    std::shared_ptr<Condition> chargerCondition = std::make_shared<Condition>();
    chargerCondition->boolVal = isCharging;
    chargerCondition->enumVal = static_cast<int>(condition);
    conditionMap_.emplace(WorkCondition::Type::CHARGER, chargerCondition);
}

void WorkInfo::RequestBatteryLevel(int32_t battLevel)
{
    std::shared_ptr<Condition> batteryLevelCondition = std::make_shared<Condition>();
    batteryLevelCondition->intVal = battLevel;
    conditionMap_.emplace(WorkCondition::Type::BATTERY_LEVEL, batteryLevelCondition);
}

void WorkInfo::RequestBatteryStatus(WorkCondition::BatteryStatus condition)
{
    std::shared_ptr<Condition> batteryCondition = std::make_shared<Condition>();
    batteryCondition->enumVal = static_cast<int>(condition);
    conditionMap_.emplace(WorkCondition::Type::BATTERY_STATUS, batteryCondition);
}

void WorkInfo::RequestStorageLevel(WorkCondition::Storage condition)
{
    std::shared_ptr<Condition> storageCondition = std::make_shared<Condition>();
    storageCondition->enumVal = static_cast<int>(condition);
    conditionMap_.emplace(WorkCondition::Type::STORAGE, storageCondition);
}

void WorkInfo::RequestRepeatCycle(uint32_t timeInterval, uint32_t cycle)
{
    std::shared_ptr<Condition> repeatCycle = std::make_shared<Condition>();
    repeatCycle->uintVal = timeInterval;
    repeatCycle->intVal = cycle;
    repeatCycle->boolVal = false;
    conditionMap_.emplace(WorkCondition::Type::TIMER, repeatCycle);
}

void WorkInfo::RequestRepeatCycle(uint32_t timeInterval)
{
    std::shared_ptr<Condition> repeatCycle = std::make_shared<Condition>();
    repeatCycle->uintVal = timeInterval;
    repeatCycle->boolVal = true;
    conditionMap_.emplace(WorkCondition::Type::TIMER, repeatCycle);
}

int32_t WorkInfo::GetWorkId()
{
    return workId_;
}

std::string WorkInfo::GetBundleName()
{
    return bundleName_;
}

std::string WorkInfo::GetAbilityName()
{
    return abilityName_;
}

bool WorkInfo::IsPersisted()
{
    return persisted_;
}

WorkCondition::Network WorkInfo::GetNetworkType()
{
    if (conditionMap_.count(WorkCondition::Type::NETWORK) > 0) {
        int enumVal = conditionMap_.at(WorkCondition::Type::NETWORK)->enumVal;
        WorkCondition::Network network = WorkCondition::Network(enumVal);
        return WorkCondition::Network(network);
    }
    return WorkCondition::Network::NETWORK_UNKNOWN;
}

WorkCondition::Charger WorkInfo::GetChargerType()
{
    if (conditionMap_.count(WorkCondition::Type::CHARGER) > 0) {
        int enumVal = conditionMap_.at(WorkCondition::Type::CHARGER)->enumVal;
        WorkCondition::Charger charger = WorkCondition::Charger(enumVal);
        return WorkCondition::Charger(charger);
    }
    return WorkCondition::Charger::CHARGING_UNKNOWN;
}

int32_t WorkInfo::GetBatteryLevel()
{
    if (conditionMap_.count(WorkCondition::Type::BATTERY_LEVEL) > 0) {
        return conditionMap_.at(WorkCondition::Type::BATTERY_LEVEL)->intVal;
    }
    return -1;
}

WorkCondition::BatteryStatus WorkInfo::GetBatteryStatus()
{
    if (conditionMap_.count(WorkCondition::Type::BATTERY_STATUS) > 0) {
        int enumVal = conditionMap_.at(WorkCondition::Type::BATTERY_STATUS)->enumVal;
        WorkCondition::BatteryStatus battery = WorkCondition::BatteryStatus(enumVal);
        return WorkCondition::BatteryStatus(battery);
    }
    return WorkCondition::BatteryStatus::BATTERY_UNKNOWN;
}

WorkCondition::Storage WorkInfo::GetStorageLevel()
{
    if (conditionMap_.count(WorkCondition::Type::STORAGE) > 0) {
        int enumVal = conditionMap_.at(WorkCondition::Type::STORAGE)->enumVal;
        WorkCondition::Storage storage = WorkCondition::Storage(enumVal);
        return WorkCondition::Storage(storage);
    }
    return WorkCondition::Storage::STORAGE_UNKNOWN;
}

bool WorkInfo::IsRepeat()
{
    if (conditionMap_.count(WorkCondition::Type::TIMER) > 0) {
        return conditionMap_.at(WorkCondition::Type::TIMER)->boolVal;
    }
    return false;
}

uint32_t WorkInfo::GetTimeInterval()
{
    if (conditionMap_.count(WorkCondition::Type::TIMER) > 0) {
        return conditionMap_.at(WorkCondition::Type::TIMER)->uintVal;
    }
    return -1;
}

std::shared_ptr<std::map<WorkCondition::Type, std::shared_ptr<Condition>>> WorkInfo::GetConditionMap()
{
    return std::make_shared<std::map<WorkCondition::Type, std::shared_ptr<Condition>>>(conditionMap_);
}

bool WorkInfo::Marshalling(Parcel &parcel) const
{
    bool ret = false;
    ret = parcel.WriteInt32(workId_);
    ret = ret && parcel.WriteString(bundleName_);
    ret = ret && parcel.WriteString(abilityName_);
    ret = ret && parcel.WriteBool(persisted_);
    ret = ret && parcel.WriteUint32(conditionMap_.size());
    for (auto it : conditionMap_) {
        switch (it.first) {
            case WorkCondition::Type::NETWORK:
            case WorkCondition::Type::BATTERY_STATUS:
            case WorkCondition::Type::STORAGE: {
                ret = ret && parcel.WriteInt32(it.first);
                ret = ret && parcel.WriteInt32(it.second->enumVal);
                break;
            }
            case WorkCondition::Type::CHARGER: {
                ret = ret && parcel.WriteInt32(it.first);
                ret = ret && parcel.WriteBool(it.second->boolVal);
                ret = ret && parcel.WriteInt32(it.second->enumVal);
                break;
            }
            case WorkCondition::Type::BATTERY_LEVEL: {
                ret = ret && parcel.WriteInt32(it.first);
                ret = ret && parcel.WriteInt32(it.second->intVal);
                break;
            }
            case WorkCondition::Type::TIMER: {
                ret = ret && parcel.WriteInt32(it.first);
                ret = ret && parcel.WriteUint32(it.second->uintVal);
                ret = ret && parcel.WriteBool(it.second->boolVal);
                if (!it.second->boolVal) {
                    ret = ret && parcel.WriteInt32(it.second->intVal);
                }
                break;
            }
            default: {
                ret = false;
            }
        }
    }
    return ret;
}

WorkInfo *WorkInfo::Unmarshalling(Parcel &parcel)
{
    auto *read = new WorkInfo();
    read->workId_ = parcel.ReadInt32();
    read->bundleName_ = parcel.ReadString();
    read->abilityName_ = parcel.ReadString();
    read->persisted_ = parcel.ReadBool();
    size_t mapsize = parcel.ReadUint32();
    read->conditionMap_ = std::map<WorkCondition::Type, std::shared_ptr<Condition>>();
    for (size_t i = 0; i < mapsize; i++) {
        int32_t key = parcel.ReadInt32();
        auto condition = std::make_shared<Condition>();
        switch (key) {
            case WorkCondition::Type::NETWORK:
            case WorkCondition::Type::BATTERY_STATUS:
            case WorkCondition::Type::STORAGE: {
                condition->enumVal = parcel.ReadInt32();
                break;
            }
            case WorkCondition::Type::CHARGER: {
                condition->boolVal = parcel.ReadBool();
                condition->enumVal = parcel.ReadInt32();
                break;
            }
            case WorkCondition::Type::BATTERY_LEVEL: {
                condition->intVal = parcel.ReadInt32();
                break;
            }
            case WorkCondition::Type::TIMER: {
                condition->uintVal = parcel.ReadUint32();
                condition->boolVal = parcel.ReadBool();
                if (!condition->boolVal) {
                    condition->intVal = parcel.ReadInt32();
                }
                break;
            }
            default: {
            }
        }
        read->conditionMap_.emplace(WorkCondition::Type(key), condition);
    }
    return read;
}

std::string WorkInfo::ParseToJsonStr()
{
    Json::Value root;
    root["workId"] = workId_;
    root["bundleName"] = bundleName_;
    root["abilityName"] = abilityName_;
    root["persisted"] = persisted_;
    Json::Value conditions;
    for (auto it : conditionMap_) {
        switch (it.first) {
            case WorkCondition::Type::NETWORK: {
                conditions["network"] = it.second->enumVal;
                break;
            }
            case WorkCondition::Type::CHARGER: {
                conditions["isCharging"] = it.second->boolVal;
                conditions["chargerType"] = it.second->enumVal;
                break;
            }
            case WorkCondition::Type::BATTERY_LEVEL: {
                conditions["batteryLevel"] = it.second->intVal;
                break;
            }
            case WorkCondition::Type::BATTERY_STATUS: {
                conditions["batteryStatus"] = it.second->enumVal;
                break;
            }
            case WorkCondition::Type::STORAGE: {
                conditions["storage"] = it.second->enumVal;
                break;
            }
            case WorkCondition::Type::TIMER: {
                conditions["timer"] = it.second->uintVal;
                conditions["repeat"] = it.second->boolVal;
                if (!it.second->boolVal) {
                    conditions["cycle"] = it.second->intVal;
                }
                break;
            }
            default: {}
        }
    }
    root["conditions"] = conditions;
    Json::StreamWriterBuilder writerBuilder;
    std::ostringstream os;
    std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
    jsonWriter->write(root, &os);
    std::string result = os.str();
    return result;
}

bool WorkInfo::ParseFromJson(const Json::Value value)
{
    if (value.empty()) {
        return false;
    }
    if (!value.isMember("workId") || !value.isMember("bundleName") || !value.isMember("abilityName")) {
        return false;
    }
    this->workId_ = value["workId"].asInt();
    this->bundleName_ = value["bundleName"].asString();
    this->abilityName_ = value["abilityName"].asString();
    this->persisted_ = value["persisted"].asBool();
    Json::Value conditions = value["conditions"];
    if (conditions.isMember("network")) {
        this->RequestNetworkType(WorkCondition::Network(conditions["network"].asInt()));
    }
    if (conditions.isMember("isCharging") && conditions.isMember("chargerType")) {
        this->RequestChargerType(conditions["isCharging"].asBool(),
            WorkCondition::Charger(conditions["chargerType"].asInt()));
    }
    if (conditions.isMember("batteryLevel")) {
        this->RequestBatteryLevel(conditions["batteryLevel"].asInt());
    }
    if (conditions.isMember("batteryStatus")) {
        this->RequestBatteryStatus(WorkCondition::BatteryStatus(conditions["batteryStatus"].asInt()));
    }
    if (conditions.isMember("storage")) {
        this->RequestStorageLevel(WorkCondition::Storage(conditions["storage"].asInt()));
    }
    if (conditions.isMember("timer") && conditions.isMember("repeat")) {
        if (!conditions["repeat"].asBool()) {
            this->RequestRepeatCycle(conditions["timer"].asInt());
        } else {
            if (conditions.isMember("cycle")) {
                this->RequestRepeatCycle(conditions["timer"].asInt(), conditions["cycle"].asInt());
            }
        }
    }
    return true;
}

void WorkInfo::Dump(std::string &result)
{
    result.append(ParseToJsonStr());
}
} // namespace WorkScheduler
} // namespace OHOS
