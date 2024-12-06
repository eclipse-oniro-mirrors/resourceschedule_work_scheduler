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

#include "workschedulerstopandcancelwork_fuzzer.h"

#include "iwork_sched_service_ipc_interface_code.h"
#include "work_scheduler_service.h"
#include "work_sched_common.h"
#include "work_condition.h"
#include "work_policy_manager.h"
#include "work_queue_manager.h"


void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
    const std::u16string WORK_SCHEDULER_STUB_TOKEN = u"ohos.workscheduler.iworkschedservice";
    static std::shared_ptr<WorkSchedulerService> workSchedulerService_;

    bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
    {
        return true;
    }

    void AddWork()
    {
        WorkInfo workInfo = WorkInfo();
        int32_t workId = 1;
        int32_t uid = 100;
        int32_t userId = 100;
        int32_t delaytime = 5000;
        workInfo.SetWorkId(workId);
        workInfo.SetElement("bundle_name", "ability_name");
        workInfo.RefreshUid(uid);
        workInfo.RequestDeepIdle(true);
        std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workInfo, uid);
        workStatus->userId_ = userId;
        workSchedulerService_->workPolicyManager_->AddWork(workStatus, uid);
        workSchedulerService_->workQueueManager_->AddWork(workStatus);
        std::vector<std::shared_ptr<WorkStatus>> readyWorkVector = {};
        readyWorkVector.emplace_back(WorkStatus);
        std::shared_ptr<std::vector<std::shared_ptr<WorkStatus>>> readyWork =
            std::make_shared<std::vector<std::shared_ptr<WorkStatus>>>(readyWorkVector);
        workSchedulerService_->workPolicyManager_->OnConditionReady(readyWork);
        workSchedulerService_->workPolicyManager_->RealStartWork(workStatus);
        workSchedulerService_->workPolicyManager_->SendRetrigger(delaytime);
        workSchedulerService_->workPolicyManager_->AddWatchdogForWork(workStatus);
        workStatus->UpdateUidLastTimeMap();
        workSchedulerService_->workPolicyManager_->IsSpecialScene(workStatus, 1);
        workStatus->MarkStatus(WorkStatus::Status::RUNNING);
        workSchedulerService_->workPolicyManager_->StopWork(workStatus, uid, true, true);
        workSchedulerService_->workQueueManager_->CancelWork(workStatus);
        workSchedulerService_->workPolicyManager_->RemoveWatchDog(workStatus);
        workSchedulerService_->workPolicyManager_->WatchdogTimeOut(1);
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
        uint32_t code = static_cast<int32_t>(IWorkSchedServiceInterfaceCode::STOP_AND_CANCEL_WORK);
        workSchedulerService_->OnStart();
        workSchedulerService_->InitBgTaskSubscriber();
        if (!workSchedulerService_->ready_) {
            workSchedulerService_->ready_ = true;
        }
        if (workSchedulerService_->workPolicyManager_ == nullptr) {
            workSchedulerService_->workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
        }
        if (workSchedulerService_->workQueueManager_ == nullptr) {
            workSchedulerService_->workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
        }
        AddWork();
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