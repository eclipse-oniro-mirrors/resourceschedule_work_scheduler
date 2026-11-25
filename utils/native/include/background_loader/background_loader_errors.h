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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_ERRORS_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_ERRORS_H

#include <cstdint>
#include <map>
#include <string>

namespace OHOS {
namespace WorkScheduler {
namespace BackgroundLoader {
constexpr int32_t BACKGROUND_LOADER_ERR_OFFSET = 100;
constexpr int32_t BACKGROUND_LOADER_ERR_THRESHOLD = 1000;

enum BackgroundLoaderError : int32_t {
    E_PERMISSION_DENIED = 201,
    E_PARAM_ERROR = 401,
    E_MEMORY_OPERATION_FAILED = 971000101,
    E_PARCEL_OPERATION_FAILED = 971000201,
    E_BACKGROUND_LOADER_SERVICE_FAILED = 971000301,
    E_SYSTEM_SERVICE_NOT_READY,
    E_SYSTEM_SERVICE_EXCEPTION,
    E_TASK_EXCEED_UPPER_LIMIT,
    E_PARAM_NUMBER_ERR = 971000401,
    E_ABILITY_NAME_INCONFORMITY,
    E_NOT_SPECIFIED_ABILITY_NAME,
    E_INVALID_TASK_ID,
    E_TASK_ID_NOT_EXIST,
    E_BACKGROUND_LOADER_FAILED = 971000501,
    E_TASK_NOT_EXIST,
    E_TASK_IS_RUNNING,
    E_ADD_REPEAT_TASK_ERR,
    E_BACKGROUND_LOADER_PROHIBITED = 971000601,
    E_TASK_EXECUTE_PERCEPTIBLE_TASK,
    E_TASK_HAS_PERCEPTIBLE_PERMISSIONS,
};

const inline std::map<int32_t, std::string> backgroundLoaderMsgMap = {
    {E_PERMISSION_DENIED, "Permission denied."},
    {E_PARAM_ERROR, "Parameter error."},
    {E_MEMORY_OPERATION_FAILED, "Memory operation failed."},
    {E_PARCEL_OPERATION_FAILED, "Parcel operation failed."},
    {E_BACKGROUND_LOADER_SERVICE_FAILED, "Background loader service operation failed."},
    {E_SYSTEM_SERVICE_NOT_READY, "The system service has not started yet."},
    {E_SYSTEM_SERVICE_EXCEPTION, "System service exception."},
    {E_PARAM_NUMBER_ERR, "Parameter error."},
    {E_ABILITY_NAME_INCONFORMITY, "Ability name does not comply with the standards."},
    {E_NOT_SPECIFIED_ABILITY_NAME, "The ability name is not the specified ability name."},
    {E_INVALID_TASK_ID, "Invalid task ID."},
    {E_TASK_ID_NOT_EXIST, "Task ID does not exist."},
    {E_BACKGROUND_LOADER_FAILED, "Background load failed."},
    {E_TASK_NOT_EXIST, "The task does not exist."},
    {E_TASK_IS_RUNNING, "The task is in progress."},
    {E_ADD_REPEAT_TASK_ERR, "Repeat registration of background loading tasks."},
    {E_BACKGROUND_LOADER_PROHIBITED, "Background load prohibited."},
    {E_TASK_EXECUTE_PERCEPTIBLE_TASK, "The task tried to do a disabled sensitive action, "
        "like playing or recording audio, or using the flash."},
    {E_TASK_HAS_PERCEPTIBLE_PERMISSIONS, "The task has requested sensitive permission."},
};

/**
 * Translate a sub error code into the public error code. The public error code is
 * the sub error code divided by BACKGROUND_LOADER_ERR_OFFSET, for example,
 * 971000302 -> 9710003
 */
inline int32_t GetBackgroundLoaderPublicCode(int32_t errCode)
{
    return errCode > BACKGROUND_LOADER_ERR_THRESHOLD ? errCode / BACKGROUND_LOADER_ERR_OFFSET : errCode;
}

/**
 * Get the public error code and the error message of the error code. If the error
 * code is not defined in this module, E_BACKGROUND_LOADER_FAILED is returned.
 */
inline bool ResolveErrorInfo(int32_t errCode, int32_t &publicCode, std::string &errMsg)
{
    publicCode = GetBackgroundLoaderPublicCode(errCode);
    auto iter = backgroundLoaderMsgMap.find(errCode);
    if (iter == backgroundLoaderMsgMap.end()) {
        publicCode = GetBackgroundLoaderPublicCode(E_BACKGROUND_LOADER_FAILED);
        errMsg = backgroundLoaderMsgMap.at(E_BACKGROUND_LOADER_FAILED);
        return false;
    }
    errMsg = iter->second;
    return true;
}
} // BackgroundLoader
} // WorkScheduler
} // OHOS

#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_ERRORS_H