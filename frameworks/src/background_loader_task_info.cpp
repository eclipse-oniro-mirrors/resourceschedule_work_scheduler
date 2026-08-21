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

#include "background_loader_task_info.h"

namespace OHOS {
namespace WorkScheduler {
BackgroundLoaderTaskInfo::BackgroundLoaderTaskInfo(int32_t taskId,
    const std::string& abilityName) : taskId_(taskId), abilityName_(abilityName) {}

BackgroundLoaderTaskInfo::BackgroundLoaderTaskInfo() = default;
BackgroundLoaderTaskInfo::~BackgroundLoaderTaskInfo() = default;

const std::string& BackgroundLoaderTaskInfo::GetAbilityName() const
{
    return abilityName_;
}

int32_t BackgroundLoaderTaskInfo::GetTaskId() const
{
    return taskId_;
}

void BackgroundLoaderTaskInfo::SetAbilityName(const std::string& abilityName)
{
    abilityName_ = abilityName;
}

void BackgroundLoaderTaskInfo::SetTaskId(int32_t taskId)
{
    taskId_ = taskId;
}

bool BackgroundLoaderTaskInfo::Marshalling(Parcel &parcel) const
{
    bool ret = parcel.WriteInt32(taskId_);
    ret = ret && parcel.WriteString(abilityName_);
    return ret;
}

BackgroundLoaderTaskInfo* BackgroundLoaderTaskInfo::Unmarshalling(Parcel &parcel)
{
    auto* info = new (std::nothrow) BackgroundLoaderTaskInfo();
    if (info == nullptr) {
        return nullptr;
    }
    if (!parcel.ReadInt32(info->taskId_)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->abilityName_)) {
        delete info;
        return nullptr;
    }
    return info;
}

} // namespace WorkScheduler
} // namespace OHOS