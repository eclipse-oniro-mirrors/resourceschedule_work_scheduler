/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <functional>
#include <gtest/gtest.h>

#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "work_scheduler_proxy.h"
#include "work_sched_hilog.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class WorkSchedulerProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkSchedulerProxy> workSchedulerProxy_;
};

std::shared_ptr<WorkSchedulerProxy> WorkSchedulerProxyTest::workSchedulerProxy_ = nullptr;
std::mutex mutexLock;

void WorkSchedulerProxyTest::SetUpTestCase()
{
    std::lock_guard<std::mutex> lock(mutexLock);
    sptr<ISystemAbilityManager> SystemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (SystemAbilityManager == nullptr) {
        return;
    }
    sptr<IRemoteObject> remoteObject = SystemAbilityManager->GetSystemAbility(WORK_SCHEDULE_SERVICE_ID);
    if (remoteObject == nullptr) {
        return;
    }
    workSchedulerProxy_ = std::make_shared<WorkSchedulerProxy>(remoteObject);
}

/**
 * @tc.name: OnWorkStart_001
 * @tc.desc: Test WorkSchedulerProxy OnWorkStart.
 * @tc.type: FUNC
 * @tc.require: #I9HP1I
 */
HWTEST_F(WorkSchedulerProxyTest, OnWorkStart_001, TestSize.Level1)
{
    WorkInfo workInfo;
    workSchedulerProxy_->OnWorkStart(workInfo);
    EXPECT_TRUE(workInfo.GetBundleName().empty());
}

/**
 * @tc.name: OnWorkStop_001
 * @tc.desc: Test WorkSchedulerProxy OnWorkStop.
 * @tc.type: FUNC
 * @tc.require: #I9HP1I
 */
HWTEST_F(WorkSchedulerProxyTest, OnWorkStop_001, TestSize.Level1)
{
    WorkInfo workInfo;
    workSchedulerProxy_->OnWorkStop(workInfo);
    EXPECT_TRUE(workInfo.GetBundleName().empty());
}
}
}