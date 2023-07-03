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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_IWORK_SCHED_SERVICE_IPC_INTERFACE_CODE_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_IWORK_SCHED_SERVICE_IPC_INTERFACE_CODE_H

/* SAID: 1904 */
namespace OHOS {
namespace WorkScheduler {
        enum class IWorkSchedServiceInterfaceCode {
        START_WORK = 0,
        STOP_WORK,
        STOP_AND_CANCEL_WORK,
        STOP_AND_CLEAR_WORKS,
        IS_LAST_WORK_TIMEOUT,
        OBTAIN_ALL_WORKS,
        GET_WORK_STATUS,
        GET_ALL_RUNNING_WORKS,
    };
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_IWORK_SCHED_SERVICE_IPC_INTERFACE_CODE_H