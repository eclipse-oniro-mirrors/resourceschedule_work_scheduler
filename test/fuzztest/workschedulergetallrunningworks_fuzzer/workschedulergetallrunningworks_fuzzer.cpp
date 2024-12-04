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

#include "workschedulergetallrunningworks_fuzzer.h"

#include "iwork_sched_service_ipc_interface_code.h"
#include "work_scheduler_service.h"
#include "work_policy_manager.h"


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
        workInfo.SetWorkId(workId);
        workInfo.SetElement("bundle_name", "ability_name");
        workInfo.RefreshUid(uid);
        workInfo.RequestDeepIdle(true);
        std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workInfo, uid);
        workStatus->MarkStatus(WorkStatus::Status::RUNNING);
        workStatus->userId_ = userId;
        workSchedulerService_->workPolicyManager_->AddWork(workStatus, uid);
        workSchedulerService_->workQueueManager_->AddWork(workStatus);
        workSchedulerService_->workPolicyManager_->RemoveWork(workStatus, uid);
        workSchedulerService_->workQueueManager_->RemoveWork(workStatus);
        workSchedulerService_->workPolicyManager_->AddWork(workStatus, uid);
        workSchedulerService_->workPolicyManager_->GetDeepIdleWorks();
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
        uint32_t code = static_cast<int32_t>(IWorkSchedServiceInterfaceCode::GET_ALL_RUNNING_WORKS);
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