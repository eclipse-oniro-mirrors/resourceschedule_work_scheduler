/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_CONSTANTS_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_CONSTANTS_H

namespace OHOS {
namespace WorkScheduler {
// services\native\src\work_policy_manager.cpp
inline constexpr int32_t MAX_RUNNING_COUNT = 3;
inline const int32_t STANDBY_MAX_RUNNING_COUNT = 2 * MAX_RUNNING_COUNT;
inline constexpr uint32_t MAX_WORK_COUNT_PER_UID = 10;
inline constexpr int32_t DELAY_TIME_LONG = 30000;
inline constexpr int32_t DELAY_TIME_SHORT = 5000;
inline constexpr uint32_t MAX_WATCHDOG_ID = 1000;
inline constexpr uint32_t INIT_WATCHDOG_ID = 1;
inline constexpr int32_t INIT_DUMP_SET_MEMORY = -1;
inline constexpr int32_t WATCHDOG_TIME = 2 * 60 * 1000;
inline constexpr int32_t MEDIUM_WATCHDOG_TIME = 10 * 60 * 1000;
inline constexpr int32_t LONG_WATCHDOG_TIME = 20 * 60 * 1000;
inline constexpr int32_t INIT_DUMP_SET_CPU = 0;
inline constexpr int32_t INVALID_VALUE = -1;
inline constexpr int32_t DUMP_SET_MAX_COUNT_LIMIT = 100;
inline static int32_t g_lastWatchdogTime = WATCHDOG_TIME;

// services\native\src\work_status.cpp
inline const std::string OK = "1";
inline const std::string NOT_OK = "0";
// watchdog timeout threshold
inline constexpr int32_t WATCHDOG_TIMEOUT_THRESHOLD_MS = 500;

// services\native\src\conditions\screen_listener.cpp
inline constexpr int MIN_DEEP_IDLE_SCREEN_OFF_TIME_MIN = 31 * 60 * 1000;

inline const std::string CONNECT_ABILITY = "CONNECT_ABILITY";
inline const std::string DISCONNECT_ABILITY = "DISCONNECT_ABILITY";
inline const std::string WATCHDOG_TIMEOUT = "WATCHDOG_TIMEOUT";
inline const std::string LOAD_WORK = "LOAD_WORK";
inline const std::string SERVICE_INIT = "SERVICE_INIT";
inline const std::string LOAD_SA = "LOAD_SA";
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_CONSTANTS_H