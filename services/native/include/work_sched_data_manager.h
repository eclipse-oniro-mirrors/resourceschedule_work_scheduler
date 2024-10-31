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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_DATA_MANAGER_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_DATA_MANAGER_H

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <list>
#include "singleton.h"
#include "ffrt.h"

namespace OHOS {
namespace WorkScheduler {
class DataManager {
DECLARE_DELAYED_SINGLETON(DataManager);
public:
    void SetDeviceSleep(const bool isSleep);
    bool GetDeviceSleep() const;

    bool IsInDeviceStandyWhitelist(const std::string& bundleName);
    void OnDeviceStandyWhitelistChanged(const std::string& bundleName, const bool add);
    void AddDeviceStandyWhitelist(const std::list<std::string>& bundleNames);
    void ClearDeviceStandyWhitelist();
    bool IsDeviceStandyWhitelistEmpty();

    void AddGroup(const std::string& bundleName, const int32_t userId, const int32_t appGroup);
    bool FindGroup(const std::string& bundleName, const int32_t userId, int32_t& appGroup);
    void ClearGroup(const std::string& bundleName, const int32_t userId);
    void ClearAllGroup();
    bool AllowToStart(const std::string& bundleName);

private:
    std::atomic<bool> deviceSleep_ = false;
    ffrt::mutex mutex_;
    // <bundle>
    std::set<std::string> deviceStandySet {};

    ffrt::mutex activeGroupMapMutex_;
    // <bundle_userId, group>
    std::unordered_map<std::string, int32_t> activeGroupMap_;

};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORK_SCHED_DATA_MANAGER_H