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
#include "work_scheduler_service.h"
#include "work_policy_manager.h"
#include "work_conn_manager.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace WorkScheduler {
namespace {
const int32_t WORK_ID = 123;
}
class WorkSchedulerConnectionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp();
    void TearDown() {}
    static std::shared_ptr<WorkSchedulerConnection> workSchedulerConnection_;
};

std::shared_ptr<WorkSchedulerConnection> WorkSchedulerConnectionTest::workSchedulerConnection_ = nullptr;

void WorkSchedulerConnectionTest::SetUpTestCase()
{
    std::shared_ptr<WorkInfo> workInfo = std::make_shared<WorkInfo>();
    workInfo->workId_ = WORK_ID;
    workInfo->bundleName_ = "com.unittest.bundleName";
    workInfo->abilityName_ = "unittestAbility";

    workSchedulerConnection_ = std::make_shared<WorkSchedulerConnection>(workInfo);
}

void WorkSchedulerConnectionTest::SetUp()
{
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (service != nullptr) {
        if (service->workPolicyManager_ == nullptr) {
            service->workPolicyManager_ = std::make_shared<WorkPolicyManager>(service);
        }
        if (service->workPolicyManager_->workConnManager_ == nullptr) {
            service->workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
        }
    }
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
    EXPECT_EQ(workSchedulerConnection_->IsConnected(), true);
}

/**
 * @tc.name: StopWork_NullProxy_001
 * @tc.desc: Test WorkSchedulerConnection StopWork with null proxy returns early.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedulerConnectionTest, StopWork_NullProxy_001, TestSize.Level2)
{
    auto conn = std::make_shared<WorkSchedulerConnection>(std::make_shared<WorkInfo>());
    conn->proxy_ = nullptr;
    conn->StopWork();
    EXPECT_EQ(conn->proxy_.get(), nullptr);
}

/**
 * @tc.name: IsConnected_001
 * @tc.desc: Test WorkSchedulerConnection IsConnected default false.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedulerConnectionTest, IsConnected_001, TestSize.Level2)
{
    auto conn = std::make_shared<WorkSchedulerConnection>(std::make_shared<WorkInfo>());
    EXPECT_EQ(conn->IsConnected(), false);
}

/**
 * @tc.name: IsConnected_002
 * @tc.desc: Test WorkSchedulerConnection IsConnected true after connect.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedulerConnectionTest, IsConnected_002, TestSize.Level2)
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(WORK_SCHEDULE_SERVICE_ID);
    ASSERT_NE(remoteObject, nullptr);
    AppExecFwk::ElementName element;
    workSchedulerConnection_->OnAbilityConnectDone(element, remoteObject, 0);
    EXPECT_EQ(workSchedulerConnection_->IsConnected(), true);
}

/**
 * @tc.name: OnAbilityDisconnectDone_NullWorkInfo_001
 * @tc.desc: Test WorkSchedulerConnection OnAbilityDisconnectDone with null workInfo_.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedulerConnectionTest, OnAbilityDisconnectDone_NullWorkInfo_001, TestSize.Level2)
{
    auto conn = std::make_shared<WorkSchedulerConnection>(nullptr);
    AppExecFwk::ElementName element;
    conn->OnAbilityDisconnectDone(element, 0);
    EXPECT_EQ(conn->workInfo_.get(), nullptr);
}
}
}