/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_FREQUENCY_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_FREQUENCY_INFO_H

#include "parcel.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace WorkScheduler {
class FrequencyInfo : public Parcelable {
public:
    FrequencyInfo();
    FrequencyInfo(const FrequencyInfo& frequencyInfo);
    ~FrequencyInfo() override;

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
     * @return FrequencyInfo.
     */
    static FrequencyInfo* Unmarshalling(Parcel &parcel);

    /**
     * @brief Get uid.
     *
     * @return The uid.
     */
    int32_t GetUid() const;

    /**
     * @brief Get workId.
     *
     * @return The workId.
     */
    int32_t GetWorkId() const;

    /**
     * @brief Get interval.
     *
     * @return The interval.
     */
    int32_t GetInterval() const;

    /**
     * @brief Set the uid of target application.
     */
    void SetUid(int32_t uid);

    /**
     * @brief Set the work id.
     */
    void SetWorkId(int32_t workId);

    /**
     * @brief Set the execution interval.
     */
    void SetInterval(int32_t interval);

    /**
     * @brief Parse to json str.
     *
     * @return Result.
     */
    std::string ParseToJsonStr() const;

    /**
     * @brief Parse from json.
     *
     * @param value The value.
     * @return True if success,else false.
     */
    bool ParseFromJson(const nlohmann:json &value);
private:
    int32_t uid_{-1};
    int32_t workId_{-1};
    int64_t interval_{-1};
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_FREQUENCY_INFO_H