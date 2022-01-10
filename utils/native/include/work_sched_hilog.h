/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef WORK_SCHED_UTILS_HILOG_H
#define WORK_SCHED_UTILS_HILOG_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG
#include "hilog/log.h"

namespace OHOS {
namespace WorkScheduler {
#define __FILENAME__            (__builtin_strchr(__FILE__, '/') ? __builtin_strchr(__FILE__, '/') + 1 : __FILE__)
#define __FORMATED(fmt, ...)    "[%{public}s] %{public}s# " fmt, __FILENAME__, __FUNCTION__, ##__VA_ARGS__

#ifdef WORKSCHED_HILOGF
#undef WORKSCHED_HILOGF
#endif

#ifdef WORKSCHED_HILOGE
#undef WORKSCHED_HILOGE
#endif

#ifdef WORKSCHED_HILOGW
#undef WORKSCHED_HILOGW
#endif

#ifdef WORKSCHED_HILOGI
#undef WORKSCHED_HILOGI
#endif

#ifdef WORKSCHED_HILOGD
#undef WORKSCHED_HILOGD
#endif

// param of log interface, such as WORKSCHED_HILOGF.
enum WorkSchedSubModule {
    MODULE_INNERKIT = 0,
    MODULE_SERVICE,
    MODULE_JAVAKIT,
    MODULE_JNI,
    MODULE_COMMON,
    MODULE_JS_NAPI,
    WORKSCHED_MODULE_BUTT,
};

// 0xD001900: subsystem:resouceschedule module:workscheduler, 8 bits reserved.
static constexpr unsigned int BASE_WORKSCHEDULER_DOMAIN_ID = 0xD001900;

enum WorkSchedDomainId {
    WORKSCHED_INNERKIT_DOMAIN = BASE_WORKSCHEDULER_DOMAIN_ID + MODULE_INNERKIT,
    WORKSCHED_SERVICE_DOMAIN,
    WORKSCHED_JAVAKIT_DOMAIN,
    WORKSCHED_JNI_DOMAIN,
    COMMON_DOMAIN,
    WORKSCHED_JS_NAPI,
    WORKSCHED_BUTT,
};

static constexpr OHOS::HiviewDFX::HiLogLabel WORKSCHED_LABEL[WORKSCHED_MODULE_BUTT] = {
    {LOG_CORE, WORKSCHED_INNERKIT_DOMAIN, "WorkSchedInnerKit"},
    {LOG_CORE, WORKSCHED_SERVICE_DOMAIN, "WorkSchedService"},
    {LOG_CORE, WORKSCHED_JAVAKIT_DOMAIN, "WorkSchedJavaKit"},
    {LOG_CORE, WORKSCHED_JNI_DOMAIN, "WorkSchedJni"},
    {LOG_CORE, COMMON_DOMAIN, "WorkSchedCommon"},
    {LOG_CORE, WORKSCHED_JS_NAPI, "WorkSchedJSNAPI"},
};

// In order to improve performance, do not check the module range.
// Besides, make sure module is less than WORKSCHED_MODULE_BUTT.
#define WS_HILOGF(module, ...) (void)OHOS::HiviewDFX::HiLog::Fatal(WORKSCHED_LABEL[module], __FORMATED(__VA_ARGS__))
#define WS_HILOGE(module, ...) (void)OHOS::HiviewDFX::HiLog::Error(WORKSCHED_LABEL[module], __FORMATED(__VA_ARGS__))
#define WS_HILOGW(module, ...) (void)OHOS::HiviewDFX::HiLog::Warn(WORKSCHED_LABEL[module], __FORMATED(__VA_ARGS__))
#define WS_HILOGI(module, ...) (void)OHOS::HiviewDFX::HiLog::Info(WORKSCHED_LABEL[module], __FORMATED(__VA_ARGS__))
#define WS_HILOGD(module, ...) (void)OHOS::HiviewDFX::HiLog::Debug(WORKSCHED_LABEL[module], __FORMATED(__VA_ARGS__))
} // namespace WorkScheduler
} // namespace OHOS

#else

#define WS_HILOGF(...)
#define WS_HILOGE(...)
#define WS_HILOGW(...)
#define WS_HILOGI(...)
#define WS_HILOGD(...)

#endif // CONFIG_HILOG

#endif // WORK_SCHED_UTILS_HILOG_H