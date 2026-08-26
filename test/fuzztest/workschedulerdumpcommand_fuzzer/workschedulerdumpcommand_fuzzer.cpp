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

#include "workschedulerdumpcommand_fuzzer.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include "event_runner.h"
#include "watchdog.h"
#include "work_condition.h"
#include "work_event_handler.h"
#include "work_info.h"
#include "work_policy_manager.h"
#include "work_queue_manager.h"
#include "work_sched_constants.h"
#include "work_scheduler_service.h"
#include "work_status.h"

void OHOS::RefBase::DecStrongRef(void const *obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr const char *WORKSCHEDULER_SERVICE_RUNNER_NAME = "WorkSchedulerService";
constexpr size_t MAX_STRING_SIZE = 48;
constexpr int32_t DEFAULT_UID = 10000;
constexpr int32_t DEFAULT_WORK_ID = 10001;
constexpr int32_t DEFAULT_SA = 1904;
constexpr uint32_t DEFAULT_REPEAT_MINUTES = 20;
constexpr uint32_t REPEAT_SECONDS_PER_MINUTE = 60;
constexpr uint32_t MILLISECONDS_PER_SECOND = 1000;
constexpr uint32_t DEFAULT_REPEAT_CYCLE =
    DEFAULT_REPEAT_MINUTES * REPEAT_SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
constexpr int32_t FUZZ_INVALID_VALUE = -1;
constexpr int32_t FUZZ_BOUND_TWO = 2;
constexpr int32_t FUZZ_BOUND_THREE = 3;
constexpr int32_t FUZZ_BOUND_FOUR = 4;
constexpr int32_t FUZZ_BOUND_FIVE = 5;
constexpr int32_t FUZZ_BOUND_SIX = 6;
constexpr int32_t FUZZ_BOUND_SEVEN = 7;
constexpr int32_t FUZZ_BOUND_EIGHT = 8;
constexpr int32_t FUZZ_BOUND_NINE = 9;
constexpr int32_t FUZZ_BOUND_TEN = 10;
constexpr int32_t FUZZ_BOUND_ELEVEN = 11;
constexpr int32_t FUZZ_BOUND_TWELVE = 12;
constexpr int32_t FUZZ_BOUND_THIRTEEN = 13;
constexpr int32_t FUZZ_BOUND_FOURTEEN = 14;
constexpr int32_t DUMP_ARG_CASE_MAX = 15;
constexpr int32_t MAX_WATCHDOG_ID_FOR_FUZZ = 16;
constexpr int32_t MAX_BATTERY_LEVEL_FOR_FUZZ = 100;
constexpr int32_t MAX_DUMP_VALUE_FOR_FUZZ = 1000;
constexpr int32_t SECONDS_PER_HOUR = 3600;
constexpr int32_t MAX_WORK_ID_FOR_FUZZ = 100000;
constexpr int32_t MAX_DUMP_TIME_FOR_FUZZ = 600000;

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
    service->handler_ = std::make_shared<WorkEventHandler>(service->eventRunner_, service);
    service->workQueueManager_ = std::make_shared<WorkQueueManager>(service);
    service->workPolicyManager_ = std::make_shared<WorkPolicyManager>(service);
    service->workPolicyManager_->handler_ = service->handler_;
    service->workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
    service->workPolicyManager_->watchdog_ =
        std::make_shared<Watchdog>(service->workPolicyManager_, service->eventRunner_);
    service->whitelist_.clear();
    service->whitelist_.insert(DEFAULT_UID);
    service->ready_.store(true);
    service->checkBundle_.store(false);
    return service;
}

