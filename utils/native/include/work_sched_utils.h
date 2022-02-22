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

    static int GetCurrentAccountId();
    static bool IsIdActive(int id);
    static int32_t GetUserIdByUid(int32_t uid);
    static bool ConvertFullPath(const std::string &inOriPath, std::string &outRealPath);
    static const int INVALID_DATA = -1;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_UTILS_H