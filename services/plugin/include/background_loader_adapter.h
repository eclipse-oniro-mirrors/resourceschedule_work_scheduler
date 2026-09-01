/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_ADAPTER_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_ADAPTER_H

#include <memory>
#include "res_data.h"
#include "single_instance.h"

namespace OHOS {
namespace WorkScheduler {
class BackgroundLoaderAdapter {
    DECLARE_SINGLE_INSTANCE(BackgroundLoaderAdapter)
public:
    void DispatchResource(const std::shared_ptr<ResourceSchedule::ResData>& resData);
};
}  // namespace WorkScheduler
}  // namespace OHOS
#endif  // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_ADAPTER_H
