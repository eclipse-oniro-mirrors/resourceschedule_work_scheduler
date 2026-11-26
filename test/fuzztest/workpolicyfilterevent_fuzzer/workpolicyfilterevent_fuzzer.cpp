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

#include "workpolicyfilterevent_fuzzer.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include "detector_value.h"
#include "event_publisher.h"
#include "event_runner.h"
#include "policy/cpu_policy.h"
#include "policy/memory_policy.h"
#include "policy/power_mode_policy.h"
#include "policy/thermal_policy.h"
#include "policy_type.h"
#include "watchdog.h"
#include "work_condition.h"
#include "work_event_handler.h"
#include "work_info.h"
#include "work_policy_manager.h"
#include "work_queue_manager.h"
#include "work_sched_constants.h"
#include "work_sched_system_policy.h"
#include "work_scheduler_service.h"
#include "work_status.h"

void OHOS::RefBase::DecStrongRef(void const *obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr const char *WORKSCHEDULER_SERVICE_RUNNER_NAME = "WorkSchedulerService";
constexpr size_t MAX_STRING_SIZE = 48;
constexpr int32_t DEFAULT_UID = 10000;
constexpr int32_t DEFAULT_REPEAT_MINUTES = 20;
constexpr int32_t REPEAT_SECONDS_PER_MINUTE = 60;
constexpr int32_t MILLISECONDS_PER_SECOND = 1000;
constexpr int32_t DEFAULT_REPEAT_CYCLE =
    DEFAULT_REPEAT_MINUTES * REPEAT_SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
constexpr int32_t MAX_WORKS_PER_INPUT = 5;
constexpr int32_t FUZZ_INVALID_VALUE = -1;
constexpr int32_t GROUP_BUCKET_COUNT = 2;
constexpr int32_t MAX_REPEAT_COUNT_FOR_FUZZ = 3;
constexpr int32_t MAX_POLICY_DUMP_ROUNDS_FOR_FUZZ = 5;
constexpr int32_t MAX_POLICY_VALUE_FOR_FUZZ = 8;
constexpr int32_t MAX_THERMAL_LEVEL_FOR_FUZZ = 10;
constexpr int32_t MAX_RUNNING_COUNT_FOR_FUZZ = 20;
constexpr int32_t MAX_CPU_USAGE_FOR_FUZZ = 120;
constexpr int32_t MAX_USER_ID_FOR_FUZZ = 200;
constexpr int32_t MEMORY_UNIT_KB = 1024;
constexpr int32_t MAX_DETECTOR_VALUE_FOR_FUZZ = 100000;

std::string ConsumeText(FuzzedDataProvider &provider, const std::string &fallback)
{
    std::string text = provider.ConsumeRandomLengthString(MAX_STRING_SIZE);
    if (text.empty()) {
        return fallback;
    }
    std::replace_if(text.begin(), text.end(), [](char chr) { return chr == '\0' || chr == '\n' || chr == '\r'; }, '_');
    return text;
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

WorkInfo BuildWorkInfo(FuzzedDataProvider &provider, int32_t index)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(index + 1);
    workInfo.SetElement(ConsumeText(provider, "bundle.policy.demo"), ConsumeText(provider, "AbilityPolicy"));
    workInfo.RefreshUid(DEFAULT_UID);
    workInfo.RequestPersisted(provider.ConsumeBool());
    workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(provider.ConsumeIntegralInRange<int32_t>(
        WorkCondition::Network::NETWORK_TYPE_ANY, WorkCondition::Network::NETWORK_TYPE_ETHERNET)));
    workInfo.RequestStorageLevel(static_cast<WorkCondition::Storage>(provider.ConsumeIntegralInRange<int32_t>(
        WorkCondition::Storage::STORAGE_LEVEL_LOW, WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY)));
    workInfo.RequestRepeatCycle(provider.ConsumeIntegralInRange<uint32_t>(0, DEFAULT_REPEAT_CYCLE),
        provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_REPEAT_COUNT_FOR_FUZZ));
    return workInfo;
}

