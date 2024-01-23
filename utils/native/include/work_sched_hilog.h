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

#include "hilog/log.h"

#ifndef WORKSCHEDULER_MGR_LOG_DOMAIN
#define WORKSCHEDULER_MGR_LOG_DOMAIN 0xD001712
#endif

#ifndef WORKSCHEDULER_MGR_LOG_TAG
#define WORKSCHEDULER_MGR_LOG_TAG "WORK_SCHEDULER"
#endif

#define FILENAME (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define WS_HILOGF(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_FATAL, WORKSCHEDULER_MGR_LOG_DOMAIN, WORKSCHEDULER_MGR_LOG_TAG, \
    "[%{public}s(%{public}s:%{public}d)]" fmt, FILENAME, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define WS_HILOGE(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_ERROR, WORKSCHEDULER_MGR_LOG_DOMAIN, WORKSCHEDULER_MGR_LOG_TAG, \
    "[%{public}s(%{public}s:%{public}d)]" fmt, FILENAME, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define WS_HILOGW(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_WARN, WORKSCHEDULER_MGR_LOG_DOMAIN, WORKSCHEDULER_MGR_LOG_TAG, \
    "[%{public}s(%{public}s:%{public}d)]" fmt, FILENAME, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define WS_HILOGI(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_INFO, WORKSCHEDULER_MGR_LOG_DOMAIN, WORKSCHEDULER_MGR_LOG_TAG, \
    "[%{public}s(%{public}s:%{public}d)]" fmt, FILENAME, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define WS_HILOGD(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, WORKSCHEDULER_MGR_LOG_DOMAIN, WORKSCHEDULER_MGR_LOG_TAG, \
    "[%{public}s(%{public}s:%{public}d)]" fmt, FILENAME, __FUNCTION__, __LINE__, ##__VA_ARGS__))

#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_UTILS_HILOG_H