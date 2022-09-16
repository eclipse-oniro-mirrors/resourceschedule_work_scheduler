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
    E_REPEAT_CYCLE_TIME_ERR,
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
};


static std::map<int32_t, std::string> saErrCodeMsgMap = {
    {E_PARCEL_READ_FALIED, "0"},
    {E_PARCEL_WRITE_FALIED, "0"},
    {E_GET_SYSTEM_ABILITY_MANAGER_FALIED, "0"},
    {E_CHECK_SYSTEM_ABILITY_FALIED, "0"},
    {E_SERVICE_NOT_READY, "0"},
    {E_ADD_REPEAT_WORK_ERR, "0"},
    {E_WORK_EXCEED_UPPER_LIMIT, "0"}
};

static std::map<int32_t, std::string> paramErrCodeMsgMap = {
    {E_PARAM_NUMBER_ERR, "The number of arguments is wrong."}


};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_ERRORS_H