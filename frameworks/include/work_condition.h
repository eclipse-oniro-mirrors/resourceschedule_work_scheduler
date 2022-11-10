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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_CONDITION_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_CONDITION_H

#include <ctime>

namespace OHOS {
namespace WorkScheduler {
class WorkCondition {
public:
    virtual ~WorkCondition() = default;

    enum Type {
        NETWORK = 0,
        CHARGER,
        BATTERY_STATUS,
        BATTERY_LEVEL,
        STORAGE,
        TIMER, // uint32_t intervalTime, int32_t cycle, bool repeat
        GROUP, // int32_t newGroup, int32_t userId, bool groupChange, string bundleName
        UNKNOWN,
    };

    enum Network {
        NETWORK_TYPE_ANY = 0,
        NETWORK_TYPE_MOBILE,
        NETWORK_TYPE_WIFI,
        NETWORK_TYPE_BLUETOOTH,
        NETWORK_TYPE_WIFI_P2P,
        NETWORK_TYPE_ETHERNET,
        NETWORK_UNKNOWN
    };

    enum Charger {
        CHARGING_PLUGGED_ANY = 0,
        CHARGING_PLUGGED_AC,
        CHARGING_PLUGGED_USB,
        CHARGING_PLUGGED_WIRELESS,
        CHARGING_UNPLUGGED,
        CHARGING_UNKNOWN,
    };

    enum BatteryStatus {
        BATTERY_STATUS_LOW = 0,
        BATTERY_STATUS_OKAY,
        BATTERY_STATUS_LOW_OR_OKAY,
        BATTERY_UNKNOWN
    };

    enum Storage {
        STORAGE_LEVEL_LOW = 0,
        STORAGE_LEVEL_OKAY,
        STORAGE_LEVEL_LOW_OR_OKAY,
        STORAGE_UNKNOWN
    };
};

struct Condition {
    int32_t enumVal;
    int32_t intVal;
    uint32_t uintVal;
    bool boolVal;
    std::string strVal;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_CONDITION_H