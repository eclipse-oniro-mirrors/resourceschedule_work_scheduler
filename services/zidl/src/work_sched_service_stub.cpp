/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "work_sched_service_stub.h"

#include <message_parcel.h>
#include <string_ex.h>

#include "work_sched_common.h"

namespace OHOS {
namespace WorkScheduler {
int WorkSchedServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    WS_HILOGD("WorkSchedServiceStub::OnRemoteRequest, cmd = %{public}u, flags = %{public}d", code, option.GetFlags());
    std::u16string descriptor = WorkSchedServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        WS_HILOGE("WorkSchedServiceStub::OnRemoteRequest failed, descriptor is not matched!");
        return E_GET_WORKSCHED_SERVICE_FALIED;
    }
    switch (code) {
        case static_cast<int>(IWorkSchedService::START_WORK): {
            int32_t ret = StartWorkStub(data);
            reply.WriteBool(ret == ERR_OK);
            return ret;
        }
        case static_cast<int>(IWorkSchedService::STOP_WORK): {
            int32_t ret = StopWorkStub(data);
            reply.WriteBool(ret == ERR_OK);
            return ret;
        }
        case static_cast<int>(IWorkSchedService::STOP_AND_CANCEL_WORK): {
            int32_t ret = StopAndCancelWorkStub(data);
            reply.WriteBool(ret == ERR_OK);
            return ret;
        }
        case static_cast<int>(IWorkSchedService::STOP_AND_CLEAR_WORKS): {
            int32_t ret = StopAndClearWorksStub(data);
            WS_HILOGD("%{public}s ret is ERR_OK ? %{public}s", __func__, (ret == ERR_OK) ? "true" : "false");
            reply.WriteBool(ret == ERR_OK);
            return ret;
        }
        case static_cast<int>(IWorkSchedService::IS_LAST_WORK_TIMEOUT): {
            int32_t ret = IsLastWorkTimeoutStub(data);
            reply.WriteBool(ret == ERR_OK);
            return ret;
        }
        case static_cast<int>(IWorkSchedService::OBTAIN_ALL_WORKS): {
            std::list<std::shared_ptr<WorkInfo>> workInfos = ObtainAllWorksStub(data);
            size_t worksize = workInfos.size();
            WS_HILOGD("BUFOAWF OBTAIN_ALL_WORKS worksize returns %{public}d", worksize);
            reply.WriteInt32(worksize);
            for (auto workInfo : workInfos) {
                reply.WriteParcelable(&*workInfo);
            }
            return ERR_OK;
        }
        case static_cast<int>(IWorkSchedService::GET_WORK_STATUS): {
            WS_HILOGI("call GetWorkStatus");
            auto workInfo = GetWorkStatusStub(data);
            reply.WriteParcelable(&*workInfo);
            return ERR_OK;
        }
        case static_cast<int>(IWorkSchedService::DUMP_INFO): {
            return ShellDumpStub(data, reply);
        }
        default: {
            WS_HILOGD("BUGOAWF OnRemoteRequest switch default, code: %{public}u", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }

    return ERR_OK;
}

int32_t WorkSchedServiceStub::StartWorkStub(MessageParcel& data)
{
    WorkInfo* pw = data.ReadParcelable<WorkInfo>();
    WorkInfo& workInfo = *pw;
    if (!StartWork(workInfo)) {
        WS_HILOGE("StartWork failed");
        return E_START_WORK_FAILED;
    }
    return ERR_OK;
}

int32_t WorkSchedServiceStub::StopWorkStub(MessageParcel& data)
{
    WorkInfo* pw = data.ReadParcelable<WorkInfo>();
    WorkInfo& workInfo = *pw;
    if (!StopWork(workInfo)) {
        return E_STOP_WORK_FAILED;
    }
    return ERR_OK;
}

int32_t WorkSchedServiceStub::StopAndCancelWorkStub(MessageParcel& data)
{
    WorkInfo* pw = data.ReadParcelable<WorkInfo>();
    WorkInfo& workInfo = *pw;
    if (!StopAndCancelWork(workInfo)) {
        return E_STOP_AND_CANCEL_WORK_FAILED;
    }
    return ERR_OK;
}

int32_t WorkSchedServiceStub::StopAndClearWorksStub(MessageParcel& data)
{
    if (!StopAndClearWorks()) {
        WS_HILOGE("StopAndClearWorks failed");
        return E_STOP_AND_CLEAR_WORKS_FAILED;
    }
    return ERR_OK;
}

int32_t WorkSchedServiceStub::IsLastWorkTimeoutStub(MessageParcel& data)
{
    int32_t workId = data.ReadInt32();
    if (!IsLastWorkTimeout(workId)) {
        return E_IS_LAST_WORK_TIMEOUT_FALSE;
    }
    return ERR_OK;
}

std::list<std::shared_ptr<WorkInfo>> WorkSchedServiceStub::ObtainAllWorksStub(MessageParcel& data)
{
    int32_t pid = 0, uid = 0;
    READ_PARCEL_WITHOUT_RET(data, Int32, uid);
    READ_PARCEL_WITHOUT_RET(data, Int32, pid);
    return ObtainAllWorks(uid, pid);
}

std::shared_ptr<WorkInfo> WorkSchedServiceStub::GetWorkStatusStub(MessageParcel& data)
{
    int32_t uid;
    int32_t workId;
    READ_PARCEL_WITHOUT_RET(data, Int32, uid);
    READ_PARCEL_WITHOUT_RET(data, Int32, workId);
    return GetWorkStatus(uid, workId);
}

ErrCode WorkSchedServiceStub::ShellDumpStub(MessageParcel& data, MessageParcel& reply)
{
    std::vector<std::string> dumpOption;
    if (!data.ReadStringVector(&dumpOption)) {
        WS_HILOGD("[HandleShellDump] fail: read dumpOption failed.");
        return ERR_INVALID_DATA;
    }
    std::vector<std::string> workSchedulerInfo;
    bool result = ShellDump(dumpOption, workSchedulerInfo);
    if (!reply.WriteBool(result)) {
        WS_HILOGD("WorkSchedServiceStub::%{public}s write result fail.", __func__);
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteStringVector(workSchedulerInfo)) {
        WS_HILOGD("WorkSchedServiceStub::%{public}s write workscheduler fail.", __func__);
        return ERR_INVALID_DATA;
    }
    return ERR_OK;
}
} // namespace WorkScheduler
} // namespace OHOS