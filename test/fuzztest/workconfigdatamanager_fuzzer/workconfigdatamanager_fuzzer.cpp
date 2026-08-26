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

#include "workconfigdatamanager_fuzzer.h"

#include <algorithm>
#include <cstdint>
#include <list>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include "event_runner.h"
#include "watchdog.h"
#include "work_event_handler.h"
#include "work_policy_manager.h"
#include "work_queue_manager.h"
#include "work_sched_config.h"
#include "work_sched_constants.h"
#include "work_sched_data_manager.h"
#include "work_scheduler_service.h"

void OHOS::RefBase::DecStrongRef(void const *obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr const char *WORKSCHEDULER_SERVICE_RUNNER_NAME = "WorkSchedulerService";
constexpr size_t MAX_STRING_SIZE = 48;
constexpr int32_t DEFAULT_UID = 10000;
constexpr int32_t MAX_ITEMS = 8;
constexpr int32_t FUZZ_INVALID_LOW_VALUE = -2;
constexpr int32_t FUZZ_INVALID_VALUE = -1;
constexpr int32_t GROUP_BUCKET_COUNT = 2;
constexpr int32_t CONFIG_OTHER_KEY_CASE = 3;
constexpr int32_t MAX_SOURCE_TYPE_FOR_FUZZ = 4;
constexpr int32_t MAX_GROUP_VALUE_FOR_FUZZ = 5;
constexpr int32_t INVALID_JSON_RATIO_FOR_FUZZ = 8;
constexpr int32_t MAX_CACHE_GROUP_FOR_FUZZ = 10;
constexpr int32_t MAX_USER_ID_FOR_FUZZ = 200;

std::string ConsumeText(FuzzedDataProvider &provider, const std::string &fallback)
{
    std::string text = provider.ConsumeRandomLengthString(MAX_STRING_SIZE);
    if (text.empty()) {
        return fallback;
    }
    std::replace_if(text.begin(), text.end(), [](char chr) { return chr == '\0' || chr == '\n' || chr == '\r'; }, '_');
    return text;
}

std::string EscapeJsonString(std::string value)
{
    std::string escaped;
    escaped.reserve(value.size());
    for (char chr : value) {
        if (chr == '"' || chr == '\\') {
            escaped.push_back('\\');
        }
        if (static_cast<unsigned char>(chr) < 0x20) {
            escaped.push_back('_');
            continue;
        }
        escaped.push_back(chr);
    }
    return escaped;
}

std::vector<std::string> BuildBundleList(FuzzedDataProvider &provider)
{
    std::vector<std::string> bundles = {
        "bundle.config.demo",
        "bundle.config.extra",
        "invalid_bundle",
    };
    int32_t count = provider.ConsumeIntegralInRange<int32_t>(0, MAX_ITEMS);
    for (int32_t index = 0; index < count; ++index) {
        bundles.emplace_back(ConsumeText(provider, "bundle.generated." + std::to_string(index)));
    }
    return bundles;
}

std::string BuildConfigJson(FuzzedDataProvider &provider, const std::vector<std::string> &bundles)
{
    switch (provider.ConsumeIntegralInRange<int32_t>(0, MAX_GROUP_VALUE_FOR_FUZZ)) {
        case 0:
            return "";
        case 1:
            return "{bad json";
        case GROUP_BUCKET_COUNT:
            return "{\"active_group_whitelist\":\"not_array\"}";
        case CONFIG_OTHER_KEY_CASE:
            return "{\"other_key\":[\"bundle.config.demo\"]}";
        default:
            break;
    }
    std::ostringstream builder;
    builder << "{\"active_group_whitelist\":[";
    for (size_t index = 0; index < bundles.size(); ++index) {
        if (index != 0) {
            builder << ",";
        }
        if (provider.ConsumeIntegralInRange<int32_t>(0, INVALID_JSON_RATIO_FOR_FUZZ) == 0) {
            builder << (provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_GROUP_VALUE_FOR_FUZZ));
        } else {
            builder << "\"" << (EscapeJsonString(bundles[index])) << "\"";
        }
    }
    builder << "]}";
    return builder.str();
}

