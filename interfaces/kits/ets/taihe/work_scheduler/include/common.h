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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_TAIHE_INCLUDE_COMMON
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_TAIHE_INCLUDE_COMMON

#include <string>
#include "ohos.resourceschedule.workScheduler.impl.hpp"
#include "work_info.h"

namespace OHOS {
namespace WorkScheduler {
class Common {
public:
    static int32_t FindErrCode(const int32_t errCodeIn);
    static std::string FindErrMsg(const int32_t errCode);
    static bool GetExtrasInfo(
        ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo);
    static bool GetNetWorkInfo(
        ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo);
    static bool GetChargeInfo(
        ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo);
    static bool GetBatteryInfo(
        ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo);
    static bool GetStorageInfo(
        ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo);
    static bool GetRepeatInfo(
        ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo);
    static bool GetDeepIdleInfo(
        ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo);
    static bool GetWorkInfo(
        ::ohos::resourceschedule::workScheduler::WorkInfo const &aniWork, OHOS::WorkScheduler::WorkInfo &workInfo);
    static bool ConvertToAniParameters(std::map<std::string, sptr<AAFwk::IInterface>>& extrasMap,
        ::ohos::resourceschedule::workScheduler::WorkInfo &aniWork);
    static void ParseExtrasInfo(std::shared_ptr<OHOS::WorkScheduler::WorkInfo> workInfo,
        ::ohos::resourceschedule::workScheduler::WorkInfo &aniWork);
    static void ParseWorkInfo(std::shared_ptr<OHOS::WorkScheduler::WorkInfo> workInfo,
        ::ohos::resourceschedule::workScheduler::WorkInfo &aniWork);
};
}  // namespace WorkScheduler
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_INTERFACES_KITS_TAIHE_INCLUDE_COMMON