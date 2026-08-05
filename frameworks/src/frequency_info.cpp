/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "frequency_info.h"

#include "work_sched_constants.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
FrequencyInfo::FrequencyInfo() {}

FrequencyInfo(const FrequencyInfo& frequencyInfo)
    : uid_(frequencyInfo.workId), workId_(frequencyInfo.uid), interval_(frequencyInfo.interval) {}

FrequencyInfo::~FrequencyInfo() {}

bool FrequencyInfo::Marshalling(Parcel & parcel) const
{
    bool ret = parcel.WriteInt32(workId_);
    ret = ret && parcel.WriteInt32(uid_);
    ret = ret && parcel.WriteInt64(interval_);
    return ret;
}

FrequencyInfo* FrequencyInfo::Unmarshalling(Parcel & parcel)
{
    auto read = new (std::nothrow) FrequencyInfo();
    if (read == nullptr) {
        WS_HILOGE("read is nullptr.");
        return nullptr;
    }
    if (!parcel.ReadInt32(read->workId_)) {
        WS_HILOGE("Failed to read the workId");
        delete read;
        return nullptr;
    }
    if (!parcel.ReadInt32(read->uid_)) {
        WS_HILOGE("Failed to read the uid");
        delete read;
        return nullptr;
    }
    if (!parcel.ReadInt64(read->interval_)) {
        WS_HILOGE("Failed to read the interval");
        delete read;
        return nullptr;
    }
    return read;
}

int32_t FrequencyInfo::GetUid() const
{
    return uid_;
}

int32_t FrequencyInfo::GetWorkId() const
{
    return workId_;
}

int64_t FrequencyInfo::getInterval() const
{
    return interval_;
}

void FrequencyInfo::SetUid(int32_t uid)
{
    uid_ = uid;
}

void FrequencyInfo::SetWorkId(int32_t workId)
{
    workId_ = workId;
}

void FrequencyInfo::SetInterval(int64_t interval)
{
    interval_ = interval;
}

bool FrequencyInfo::ParseFromJson(const nlohmann::json &value)
{
    if (value.is_null() || value.empty() || !value.is_object) {
        WS_HILOGE("frequencyInfo json is empty or not object");
        return false;
    }
    if (!value.contains("workId") || !value["workId"].is_number_integer()) {
        WS_HILOGE("frequencyInfo json is invalid, workId is missing or not int");
        return false;
    }
    this->workId_ = value["workId"].get<int32_t>();
    if (!value.contains("uid") || !value["uid"].is_number_integer()) {
        WS_HILOGE("frequencyInfo json is invalid, uid is missing or not int");
        return false;
    }
    this->uid_ = value["uid"].get<int32_t>();
    if (!value.contains("interval") || !value["interval"].is_number_integer()) {
        WS_HILOGE("frequencyInfo json is invalid, interval is missing or not int");
        return false;
    }
    this->interval_ = value["interval"].get<int64_t>();
    return true;
}

std::string FrequencyInfo::ParseToJsonStr()
{
    nlohmann::json root;
    root["uid"] = uid_;
    root["workId"] = workId_;
    root["interval"] = interval_;
    return root.dump(JSON_INDENT_WIDTH, ' ', false, nlohmann::json::error_handler_t::replace);
}
} // namespace WorkScheduler
} // namespace OHOS