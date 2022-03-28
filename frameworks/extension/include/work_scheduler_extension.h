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

#ifndef WORKSCHED_EXTENSION_H
#define WORKSCHED_EXTENSION_H

#include <memory>
#include "ability_local_record.h"
#include "ohos_application.h"
#include "ability_handler.h"
#include "runtime.h"
#include "extension_base.h"
#include "work_info.h"
#include "work_scheduler_extension_context.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerExtension : public AbilityRuntime::ExtensionBase<WorkSchedulerExtensionContext> {
public:
    WorkSchedulerExtension() = default;
    virtual ~WorkSchedulerExtension() = default;

    /**
     * @brief Create and init context.
     *
     * @param record The record info.
     * @param application The application info.
     * @param handler The handler.
     * @param token The remote object token.
     * @return The context
     */
    std::shared_ptr<WorkSchedulerExtensionContext> CreateAndInitContext(
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord>& record,
        const std::shared_ptr<AppExecFwk::OHOSApplication>& application,
        std::shared_ptr<AppExecFwk::AbilityHandler>& handler,
        const sptr<IRemoteObject>& token) override;

    /**
     * @brief The Init.
     *
     * @param record The record.
     * @param application The application.
     * @param handler The handler.
     * @param token The remmote object token.
     */
    void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord>& record,
        const std::shared_ptr<AppExecFwk::OHOSApplication>& application,
        std::shared_ptr<AppExecFwk::AbilityHandler>& handler,
        const sptr<IRemoteObject>& token) override;

    /**
     * @brief Create WorkSchedulerExtension.
     *
     * @param runtime The runtime.
     * @return The WorkSchedulerExtension instance.
     */
    static WorkSchedulerExtension* Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime);

    /**
     * @brief The OnWorkStart callback.
     *
     * @param workInfo The info of work.
     */
    virtual void OnWorkStart(WorkInfo& workInfo);

    /**
     * @brief The OnWorkStop callback.
     *
     * @param workInfo The info of work.
     */
    virtual void OnWorkStop(WorkInfo& workInfo);
};
}  // namespace WorkScheduler
}  // namespace OHOS
#endif  // WORKSCHED_EXTENSION_H