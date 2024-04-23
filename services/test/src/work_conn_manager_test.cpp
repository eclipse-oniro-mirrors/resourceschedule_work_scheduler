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
HWTEST_F(WorkConnManagerTest, AddConnInfo_001, TestSize.Level1)
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
HWTEST_F(WorkConnManagerTest, RemoveConnInfo_001, TestSize.Level1)
{
    string workId = "u1000_123";
    sptr<WorkSchedulerConnection> connection;
    workConnManager_->AddConnInfo(workId, connection);
    workConnManager_->RemoveConnInfo(workId);
    EXPECT_FALSE(workConnManager_->connMap_.count(workId) > 0);
}

/**
 * @tc.name: GetConnInfo_001
 * @tc.desc: Test WorkConnManager GetConnInfo.
 * @tc.type: FUNC
 * @tc.require: #I9HYBW
 */
HWTEST_F(WorkConnManagerTest, GetConnInfo_001, TestSize.Level1)
{
    string workId = "u1000_123";
    sptr<WorkSchedulerConnection> connection;
    workConnManager_->AddConnInfo(workId, connection);
    workConnManager_->GetConnInfo(workId);
    EXPECT_TRUE(workConnManager_->connMap_.size() == 1);
}

/**
 * @tc.name: GetConnInfo_002
 * @tc.desc: Test WorkConnManager GetConnInfo.
 * @tc.type: FUNC
 * @tc.require: #I9HYBW
 */
HWTEST_F(WorkConnManagerTest, GetConnInfo_002, TestSize.Level1)
{
    workConnManager_->connMap_.clear();
    string workId = "u1000_123";
    sptr<WorkSchedulerConnection> ret = workConnManager_->GetConnInfo(workId);
    EXPECT_TRUE(ret == nullptr);
}

/**
 * @tc.name: StartWork_001
 * @tc.desc: Test WorkConnManager StartWork.
 * @tc.type: FUNC
 * @tc.require: #I9HYBW
 */
HWTEST_F(WorkConnManagerTest, StartWork_001, TestSize.Level1)
{
    string workId = "u1000_123";
    sptr<WorkSchedulerConnection> connection;
    workConnManager_->AddConnInfo(workId, connection);

    WorkInfo workInfo;
    workInfo.workId_ = 123;
    workInfo.bundleName_ = "com.unittest.bundleName";
    workInfo.abilityName_ = "unittestAbility";
    int32_t uid = 1234;
    shared_ptr<WorkStatus> workStatus = make_shared<WorkStatus>(workInfo, uid);
    bool ret = workConnManager_->StartWork(workStatus);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StartWork_002
 * @tc.desc: Test WorkConnManager StartWork.
 * @tc.type: FUNC
 * @tc.require: #I9HYBW
 */
HWTEST_F(WorkConnManagerTest, StartWork_002, TestSize.Level1)
{
    WorkInfo workInfo;
    workInfo.workId_ = 123;
    workInfo.bundleName_ = "com.unittest.bundleName";
    workInfo.abilityName_ = "unittestAbility";
    int32_t uid = 1234;
    shared_ptr<WorkStatus> workStatus = make_shared<WorkStatus>(workInfo, uid);
    bool ret = workConnManager_->StartWork(workStatus);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: DisConnect_001
 * @tc.desc: Test WorkConnManager DisConnect.
 * @tc.type: FUNC
 * @tc.require: #I9HYBW
 */
HWTEST_F(WorkConnManagerTest, DisConnect_001, TestSize.Level1)
{
    sptr<WorkSchedulerConnection> connection;
    bool ret = workConnManager_->DisConnect(connection);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StopWork_001
 * @tc.desc: Test WorkConnManager StopWork.
 * @tc.type: FUNC
 * @tc.require: #I9HYBW
 */
HWTEST_F(WorkConnManagerTest, StopWork_001, TestSize.Level1)
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
 * @tc.name: StopWork_002
 * @tc.desc: Test WorkConnManager StopWork.
 * @tc.type: FUNC
 * @tc.require: #I9HYBW
 */
HWTEST_F(WorkConnManagerTest, StopWork_002, TestSize.Level1)
{
    string workId = "u1000_123";
    sptr<WorkSchedulerConnection> connection;
    workConnManager_->AddConnInfo(workId, connection);

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
 * @tc.name: WriteStartWorkEvent_001
 * @tc.desc: Test WorkConnManager WriteStartWorkEvent.
 * @tc.type: FUNC
 * @tc.require: #I9HYBW
 */
HWTEST_F(WorkConnManagerTest, WriteStartWorkEvent_001, TestSize.Level1)
{
    WorkInfo workInfo;
    workInfo.workId_ = 123;
    workInfo.bundleName_ = "com.unittest.bundleName";
    workInfo.abilityName_ = "unittestAbility";
    workInfo.RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    workInfo.RequestBatteryLevel(80);
    workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW);
    workInfo.RequestRepeatCycle(20000);
    int32_t uid = 1234;
    shared_ptr<WorkStatus> workStatus = make_shared<WorkStatus>(workInfo, uid);
    workConnManager_->WriteStartWorkEvent(workStatus);
    EXPECT_TRUE(workInfo.IsRepeat());
}
}
}