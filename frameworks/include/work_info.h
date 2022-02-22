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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_INFO_H

#include <list>
#include <map>
#include <memory>
#include <string>

#include <parcel.h>
#include "json/json.h"

#include "work_condition.h"

namespace OHOS {
namespace WorkScheduler {
class WorkInfo : public Parcelable {
public:
    explicit WorkInfo();
    ~WorkInfo();
    void SetWorkId(int32_t workId);
    void SetElement(std::string bundleName, std::string abilityName);
    void RequestPersisted(bool persisted);
    void RequestNetworkType(WorkCondition::Network condition);
    void RequestChargerType(bool isCharging, WorkCondition::Charger condition);
    void RequestBatteryLevel(int32_t battLevel);
    void RequestBatteryStatus(WorkCondition::BatteryStatus condition);
    void RequestStorageLevel(WorkCondition::Storage condition);
    void RequestRepeatCycle(uint32_t timeInterval, int32_t cycle);
    void RequestRepeatCycle(uint32_t timeInterval);
    void RefreshUid(int32_t uid);
    int32_t GetUid();

    int32_t GetWorkId();
    std::string GetBundleName();
    std::string GetAbilityName();
    bool IsPersisted();
    WorkCondition::Network GetNetworkType();
    WorkCondition::Charger GetChargerType();
    int32_t GetBatteryLevel();
    WorkCondition::BatteryStatus GetBatteryStatus();
    WorkCondition::Storage GetStorageLevel();
    bool IsRepeat();
    uint32_t GetTimeInterval();
    int32_t GetCycleCount();
    std::shared_ptr<std::map<WorkCondition::Type, std::shared_ptr<Condition>>> GetConditionMap();
    bool Marshalling(Parcel &parcel) const override;
    static WorkInfo *Unmarshalling(Parcel &parcel);
    void Dump(std::string &result);
    std::string ParseToJsonStr();
    bool ParseFromJson(const Json::Value value);

private:
    int32_t workId_;
    std::string bundleName_;
    std::string abilityName_;
    bool persisted_;
    int32_t uid_;
    std::map<WorkCondition::Type, std::shared_ptr<Condition>> conditionMap_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_INFO_H