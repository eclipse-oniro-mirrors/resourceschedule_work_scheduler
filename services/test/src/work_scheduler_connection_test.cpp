/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "work_info.h"
#include "work_scheduler_connection.h"
#include "work_sched_hilog.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace WorkScheduler {

class WorkSchedulerConnectionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkSchedulerConnection> workSchedulerConnection_;
};

std::shared_ptr<WorkSchedulerConnection> WorkSchedulerConnectionTest::workSchedulerConnection_ = nullptr;

void WorkSchedulerConnectionTest::SetUpTestCase()
{
    std::shared_ptr<WorkInfo> workInfo = std::make_shared<WorkInfo>();
    workInfo->workId_ = 123;
    workInfo->bundleName_ = "com.unittest.bundleName";
    workInfo->abilityName_ = "unittestAbility";

    workSchedulerConnection_ = std::make_shared<WorkSchedulerConnection>(workInfo);
}

/**
 * @tc.name: StopWork_001
 * @tc.desc: Test WorkSchedulerConnection StopWork.
 * @tc.type: FUNC
 * @tc.require: https://gitee.com/openharmony/resourceschedule_work_scheduler/issues/ICBI5I
 */
HWTEST_F(WorkSchedulerConnectionTest, StopWork_001, TestSize.Level2)
{
    workSchedulerConnection_->StopWork();
    EXPECT_TRUE(workSchedulerConnection_->proxy_ == nullptr);
}

/**
 * @tc.name: StopWork_002
 * @tc.desc: Test WorkSchedulerConnection StopWork.
 * @tc.type: FUNC
 * @tc.require: https://gitee.com/openharmony/resourceschedule_work_scheduler/issues/ICBI5I
 */
HWTEST_F(WorkSchedulerConnectionTest, StopWork_002, TestSize.Level2)
{
    sptr<ISystemAbilityManager> SystemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(SystemAbilityManager, nullptr);
    sptr<IRemoteObject> remoteObject = SystemAbilityManager->GetSystemAbility(WORK_SCHEDULE_SERVICE_ID);
    ASSERT_NE(remoteObject, nullptr);
    AppExecFwk::ElementName element;
    int32_t resultCode = 0;
    workSchedulerConnection_->OnAbilityConnectDone(element, remoteObject, resultCode);
    workSchedulerConnection_->StopWork();
    EXPECT_FALSE(workSchedulerConnection_->proxy_ == nullptr);
}
}
}