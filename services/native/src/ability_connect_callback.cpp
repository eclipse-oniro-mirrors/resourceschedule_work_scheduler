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

#include "ability_connect_callback.h"
#include "background_loader_mgr.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
void AbilityConnectCallback::OnAbilityConnectDone(const OHOS::AppExecFwk::ElementName& element,
    const OHOS::sptr<IRemoteObject>& remoteObject, int32_t resultCode)
{
    const std::string& bundleName = element.GetBundleName();
    WS_HILOGI("bundleName: %{public}s, resultCode: %{public}d", bundleName.c_str(), resultCode);
    if (resultCode != ERR_OK) {
        WS_HLOGE("OnAbilityConnectDone failed: resultCode: %{public}d", resultCode);
        return;
    }
    if (remoteObject == nullptr) {
        WS_HILOGE("OnAbilityConnectDone failed: remoteObject is null");
        return;
    }

    const std::string& abilityName = element.GetAbilityName();
    BackgroundLoaderMgr::GetInstance().SaveRemoteObject(bundleName, abilityName, appIndex_, remoteObject);
    BackgroundLoaderMgr::GetInstance().SendOnstart(remoteObject, bundleName, appIndex_);
}

void AbilityConnectCallback::OnAbilityDisconnectDone(const OHOS::AppExecFwk::ElementName& element, int32_t resultCode)
{
    WS_HILOGI("OnAbilityDisconnectDone: bundleName: %{public}s, appIndex: %{public}d, resultCode: %{public}d",
        element.GetBundleName().c_str(), appIndex_, resultCode);
    BackgroundLoaderMgr::GetInstance().RemoveRemoteObject(bundleName_, abilityName_, appIndex_);
}
} // namespace WorkScheduler
} // namespace OHOS