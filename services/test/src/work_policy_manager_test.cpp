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

#include "work_scheduler_service.h"
#include "work_policy_manager.h"
#include "work_status.h"
#include "work_sched_errors.h"


using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class WorkPolicyManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkPolicyManager> workPolicyManager_;
};

std::shared_ptr<WorkPolicyManager> WorkPolicyManagerTest::workPolicyManager_ = nullptr;

void WorkPolicyManagerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
}

/**
 * @tc.name: RealStartWork_001
 * @tc.desc: Test WorkPolicyManagerTest RealStartWork.
 * @tc.type: FUNC
 * @tc.require: I8OLHT
 */
HWTEST_F(WorkPolicyManagerTest, RealStartWork_001, TestSize.Level1)
{
    WorkInfo workinfo;
    int32_t uid;
    std::shared_ptr<WorkStatus> topWork = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->RealStartWork(topWork);
    EXPECT_FALSE(topWork->IsRunning());
}

/**
 * @tc.name: CheckWorkToRun_001
 * @tc.desc: Test WorkPolicyManagerTest CheckWorkToRun.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, CheckWorkToRun_001, TestSize.Level1)
{
    workPolicyManager_->conditionReadyQueue_->ClearAll();
    workPolicyManager_->CheckWorkToRun();
    EXPECT_TRUE(workPolicyManager_->conditionReadyQueue_->GetSize() == 0);
}

/**
 * @tc.name: AddWork_001
 * @tc.desc: Test WorkPolicyManagerTest AddWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, AddWork_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    int32_t ret = workPolicyManager_->AddWork(workStatus, uid);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: AddWork_002
 * @tc.desc: Test WorkPolicyManagerTest AddWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, AddWork_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->AddWork(workStatus, uid);

    WorkInfo workinfo1;
    workinfo1.SetWorkId(10000);
    workinfo1.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo1.RequestBatteryLevel(80);
    std::shared_ptr<WorkStatus> workStatus1 = std::make_shared<WorkStatus>(workinfo1, uid);
    int32_t ret = workPolicyManager_->AddWork(workStatus1, uid);

    EXPECT_EQ(ret, E_ADD_REPEAT_WORK_ERR);
}

/**
 * @tc.name: AddWork_003
 * @tc.desc: Test WorkPolicyManagerTest AddWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, AddWork_003, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    int32_t uid = 10000;
    for (int32_t i = 0; i < 10; i++) {
        WorkInfo workinfo;
        workinfo.SetWorkId(10000 + i);
        workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
        workinfo.RequestBatteryLevel(80);
        std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
        workPolicyManager_->AddWork(workStatus, uid);
    }

    WorkInfo workinfo1;
    workinfo1.SetWorkId(10010);
    workinfo1.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo1.RequestBatteryLevel(80);
    std::shared_ptr<WorkStatus> workStatus1 = std::make_shared<WorkStatus>(workinfo1, uid);
    int32_t ret = workPolicyManager_->AddWork(workStatus1, uid);

    EXPECT_EQ(ret, E_WORK_EXCEED_UPPER_LIMIT);
}

/**
 * @tc.name: RemoveWork_001
 * @tc.desc: Test WorkPolicyManagerTest RemoveWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, RemoveWork_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    bool ret = workPolicyManager_->RemoveWork(workStatus, uid);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: RemoveWork_002
 * @tc.desc: Test WorkPolicyManagerTest RemoveWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, RemoveWork_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->AddWork(workStatus, uid);

    bool ret = workPolicyManager_->RemoveWork(workStatus, uid);
    EXPECT_TRUE(ret);
}
}
}