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

#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
const int32_t INVALID_VALUE = -1;
const int32_t INVALID_TIME_VALUE = 0;
const uint32_t MAX_SIZE = 1024;

WorkInfo::WorkInfo()
{
    workId_ = INVALID_VALUE;
    uid_ = INVALID_VALUE;
    persisted_ = false;
    extras_ = nullptr;
}

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
    networkCondition->enumVal = static_cast<int32_t>(condition);
    conditionMap_.emplace(WorkCondition::Type::NETWORK, networkCondition);
}

void WorkInfo::RequestChargerType(bool isCharging, WorkCondition::Charger condition)
{
    std::shared_ptr<Condition> chargerCondition = std::make_shared<Condition>();
    chargerCondition->boolVal = isCharging;
    chargerCondition->enumVal = static_cast<int32_t>(condition);
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
    batteryCondition->enumVal = static_cast<int32_t>(condition);
    conditionMap_.emplace(WorkCondition::Type::BATTERY_STATUS, batteryCondition);
}

void WorkInfo::RequestStorageLevel(WorkCondition::Storage condition)
{
    std::shared_ptr<Condition> storageCondition = std::make_shared<Condition>();
    storageCondition->enumVal = static_cast<int32_t>(condition);
    conditionMap_.emplace(WorkCondition::Type::STORAGE, storageCondition);
}

void WorkInfo::RequestRepeatCycle(uint32_t timeInterval, int32_t cycle)
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

void WorkInfo::RequestExtras(AAFwk::WantParams extras)
{
    extras_ = std::make_shared<AAFwk::WantParams>(extras);
}

void WorkInfo::RefreshUid(int32_t uid)
{
    uid_ = uid;
}

int32_t WorkInfo::GetUid()
{
    return uid_;
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
        int32_t enumVal = conditionMap_.at(WorkCondition::Type::NETWORK)->enumVal;
        WorkCondition::Network network = WorkCondition::Network(enumVal);
        return WorkCondition::Network(network);
    }
    return WorkCondition::Network::NETWORK_UNKNOWN;
}

