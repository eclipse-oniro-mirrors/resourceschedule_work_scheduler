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

#include "workschedulersadeepidle_fuzzer.h"

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
#include "work_sched_data_manager.h"
#include "work_scheduler_service.h"
#include "work_status.h"

void OHOS::RefBase::DecStrongRef(void const *obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr const char *WORKSCHEDULER_SERVICE_RUNNER_NAME = "WorkSchedulerService";
constexpr size_t MAX_STRING_SIZE = 48;
constexpr int32_t DEFAULT_UID = 10000;
constexpr int32_t DEFAULT_SA_ID = 1905;
constexpr int32_t DEFAULT_REPEAT_MINUTES = 20;
constexpr int32_t REPEAT_SECONDS_PER_MINUTE = 60;
constexpr int32_t MILLISECONDS_PER_SECOND = 1000;
constexpr int32_t DEFAULT_REPEAT_CYCLE =
    DEFAULT_REPEAT_MINUTES * REPEAT_SECONDS_PER_MINUTE * MILLISECONDS_PER_SECOND;
constexpr int32_t MAX_SA_WORKS = 5;
constexpr int32_t FUZZ_INVALID_VALUE = -1;
constexpr int32_t FUZZ_BOUND_TWO = 2;
constexpr int32_t MAX_REPEAT_COUNT_FOR_FUZZ = 3;
constexpr int32_t FUZZ_BOUND_FOUR = 4;
constexpr int32_t SA_ACTION_CASE_MAX = 5;
constexpr int32_t MAX_DEEP_IDLE_ROUNDS_FOR_FUZZ = 8;
constexpr int32_t UID_RANGE_PADDING_FOR_FUZZ = 10;
constexpr int32_t MAX_SA_ID_FOR_FUZZ = 5000;

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
    service->deepIdleTimeMap_.clear();
    return service;
}

WorkInfo BuildSaWorkInfo(FuzzedDataProvider &provider, int32_t index)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(index + 1);
    workInfo.SetElement(ConsumeText(provider, "bundle.sa.demo"), ConsumeText(provider, "AbilitySa"));
    workInfo.RefreshUid(provider.ConsumeBool() ? DEFAULT_UID :
        provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, DEFAULT_UID + UID_RANGE_PADDING_FOR_FUZZ));
    int32_t saId = provider.ConsumeBool() ? DEFAULT_SA_ID + index : provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_VALUE, MAX_SA_ID_FOR_FUZZ);
    workInfo.RefreshSaId(saId);
    workInfo.RequestPersisted(provider.ConsumeBool());
    workInfo.RequestDeepIdle(provider.ConsumeBool());
    workInfo.deepIdleTime_ = provider.ConsumeIntegralInRange<int32_t>(
        FUZZ_INVALID_VALUE, DEFAULT_REPEAT_CYCLE * FUZZ_BOUND_TWO);
    if (provider.ConsumeBool()) {
        workInfo.RequestRepeatCycle(provider.ConsumeIntegralInRange<uint32_t>(0, DEFAULT_REPEAT_CYCLE),
            provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_REPEAT_COUNT_FOR_FUZZ));
    }
    return workInfo;
}

std::vector<std::shared_ptr<WorkStatus>> AddSaWorks(const std::shared_ptr<WorkSchedulerService> &service,
    FuzzedDataProvider &provider)
{
    std::vector<std::shared_ptr<WorkStatus>> works;
    int32_t count = provider.ConsumeIntegralInRange<int32_t>(1, MAX_SA_WORKS);
    for (int32_t index = 0; index < count; ++index) {
        auto workInfo = BuildSaWorkInfo(provider, index);
        int32_t uid = workInfo.GetUid() >= 0 ? workInfo.GetUid() : DEFAULT_UID;
        auto workStatus = std::make_shared<WorkStatus>(workInfo, uid);
        workStatus->MarkStatus(provider.ConsumeBool() ? WorkStatus::Status::CONDITION_READY :
            WorkStatus::Status::RUNNING);
        service->workPolicyManager_->AddWork(workStatus, uid);
        service->workQueueManager_->AddWork(workStatus);
        if (workInfo.deepIdleTime_ >= 0) {
            service->deepIdleTimeMap_[workInfo.GetSaId()] = std::make_pair(workInfo.deepIdleTime_, uid);
        }
        works.emplace_back(workStatus);
    }
    return works;
}

