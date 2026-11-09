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

#include "background_loader/background_loader_mgr.h"
#include "background_loader/background_loader_adapter.h"
#include "res_type.h"
#include "res_data.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
using namespace OHOS::ResourceSchedule;
IMPLEMENT_SINGLE_INSTANCE(BackgroundLoaderAdapter)

void BackgroundLoaderAdapter::DispatchResource(const std::shared_ptr<ResourceSchedule::ResData>& resData)
{
    if (resData == nullptr) {
        WS_HILOGE("BackgroundLoaderAdapter data is nullptr");
        return;
    }
    switch (resData->resType) {
        case ResType::RES_TYPE_START_BACKGROUND_LOADER_TASK: {
            BackgroundLoaderMgr::GetInstance().HandleBackgroundLoaderTask(resData->payload);
            break;
        }
        case ResType::RES_TYPE_APP_INSTALL_UNINSTALL: {
            BackgroundLoaderMgr::GetInstance().HandleAppInstallUninstall(resData->value, resData->payload);
            break;
        }
        case ResType::RES_TYPE_APP_STATE_CHANGE: {
            BackgroundLoaderMgr::GetInstance().HandleAppStateChangeEvent(resData->value, resData->payload);
            break;
        }
        case ResType::RES_TYPE_AUDIO_RENDER_STATE_CHANGE: {
            BackgroundLoaderMgr::GetInstance().HandleAppAudioRenderStateChangeEvent(resData->value, resData->payload);
            break;
        }
        case ResType::RES_TYPE_AUDIO_CAPTURE_STATUS_CHANGED: {
            BackgroundLoaderMgr::GetInstance().HandleAppAudioCaptureStateChangeEvent(resData->value, resData->payload);
            break;
        }
        case ResType::RES_TYPE_LOCATION_STATUS_CHANGE: {
            BackgroundLoaderMgr::GetInstance().HandleAppLocationStateEvent(resData->value, resData->payload);
            break;
        }
        case ResType::RES_TYPE_CAMERA_TORCH_STATE_CHANGE: {
            BackgroundLoaderMgr::GetInstance().HandleAppTorchStateEvent(resData->value, resData->payload);
            break;
        }
        case ResType::RES_TYPE_TRANSIENT_TASK: {
            BackgroundLoaderMgr::GetInstance().HandleTransientTaskEvent(resData->value, resData->payload);
            break;
        }
        case ResType::RES_TYPE_CONTINUOUS_TASK: {
            BackgroundLoaderMgr::GetInstance().HandleContinuousTaskEvent(resData->value, resData->payload);
            break;
        }
        case ResType::RES_TYPE_PROCESS_STATE_CHANGE: {
            BackgroundLoaderMgr::GetInstance().HandleProcessDiedEvent(resData->value, resData->payload);
            break;
        }
        default:
            break;
    }
}
}  // namespace WorkScheduler
}  // namespace OHOS
