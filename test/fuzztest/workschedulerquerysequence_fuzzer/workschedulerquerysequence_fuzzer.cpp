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

#include "workschedulerquerysequence_fuzzer.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include "detector_value.h"
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
constexpr int32_t DEFAULT_REPEAT_MINUTES = 20;
constexpr int32_t REPEAT_SECONDS_PER_MINUTE = 60;
constexpr int32_t MILLISECONDS_PER_SECOND = 1000;
constexpr int32_t DEFAULT_REPEAT_CYCLE =
    DEFAULT_REPEAT_MINUTES * REPEAT_SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
constexpr int32_t MAX_WORKS_PER_INPUT = 6;
constexpr int32_t FUZZ_INVALID_VALUE = -1;
constexpr int32_t FUZZ_BOUND_TWO = 2;
constexpr int32_t MAX_APP_INDEX_FOR_FUZZ = 3;
constexpr int32_t MAX_REPEAT_COUNT_FOR_FUZZ = 4;
constexpr int32_t FUZZ_BOUND_FIVE = 5;
constexpr int32_t FUZZ_BOUND_SIX = 6;
constexpr int32_t QUERY_ACTION_CASE_MAX = 7;
constexpr int32_t MAX_QUERY_ROUNDS_FOR_FUZZ = 8;
constexpr int32_t UID_RANGE_PADDING_FOR_FUZZ = 10;
constexpr int32_t MAX_WORK_ID_FOR_FUZZ = 100000;

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
    service->persistedMap_.clear();
    return service;
}

WorkInfo BuildWorkInfo(FuzzedDataProvider &provider, int32_t index)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(provider.ConsumeBool() ? index + 1 : provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_VALUE, MAX_WORK_ID_FOR_FUZZ));
    workInfo.SetElement(ConsumeText(provider, "bundle.query.demo"), ConsumeText(provider, "AbilityQuery"));
    workInfo.RefreshUid(provider.ConsumeBool() ? DEFAULT_UID :
        provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, DEFAULT_UID + UID_RANGE_PADDING_FOR_FUZZ));
    workInfo.RefreshAppIndex(provider.ConsumeIntegralInRange<int32_t>(0, MAX_APP_INDEX_FOR_FUZZ));
    workInfo.RequestPersisted(provider.ConsumeBool());
    workInfo.SetCallBySystemApp(true);
    if (provider.ConsumeBool()) {
        workInfo.RequestNetworkType(static_cast<WorkCondition::Network>(provider.ConsumeIntegralInRange<int32_t>(
            WorkCondition::Network::NETWORK_TYPE_ANY, WorkCondition::Network::NETWORK_TYPE_ETHERNET)));
    }
    if (provider.ConsumeBool()) {
        workInfo.RequestStorageLevel(static_cast<WorkCondition::Storage>(provider.ConsumeIntegralInRange<int32_t>(
            WorkCondition::Storage::STORAGE_LEVEL_LOW, WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY)));
    }
    if (provider.ConsumeBool()) {
        workInfo.RequestRepeatCycle(
            provider.ConsumeIntegralInRange<uint32_t>(DEFAULT_REPEAT_CYCLE, DEFAULT_REPEAT_CYCLE * FUZZ_BOUND_TWO),
            provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_REPEAT_COUNT_FOR_FUZZ));
    }
    return workInfo;
}

std::vector<WorkInfo> AddPresetWorks(const std::shared_ptr<WorkSchedulerService> &service, FuzzedDataProvider &provider)
{
    std::vector<WorkInfo> works;
    int32_t count = provider.ConsumeIntegralInRange<int32_t>(1, MAX_WORKS_PER_INPUT);
    for (int32_t index = 0; index < count; ++index) {
        auto workInfo = BuildWorkInfo(provider, index);
        int32_t uid = workInfo.GetUid() >= 0 ? workInfo.GetUid() : DEFAULT_UID;
        if (provider.ConsumeBool()) {
            service->StartWorkInner(workInfo, uid);
        } else {
            auto workStatus = std::make_shared<WorkStatus>(workInfo, uid);
            if (provider.ConsumeBool()) {
                workStatus->MarkStatus(WorkStatus::Status::RUNNING);
            }
            if (provider.ConsumeBool()) {
                workStatus->SetTimeout(true);
                workStatus->lastTimeout_ = true;
            }
            service->workPolicyManager_->AddWork(workStatus, uid);
            service->workQueueManager_->AddWork(workStatus);
        }
        works.emplace_back(workInfo);
    }
    return works;
}

