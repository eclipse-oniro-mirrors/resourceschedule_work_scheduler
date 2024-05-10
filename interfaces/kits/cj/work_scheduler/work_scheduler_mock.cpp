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

#include "cj_common_ffi.h"

extern "C" {
FFI_EXPORT int CJ_StartWork = 0;
FFI_EXPORT int CJ_StopWork = 0;
FFI_EXPORT int CJ_GetWorkStatus = 0;
FFI_EXPORT int CJ_ObtainAllWorks = 0;
FFI_EXPORT int CJ_IsLastWorkTimeOut = 0;
FFI_EXPORT int GetWorkInfo = 0;
FFI_EXPORT int GetNetWorkInfo = 0;
FFI_EXPORT int GetChargeInfo = 0;
FFI_EXPORT int GetBatteryInfo = 0;
FFI_EXPORT int GetStorageInfo = 0;
FFI_EXPORT int GetRepeatInfo = 0;
FFI_EXPORT int ParseWorkInfo = 0;
FFI_EXPORT int MallocCString = 0;
}