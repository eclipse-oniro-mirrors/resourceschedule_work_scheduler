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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_HILOG_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_HILOG_H

#include <string>

#include "hilog/log.h"

namespace OHOS {
namespace WorkScheduler {
#ifndef WORKSCHEDULER_DOMAIN_ID
#define WORKSCHEDULER_DOMAIN_ID 0xD001712
#endif

#ifndef WORKSCHEDULER_MGR_LOG_TAG
#define WORKSCHEDULER_MGR_LOG_TAG "WORK_SCHEDULER"
#endif

static constexpr OHOS::HiviewDFX::HiLogLabel WORKSCHEDULER_LABEL = {LOG_CORE,
    WORKSCHEDULER_DOMAIN_ID, WORKSCHEDULER_MGR_LOG_TAG};

enum class WorkSchedLogLevel : uint8_t { DEBUG = 0, INFO, WARN, ERROR, FATAL };

class WorkSchedHilog {
public:
    WorkSchedHilog() = delete;
    ~WorkSchedHilog() = delete;

    /**
     * @brief Judge level.
     *
     * @param level The level.
     * @return True if success,else false.
     */
    static bool JudgeLevel(const WorkSchedLogLevel &level);

    /**
     * @brief Set log level.
     *
     * @param level The level.
     */
    static void SetLogLevel(const WorkSchedLogLevel &level)
    {
        level_ = level;
    }

    /**
     * @brief Get log level.
     *
     * @return Level.
     */
    static const WorkSchedLogLevel &GetLogLevel()
    {
        return level_;
    }

    /**
     * @brief Get brief file name.
     *
     * @param str The str.
     * @return Brief file name.
     */
    static std::string GetBriefFileName(const char *str);

private:
    static WorkSchedLogLevel level_;
};

#define WS_PRINT_LOG(LEVEL, Level, fmt, ...)                  \
    if (WorkScheduler::WorkSchedHilog::JudgeLevel(WorkScheduler::WorkSchedLogLevel::LEVEL))     \
    OHOS::HiviewDFX::HiLog::Level(WorkScheduler::WORKSCHEDULER_LABEL,               \
        "[%{public}s(%{public}s):%{public}d] " fmt,        \
        WorkScheduler::WorkSchedHilog::GetBriefFileName(__FILE__).c_str(), \
        __FUNCTION__,                                      \
        __LINE__,                                          \
        ##__VA_ARGS__)

#define WS_HILOGD(fmt, ...) WS_PRINT_LOG(DEBUG, Debug, fmt, ##__VA_ARGS__)
#define WS_HILOGI(fmt, ...) WS_PRINT_LOG(INFO, Info, fmt, ##__VA_ARGS__)
#define WS_HILOGW(fmt, ...) WS_PRINT_LOG(WARN, Warn, fmt, ##__VA_ARGS__)
#define WS_HILOGE(fmt, ...) WS_PRINT_LOG(ERROR, Error, fmt, ##__VA_ARGS__)
#define WS_HILOGF(fmt, ...) WS_PRINT_LOG(FATAL, Fatal, fmt, ##__VA_ARGS__)
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_HILOG_H