void DriveQueryApis(const std::shared_ptr<WorkSchedulerService> &service, std::vector<WorkInfo> &works,
    FuzzedDataProvider &provider)
{
    std::vector<WorkInfo> workInfos;
    service->ObtainAllWorks(workInfos);
    for (int32_t index = 0; index < provider.ConsumeIntegralInRange<int32_t>(1, MAX_QUERY_ROUNDS_FOR_FUZZ); ++index) {
        auto &pivot = works[provider.ConsumeIntegralInRange<size_t>(0, works.size() - 1)];
        int32_t uid = provider.ConsumeBool() ? DEFAULT_UID :
            provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, DEFAULT_UID + UID_RANGE_PADDING_FOR_FUZZ);
        int32_t workId = provider.ConsumeBool() ? pivot.GetWorkId() :
            provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_WORK_ID_FOR_FUZZ);
        WorkInfo statusInfo;
        bool timeoutResult = false;
        std::vector<WorkInfo> innerWorks;
        service->ObtainWorksByUidAndWorkIdForInner(uid, innerWorks, workId);
        service->GetWorkStatus(workId, statusInfo);
        service->IsLastWorkTimeout(workId, timeoutResult);
        service->GetAllRunningWorks(workInfos);
        int32_t uidCopy = uid;
        int32_t workIdCopy = workId;
        service->workPolicyManager_->ObtainAllWorks(uidCopy);
        service->workPolicyManager_->GetWorkStatus(uidCopy, workIdCopy);
        service->workPolicyManager_->GetAllWorkStatus(uidCopy);
        service->workPolicyManager_->IsLastWorkTimeout(workIdCopy, uidCopy, timeoutResult);
    }
}

void DriveLifecycleAroundQueries(const std::shared_ptr<WorkSchedulerService> &service,
    std::vector<WorkInfo> &works, FuzzedDataProvider &provider)
{
    for (int32_t index = 0; index < provider.ConsumeIntegralInRange<int32_t>(1, MAX_QUERY_ROUNDS_FOR_FUZZ); ++index) {
        auto &pivot = works[provider.ConsumeIntegralInRange<size_t>(0, works.size() - 1)];
        switch (provider.ConsumeIntegralInRange<int32_t>(0, QUERY_ACTION_CASE_MAX)) {
            case 0:
                service->PauseRunningWorks(provider.ConsumeBool() ? DEFAULT_UID : pivot.GetUid());
                break;
            case 1:
                service->ResumePausedWorks(provider.ConsumeBool() ? DEFAULT_UID : pivot.GetUid());
                break;
            case FUZZ_BOUND_TWO:
                service->StopWorkForInner(pivot, provider.ConsumeBool());
                break;
            case MAX_APP_INDEX_FOR_FUZZ:
                service->StopWork(pivot);
                break;
            case MAX_REPEAT_COUNT_FOR_FUZZ:
                service->StopAndCancelWork(pivot);
                break;
            case FUZZ_BOUND_FIVE:
                service->StartWorkInner(pivot, provider.ConsumeBool() ? DEFAULT_UID : pivot.GetUid());
                break;
            case FUZZ_BOUND_SIX:
                service->StopAndClearWorksByUid(provider.ConsumeBool() ? DEFAULT_UID : pivot.GetUid());
                break;
            default:
                service->TriggerWorkIfConditionReady();
                break;
        }
    }
}
} // namespace

bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
{
    uid = DEFAULT_UID;
    return true;
}

void RunWorkSchedulerQuerySequenceFuzzer(FuzzedDataProvider &provider)
{
    auto service = PrepareService();
    auto works = AddPresetWorks(service, provider);
    if (works.empty()) {
        return;
    }
    DriveQueryApis(service, works, provider);
    DriveLifecycleAroundQueries(service, works, provider);
    DriveQueryApis(service, works, provider);
    std::string result;
    service->workPolicyManager_->Dump(result);
    service->workQueueManager_->Dump(result);
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    OHOS::WorkScheduler::RunWorkSchedulerQuerySequenceFuzzer(provider);
    return 0;
}
