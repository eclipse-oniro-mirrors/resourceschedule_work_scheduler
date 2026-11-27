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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_NAPI_HELPER_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_NAPI_HELPER_H

#include <string>

#include <errors.h>

#include "napi/native_api.h"
#include "background_loader/background_loader_errors.h"

namespace OHOS {
namespace WorkScheduler {
namespace BackgroundLoader {

class BackgroundLoaderNapiHelper {
public:
    /**
     * @brief Handle error code and throw error.
     *
     * @param env The env.
     * @param errCode The error code.
     */
    static void HandleBackgroundLoaderError(const napi_env &env, int32_t errCode);

    /**
     * @brief Get int property.
     *
     * @param env The env.
     * @param object The object.
     * @param propertyName The name of property.
     * @param errCode Throw the errCode if failed.
     * @return Int value.
     */
    static int32_t GetIntProperty(napi_env env, napi_value object,
        const std::string &propertyName, ErrCode errCode);
    
    /**
     * @brief Get string property.
     *
     * @param env The env.
     * @param object The object.
     * @param propertyName The name of property.
     * @param errCode Throw the errCode if failed.
     * @return String property.
     */
    static std::string GetStringProperty(napi_env env, napi_value object,
        const std::string &propertyName, ErrCode errCode);
};
} // namespace BackgroundLoader
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_NAPI_HELPER_H