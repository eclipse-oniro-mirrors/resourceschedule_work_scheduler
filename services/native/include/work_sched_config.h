/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
 
#ifndef FOUNDATION_RESOURCESCHEDULE_WORK_SCHEDULER_CONFIG_H
#define FOUNDATION_RESOURCESCHEDULE_WORK_SCHEDULER_CONFIG_H
#include <string>
#include <set>
#include <mutex>
#include "singleton.h"
 
namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerConfig : public DelayedSingleton<WorkSchedulerConfig> {
public:
    void InitActiveGroupWhitelist(const std::string &configData);
    bool IsInActiveGroupWhitelist(const std::string &bundleName);
 
private:
    std::mutex configMutex_;
    std::set<std::string> activeGroupWhitelist_ {};
};
}
}
#endif