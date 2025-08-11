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

#include "workschedulerstopwork_fuzzer.h"

#include "iwork_sched_service.h"
#include "work_scheduler_service.h"
#include "work_sched_common.h"
#include "work_condition.h"
#include "work_queue_manager.h"
#include "work_policy_manager.h"
#include "work_standby_state_change_callback.h"
#include "work_bundle_group_change_callback.h"
#include "work_conn_manager.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
    const std::u16string WORK_SCHEDULER_STUB_TOKEN = u"ohos.workscheduler.iworkschedservice";
    static std::shared_ptr<WorkSchedulerService> workSchedulerService_;

    bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
    {
        return true;
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        MessageParcel dataMessageParcel;
        dataMessageParcel.WriteInterfaceToken(WORK_SCHEDULER_STUB_TOKEN);
        dataMessageParcel.WriteBuffer(data, size);
        dataMessageParcel.RewindRead(0);
        MessageParcel reply;
        MessageOption option;
        workSchedulerService_ = DelayedSingleton<WorkSchedulerService>::GetInstance();
        uint32_t code = static_cast<int32_t>(IWorkSchedServiceIpcCode::COMMAND_STOP_WORK);
        WorkInfo workInfo = WorkInfo();
        int32_t workId = 1;
        workInfo.SetWorkId(workId);
        workInfo.SetElement("bundle_name", "ability_name");
        workInfo.RequestPersisted(true);
        workInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY);
        if (!dataMessageParcel.WriteParcelable(&workInfo)) {
            return false;
        }
        workSchedulerService_->OnStart();
        workSchedulerService_->InitBgTaskSubscriber();
        if (!workSchedulerService_->ready_) {
            workSchedulerService_->ready_ = true;
        }
        if (workSchedulerService_->workQueueManager_ == nullptr) {
            workSchedulerService_->workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
        }
        if (workSchedulerService_->workPolicyManager_ == nullptr) {
            workSchedulerService_->workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
            workSchedulerService_->workPolicyManager_->Init(workSchedulerService_->eventRunner_);
        }
        if (workSchedulerService_->workPolicyManager_->workConnManager_ == nullptr) {
            workSchedulerService_->workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
        }
        if (workSchedulerService_->standbyStateObserver_ == nullptr) {
            workSchedulerService_->standbyStateObserver_ =
                new (std::nothrow) WorkStandbyStateChangeCallback(workSchedulerService_->workQueueManager_);
        }
        if (workSchedulerService_->groupObserver_ == nullptr) {
            workSchedulerService_->groupObserver_ =
                new (std::nothrow) WorkBundleGroupChangeCallback(workSchedulerService_->workQueueManager_);
        }
        workSchedulerService_->StartWork(workInfo);
        if (workSchedulerService_->checkBundle_) {
            workSchedulerService_->checkBundle_ = false;
        }
        workSchedulerService_->OnRemoteRequest(code, dataMessageParcel, reply, option);
        workSchedulerService_->OnStop();
        return true;
    }
} // WorkScheduler
} // OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}