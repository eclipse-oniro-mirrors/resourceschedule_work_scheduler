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
#include "work_sched_data_manager.h"
#include "work_sched_hilog.h"
#include "work_sched_utils.h"
#include "work_scheduler_service.h"

namespace OHOS {
namespace WorkScheduler {
DataManager::DataManager() {}

DataManager::~DataManager() {}

bool DataManager::GetDeviceSleep() const
{
    return deviceSleep_;
}

void DataManager::SetDeviceSleep(const bool isSleep)
{
    deviceSleep_ = isSleep;
}

bool DataManager::IsInDeviceStandyWhitelist(const std::string& bundleName)
{
    std::lock_guard<ffrt::mutex> lock(deviceStandySetMutex_);
    return deviceStandySet.count(bundleName) > 0;
}

void DataManager::OnDeviceStandyWhitelistChanged(const std::string& bundleName, const bool add)
{
    std::lock_guard<ffrt::mutex> lock(deviceStandySetMutex_);
    if (add) {
        deviceStandySet.insert(bundleName);
    } else {
        deviceStandySet.erase(bundleName);
    }
}

void DataManager::AddDeviceStandyWhitelist(const std::list<std::string>& bundleNames)
{
    std::lock_guard<ffrt::mutex> lock(deviceStandySetMutex_);
    for (const auto& item : bundleNames) {
        deviceStandySet.insert(item);
    }
}

void DataManager::ClearDeviceStandyWhitelist()
{
    std::lock_guard<ffrt::mutex> lock(deviceStandySetMutex_);
    deviceStandySet.clear();
}

bool DataManager::IsDeviceStandyWhitelistEmpty()
{
    std::lock_guard<ffrt::mutex> lock(deviceStandySetMutex_);
    return deviceStandySet.empty();
}

bool DataManager::FindGroup(const std::string& bundleName, const int32_t userId, int32_t& appGroup)
{
    std::lock_guard<ffrt::mutex> lock(activeGroupMapMutex_);
    std::string key = bundleName + "_" + std::to_string(userId);
    auto iter = activeGroupMap_.find(key);
    if (iter != activeGroupMap_.end()) {
        appGroup = iter->second;
        return true;
    } else {
        return false;
    }
}

void DataManager::AddGroup(const std::string& bundleName, const int32_t userId, const int32_t appGroup)
{
    std::lock_guard<ffrt::mutex> lock(activeGroupMapMutex_);
    std::string key = bundleName + "_" + std::to_string(userId);
    auto iter = activeGroupMap_.find(key);
    if (iter != activeGroupMap_.end()) {
        activeGroupMap_.erase(iter);
    }
    activeGroupMap_.emplace(key, appGroup);
    WS_HILOGI("activeGroupMap_ size %{public}d", static_cast<int32_t>(activeGroupMap_.size()));
}

void DataManager::ClearGroup(const std::string& bundleName, const int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(activeGroupMapMutex_);
    std::string key = bundleName + "_" + std::to_string(userId);
    auto iter = activeGroupMap_.find(key);
    if (iter != activeGroupMap_.end()) {
        activeGroupMap_.erase(iter);
    }
}

void DataManager::ClearAllGroup()
{
    std::lock_guard<ffrt::mutex> lock(activeGroupMapMutex_);
    activeGroupMap_.clear();
}
} // namespace WorkScheduler
} // namespace OHOS