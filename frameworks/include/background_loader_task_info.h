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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_TASK_INFO_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_TASK_INFO_H

#include <string>

#include "parcel.h"

namespace OHOS {
namespace WorkScheduler {
class BackgroundLoaderTaskInfo : public Parcelable {
public:
    BackgroundLoaderTaskInfo();
    BackgroundLoaderTaskInfo(int32_t taskId, const std::string& abilityName);
    ~BackgroundLoaderTaskInfo() override;

    /**
     * @brief Get the id of task.
     *
     * @return The id of task.
     */
    int32_t GetTaskId() const;

    /**
     * @brief Get the name of ability.
     *
     * @return the name of ability.
     */
    const std::string& GetAbilityName() const;

    /**
     * @brief Set the id of task.
     */
    void SetTaskId(int32_t taskId);

    /**
     * @brief Set the name of ability.
     */
    void SetAbilityName(const std::string& abilityName);

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
    static BackgroundLoaderTaskInfo* Unmarshalling(Parcel &parcel);
private:
    int32_t taskId_ = 0;
    std::string abilityName_ = "";
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_TASK_INFO_H