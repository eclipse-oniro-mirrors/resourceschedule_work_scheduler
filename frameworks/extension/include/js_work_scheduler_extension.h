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

#ifndef JS_WORKSCHED_EXTENSION_H
#define JS_WORKSCHED_EXTENSION_H

#include "work_scheduler_extension.h"

#include "js_runtime.h"
#include "work_info.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
class JsWorkSchedulerExtension : public WorkSchedulerExtension {
public:
    explicit JsWorkSchedulerExtension(AbilityRuntime::JsRuntime &jsRuntime);
    ~JsWorkSchedulerExtension() override;

    /**
     * @brief Create JsWorkSchedulerExtension.
     *
     * @param runtime The runtime.
     * @return The JsWorkSchedulerExtension instance.
     */
    static JsWorkSchedulerExtension* Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime);

    void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
              const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
              std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
              const sptr<IRemoteObject> &token) override;

    void OnStart(const AAFwk::Want &want) override;

    sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;

    void OnDisconnect(const AAFwk::Want &want) override;

    /**
     * @brief The OnStop callback.
     *
     */
    void OnStop() override;

    /**
     * @brief The OnWorkStart callback.
     *
     * @param WorkInfo The info of work.
     */
    void OnWorkStart(WorkInfo& workInfo) override;

    /**
     * @brief The OnWorkStop callback.
     *
     * @param WorkInfo The info of work.
     */
    void OnWorkStop(WorkInfo& workInfo) override;

private:
    void GetSrcPath(std::string &srcPath);
    AbilityRuntime::JsRuntime &jsRuntime_;
    std::unique_ptr<NativeReference> jsObj_;
};
}  // namespace WorkScheduler
}  // namespace OHOS
#endif  // JS_WORKSCHED_EXTENSION_H