bool QueryDeepIdleState(const std::shared_ptr<WorkSchedulerService> &service)
{
    auto queryState = &WorkSchedulerService::HasDeepIdleTime;
    return (service.get()->*queryState)();
}

void DriveDeepIdleMaps(const std::shared_ptr<WorkSchedulerService> &service,
    const std::vector<std::shared_ptr<WorkStatus>> &works, FuzzedDataProvider &provider)
{
    for (auto &workStatus : works) {
        int32_t saId = provider.ConsumeBool() ? workStatus->workInfo_->GetSaId() :
            provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_SA_ID_FOR_FUZZ);
        int32_t uid = provider.ConsumeBool() ? workStatus->uid_ :
            provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, DEFAULT_UID + UID_RANGE_PADDING_FOR_FUZZ);
        int32_t delay = provider.ConsumeBool() ? workStatus->workInfo_->deepIdleTime_ :
            provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, DEFAULT_REPEAT_CYCLE * FUZZ_BOUND_TWO);
        static_cast<void>(service->NeedCreateTimer(saId, uid, delay));
        static_cast<void>(QueryDeepIdleState(service));
        static_cast<void>(service->GetDeepIdleTimeMap());
        if (provider.ConsumeBool()) {
            service->deepIdleTimeMap_[saId] = std::make_pair(delay, uid);
        }
    }
}

void DriveSaActions(const std::shared_ptr<WorkSchedulerService> &service,
    const std::vector<std::shared_ptr<WorkStatus>> &works, FuzzedDataProvider &provider)
{
    const std::vector<std::string> actions = {
        "ohos.workscheduler.sa.START",
        "ohos.workscheduler.sa.STOP",
        ConsumeText(provider, "invalid.sa.action"),
    };
    for (int32_t index = 0; index < provider.ConsumeIntegralInRange<int32_t>(
        1, MAX_DEEP_IDLE_ROUNDS_FOR_FUZZ); ++index) {
        auto workStatus = works[provider.ConsumeIntegralInRange<size_t>(0, works.size() - 1)];
        int32_t saId = provider.ConsumeBool() ? workStatus->workInfo_->GetSaId() :
            provider.ConsumeIntegralInRange<int32_t>(FUZZ_INVALID_VALUE, MAX_SA_ID_FOR_FUZZ);
        switch (provider.ConsumeIntegralInRange<int32_t>(0, SA_ACTION_CASE_MAX)) {
            case 0:
                service->HandleDeepIdleMsg(saId);
                break;
            case 1:
                service->StopWorkForSA(saId);
                break;
            case FUZZ_BOUND_TWO:
                service->LoadSa(workStatus, actions[provider.ConsumeIntegralInRange<size_t>(0, actions.size() - 1)]);
                break;
            case MAX_REPEAT_COUNT_FOR_FUZZ:
                service->StopWorkInner(workStatus, workStatus->uid_, provider.ConsumeBool(), provider.ConsumeBool());
                break;
            case FUZZ_BOUND_FOUR:
                service->workPolicyManager_->GetDeepIdleWorks();
                break;
            default:
                service->StopDeepIdleWorks();
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

void RunWorkSchedulerSaDeepIdleFuzzer(FuzzedDataProvider &provider)
{
    auto service = PrepareService();
    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    dataManager->SetDeepIdle(provider.ConsumeBool());
    dataManager->SetDeviceSleep(provider.ConsumeBool());
    auto works = AddSaWorks(service, provider);
    if (works.empty()) {
        return;
    }
    DriveDeepIdleMaps(service, works, provider);
    DriveSaActions(service, works, provider);
    service->StopDeepIdleWorks();
    static_cast<void>(QueryDeepIdleState(service));
    static_cast<void>(service->GetDeepIdleTimeMap());
    std::string result;
    service->workPolicyManager_->Dump(result);
    service->workQueueManager_->Dump(result);
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    OHOS::WorkScheduler::RunWorkSchedulerSaDeepIdleFuzzer(provider);
    return 0;
}
