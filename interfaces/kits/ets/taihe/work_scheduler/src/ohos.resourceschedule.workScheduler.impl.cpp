/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ohos.resourceschedule.workScheduler.proj.hpp"
#include "ohos.resourceschedule.workScheduler.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "work_sched_hilog.h"
#include "workscheduler_srv_client.h"
#include "work_info.h"
#include "common.h"

using namespace taihe;
using namespace OHOS;
using namespace ohos::resourceschedule::workScheduler;
using namespace OHOS::WorkScheduler;

namespace {
// To be implemented.
struct AsyncCallbackIsLastWorkTimeOut {
    int32_t workId {-1};
    bool result {false};
};

void StartWork(::ohos::resourceschedule::workScheduler::WorkInfo const &work)
{
    WorkScheduler::WorkInfo workInfo = WorkScheduler::WorkInfo();
    if (!Common::GetWorkInfo(work, workInfo)) {
        WS_HILOGE("StartWork parse parameter failed");
        return;
    }
    ErrCode errCode = WorkScheduler::WorkSchedulerSrvClient::GetInstance().StartWork(workInfo);
    if (errCode) {
        WS_HILOGE("StartWork failed errCode: %{public}d", errCode);
        set_business_error(errCode, Common::FindErrMsg(errCode));
    }
}

void StopWork(::ohos::resourceschedule::workScheduler::WorkInfo const &work, optional_view<bool> needCancel)
{
    WorkScheduler::WorkInfo workInfo = WorkScheduler::WorkInfo();
    if (!Common::GetWorkInfo(work, workInfo)) {
        WS_HILOGE("StartWork parse parameter failed");
        return;
    }
    ErrCode errCode = ERR_OK;
    auto cancel = needCancel.has_value() ? needCancel.value() : false;
    WS_HILOGD("cancel is: %{public}d", cancel);
    if (cancel) {
        errCode = WorkScheduler::WorkSchedulerSrvClient::GetInstance().StopAndCancelWork(workInfo);
    } else {
        errCode = WorkScheduler::WorkSchedulerSrvClient::GetInstance().StopWork(workInfo);
    }
    if (errCode) {
        WS_HILOGE("StopWork failed errCode: %{public}d", errCode);
        set_business_error(errCode, Common::FindErrMsg(errCode));
    }
}

::ohos::resourceschedule::workScheduler::WorkInfo GetWorkStatusSync(double workId)
{
    std::shared_ptr<WorkScheduler::WorkInfo> workInfo {nullptr};
    ErrCode errCode = WorkScheduler::WorkSchedulerSrvClient::GetInstance().GetWorkStatus(
        static_cast<int32_t>(workId), workInfo);
    if (errCode) {
        WS_HILOGE("get work status failed errCode: %{public}d", errCode);
        set_business_error(errCode, Common::FindErrMsg(errCode));
        return {};
    }
    if (workInfo == nullptr) {
        WS_HILOGE("workInfo is null");
        return {};
    }

    ::ohos::resourceschedule::workScheduler::WorkInfo aniWork{
        .workId = workInfo->GetWorkId(),
        .bundleName = workInfo->GetBundleName(),
        .abilityName = workInfo->GetAbilityName(),
    };
    Common::ParseWorkInfo(workInfo, aniWork);
    return aniWork;
}

array<::ohos::resourceschedule::workScheduler::WorkInfo> ObtainAllWorksSync()
{
    std::list<std::shared_ptr<WorkScheduler::WorkInfo>> workInfoList;
    ErrCode errCode = WorkSchedulerSrvClient::GetInstance().ObtainAllWorks(workInfoList);
    if (errCode) {
        WS_HILOGE("get work status failed errCode: %{public}d", errCode);
        set_business_error(errCode, Common::FindErrMsg(errCode));
        return {};
    }
    if (workInfoList.empty()) {
        WS_HILOGE("workInfoList is nullptr");
        return {};
    }
    std::vector<::ohos::resourceschedule::workScheduler::WorkInfo> aniWorkInfoList;
    for (auto &workInfo: workInfoList) {
        ::ohos::resourceschedule::workScheduler::WorkInfo aniWork{
            .workId = workInfo->GetWorkId(),
            .bundleName = workInfo->GetBundleName(),
            .abilityName = workInfo->GetAbilityName(),
        };
        Common::ParseWorkInfo(workInfo, aniWork);
        aniWorkInfoList.push_back(aniWork);
    }
    return array<::ohos::resourceschedule::workScheduler::WorkInfo>{copy_data_t{},
        aniWorkInfoList.data(), aniWorkInfoList.size()};
}

void StopAndClearWorks()
{
    ErrCode errCode = WorkScheduler::WorkSchedulerSrvClient::GetInstance().StopAndClearWorks();
    if (errCode) {
        WS_HILOGE("stop and clear works failed errCode: %{public}d", errCode);
        set_business_error(errCode, Common::FindErrMsg(errCode));
    }
}

bool IsLastWorkTimeOutSync(double workId)
{
    AsyncCallbackIsLastWorkTimeOut asyncCallbackInfo;
    asyncCallbackInfo.workId = static_cast<int32_t>(workId);
    ErrCode errCode = WorkScheduler::WorkSchedulerSrvClient::GetInstance().IsLastWorkTimeout(
        asyncCallbackInfo.workId, asyncCallbackInfo.result);
    if (errCode) {
        WS_HILOGE("IsLastWorkTimeOutSync errCode: %{public}d", errCode);
        set_business_error(errCode, Common::FindErrMsg(errCode));
        return false;
    }
    return asyncCallbackInfo.result;
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_StartWork(StartWork);
TH_EXPORT_CPP_API_StopWork(StopWork);
TH_EXPORT_CPP_API_GetWorkStatusSync(GetWorkStatusSync);
TH_EXPORT_CPP_API_ObtainAllWorksSync(ObtainAllWorksSync);
TH_EXPORT_CPP_API_StopAndClearWorks(StopAndClearWorks);
TH_EXPORT_CPP_API_IsLastWorkTimeOutSync(IsLastWorkTimeOutSync);
// NOLINTEND