WorkCondition::Charger WorkInfo::GetChargerType()
{
    if (conditionMap_.count(WorkCondition::Type::CHARGER) > 0) {
        int32_t enumVal = conditionMap_.at(WorkCondition::Type::CHARGER)->enumVal;
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
    return INVALID_VALUE;
}

WorkCondition::BatteryStatus WorkInfo::GetBatteryStatus()
{
    if (conditionMap_.count(WorkCondition::Type::BATTERY_STATUS) > 0) {
        int32_t enumVal = conditionMap_.at(WorkCondition::Type::BATTERY_STATUS)->enumVal;
        WorkCondition::BatteryStatus battery = WorkCondition::BatteryStatus(enumVal);
        return WorkCondition::BatteryStatus(battery);
    }
    return WorkCondition::BatteryStatus::BATTERY_UNKNOWN;
}

WorkCondition::Storage WorkInfo::GetStorageLevel()
{
    if (conditionMap_.count(WorkCondition::Type::STORAGE) > 0) {
        int32_t enumVal = conditionMap_.at(WorkCondition::Type::STORAGE)->enumVal;
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
    return INVALID_TIME_VALUE;
}

int32_t WorkInfo::GetCycleCount()
{
    if (conditionMap_.count(WorkCondition::Type::TIMER) > 0) {
        if (IsRepeat()) {
            return INVALID_VALUE;
        }
        return conditionMap_.at(WorkCondition::Type::TIMER)->intVal;
    }
    return INVALID_VALUE;
}

std::shared_ptr<std::map<WorkCondition::Type, std::shared_ptr<Condition>>> WorkInfo::GetConditionMap()
{
    return std::make_shared<std::map<WorkCondition::Type, std::shared_ptr<Condition>>>(conditionMap_);
}

std::shared_ptr<AAFwk::WantParams> WorkInfo::GetExtras() const
{
    return extras_;
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
    if (extras_) {
        ret = ret && extras_->Marshalling(parcel);
    }
    return ret;
}

sptr<WorkInfo> WorkInfo::Unmarshalling(Parcel &parcel)
{
    sptr<WorkInfo> read = new (std::nothrow) WorkInfo();
    if (read == nullptr) {
        WS_HILOGE("read is nullptr.");
        return nullptr;
    }
    read->workId_ = parcel.ReadInt32();
    read->bundleName_ = parcel.ReadString();
    read->abilityName_ = parcel.ReadString();
    read->persisted_ = parcel.ReadBool();
    uint32_t mapsize = parcel.ReadUint32();
    if (mapsize >= MAX_SIZE) {
        WS_HILOGE("mapsize is too big.");
        return nullptr;
    }

    UnmarshallCondition(parcel, read, mapsize);
    AAFwk::WantParams *wantParams = AAFwk::WantParams::Unmarshalling(parcel);
    if (wantParams != nullptr) {
        read->extras_ = std::make_shared<AAFwk::WantParams>(*wantParams);
    }
    return read;
}

void WorkInfo::UnmarshallCondition(Parcel &parcel, sptr<WorkInfo> &read, uint32_t mapsize)
{
    read->conditionMap_ = std::map<WorkCondition::Type, std::shared_ptr<Condition>>();
    for (uint32_t i = 0; i < mapsize; i++) {
        int32_t key = parcel.ReadInt32();
        auto condition = std::make_shared<Condition>();
        switch (key) {
            case WorkCondition::Type::NETWORK:
            // fall-through
            case WorkCondition::Type::BATTERY_STATUS:
            // fall-through
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
}

std::string WorkInfo::ParseToJsonStr()
{
    Json::Value root;
    if (uid_ != INVALID_VALUE) {
        root["uid"] = uid_;
    }
    root["workId"] = workId_;
    root["bundleName"] = bundleName_;
    root["abilityName"] = abilityName_;
    root["persisted"] = persisted_;
    ParseConditionToJsonStr(root);
    if (extras_) {
        Json::Value extras;
        Json::Value extrasType;
        std::map<std::string, sptr<AAFwk::IInterface>> extrasMap = extras_->GetParams();
        int typeId = INVALID_VALUE;
        for (auto it : extrasMap) {
            typeId = AAFwk::WantParams::GetDataType(it.second);
            extrasType[it.first] = typeId;
            if (typeId != INVALID_VALUE) {
                std::string value = AAFwk::WantParams::GetStringByType(it.second, typeId);
                extras[it.first] = value;
            } else {
                WS_HILOGE("parameters: type error.");
            }
        }
        root["parameters"] = extras;
        root["parametersType"] = extrasType;
    }
    Json::StreamWriterBuilder writerBuilder;
    std::ostringstream os;
    std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
    jsonWriter->write(root, &os);
    std::string result = os.str();
    return result;
}

void WorkInfo::ParseConditionToJsonStr(Json::Value &root)
{
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
    ParseConditionFromJsonStr(value);
    if (!value.isMember("parameters")) {
        return true;
    }
    Json::Value extrasJson = value["parameters"];
    Json::Value extrasType = value["parametersType"];
    AAFwk::WantParams extras;
    Json::Value::Members keyList = extrasJson.getMemberNames();
    int typeId = INVALID_VALUE;
    for (auto key : keyList) {
        typeId = extrasType[key].asInt();
        if (typeId != INVALID_VALUE) {
            sptr<AAFwk::IInterface> exInterface = AAFwk::WantParams::GetInterfaceByType(typeId,
                extrasJson[key].asString());
            extras.SetParam(key, exInterface);
        }
    }
    this->RequestExtras(extras);
    return true;
}

void WorkInfo::ParseConditionFromJsonStr(const Json::Value value)
{
    if (value.isMember("uid")) {
        this->uid_ = value["uid"].asInt();
    }
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
        if (conditions["repeat"].asBool()) {
            this->RequestRepeatCycle(conditions["timer"].asInt());
        } else {
            if (conditions.isMember("cycle")) {
                this->RequestRepeatCycle(conditions["timer"].asInt(), conditions["cycle"].asInt());
            }
        }
    }
}

void WorkInfo::Dump(std::string &result)
{
    result.append(ParseToJsonStr());
}
} // namespace WorkScheduler
} // namespace OHOS