std::shared_ptr<WorkStatus> AddPresetWork(const std::shared_ptr<WorkSchedulerService> &service,
    FuzzedDataProvider &provider)
{
    WorkInfo workInfo;
    int32_t workId = provider.ConsumeBool() ? DEFAULT_WORK_ID : provider.ConsumeIntegralInRange<int32_t>(
        1, MAX_WORK_ID_FOR_FUZZ);
    workInfo.SetWorkId(workId);
    workInfo.SetElement(provider.ConsumeBool() ? "bundleName" : ConsumeText(provider, "bundleName"),
        provider.ConsumeBool() ? "abilityName" : ConsumeText(provider, "abilityName"));
    workInfo.RefreshUid(DEFAULT_UID);
    workInfo.RequestPersisted(false);
    workInfo.RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_WIFI);
    workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
    workInfo.RequestBatteryLevel(provider.ConsumeIntegralInRange<int32_t>(0, MAX_BATTERY_LEVEL_FOR_FUZZ));
    workInfo.RequestRepeatCycle(provider.ConsumeBool() ? DEFAULT_REPEAT_CYCLE :
        provider.ConsumeIntegralInRange<uint32_t>(1, DEFAULT_REPEAT_CYCLE * FUZZ_BOUND_TWO));

    auto workStatus = std::make_shared<WorkStatus>(workInfo, DEFAULT_UID);
    service->workPolicyManager_->AddWork(workStatus, DEFAULT_UID);
    service->workQueueManager_->AddWork(workStatus);
    if (provider.ConsumeBool()) {
        workStatus->MarkStatus(WorkStatus::Status::RUNNING);
        service->workPolicyManager_->watchdogIdMap_[provider.ConsumeIntegralInRange<uint32_t>(
            1, MAX_WATCHDOG_ID_FOR_FUZZ)] = workStatus;
    }
    return workStatus;
}

void AddPresetSaWork(const std::shared_ptr<WorkSchedulerService> &service, FuzzedDataProvider &provider)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(provider.ConsumeIntegralInRange<int32_t>(1, MAX_WORK_ID_FOR_FUZZ));
    workInfo.RefreshUid(DEFAULT_UID);
    workInfo.RefreshSaId(DEFAULT_SA);
    workInfo.RequestDeepIdle(provider.ConsumeBool());
    workInfo.deepIdleTime_ = provider.ConsumeIntegralInRange<int32_t>(0, SECONDS_PER_HOUR);
    auto workStatus = std::make_shared<WorkStatus>(workInfo, DEFAULT_UID);
    service->workPolicyManager_->AddWork(workStatus, DEFAULT_UID);
    service->workQueueManager_->AddWork(workStatus);
}

std::string ConsumeNumberLike(FuzzedDataProvider &provider, int32_t minValue, int32_t maxValue)
{
    if (provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_BOUND_FIVE) == 0) {
        return ConsumeText(provider, "not_number");
    }
    return std::to_string(provider.ConsumeIntegralInRange<int32_t>(minValue, maxValue));
}

std::vector<std::string> BuildBasicDumpArgs(FuzzedDataProvider &provider, int32_t caseValue)
{
    switch (caseValue) {
        case 0:
            return {};
        case 1:
            return {"-h"};
        case FUZZ_BOUND_TWO:
            return {"-a"};
        case FUZZ_BOUND_THREE:
            return {"-r"};
        default:
            return {"-k", ConsumeText(provider, "persisted_work")};
    }
}

std::vector<std::string> BuildParamDumpArgs(FuzzedDataProvider &provider)
{
    const std::vector<std::string> keys = {
        "-memory", "-watchdog_time", "-repeat_time_min", "-min_interval", "-cpu", "-count", "-thermalLevel",
    };
    return {keys[provider.ConsumeIntegralInRange<size_t>(0, keys.size() - 1)],
        ConsumeNumberLike(provider, 0, MAX_DUMP_TIME_FOR_FUZZ)};
}

std::vector<std::string> BuildEventDumpArgs(FuzzedDataProvider &provider, int32_t caseValue)
{
    switch (caseValue) {
        case FUZZ_BOUND_SIX:
            return {"-d", provider.ConsumeBool() ? "battery" : ConsumeText(provider, "event"),
                ConsumeNumberLike(provider, FUZZ_INVALID_VALUE, FUZZ_BOUND_FIVE)};
        case FUZZ_BOUND_SEVEN:
            return {"-t", provider.ConsumeBool() ? "bundleName" : ConsumeText(provider, "bundleName"),
                provider.ConsumeBool() ? "abilityName" : ConsumeText(provider, "abilityName")};
        case FUZZ_BOUND_EIGHT:
            return {"-x", ConsumeNumberLike(provider, 0, DEFAULT_UID + FUZZ_BOUND_TWO),
                provider.ConsumeBool() ? "p" : "r"};
        default:
            return {"-s", std::to_string(DEFAULT_SA), std::to_string(DEFAULT_UID)};
    }
}

std::vector<std::string> BuildWorkDumpArgs(FuzzedDataProvider &provider, int32_t caseValue)
{
    std::string command = caseValue == FUZZ_BOUND_TEN ? "-g" : "-f";
    std::string workId = provider.ConsumeBool() ? std::to_string(DEFAULT_WORK_ID) :
        ConsumeNumberLike(provider, FUZZ_INVALID_VALUE, MAX_WORK_ID_FOR_FUZZ);
    return {command, ConsumeNumberLike(provider, FUZZ_INVALID_VALUE, DEFAULT_UID + FUZZ_BOUND_TWO), workId};
}