void AddPresetWorks(const std::shared_ptr<WorkSchedulerService> &service, FuzzedDataProvider &provider)
{
    int32_t count = provider.ConsumeIntegralInRange<int32_t>(1, MAX_WORKS_PER_INPUT);
    for (int32_t index = 0; index < count; ++index) {
        auto workInfo = BuildWorkInfo(provider, index);
        auto workStatus = std::make_shared<WorkStatus>(workInfo, DEFAULT_UID + (index % GROUP_BUCKET_COUNT));
        workStatus->priority_ = provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_POLICY_VALUE_FOR_FUZZ);
        if (provider.ConsumeBool()) {
            workStatus->MarkStatus(WorkStatus::Status::CONDITION_READY);
        }
        service->workPolicyManager_->AddWork(workStatus, workStatus->uid_);
        service->workQueueManager_->AddWork(workStatus);
    }
}

WorkSchedSystemPolicy BuildSystemPolicy(FuzzedDataProvider &provider)
{
    WorkSchedSystemPolicy policy;
    policy.policyName = ConsumeText(provider, "NONE");
    policy.cpuUsage = provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_CPU_USAGE_FOR_FUZZ);
    policy.memAvailable = provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MEMORY_UNIT_KB * MEMORY_UNIT_KB);
    policy.thermalLevel = provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_THERMAL_LEVEL_FOR_FUZZ);
    policy.powerMode = provider.ConsumeIntegralInRange<uint32_t>(0, MAX_POLICY_VALUE_FOR_FUZZ);
    return policy;
}

std::pair<std::string, std::string> BuildEventPair(FuzzedDataProvider &provider)
{
    const std::vector<std::pair<std::string, std::string>> events = {
        {"event", "info"},
        {"network", "wifi"},
        {"network", "disconnect"},
        {"network", "invalid"},
        {"charging", "usb"},
        {"charging", "ac"},
        {"charging", "wireless"},
        {"charging", "none"},
        {"storage", "low"},
        {"storage", "ok"},
        {"batteryStatus", "low"},
        {"batteryStatus", "ok"},
        {ConsumeText(provider, "invalidEvent"), ConsumeText(provider, "invalidValue")},
    };
    return events[provider.ConsumeIntegralInRange<size_t>(0, events.size() - 1)];
}

void DriveEventPublisher(EventPublisher &publisher, FuzzedDataProvider &provider)
{
    std::string result;
    for (int32_t index = 0; index < provider.ConsumeIntegralInRange<int32_t>(1, MAX_POLICY_VALUE_FOR_FUZZ); ++index) {
        auto event = BuildEventPair(provider);
        publisher.Dump(result, event.first, event.second);
        publisher.PublishEvent(result, event.first, event.second);
        publisher.PublishNetworkEvent(result, event.second);
        publisher.PublishChargingEvent(result, event.second);
        publisher.PublishStorageEvent(result, event.second);
        publisher.PublishBatteryStatusEvent(result, event.second);
    }
}

