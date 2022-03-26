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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_UTILS_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_UTILS_H

#include <string>

#include "hilog/log.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedUtils {
public:
    WorkSchedUtils() = delete;
    ~WorkSchedUtils() = delete;

    /**
     * @brief Get current account id.
     *
     * @return Current account id.
     */
    static int GetCurrentAccountId();
    /**
     * @brief The id active.
     *
     * @param id The id.
     * @return ERR_OK on success, others on failure.
     */
    static bool IsIdActive(int id);
    /**
     * @brief Get user id by uid.
     *
     * @param uid The uid.
     * @return User id by uid.
     */
    static int32_t GetUserIdByUid(int32_t uid);
    /**
     * @brief Convert full path.
     *
     * @param inOriPath The inOriPath.
     * @param outRealPath The outRealPath.
     * @return ERR_OK on success, others on failure.
     */
    static bool ConvertFullPath(const std::string &inOriPath, std::string &outRealPath);
    static const int INVALID_DATA = -1;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_UTILS_H