std::vector<std::string> BuildInvalidDumpArgs(FuzzedDataProvider &provider, int32_t caseValue)
{
    if (caseValue == FUZZ_BOUND_TWELVE) {
        return {ConsumeText(provider, "-bad")};
    }
    if (caseValue == FUZZ_BOUND_THIRTEEN) {
        return {"-memory", ConsumeText(provider, "bad_value")};
    }
    if (caseValue == FUZZ_BOUND_FOURTEEN) {
        return {"-x", "0", provider.ConsumeBool() ? "p" : "z"};
    }
    return {ConsumeText(provider, "-bad"), ConsumeText(provider, "bad_param"),
        ConsumeText(provider, "bad_value"), ConsumeText(provider, "tail")};
}

std::vector<std::string> BuildArgs(FuzzedDataProvider &provider)
{
    int32_t caseValue = provider.ConsumeIntegralInRange<int32_t>(0, DUMP_ARG_CASE_MAX);
    if (caseValue <= FUZZ_BOUND_FOUR) {
        return BuildBasicDumpArgs(provider, caseValue);
    }
    if (caseValue == FUZZ_BOUND_FIVE) {
        return BuildParamDumpArgs(provider);
    }
    if (caseValue <= FUZZ_BOUND_NINE) {
        return BuildEventDumpArgs(provider, caseValue);
    }
    if (caseValue <= FUZZ_BOUND_ELEVEN) {
        return BuildWorkDumpArgs(provider, caseValue);
    }
    return BuildInvalidDumpArgs(provider, caseValue);
}

void DriveDirectDumpEntrypoints(const std::shared_ptr<WorkSchedulerService> &service, FuzzedDataProvider &provider)
{
    std::string result;
    auto engArgs = BuildArgs(provider);
    service->DumpProcessForEngMode(engArgs, result);
    std::vector<std::string> userArgs = {"-t", "bundleName", "abilityName"};
    service->DumpProcessForUserMode(userArgs, result);
    userArgs = {"-s", std::to_string(DEFAULT_SA), std::to_string(DEFAULT_UID)};
    service->DumpProcessForUserMode(userArgs, result);
    service->DumpUsage(result);
    service->DumpAllInfo(result);
    std::string key = "-memory";
    std::string value = ConsumeNumberLike(provider, 0, MAX_DUMP_VALUE_FOR_FUZZ);
    service->DumpParamSet(key, value, result);
    key = "-watchdog_time";
    value = ConsumeNumberLike(provider, 0, LONG_WATCHDOG_TIME);
    service->DumpParamSet(key, value, result);
    service->DumpParamRestore(result);
    service->DumpProcessWorks("bundleName", "abilityName", result);
    service->DumpRunningWorks(std::to_string(DEFAULT_UID), provider.ConsumeBool() ? "p" : "r", result);
    service->DumpLoadSaWorks(std::to_string(DEFAULT_SA), std::to_string(DEFAULT_UID), result);
    service->DumpGetWorks(std::to_string(DEFAULT_UID), std::to_string(DEFAULT_WORK_ID), result);
    service->DumpTriggerWork(std::to_string(DEFAULT_UID), std::to_string(DEFAULT_WORK_ID), result);
}
} // namespace

bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
{
    uid = DEFAULT_UID;
    return true;
}

void RunWorkSchedulerDumpCommandFuzzer(FuzzedDataProvider &provider)
{
    auto service = PrepareService();
    auto presetWork = AddPresetWork(service, provider);
    AddPresetSaWork(service, provider);

    std::string result;
    for (int32_t i = 0; i < provider.ConsumeIntegralInRange<int32_t>(1, FUZZ_BOUND_EIGHT); ++i) {
        auto args = BuildArgs(provider);
        service->DumpProcessForEngMode(args, result);
    }
    DriveDirectDumpEntrypoints(service, provider);
    service->workPolicyManager_->DumpTriggerWork(DEFAULT_UID, presetWork->workInfo_->GetWorkId(), result);
    service->workPolicyManager_->DumpCheckIdeWorkToRun("bundleName", "abilityName");
    service->workPolicyManager_->TriggerIdeWork();
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    OHOS::WorkScheduler::RunWorkSchedulerDumpCommandFuzzer(provider);
    return 0;
}
