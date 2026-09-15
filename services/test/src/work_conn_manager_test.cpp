/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "work_conn_manager.h"
#include "work_info.h"
#include "work_status.h"
#include "work_sched_hilog.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace WorkScheduler {

class WorkConnManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkConnManager> workConnManager_;
};

class MyWorkConnManager : public WorkConnManager {
    bool DisConnect(sptr<WorkSchedulerConnection> connect)
    {
        return true;
    }
};

std::shared_ptr<WorkConnManager> WorkConnManagerTest::workConnManager_ = nullptr;

void WorkConnManagerTest::SetUpTestCase()
{
    workConnManager_ = std::make_shared<WorkConnManager>();
}

/**
 * @tc.name: AddConnInfo_001
 * @tc.desc: Test WorkConnManager AddConnInfo.
 * @tc.type: FUNC
 * @tc.require: #I9HYBW
 */
HWTEST_F(WorkConnManagerTest, AddConnInfo_001, TestSize.Level2)
{
    string workId = "u1000_123";
    sptr<WorkSchedulerConnection> connection;
    workConnManager_->AddConnInfo(workId, connection);
    EXPECT_TRUE(workConnManager_->connMap_.count(workId) > 0);
}

/**
 * @tc.name: RemoveConnInfo_001
 * @tc.desc: Test WorkConnManager RemoveConnInfo.
 * @tc.type: FUNC
 * @tc.require: #I9HYBW
 */
HWTEST_F(WorkConnManagerTest, RemoveConnInfo_001, TestSize.Level2)
{
    string workId = "u1000_123";
    sptr<WorkSchedulerConnection> connection;
    workConnManager_->AddConnInfo(workId, connection);
    workConnManager_->RemoveConnInfo(workId);
    EXPECT_FALSE(workConnManager_->connMap_.count(workId) > 0);
}

/**
 * @tc.name: StopWork_001
 * @tc.desc: Test WorkConnManager StopWork.
 * @tc.type: FUNC
 * @tc.require: #I9HYBW
 */
HWTEST_F(WorkConnManagerTest, StopWork_001, TestSize.Level2)
{
    workConnManager_->connMap_.clear();
    WorkInfo workInfo;
    workInfo.workId_ = 123;
    workInfo.bundleName_ = "com.unittest.bundleName";
    workInfo.abilityName_ = "unittestAbility";
    int32_t uid = 1234;
    shared_ptr<WorkStatus> workStatus = make_shared<WorkStatus>(workInfo, uid);
    bool ret = workConnManager_->StopWork(workStatus, false);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetConnInfo_001
 * @tc.desc: Test WorkConnManager GetConnInfo returns connection for existing workId.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkConnManagerTest, GetConnInfo_001, TestSize.Level2)
{
    workConnManager_->connMap_.clear();
    string workId = "u1000_456";
    sptr<WorkSchedulerConnection> conn = new WorkSchedulerConnection(std::make_shared<WorkInfo>());
    workConnManager_->AddConnInfo(workId, conn);
    auto ret = workConnManager_->GetConnInfo(workId);
    EXPECT_EQ(ret->AsObject(), conn->AsObject());
    workConnManager_->connMap_.clear();
}

/**
 * @tc.name: GetConnInfo_002
 * @tc.desc: Test WorkConnManager GetConnInfo returns nullptr for non-existing workId.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkConnManagerTest, GetConnInfo_002, TestSize.Level2)
{
    workConnManager_->connMap_.clear();
    auto ret = workConnManager_->GetConnInfo("nonexistent");
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: StopWork_NotConnected_001
 * @tc.desc: Test WorkConnManager StopWork with conn not connected and !isTimeOut returns false.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkConnManagerTest, StopWork_NotConnected_001, TestSize.Level2)
{
    workConnManager_->connMap_.clear();
    WorkInfo workInfo;
    workInfo.workId_ = 789;
    workInfo.bundleName_ = "com.test.notconn";
    workInfo.abilityName_ = "NotConnAbility";
    int32_t uid = 5678;
    shared_ptr<WorkStatus> workStatus = make_shared<WorkStatus>(workInfo, uid);
    sptr<WorkSchedulerConnection> conn = new WorkSchedulerConnection(std::make_shared<WorkInfo>());
    workConnManager_->AddConnInfo(workStatus->workId_, conn);
    bool ret = workConnManager_->StopWork(workStatus, false);
    EXPECT_FALSE(ret);
    workConnManager_->connMap_.clear();
}
}
}