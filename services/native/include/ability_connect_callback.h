/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_ABILITY_CONNECT_CALLBACK_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_ABILITY_CONNECT_CALLBACK_H

#include <string>

#include "ability_connect_callback_stub.h"
#include "iremote_object.h"

namespace OHOS {
namespace WorkScheduler {
class AbilityConnectCallback : public OHOS::AAFwk::AbilityConnectionStub {
public:
    AbilityConnectCallback(const std::string& bundleName, const std::string&abilityName, int32_t appIndex)
        : bundleName_(bundleName), abilityName_(abilityName), appIndex_(appIndex) {}
    virtual ~AbilityConnectCallback() = default;
    void OnAbilityConnectDone(
        const OHOS::AppExecFwk::ElementName& element, const OHOS::sptr<IRemoteObject>& remoteObject,
        int32_t resultCode) override;
    void OnAbilityDisconnectDone(const OHOS::AppExecFwk::ElementName& element, int32_t resultCode) override;
private:
        std::string bundleName_;
        std::string abilityName_;
        int32_t appIndex_;
};
} // namespace WorkScheduler
} // namespace OHOS

#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_ABILITY_CONNECT_CALLBACK_H