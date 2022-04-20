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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_STATIC_SUBSCRIBER_CONNECTION_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_STATIC_SUBSCRIBER_CONNECTION_H

#include "ability_connect_callback_stub.h"
#include "work_info.h"
#include "work_scheduler_proxy.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerConnection : public AAFwk::AbilityConnectionStub {
public:
    explicit WorkSchedulerConnection(std::shared_ptr<WorkInfo> workInfo);
    /**
     * @brief Stop work.
     */
    void StopWork();
    /**
     * @brief The OnAbilityConnectDone callback.
     *
     * @param element The element.
     * @param remoteObject The remote object.
     * @param resultCode The result code.
     */
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override;

    /**
     * @brief The OnAbilityDisconnectDone callback.
     *
     * @param element The element.
     * @param resultCode The result code.
     */
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override;
private:
    sptr<WorkSchedulerProxy> proxy_ = nullptr;
    std::shared_ptr<WorkInfo> workInfo_;
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_STATIC_SUBSCRIBER_CONNECTION_H
