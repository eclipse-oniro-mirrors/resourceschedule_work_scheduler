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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_ERRORS_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_ERRORS_H

#include <errors.h>
#include <map>

namespace OHOS {
namespace WorkScheduler {
const int OFFSET = 100;
const int THRESHOLD = 1000;
enum {
    E_PERMISSION_DENIED = 201,
    E_PARAM_ERROR = 401,
    E_MEMORY_OPERATION_FAILED = 970000101,
    E_PARCEL_OPERATION_FAILED = 970000201,
    E_CLIENT_CONNECT_SERVICE_FAILED = 970000301,
    E_SERVICE_NOT_READY,
    E_CHECK_WORKINFO_FAILED = 970000401,
    E_WORK_NOT_EXIST_FAILED,
    E_ADD_REPEAT_WORK_ERR = 970000501,
    E_WORK_EXCEED_UPPER_LIMIT,
    E_REPEAT_CYCLE_TIME_ERR,
    // inner error code
    E_INNER_ERR,
    E_GROUP_CHANGE_NOT_MATCH_HAP,
};

enum ParamError {
    E_PARAM_NUMBER_ERR = 9700401,
    E_WORK_INFO_TYPE_ERR,
    E_BUNDLE_OR_ABILITY_NAME_ERR,
    E_WORKID_ERR,
    E_CALLBACK_TYPE_ERR,
    E_CONDITION_EMPTY,
    E_NETWORK_TYPE_ERR,
    E_CHARGER_TYPE_ERR,
    E_IS_CHARGING_ERR,
    E_BATTERY_LEVEL_ERR,
    E_BATTERY_STATUS_ERR,
    E_STORAGE_REQUEST_ERR,
    E_REPEAT_COUNT_ERR,
    E_REPEAT_CYCLE_TIME_TYPE_ERR,
    E_IS_REPEAT_ERR,
    E_IS_PERSISTED_ERR,
    E_PARAMETERS_TYPE_ERR,
    E_NEED_CANCLE_TYPE_ERR,
};

static std::map<int32_t, std::string> saErrCodeMsgMap = {
    {E_MEMORY_OPERATION_FAILED, "Memory operation failed. Failed to allocate the memory."},
    {E_PARCEL_OPERATION_FAILED, "Parcel operation failed. Failed to read or write the parcel."},
    {E_CLIENT_CONNECT_SERVICE_FAILED, "System service operation failed. Failed to connect the system service."},
    {E_SERVICE_NOT_READY, "System service operation failed. The service is not ready."},
    {E_CHECK_WORKINFO_FAILED, "Check workInfo failed. Current bundleUid and input uid do not match."},
    {E_ADD_REPEAT_WORK_ERR, "StartWork failed. The work has been already added."},
    {E_REPEAT_CYCLE_TIME_ERR, "StartWork failed. The repeatTime must be greater than or equal to 20 minutes."},
    {E_WORK_EXCEED_UPPER_LIMIT, "StartWork failed. Each uid can add up to 10 works."},
    {E_WORK_NOT_EXIST_FAILED, "The workId do not exist."},
};

static std::map<int32_t, std::string> paramErrCodeMsgMap = {
    {E_PARAM_NUMBER_ERR, "The number of arguments is wrong."},
    {E_WORK_INFO_TYPE_ERR, "The type of workInfo must be {key: value} object."},
    {E_BUNDLE_OR_ABILITY_NAME_ERR, "The bundleName and abilityName cannot be empty and must be string."},
    {E_WORKID_ERR, "The workId must be number greater than or equal to 0."},
    {E_CALLBACK_TYPE_ERR, "The callback should be function."},
    {E_CONDITION_EMPTY, "The workinfo condition cannot be empty."},
    {E_NETWORK_TYPE_ERR, "The value of networkType ranges from NETWORK_TYPE_ANY to NETWORK_TYPE_ETHERNET."},
    {E_CHARGER_TYPE_ERR, "The value of chargerType ranges from CHARGING_PLUGGED_ANY to CHARGING_UNPLUGGED."},
    {E_BATTERY_LEVEL_ERR, "The value of batteryLevel ranges from 0 to 100."},
    {E_BATTERY_STATUS_ERR,
        "The value of batteryStatus ranges from BATTERY_STATUS_LOW to BATTERY_STATUS_LOW_OR_OKAY."},
    {E_STORAGE_REQUEST_ERR,
        "The value of storageRequest ranges from STORAGE_LEVEL_LOW to STORAGE_LEVEL_LOW_OR_OKAY."},
    {E_REPEAT_COUNT_ERR, "The repeatCount must be number greater than or equal to 0."},
    {E_IS_CHARGING_ERR, "The type of isCharging must be boolean."},
    {E_REPEAT_CYCLE_TIME_TYPE_ERR, "The type of repeatCycleTime must be number."},
    {E_IS_REPEAT_ERR, "The type of isRepeat must be boolean."},
    {E_IS_PERSISTED_ERR, "The type of isPersisted must be boolean."},
    {E_PARAMETERS_TYPE_ERR, "The type of parameters must be string, boolean or number."},
    {E_NEED_CANCLE_TYPE_ERR, "The type of needCancle must be boolean."},
    {E_PARAM_ERROR, "The input param is error."},
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_ERRORS_H