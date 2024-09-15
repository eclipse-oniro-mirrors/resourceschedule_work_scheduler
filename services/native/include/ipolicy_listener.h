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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_IPOLICY_DETECTOR_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_IPOLICY_DETECTOR_H

#include <memory>

#include "detector_value.h"
#include "policy_type.h"

namespace OHOS {
namespace WorkScheduler {
class IPolicyListener {
public:
    virtual ~IPolicyListener() = default;
    /**
     * @brief The OnPolicyChanged callback.
     *
     * @param policyType The policy type.
     * @param detectorVal The detector val.
     */
    virtual void OnPolicyChanged(PolicyType policyType, std::shared_ptr<DetectorValue> detectorVal) = 0;
    /**
     * @brief Start.
     *
     * @return True if success,else false.
     */
    virtual bool Start();
    /**
     * @brief Stop.
     *
     * @return True if success,else false.
     */
    virtual bool Stop();
};
} // namesapce WorkScheduler
} // namespace OHOS

#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_IPOLICY_DETECTOR_H