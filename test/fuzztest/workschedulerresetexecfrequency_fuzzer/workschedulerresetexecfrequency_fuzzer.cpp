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

#include "workschedulerresetexecfrequency_fuzzer.h"

#include "iwork_sched_service.h"
#include "work_scheduler_service.h"
#include "work_sched_common.h"
#include "work_queue_manager.h"
#include "work_policy_manager.h"
#include "frequency_info.h"

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
        int32_t uid = 101;
        workInfo.SetWorkId(workId);
        workInfo.SetElement("bundle_name", "ability_name");
        workInfo.RefreshUid(uid);
        workInfo.RequestDeepIdle(true);
        std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workInfo, uid);
        workStatus->getOppositeTime();
        workSchedulerService_->workPolicyManager_->AddWork(workStatus, uid);
        workSchedulerService_->workQueueManager_->AddWork(workStatus);

        std::vector<std::string> argsInStr;
        std::string result;
        argsInStr.push_back("-group");
        argsInStr.push_back("101");
        argsInStr.push_back("-1");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
        argsInStr.clear();
        result.clear();
        argsInStr.push_back("-group");
        argsInStr.push_back("101");
        argsInStr.push_back("10");
        workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
        workStatus->SetMinInterval();
    }

    void SetFrequencyData()
    {
        int32_t callingUid = 100;
        int32_t workId = 1;
        int64_t time = 20 * 60 * 1000;
        FrequencyInfo freqInfo1;
        freqInfo1.SetUid(callingUid);
        freqInfo1.SetWorkId(workId);
        freqInfo1.SetInterval(time);
        workSchedulerService_->SetExecFrequencyInner(callingUid, freqInfo1);
        workSchedulerService_->ResetExecFrequencyWhenAppRemove(callingUid);

        int32_t uid = 101;
        FrequencyInfo freqInfo2;
        freqInfo2.SetUid(uid);
        freqInfo2.SetWorkId(workId);
        freqInfo2.SetInterval(time);
        workSchedulerService_->SetExecFrequencyInner(callingUid, freqInfo2);
        AddWork();
        workSchedulerService_->ResetExecFrequencyByUid(uid);

        workSchedulerService_->ResetExecFrequencyByCallingUid(1, uid);
        workSchedulerService_->SetExecFrequencyInner(callingUid, freqInfo2);
        workSchedulerService_->GetExecFrequency(uid, callingUid);
        workSchedulerService_->ResetExecFrequencyByCallingUid(callingUid, uid);
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
        uint32_t code = static_cast<int32_t>(IWorkSchedServiceIpcCode::COMMAND_RESET_EXEC_FREQUENCY);

        int32_t uid = 100;
        if (!dataMessageParcel.WriteInt32(uid)) {
            return false;
        }
        workSchedulerService_->OnStart();
        workSchedulerService_->InitWorkSchedPluginMgr();
        if (!workSchedulerService_->ready_) {
            workSchedulerService_->ready_ = true;
        }
        if (workSchedulerService_->workQueueManager_ == nullptr) {
            workSchedulerService_->workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
        }
        if (workSchedulerService_->workPolicyManager_ == nullptr) {
            workSchedulerService_->workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
        }
        SetFrequencyData();
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