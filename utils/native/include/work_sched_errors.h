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

#ifndef WORK_SCHED_UTILS_ERRORS_H
#define WORK_SCHED_UTILS_ERRORS_H

#include <errors.h>

namespace OHOS {
namespace WorkScheduler {
// WorkScheduler's module const defined.
enum {
    WORKSCHED_MODULE_TYPE = 0x00,
};

// offset of workscheduler error, only be used in this file.
constexpr ErrCode WORKSCHED_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_IAWARE, WORKSCHED_MODULE_TYPE);

enum {
    E_WRITE_PARCEL_ERROR = WORKSCHED_SERVICE_ERR_OFFSET + 1,
    E_READ_PARCEL_ERROR,
    E_GET_SYSTEM_ABILITY_MANAGER_FAILED_WORKSCHED,
    E_GET_WORKSCHED_SERVICE_FALIED,
    E_START_WORK_FAILED,
    E_STOP_WORK_FAILED,
    E_STOP_AND_CANCEL_WORK_FAILED,
    E_STOP_AND_CLEAR_WORKS_FAILED,
    E_IS_LAST_WORK_TIMEOUT_FALSE,
    E_ADD_WORK_SCHED_DEATH_RECIPIENT_FAILED,
    E_INNER_ERR
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // WORK_SCHED_UTILS_ERRORS_H