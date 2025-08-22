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

#ifndef WORK_SCHEDULER_FFI_H
#define WORK_SCHEDULER_FFI_H

#include "ffi_remote_data.h"
#include "cj_common_ffi.h"
#include "work_info.h"
#include <cstdint>
#include <memory>
#include <string>

extern "C" {
    typedef struct {
        int8_t valueType;
        char *key;
        void *value;
        int64_t size;
    } CParameters;

    typedef struct {
        CParameters *head;
        int64_t size;
    } CArrParameters;

    struct RetWorkInfo {
        int32_t workId;
        char* bundleName;
        char* abilityName;
        int32_t netWorkType;
        int32_t isCharging;
        int32_t chargerType;
        int32_t batteryLevel;
        int32_t batteryStatus;
        int32_t storageRequest;
        int32_t isRepeat;
        int32_t repeatCycleTime;
        int32_t repeatCount;
        bool isPersisted;
        int32_t isDeepIdle;
        int32_t idleWaitTime;
    };

    struct RetWorkInfoV2 {
        RetWorkInfo v1;
        CArrParameters parameters;
    };

    struct RetArrRetWorkInfo {
        int32_t code;
        int64_t size;
        RetWorkInfo* data;
    };

    struct RetArrRetWorkInfoV2 {
        int32_t code;
        int64_t size;
        RetWorkInfoV2* data;
    };

    const int32_t UNSET_INT_PARAM = -1;

    FFI_EXPORT int32_t CJ_StartWork(RetWorkInfo work);
    FFI_EXPORT int32_t CJ_StopWork(RetWorkInfo work, bool needCancel);
    FFI_EXPORT int32_t CJ_GetWorkStatus(int32_t workId, RetWorkInfo& result);
    FFI_EXPORT RetArrRetWorkInfo CJ_ObtainAllWorks();
    FFI_EXPORT int32_t CJ_IsLastWorkTimeOut(int32_t workId, bool& result);
    FFI_EXPORT int32_t CJ_StopAndClearWorks();
    FFI_EXPORT int32_t CJ_StartWorkV2(RetWorkInfoV2 work);
    FFI_EXPORT int32_t CJ_StopWorkV2(RetWorkInfoV2 work, bool needCancel);
    FFI_EXPORT int32_t CJ_GetWorkStatusV2(int32_t workId, RetWorkInfoV2& result);
    FFI_EXPORT RetArrRetWorkInfoV2 CJ_ObtainAllWorksV2();

    int32_t GetWorkInfo(RetWorkInfo cwork, OHOS::WorkScheduler::WorkInfo& workInfo);
    int32_t GetNetWorkInfo(RetWorkInfo cwork, OHOS::WorkScheduler::WorkInfo& workInfo, bool& hasCondition);
    int32_t GetChargeInfo(RetWorkInfo cwork, OHOS::WorkScheduler::WorkInfo& workInfo, bool& hasCondition);
    int32_t GetBatteryInfo(RetWorkInfo cwork, OHOS::WorkScheduler::WorkInfo& workInfo, bool& hasCondition);
    int32_t GetStorageInfo(RetWorkInfo cwork, OHOS::WorkScheduler::WorkInfo& workInfo, bool& hasCondition);
    int32_t GetRepeatInfo(RetWorkInfo cwork, OHOS::WorkScheduler::WorkInfo& workInfo, bool& hasCondition);
    int32_t GetExtrasInfo(RetWorkInfoV2 cwork, OHOS::WorkScheduler::WorkInfo& workInfo);
    int32_t ParseExtrasInfo(std::shared_ptr<OHOS::WorkScheduler::WorkInfo> workInfo, CArrParameters& arrParam);
    void ParseWorkInfo(std::shared_ptr<OHOS::WorkScheduler::WorkInfo> workInfo, RetWorkInfo& cwork);
    char* MallocCString(const std::string& origin);
}

#endif
