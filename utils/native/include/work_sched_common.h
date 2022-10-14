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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_COMMON_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_COMMON_H

#include <fstream>

#include <refbase.h>

#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
#define TEST_LOG_FILE_PATH "/data/test_log_file"
#define RETURN_IF_WITH_RET(cond, retval) if (cond) {return (retval);}
#define RETURN_IF(cond) if (cond) {return;}

#define RETURN_IF_WITH_LOG(cond, loginfo)                                           \
    do {                                                                            \
        if (cond) {                                                                 \
            WS_HILOGE("%{public}s "#loginfo" ", __func__);           \
            return;                                                                 \
        }                                                                           \
    } while (0)

#define READ_PARCEL_NO_RET(parcel, type, out)                                       \
    do {                                                                            \
        if (!(parcel).Read##type(out)) {                                            \
            WS_HILOGE("%{public}s read"#out" failed", __func__);     \
            return;                                                                 \
        }                                                                           \
    } while (0)
} // namespace WorkScheduler

#define READ_PARCEL_WITHOUT_RET(parcel, type, out)                                       \
    do {                                                                            \
        if (!(parcel).Read##type(out)) {                                            \
            WS_HILOGE("%{public}s read"#out" failed", __func__);     \
        }                                                                           \
    } while (0)
} // namespace OHOS

#define WRITE_PARCEL_NO_RET(parcel, type, data)                                     \
    do {                                                                            \
        if (!(parcel).Write##type(data)) {                                          \
            WS_HILOGE("%{public}s write "#data" failed", __func__);  \
            return;                                                                 \
        }                                                                           \
    } while (0)

#define READ_PARCEL_WITH_RET(parcel, type, out, retval)                             \
    do {                                                                            \
        if (!(parcel).Read##type(out)) {                                            \
            WS_HILOGE("%{public}s read "#out" failed", __func__);    \
            return (retval);                                                        \
        }                                                                           \
    } while (0)

#define WRITE_PARCEL_WITHOUT_RET(parcel, type, data)                           \
    do {                                                                            \
        if (!(parcel).Write##type(data)) {                                          \
            WS_HILOGE("%{public}s write "#data" failed", __func__);  \
        }                                                                           \
    } while (0)

#define WRITE_PARCEL_WITH_RET(parcel, type, data, retval)                           \
    do {                                                                            \
        if (!(parcel).Write##type(data)) {                                          \
            WS_HILOGE("%{public}s write "#data" failed", __func__);  \
            return (retval);                                                        \
        }                                                                           \
    } while (0)

#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_COMMON_H