std::shared_ptr<WorkSchedulerService> PrepareService()
{
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (!service->eventRunner_) {
        service->eventRunner_ = AppExecFwk::EventRunner::Create(WORKSCHEDULER_SERVICE_RUNNER_NAME,
            AppExecFwk::ThreadMode::FFRT);
    }
    if (!service->handler_) {
        service->handler_ = std::make_shared<WorkEventHandler>(service->eventRunner_, service);
    }
    service->workQueueManager_ = std::make_shared<WorkQueueManager>(service);
    service->workPolicyManager_ = std::make_shared<WorkPolicyManager>(service);
    service->workPolicyManager_->handler_ = service->handler_;
    service->workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
    service->workPolicyManager_->watchdog_ =
        std::make_shared<Watchdog>(service->workPolicyManager_, service->eventRunner_);
    service->ready_.store(true);
    service->checkBundle_.store(false);
    service->whitelist_.clear();
    service->whitelist_.insert(DEFAULT_UID);
    return service;
}

void DriveDeviceStandbyLists(DataManager &manager, const std::vector<std::string> &bundles,
    FuzzedDataProvider &provider)
{
    std::list<std::string> firstBatch;
    std::list<std::string> secondBatch;
    for (size_t index = 0; index < bundles.size(); ++index) {
        if ((index % GROUP_BUCKET_COUNT) == 0) {
            firstBatch.emplace_back(bundles[index]);
        } else {
            secondBatch.emplace_back(bundles[index]);
        }
    }
    manager.AddDeviceStandyWhitelist(firstBatch);
    manager.AddDeviceStandyRestrictlist(secondBatch);
    for (const auto &bundle : bundles) {
        manager.OnDeviceStandyWhitelistChanged(bundle, provider.ConsumeBool());
        manager.OnDeviceStandyRestrictlistChanged(bundle, provider.ConsumeBool());
        static_cast<void>(manager.IsInDeviceStandyWhitelist(bundle));
        static_cast<void>(manager.IsInDeviceStandyRestrictlist(bundle));
    }
    static_cast<void>(manager.IsDeviceStandyWhitelistEmpty());
    if (provider.ConsumeBool()) {
        manager.ClearDeviceStandyWhitelist();
    }
    if (provider.ConsumeBool()) {
        manager.ClearDeviceStandyRestrictlist();
    }
}

void DriveGroupCache(DataManager &manager, const std::vector<std::string> &bundles, FuzzedDataProvider &provider)
{
    for (const auto &bundle : bundles) {
        int32_t userId = provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_USER_ID_FOR_FUZZ);
        int32_t group = provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_LOW_VALUE, MAX_CACHE_GROUP_FOR_FUZZ);
        manager.AddGroup(bundle, userId, group);
        int32_t foundGroup = FUZZ_INVALID_VALUE;
        static_cast<void>(manager.FindGroup(bundle, userId, foundGroup));
        static_cast<void>(manager.FindGroup(bundle, userId + 1, foundGroup));
        if (provider.ConsumeBool()) {
            manager.ClearGroup(bundle, userId);
        }
    }
    if (provider.ConsumeBool()) {
        manager.ClearAllGroup();
    }
}
} // namespace

bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
{
    uid = DEFAULT_UID;
    return true;
}

void RunWorkConfigDataManagerFuzzer(FuzzedDataProvider &provider)
{
    auto service = PrepareService();
    auto config = DelayedSingleton<WorkSchedulerConfig>::GetInstance();
    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    auto bundles = BuildBundleList(provider);
    std::string configData = BuildConfigJson(provider, bundles);

    config->activeGroupWhitelist_.clear();
    config->InitActiveGroupWhitelist(configData);
    for (const auto &bundle : bundles) {
        static_cast<void>(config->IsInActiveGroupWhitelist(bundle));
    }
    int32_t sourceType = provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_LOW_VALUE, MAX_SOURCE_TYPE_FOR_FUZZ);
    service->SetWorkSchedulerConfig(configData, sourceType);

    dataManager->SetDeviceSleep(provider.ConsumeBool());
    dataManager->SetDeepIdle(provider.ConsumeBool());
    static_cast<void>(dataManager->GetDeviceSleep());
    static_cast<void>(dataManager->GetDeepIdle());
    DriveDeviceStandbyLists(*dataManager, bundles, provider);
    DriveGroupCache(*dataManager, bundles, provider);
    service->InitDeviceStandyWhitelist();
    service->InitDeviceStandyRestrictlist();
    static_cast<void>(service->IsExemptionBundle(ConsumeText(provider, "bundle.config.demo")));
    static_cast<void>(service->IsPreinstalledBundle(ConsumeText(provider, "bundle.config.demo")));
    std::string result;
    service->DumpAllInfo(result);
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    OHOS::WorkScheduler::RunWorkConfigDataManagerFuzzer(provider);
    return 0;
}