void DrivePolicyFilters(const std::shared_ptr<WorkPolicyManager> &manager, FuzzedDataProvider &provider)
{
    auto memoryPolicy = std::make_shared<MemoryPolicy>(manager);
    auto cpuPolicy = std::make_shared<CpuPolicy>(manager);
    manager->AddPolicyFilter(memoryPolicy);
    manager->AddPolicyFilter(cpuPolicy);
#ifdef POWERMGR_THERMAL_MANAGER_ENABLE
    auto thermalPolicy = std::make_shared<ThermalPolicy>(manager);
    manager->AddPolicyFilter(thermalPolicy);
#endif
#ifdef POWERMGR_POWER_MANAGER_ENABLE
    auto powerModePolicy = std::make_shared<PowerModePolicy>(manager);
    manager->AddPolicyFilter(powerModePolicy);
#endif

    for (int32_t index = 0; index < provider.ConsumeIntegralInRange<int32_t>(
        1, MAX_POLICY_DUMP_ROUNDS_FOR_FUZZ); ++index) {
        manager->SetMemoryByDump(provider.ConsumeIntegralInRange<int32_t>(
            FUZZ_INVALID_VALUE, MEMORY_UNIT_KB * MEMORY_UNIT_KB));
        manager->SetCpuUsageByDump(provider.ConsumeIntegralInRange<int32_t>(
            FUZZ_INVALID_VALUE, MAX_CPU_USAGE_FOR_FUZZ));
        manager->SetThermalLevelByDump(provider.ConsumeIntegralInRange<int32_t>(
            FUZZ_INVALID_VALUE, MAX_THERMAL_LEVEL_FOR_FUZZ));
        manager->SetMaxRunningCountByDump(provider.ConsumeIntegralInRange<int32_t>(
            FUZZ_INVALID_VALUE, MAX_RUNNING_COUNT_FOR_FUZZ));
        manager->SetWatchdogTimeByDump(provider.ConsumeIntegralInRange<int32_t>(0, DEFAULT_REPEAT_CYCLE));
        auto policy = BuildSystemPolicy(provider);
        static_cast<void>(memoryPolicy->GetPolicyMaxRunning(policy));
        static_cast<void>(cpuPolicy->GetPolicyMaxRunning(policy));
#ifdef POWERMGR_THERMAL_MANAGER_ENABLE
        static_cast<void>(thermalPolicy->GetPolicyMaxRunning(policy));
#endif
#ifdef POWERMGR_POWER_MANAGER_ENABLE
        static_cast<void>(powerModePolicy->GetPolicyMaxRunning(policy));
#endif
        static_cast<void>(manager->GetMaxRunningCount(policy));
        static_cast<void>(policy.GetInfo());
        policy.SetPolicyName(ConsumeText(provider, "policyName"), provider.ConsumeIntegralInRange<int32_t>(
            FUZZ_INVALID_VALUE, MAX_RUNNING_COUNT_FOR_FUZZ));
    }
}

void DrivePolicyEvents(const std::shared_ptr<WorkPolicyManager> &manager, FuzzedDataProvider &provider)
{
    const std::vector<PolicyType> policies = {
        PolicyType::APP_REMOVED,
        PolicyType::APP_DATA_CLEAR,
        PolicyType::APP_ADDED,
        PolicyType::APP_CHANGED,
        PolicyType::USER_SWITCHED,
        PolicyType::USER_STARTED,
    };
    for (int32_t index = 0; index < provider.ConsumeIntegralInRange<int32_t>(1, MAX_POLICY_VALUE_FOR_FUZZ); ++index) {
        auto detector = std::make_shared<DetectorValue>(provider.ConsumeIntegralInRange<int32_t>(
            FUZZ_INVALID_VALUE, MAX_DETECTOR_VALUE_FOR_FUZZ),
            provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_USER_ID_FOR_FUZZ), provider.ConsumeBool(),
            ConsumeText(provider, "bundle.policy.demo"));
        manager->OnPolicyChanged(policies[provider.ConsumeIntegralInRange<size_t>(0, policies.size() - 1)], detector);
    }
}
} // namespace

bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
{
    uid = DEFAULT_UID;
    return true;
}

void RunWorkPolicyFilterEventFuzzer(FuzzedDataProvider &provider)
{
    auto service = PrepareService();
    auto manager = service->workPolicyManager_;
    AddPresetWorks(service, provider);

    EventPublisher publisher;
    DriveEventPublisher(publisher, provider);
    DrivePolicyFilters(manager, provider);
    DrivePolicyEvents(manager, provider);

    manager->CheckWorkToRun();
    manager->TriggerIdeWork();
    manager->SendRetrigger(provider.ConsumeIntegralInRange<int32_t>(0, DEFAULT_REPEAT_CYCLE));
    int32_t uid = DEFAULT_UID;
    static_cast<void>(manager->ObtainAllWorks(uid));
    static_cast<void>(manager->GetAllRunningWorks());
    static_cast<void>(manager->FindWork(provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_VALUE, DEFAULT_UID + GROUP_BUCKET_COUNT)));
    static_cast<void>(manager->FindWork(provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_VALUE, MAX_USER_ID_FOR_FUZZ),
        ConsumeText(provider, "bundle.policy.demo")));
    std::string result;
    manager->Dump(result);
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    OHOS::WorkScheduler::RunWorkPolicyFilterEventFuzzer(provider);
    return 0;
}
