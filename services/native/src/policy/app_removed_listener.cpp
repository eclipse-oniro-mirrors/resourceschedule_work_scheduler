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

#include "policy/app_removed_listener.h"

#include "common_event_support.h"
#include "common_event_manager.h"
#include "matching_skills.h"
#include "want.h"
#include "work_sched_hilog.h"

using namespace std;

namespace OHOS {
namespace WorkScheduler {
static const std::string UID_PARAM = "uid";
AppRemovedSubscriber::AppRemovedSubscriber(const CommonEventSubscribeInfo &subscribeInfo, AppRemovedListener &listener)
    : CommonEventSubscriber(subscribeInfo), listener_(listener) {}

void AppRemovedSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    const string action = data.GetWant().GetAction();
    WS_HILOGI("OnReceiveEvent get action: %{public}s", action.c_str());
    if ((action == CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED)
        || (action == CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED)) {
            string bundle = data.GetWant().GetBundle();
            int32_t uid = data.GetWant().GetIntParam(UID_PARAM, -1);
            WS_HILOGI("bundleName: %{public}s , uid: %{public}d", bundle.c_str(), uid);
            auto detectorVal = make_shared<DetectorValue>(uid, 0, 0, bundle);
            listener_.OnPolicyChanged(PolicyType::APP_REMOVED, detectorVal);
    }
}

shared_ptr<CommonEventSubscriber> CreateAppRemovedSubscriber(AppRemovedListener &listener)
{
    MatchingSkills skill = MatchingSkills();
    skill.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    skill.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);
    CommonEventSubscribeInfo info(skill);
    return make_shared<AppRemovedSubscriber>(info, listener);
}

AppRemovedListener::AppRemovedListener(std::shared_ptr<WorkPolicyManager> workPolicyManager)
{
    workPolicyManager_ = workPolicyManager;
}

AppRemovedListener::~AppRemovedListener()
{
    this->Stop();
}

bool AppRemovedListener::Start()
{
    this->commonEventSubscriber = CreateAppRemovedSubscriber(*this);
    return CommonEventManager::SubscribeCommonEvent(this->commonEventSubscriber);
}

bool AppRemovedListener::Stop()
{
    if (this->commonEventSubscriber != nullptr) {
        bool result = CommonEventManager::UnSubscribeCommonEvent(this->commonEventSubscriber);
        if (result) {
            this->commonEventSubscriber = nullptr;
        }
        return result;
    }
    return true;
}

void AppRemovedListener::OnPolicyChanged(PolicyType policyType, shared_ptr<DetectorValue> detectorVal)
{
    workPolicyManager_->OnPolicyChanged(policyType, detectorVal);
}
} // namespace WorkScheduler
} // namespace OHOS