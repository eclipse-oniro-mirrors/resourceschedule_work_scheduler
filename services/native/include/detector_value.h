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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_DETECTOR_VALUE_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_DETECTOR_VALUE_H

#include <string>
#include <ctime>

namespace OHOS {
namespace WorkScheduler {
struct DetectorValue {
    int32_t intVal;
    int32_t timeVal;
    bool boolVal;
    std::string strVal;
    DetectorValue(int32_t intVal, int32_t timeVal, bool boolVal, const std::string &strVal)
        : intVal(intVal), timeVal(timeVal), boolVal(boolVal), strVal(strVal) {}
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_DETECTOR_VALUE_H