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
    /**
     * @brief Set the id of workId.
     *
     * @param workId The workId.
     */
    void SetWorkId(int32_t workId);
    /**
     * @brief Set element.
     *
     * @param bundleName The name of bundle.
     * @param abilityName The name of ability
     */
    void SetElement(std::string bundleName, std::string abilityName);
    /**
     * @brief Request persisted.
     *
     * @param persisted The persisted.
     */
    void RequestPersisted(bool persisted);
    /**
     * @brief Request network type.
     *
     * @param condition The condition.
     */
    void RequestNetworkType(WorkCondition::Network condition);
    /**
     * @brief Request charger type.
     *
     * @param isCharging Charging or not.
     * @param condition The condition.
     */
    void RequestChargerType(bool isCharging, WorkCondition::Charger condition);
    /**
     * @brief Request battery level.
     *
     * @param battLevel The battery level.
     */
    void RequestBatteryLevel(int32_t battLevel);
    /**
     * @brief Request battery status.
     *
     * @param condition The condition.
     */
    void RequestBatteryStatus(WorkCondition::BatteryStatus condition);
    /**
     * @brief Request storage level.
     *
     * @param condition The condition.
     */
    void RequestStorageLevel(WorkCondition::Storage condition);
    /**
     * @brief Request repeat cycle.
     *
     * @param timeInterval The timeInterval.
     * @param cycle The cycle.
     */
    void RequestRepeatCycle(uint32_t timeInterval, int32_t cycle);
    /**
     * @brief Request repeat cycle.
     *
     * @param timeInterval The timeInterval.
     */
    void RequestRepeatCycle(uint32_t timeInterval);
    /**
     * @brief Refresh uid.
     *
     * @param uid The uid.
     */
    void RefreshUid(int32_t uid);
    /**
     * @brief Get uid.
     *
     * @return uid.
     */
    int32_t GetUid();

    /**
     * @brief Get workId.
     *
     * @return workId.
     */
    int32_t GetWorkId();
    /**
     * @brief Get bundle name.
     *
     * @return bundle name.
     */
    std::string GetBundleName();
    /**
     * @brief Get ability name.
     *
     * @return ability name.
     */
    std::string GetAbilityName();
    /**
     * @brief Check whether the work is persist.
     *
     * @return Persist or not.
     */
    bool IsPersisted();
    /**
     * @brief Get network type.
     *
     * @return Network type.
     */
    WorkCondition::Network GetNetworkType();
    /**
     * @brief Get charger type.
     *
     * @return Charger type.
     */
    WorkCondition::Charger GetChargerType();
    /**
     * @brief Get battery level.
     *
     * @return Battery level.
     */
    int32_t GetBatteryLevel();
    /**
     * @brief Get battery status.
     *
     * @return Battery status.
     */
    WorkCondition::BatteryStatus GetBatteryStatus();
    /**
     * @brief Get storage level.
     *
     * @return Storage level.
     */
    WorkCondition::Storage GetStorageLevel();
    /**
     * @brief Check whether the work is repeat.
     *
     * @return Repeat or not.
     */
    bool IsRepeat();
    /**
     * @brief Get timeInterval.
     *
     * @return TimeInterval.
     */
    uint32_t GetTimeInterval();
    /**
     * @brief Get cycle count.
     *
     * @return Cycle count.
     */
    int32_t GetCycleCount();
    /**
     * @brief Get condition map.
     *
     * @return Condition map.
     */
    std::shared_ptr<std::map<WorkCondition::Type, std::shared_ptr<Condition>>> GetConditionMap();
    /**
     * @brief Marshalling.
     *
     * @param parcel The parcel.
     * @return True if success,else false.
     */
    bool Marshalling(Parcel &parcel) const override;
    /**
     * @brief Unmarshalling.
     *
     * @param parcel The parcel.
     * @return Read.
     */
    static sptr<WorkInfo> Unmarshalling(Parcel &parcel);
    /**
     * @brief Dump.
     *
     * @param result The result.
     */
    void Dump(std::string &result);
    /**
     * @brief Parse to json str.
     *
     * @return Result.
     */
    std::string ParseToJsonStr();
    /**
     * @brief Parse from json.
     *
     * @param value The value.
     * @return True if success,else false.
     */
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