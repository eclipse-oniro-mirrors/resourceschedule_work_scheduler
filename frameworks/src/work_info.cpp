/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "work_sched_constants.h"

namespace OHOS {
namespace WorkScheduler {
const int32_t INVALID_TIME_VALUE = 0;
const uint32_t MAX_SIZE = 1024;
const int32_t APPINDEX_INIT_VALUE = 0;

WorkInfo::WorkInfo()
{
    workId_ = INVALID_VALUE;
    uid_ = INVALID_VALUE;
    persisted_ = false;
    extras_ = nullptr;
    appIndex_ = APPINDEX_INIT_VALUE;
    extension_ = true;
    saId_ = INVALID_VALUE;
    residentSa_ = false;
    time(&createTime_);
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

void WorkInfo::RequestBaseTimeAndCycle(time_t baseTime, int32_t cycle)
{
    if (conditionMap_.count(WorkCondition::Type::TIMER) > 0) {
        conditionMap_.at(WorkCondition::Type::TIMER)->timeVal = baseTime;
        conditionMap_.at(WorkCondition::Type::TIMER)->intVal = cycle;
    }
}

void WorkInfo::RequestBaseTime(time_t baseTime)
{
    if (conditionMap_.count(WorkCondition::Type::TIMER) > 0) {
        conditionMap_.at(WorkCondition::Type::TIMER)->timeVal = baseTime;
    }
}

void WorkInfo::RequestExtras(AAFwk::WantParams extras)
{
    extras_ = std::make_shared<AAFwk::WantParams>(extras);
}

void WorkInfo::RefreshUid(int32_t uid)
{
    uid_ = uid;
}

void WorkInfo::RefreshAppIndex(int32_t appIndex)
{
    appIndex_ = appIndex;
}

void WorkInfo::RefreshExtension(bool extension)
{
    extension_ = extension;
}

void WorkInfo::RequestDeepIdle(bool deepIdle)
{
    std::shared_ptr<Condition> deepIdleCondition = std::make_shared<Condition>();
    deepIdleCondition->boolVal = deepIdle;
    conditionMap_.emplace(WorkCondition::Type::DEEP_IDLE, deepIdleCondition);
}

void WorkInfo::SetCallBySystemApp(bool callBySystemApp)
{
    callBySystemApp_ = callBySystemApp;
}

void WorkInfo::SetPreinstalled(bool preinstalled)
{
    preinstalled_ = preinstalled;
}

bool WorkInfo::IsPreinstalled()
{
    return preinstalled_;
}

std::string WorkInfo::GetUriKey()
{
    return uriKey_;
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

int32_t WorkInfo::GetAppIndex() const
{
    return appIndex_;
}

bool WorkInfo::GetExtension() const
{
    return extension_;
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

time_t WorkInfo::GetBaseTime()
{
    if (conditionMap_.count(WorkCondition::Type::TIMER) > 0) {
        return conditionMap_.at(WorkCondition::Type::TIMER)->timeVal;
    }
    time_t result;
    time(&result);
    return result;
}

WorkCondition::DeepIdle WorkInfo::GetDeepIdle()
{
    if (conditionMap_.count(WorkCondition::Type::DEEP_IDLE) <= 0) {
        return WorkCondition::DeepIdle::DEEP_IDLE_UNKNOWN;
    }
    return conditionMap_.at(WorkCondition::Type::DEEP_IDLE)->boolVal ?
        WorkCondition::DeepIdle::DEEP_IDLE_IN : WorkCondition::DeepIdle::DEEP_IDLE_OUT;
}

std::shared_ptr<std::map<WorkCondition::Type, std::shared_ptr<Condition>>> WorkInfo::GetConditionMap()
{
    return std::make_shared<std::map<WorkCondition::Type, std::shared_ptr<Condition>>>(conditionMap_);
}

std::shared_ptr<AAFwk::WantParams> WorkInfo::GetExtras() const
{
    return extras_;
}

bool WorkInfo::IsCallBySystemApp()
{
    return callBySystemApp_;
}

bool WorkInfo::Marshalling(Parcel &parcel) const
{
    bool ret = false;
    ret = parcel.WriteInt32(workId_);
    ret = ret && parcel.WriteString(bundleName_);
    ret = ret && parcel.WriteString(abilityName_);
    ret = ret && parcel.WriteInt32(earliestStartTime_);
    ret = ret && parcel.WriteBool(persisted_);
    ret = ret && parcel.WriteInt32(uid_);
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
            case WorkCondition::Type::CHARGER:
            case WorkCondition::Type::DEEP_IDLE: {
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
    ret = ret && parcel.WriteBool(extras_ ? true : false);
    if (extras_) {
        ret = ret && extras_->Marshalling(parcel);
    }
    return ret;
}

WorkInfo* WorkInfo::Unmarshalling(Parcel &parcel)
{
    auto read = new (std::nothrow) WorkInfo();
    if (read == nullptr) {
        WS_HILOGE("read is nullptr.");
        return nullptr;
    }
    if (!parcel.ReadInt32(read->workId_) || !parcel.ReadString(read->bundleName_) ||
        !parcel.ReadString(read->abilityName_)) {
        WS_HILOGE("Failed to read the workId or bundleName or abilityName.");
        delete read;
        return nullptr;
    }
    if (!parcel.ReadInt32(read->earliestStartTime_)) {
        WS_HILOGE("Failed to read the earliestStartTime.");
        delete read;
        return nullptr;
    }
    if (!parcel.ReadBool(read->persisted_)) {
        WS_HILOGE("Failed to read the persisted.");
        delete read;
        return nullptr;
    }
    if (!parcel.ReadInt32(read->uid_)) {
        WS_HILOGE("Failed to read the uid.");
        delete read;
        return nullptr;
    }
    uint32_t mapsize;
    if (!parcel.ReadUint32(mapsize) || mapsize >= MAX_SIZE) {
        WS_HILOGE("Failed to read the mapsize or mapsize is too big.");
        delete read;
        return nullptr;
    }
    if (!UnmarshallCondition(parcel, read, mapsize)) {
        WS_HILOGE("Failed to read the work condition map.");
        delete read;
        return nullptr;
    }
    bool hasExtras;
    if (!parcel.ReadBool(hasExtras)) {
        WS_HILOGE("Failed to read the extras existence.");
        delete read;
        return nullptr;
    }
    if (!hasExtras) {
        return read;
    }
    AAFwk::WantParams *wantParams = AAFwk::WantParams::Unmarshalling(parcel);
    if (wantParams != nullptr) {
        read->extras_ = std::make_shared<AAFwk::WantParams>(*wantParams);
    }
    return read;
}

bool WorkInfo::UnmarshallCondition(Parcel &parcel, WorkInfo* read, uint32_t mapsize)
{
    read->conditionMap_ = std::map<WorkCondition::Type, std::shared_ptr<Condition>>();
    for (uint32_t i = 0; i < mapsize; i++) {
        int32_t key;
        if (!parcel.ReadInt32(key)) {
            return false;
        }
        auto condition = std::make_shared<Condition>();
        switch (key) {
            case WorkCondition::Type::NETWORK:
            case WorkCondition::Type::BATTERY_STATUS:
            case WorkCondition::Type::STORAGE: {
                if (!parcel.ReadInt32(condition->enumVal)) {
                    return false;
                }
                break;
            }
            case WorkCondition::Type::DEEP_IDLE:
            case WorkCondition::Type::CHARGER: {
                if (!parcel.ReadBool(condition->boolVal) || !parcel.ReadInt32(condition->enumVal)) {
                    return false;
                }
                break;
            }
            case WorkCondition::Type::BATTERY_LEVEL: {
                if (!parcel.ReadInt32(condition->intVal)) {
                    return false;
                }
                break;
            }
            case WorkCondition::Type::TIMER: {
                if (!parcel.ReadUint32(condition->uintVal) || !parcel.ReadBool(condition->boolVal)) {
                    return false;
                }
                if (!condition->boolVal && !parcel.ReadInt32(condition->intVal)) {
                    return false;
                }
                break;
            }
            default: {
            }
        }
        read->conditionMap_.emplace(WorkCondition::Type(key), condition);
    }
    return true;
}

std::string WorkInfo::ParseToJsonStr()
{
    nlohmann::json root;
    if (uid_ != INVALID_VALUE) {
        root["uid"] = uid_;
    }
    root["workId"] = workId_;
    if (IsSA()) {
        root["saId"] = saId_;
        root["resident"] = IsResidentSa() ? "true" : "false";
    } else {
        root["bundleName"] = bundleName_;
        root["abilityName"] = abilityName_;
        root["callBySystemApp"] = callBySystemApp_;
        root["appIndex"] = appIndex_;
        root["extension"] = extension_;
    }
    root["earliestStartTime"] = earliestStartTime_;
    root["createTime"] = createTime_;
    root["persisted"] = persisted_;
    root["preinstalled"] = preinstalled_;
    root["uriKey"] = uriKey_;
    ParseConditionToJsonStr(root);
    if (extras_) {
        nlohmann::json extras;
        nlohmann::json extrasType;
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
    std::string result = root.dump(JSON_INDENT_WIDTH);
    return result;
}

void WorkInfo::ParseConditionToJsonStr(nlohmann::json &root)
{
    nlohmann::json conditions;
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
                conditions["baseTime"] = it.second->timeVal;
                if (!it.second->boolVal) {
                    conditions["cycle"] = it.second->intVal;
                }
                break;
            }
            case WorkCondition::Type::DEEP_IDLE: {
                conditions["isDeepIdle"] = it.second->boolVal;
                break;
            }
            default: {}
        }
    }
    root["conditions"] = conditions;
}

bool WorkInfo::ParseFromJson(const nlohmann::json &value)
{
    if (value.is_null() || value.empty()) {
        WS_HILOGE("workinfo json is empty");
        return false;
    }
    if (!value.contains("workId") || !value["workId"].is_number_integer()) {
        WS_HILOGE("workinfo json is invalid, workId is missing or not int");
        return false;
    }
    this->workId_ = value["workId"].get<int32_t>();
    if ((value.contains("saId") && value["saId"].is_number_integer()) && IsHasBoolProp(value, "residentSa")) {
        this->saId_ = value["saId"].get<int32_t>();
        this->residentSa_ = value["residentSa"].get<bool>();
    }
    if (!ParseElementFromJson(value)) {
        return false;
    }
    if (IsHasBoolProp(value, "persisted")) {
        this->persisted_ = value["persisted"].get<bool>();
    }
    if (IsHasBoolProp(value, "preinstalled")) {
        this->preinstalled_ = value["preinstalled"].get<bool>();
    }
    if (value.contains("uriKey") && value["uriKey"].is_string()) {
        this->uriKey_ = value["uriKey"].get<std::string>();
    }
    if (IsHasBoolProp(value, "callBySystemApp")) {
        this->callBySystemApp_ = value["callBySystemApp"].get<bool>();
    }
    if (value.contains("appIndex") && value["appIndex"].is_number_integer()) {
        this->appIndex_ = value["appIndex"].get<int32_t>();
    }
    if (value.contains("earliestStartTime") && value["earliestStartTime"].is_number_integer()) {
        this->earliestStartTime_ = value["earliestStartTime"].get<int32_t>();
    }
    if (value.contains("createTime") && value["createTime"].is_number_integer()) {
        this->createTime_ = static_cast<time_t>(value["createTime"].get<int64_t>());
    }
    if (IsHasBoolProp(value, "extension")) {
        this->extension_ = value["extension"].get<bool>();
    }
    ParseConditionFromJsonStr(value);
    if (!value.contains("parameters")) {
        return true;
    }
    ParseParametersFromJsonStr(value);
    return true;
}

bool WorkInfo::ParseElementFromJson(const nlohmann::json &value)
{
    if (!IsSA()) {
        if (!value.contains("bundleName") || !value["bundleName"].is_string() ||
            !value.contains("abilityName") || !value["abilityName"].is_string()) {
            WS_HILOGE("workinfo json is invalid, bundleName or abilityName is missing or not string");
            return false;
        }
        this->bundleName_ = value["bundleName"].get<std::string>();
        this->abilityName_ = value["abilityName"].get<std::string>();
    }
    return true;
}

void WorkInfo::ParseParametersFromJsonStr(const nlohmann::json &value)
{
    nlohmann::json extrasJson = value["parameters"];
    nlohmann::json extrasType = value["parametersType"];
    AAFwk::WantParams extras;
    int typeId = INVALID_VALUE;
    for (const auto &[key, parameter] : extrasJson.items()) {
        if (extrasType[key].is_number_integer()) {
            typeId = extrasType[key].get<int32_t>();
        }
        if (typeId != INVALID_VALUE && extrasJson[key].is_string()) {
            sptr<AAFwk::IInterface> exInterface = AAFwk::WantParams::GetInterfaceByType(typeId,
                extrasJson[key].get<std::string>());
            extras.SetParam(key, exInterface);
        }
    }
    this->RequestExtras(extras);
}

void WorkInfo::ParseConditionFromJsonStr(const nlohmann::json &value)
{
    if (value.contains("uid") && value["uid"].is_number_integer()) {
        this->uid_ = value["uid"].get<int32_t>();
    }
    nlohmann::json conditions = value["conditions"];
    if (conditions.contains("network") && conditions["network"].is_number_integer()) {
        this->RequestNetworkType(WorkCondition::Network(conditions["network"].get<int32_t>()));
    }
    if (conditions.contains("isCharging") && conditions["isCharging"].is_boolean() &&
        conditions.contains("chargerType") && conditions["chargerType"].is_number_integer()) {
        this->RequestChargerType(conditions["isCharging"].get<bool>(),
            WorkCondition::Charger(conditions["chargerType"].get<int32_t>()));
    }
    if (conditions.contains("batteryLevel") && conditions["batteryLevel"].is_number_integer()) {
        this->RequestBatteryLevel(conditions["batteryLevel"].get<int32_t>());
    }
    if (conditions.contains("batteryStatus") && conditions["batteryStatus"].is_number_integer()) {
        this->RequestBatteryStatus(WorkCondition::BatteryStatus(conditions["batteryStatus"].get<int32_t>()));
    }
    if (conditions.contains("storage") && conditions["storage"].is_number_integer()) {
        this->RequestStorageLevel(WorkCondition::Storage(conditions["storage"].get<int32_t>()));
    }
    if (conditions.contains("isDeepIdle") && conditions["isDeepIdle"].is_boolean()) {
        this->RequestDeepIdle(conditions["isDeepIdle"].get<bool>());
    }
    ParseTimerFormJsonStr(conditions);
}

void WorkInfo::ParseTimerFormJsonStr(const nlohmann::json &conditions)
{
    if (conditions.contains("timer") && conditions["timer"].is_number_integer() &&
        conditions.contains("repeat") && conditions["repeat"].is_boolean()) {
        if (conditions["repeat"].get<bool>()) {
            this->RequestRepeatCycle(conditions["timer"].get<int32_t>());
        } else {
            if (conditions.contains("cycle") && conditions["cycle"].is_number_integer()) {
                this->RequestRepeatCycle(conditions["timer"].get<int32_t>(), conditions["cycle"].get<int32_t>());
            }
        }
        if (conditions.contains("baseTime") && conditions["baseTime"].is_number_integer()) {
            time_t baseTime = (time_t)(conditions["baseTime"].get<int64_t>());
            this->RequestBaseTime(baseTime);
        }
    }
}

void WorkInfo::Dump(std::string &result)
{
    result.append(ParseToJsonStr());
}

bool WorkInfo::IsHasBoolProp(const nlohmann::json &value, const std::string &key)
{
    if (value.contains(key) && value[key].is_boolean()) {
        return true;
    }
    return false;
}

int32_t WorkInfo::GetSaId() const
{
    return saId_;
}

void WorkInfo::RefreshSaId(int32_t saId)
{
    saId_ = saId;
}

bool WorkInfo::IsResidentSa() const
{
    return residentSa_;
}

bool WorkInfo::IsSA()
{
    return saId_ != INVALID_VALUE;
}

std::string WorkInfo::GetBriefInfo()
{
    if (IsSA()) {
        return std::to_string(GetSaId()) + "_" + std::to_string(GetWorkId());
    } else {
        return GetBundleName() + "_" + std::to_string(GetWorkId());
    }
}

void WorkInfo::SetIsInnerApply(bool isInnerApply)
{
    isInnerApply_ = isInnerApply;
}

bool WorkInfo::GetIsInnerApply() const
{
    return isInnerApply_;
}

void WorkInfo::SetEarliestStartTime(int32_t earliestStartTime)
{
    earliestStartTime_ = earliestStartTime;
}

int32_t WorkInfo::GetEarliestStartTime() const
{
    return earliestStartTime_;
}

time_t WorkInfo::GetCreateTime() const
{
    return createTime_;
}
} // namespace WorkScheduler
} // namespace OHOS
