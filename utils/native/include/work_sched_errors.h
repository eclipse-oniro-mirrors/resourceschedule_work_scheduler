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

namespace OHOS {
namespace WorkScheduler {
enum {
    WORKSCHED_MODULE_TYPE = 0x00,
};

// offset of workscheduler error, only be used in this file.
constexpr ErrCode WORKSCHED_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_IAWARE, WORKSCHED_MODULE_TYPE);

enum {
    E_WORK_ID_INVALID = WORKSCHED_SERVICE_ERR_OFFSET + 1,
    E_CLIENT_CONNECT_SERVICE_FAILED,
    E_GET_WORK_STATUS_ERROR,
    E_GET_WORKSCHED_SERVICE_FALIED,
    E_START_WORK_FAILED,
    E_STOP_WORK_FAILED,
    E_STOP_AND_CANCEL_WORK_FAILED,
    E_STOP_AND_CLEAR_WORKS_FAILED,
    E_IS_LAST_WORK_TIMEOUT_FALSE,
    E_GROUP_CHANGE_NOT_MATCH_HAP
};

enum {
    E_PERMISSION_DENIED = 201,
    E_PARAM_ERROR = 401,
    E_MEMORY_OPERATION_FAILED = 9700001,
    E_PARCEL_OPERATION_FALIED,
    E_SYSTEM_SERVICE_OPERATION_FAILED,
    E_IPC_COMMUNICATION_FAILED,
    E_CHECK_WORKINFO_FAILED,
    E_STARTWORK_FAILED,
    E_WORK_NOT_EXIST_FAILED
};

enum ParamError {
    E_PARAM_NUMBER_ERR = 9700101,
    E_WORK_INFO_TYPE_ERR,
    E_BUNDLE_OR_ABILITY_NAME_EMPTY,
    E_WORKID_ERR,
    E_CONDITION_EMPTY,
    E_NETWORK_TYPE_ERR,
    E_CHARGER_TYPE_ERR,
    E_BATTERY_LEVEL_ERR,
    E_BATTERY_STATUS_ERR,
    E_STORAGE_REQUEST_ERR,
    E_REPEAT_COUNT_ERR,
    E_PARAMETERS_ERR,
    E_PARAMETERS_TYPE_ERR,
    E_PARAMETERS_FORMAT_ERR,
    E_NEED_CANCLE_TYPE_ERR,
};

enum ServiceError {
    E_PARCEL_READ_FALIED = 9700201,
    E_PARCEL_WRITE_FALIED,
    E_GET_SYSTEM_ABILITY_MANAGER_FALIED,
    E_CHECK_SYSTEM_ABILITY_FALIED,
    E_SERVICE_NOT_READY,
    E_ADD_REPEAT_WORK_ERR,
    E_WORK_EXCEED_UPPER_LIMIT,
    E_REPEAT_CYCLE_TIME_ERR,
};


static std::map<int32_t, std::string> saErrCodeMsgMap = {
    {E_PARCEL_READ_FALIED, "Parcel operation failed. Failed to read the parcel."},
    {E_PARCEL_WRITE_FALIED, "Parcel operation failed. Failed to write the parcel."},
    {E_GET_SYSTEM_ABILITY_MANAGER_FALIED, "System service operation failed. Failed to get system ability manager."},
    {E_CHECK_SYSTEM_ABILITY_FALIED, "System service operation failed. Failed to get system ability."},
    {E_SERVICE_NOT_READY, "System service operation failed. The service is not ready."},
    {E_IPC_COMMUNICATION_FAILED, "IPC communication failed. Failed to access the system service."},
    {E_CHECK_WORKINFO_FAILED, "Check workInfo failed. Current bundleUid and input uid do not match."},
    {E_ADD_REPEAT_WORK_ERR, "StartWork failed. The work has been already added."},
    {E_WORK_EXCEED_UPPER_LIMIT, "StartWork failed. Each uid can add up to 10 works."},
    {E_WORK_NOT_EXIST_FAILED, "The workId do not exist."},
};

static std::map<int32_t, std::string> paramErrCodeMsgMap = {
    {E_PARAM_NUMBER_ERR, "The number of arguments is wrong."},
    {E_WORK_INFO_TYPE_ERR, "The type of workInfo must be {key: value} object."},
    {E_BUNDLE_OR_ABILITY_NAME_EMPTY, "The bundleName and abilityName cannot be empty."},
    {E_WORKID_ERR, "The workId must be greater than 0."},
    {E_CONDITION_EMPTY, "The workinfo condition cannot be empty."},
    {E_NETWORK_TYPE_ERR, "The value of networkType ranges from NETWORK_TYPE_ANY to NETWORK_TYPE_ETHERNET."},
    {E_CHARGER_TYPE_ERR, "The value of chargerType ranges from CHARGING_PLUGGED_ANY to CHARGING_UNPLUGGED."},
    {E_BATTERY_LEVEL_ERR, "The value of batteryLevel ranges from 0 to 100."},
    {E_BATTERY_STATUS_ERR, "The value of batteryStatus ranges from BATTERY_STATUS_LOW to BATTERY_STATUS_LOW_OR_OKAY."},
    {E_STORAGE_REQUEST_ERR, "The value of storageRequest ranges from STORAGE_LEVEL_LOW to STORAGE_LEVEL_LOW_OR_OKAY."},
    {E_REPEAT_COUNT_ERR, "The number of repeatCount must be greater than or equal to 0."},
    {E_PARAMETERS_ERR, "The type of parameters must be string, boolean or number."},
    {E_PARAMETERS_TYPE_ERR, "The type of parameters must be string, boolean or number."},
    {E_PARAMETERS_FORMAT_ERR, "The format of parameters must be {key: value} object"},
    {E_NEED_CANCLE_TYPE_ERR, "The type of needCancle must be boolean."},
    {E_REPEAT_CYCLE_TIME_ERR, "The repeat time should be greater than or equal to 20 minutes."},
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_ERRORS_H