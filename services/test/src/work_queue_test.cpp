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

#include "work_queue.h"
#include "work_status.h"
#include "work_scheduler_service.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class WorkQueueTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkQueue> workQueue_;
};

std::shared_ptr<WorkQueue> WorkQueueTest::workQueue_ = nullptr;

void WorkQueueTest::SetUpTestCase()
{
    workQueue_ = std::make_shared<WorkQueue>();
}

/**
 * @tc.name: ParseCondition_001
 * @tc.desc: Test WorkQueue ParseCondition.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ParseCondition_001, TestSize.Level1)
{
    WorkCondition::Type type = WorkCondition::Type::NETWORK;
    std::shared_ptr<DetectorValue> value = std::make_shared<DetectorValue>(0, 0, false, "");
    std::shared_ptr<Condition> ret = workQueue_->ParseCondition(type, value);
    EXPECT_TRUE(ret->enumVal == WorkCondition::Network::NETWORK_TYPE_ANY);
}

/**
 * @tc.name: ParseCondition_002
 * @tc.desc: Test WorkQueue ParseCondition.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ParseCondition_002, TestSize.Level1)
{
    WorkCondition::Type type = WorkCondition::Type::BATTERY_STATUS;
    std::shared_ptr<DetectorValue> value = std::make_shared<DetectorValue>(0, 0, false, "");
    std::shared_ptr<Condition> ret = workQueue_->ParseCondition(type, value);
    EXPECT_TRUE(ret->enumVal == WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
}

/**
 * @tc.name: ParseCondition_003
 * @tc.desc: Test WorkQueue ParseCondition.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ParseCondition_003, TestSize.Level1)
{
    WorkCondition::Type type = WorkCondition::Type::STORAGE;
    std::shared_ptr<DetectorValue> value = std::make_shared<DetectorValue>(0, 0, false, "");
    std::shared_ptr<Condition> ret = workQueue_->ParseCondition(type, value);
    EXPECT_TRUE(ret->enumVal == WorkCondition::Storage::STORAGE_LEVEL_LOW);
}

/**
 * @tc.name: ParseCondition_004
 * @tc.desc: Test WorkQueue ParseCondition.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ParseCondition_004, TestSize.Level1)
{
    WorkCondition::Type type = WorkCondition::Type::CHARGER;
    std::shared_ptr<DetectorValue> value = std::make_shared<DetectorValue>(0, 0, false, "");
    std::shared_ptr<Condition> ret = workQueue_->ParseCondition(type, value);
    EXPECT_TRUE(ret->enumVal == WorkCondition::Charger::CHARGING_PLUGGED_ANY);
}

/**
 * @tc.name: ParseCondition_005
 * @tc.desc: Test WorkQueue ParseCondition.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ParseCondition_005, TestSize.Level1)
{
    WorkCondition::Type type = WorkCondition::Type::BATTERY_LEVEL;
    std::shared_ptr<DetectorValue> value = std::make_shared<DetectorValue>(81, 0, false, "");
    std::shared_ptr<Condition> ret = workQueue_->ParseCondition(type, value);
    EXPECT_TRUE(ret->intVal == 81);
}

/**
 * @tc.name: ParseCondition_006
 * @tc.desc: Test WorkQueue ParseCondition.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ParseCondition_006, TestSize.Level1)
{
    WorkCondition::Type type = WorkCondition::Type::TIMER;
    std::shared_ptr<DetectorValue> value = std::make_shared<DetectorValue>(0, 0, false, "");
    std::shared_ptr<Condition> ret = workQueue_->ParseCondition(type, value);
    EXPECT_TRUE(ret->intVal == 0);
}

/**
 * @tc.name: ParseCondition_007
 * @tc.desc: Test WorkQueue ParseCondition.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ParseCondition_007, TestSize.Level1)
{
    WorkCondition::Type type = WorkCondition::Type::GROUP;
    std::shared_ptr<DetectorValue> value = std::make_shared<DetectorValue>(0, 0, false, "");
    std::shared_ptr<Condition> ret = workQueue_->ParseCondition(type, value);
    EXPECT_TRUE(ret->enumVal == 0);
}

/**
 * @tc.name: ParseCondition_008
 * @tc.desc: Test WorkQueue ParseCondition.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ParseCondition_008, TestSize.Level1)
{
    WorkCondition::Type type = WorkCondition::Type::DEEP_IDLE;
    std::shared_ptr<DetectorValue> value = std::make_shared<DetectorValue>(0, 0, false, "");
    std::shared_ptr<Condition> ret = workQueue_->ParseCondition(type, value);
    EXPECT_FALSE(ret->boolVal);
}

/**
 * @tc.name: ParseCondition_009
 * @tc.desc: Test WorkQueue ParseCondition.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ParseCondition_009, TestSize.Level1)
{
    WorkCondition::Type type = WorkCondition::Type::UNKNOWN;
    std::shared_ptr<DetectorValue> value = std::make_shared<DetectorValue>(0, 0, false, "");
    std::shared_ptr<Condition> ret = workQueue_->ParseCondition(type, value);
    EXPECT_FALSE(ret->boolVal);
}

/**
 * @tc.name: Push_001
 * @tc.desc: Test WorkQueue Push.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, Push_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    vector<std::shared_ptr<WorkStatus>> workVector;
    for (int i = 0; i < 10; i++) {
        auto workInfo_ = WorkInfo();
        workInfo_.SetWorkId(i);
        std::string bundleName = "com.example.workStatus";
        std::string abilityName = "workStatusAbility";
        workInfo_.SetElement(bundleName, abilityName);
        auto workStatus = std::make_shared<WorkStatus>(workInfo_, i);
        workVector.push_back(workStatus);
    }
    std::shared_ptr<vector<std::shared_ptr<WorkStatus>>> workStatusVector =
        std::make_shared<vector<std::shared_ptr<WorkStatus>>>(workVector);
    workQueue_->Push(workStatusVector);
    EXPECT_TRUE(workQueue_->GetSize() == 10);
}

/**
 * @tc.name: Push_002
 * @tc.desc: Test WorkQueue Push.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, Push_002, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_.SetElement(bundleName, abilityName);
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workQueue_->Push(workStatus);
    EXPECT_TRUE(workQueue_->GetSize() == 1);
}

/**
 * @tc.name: Remove_001
 * @tc.desc: Test WorkQueue Remove.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, Remove_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_.SetElement(bundleName, abilityName);
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workQueue_->Push(workStatus);
    workQueue_->Remove(workStatus);
    EXPECT_TRUE(workQueue_->GetSize() == 0);
}

/**
 * @tc.name: Contains_001
 * @tc.desc: Test WorkQueue Contains.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, Contains_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_.SetElement(bundleName, abilityName);
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workQueue_->Push(workStatus);
    bool ret = workQueue_->Contains(std::make_shared<std::string>(workStatus->workId_));
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: Contains_002
 * @tc.desc: Test WorkQueue Contains.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, Contains_002, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_.SetElement(bundleName, abilityName);
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workQueue_->Push(workStatus);
    bool ret = workQueue_->Contains(std::make_shared<std::string>("u1_2"));
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: Find_001
 * @tc.desc: Test WorkQueue Find.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, Find_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_.SetElement(bundleName, abilityName);
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workQueue_->Push(workStatus);
    bool ret = workQueue_->Contains(std::make_shared<std::string>(workStatus->workId_));
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: Find_002
 * @tc.desc: Test WorkQueue Find.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, Find_002, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_.SetElement(bundleName, abilityName);
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workQueue_->Push(workStatus);
    bool ret = workQueue_->Contains(std::make_shared<std::string>("u1_2"));
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetWorkToRunByPriority_001
 * @tc.desc: Test WorkQueue GetWorkToRunByPriority.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, GetWorkToRunByPriority_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_.SetElement(bundleName, abilityName);
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workStatus->MarkStatus(WorkStatus::Status::CONDITION_READY);
    workQueue_->Push(workStatus);
    auto ret = workQueue_->GetWorkToRunByPriority();
    EXPECT_TRUE(ret != nullptr);
}

/**
 * @tc.name: CancelWork_001
 * @tc.desc: Test WorkQueue CancelWork.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, CancelWork_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_.SetElement(bundleName, abilityName);
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workStatus->MarkStatus(WorkStatus::Status::CONDITION_READY);
    workQueue_->Push(workStatus);
    auto ret = workQueue_->CancelWork(workStatus);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: GetWorkList_001
 * @tc.desc: Test WorkQueue GetWorkList.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, GetWorkList_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_.SetElement(bundleName, abilityName);
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workStatus->MarkStatus(WorkStatus::Status::CONDITION_READY);
    workQueue_->Push(workStatus);
    std::list<std::shared_ptr<WorkStatus>> ret = workQueue_->GetWorkList();
    EXPECT_TRUE(ret.size() == 1);
}

/**
 * @tc.name: RemoveUnReady_001
 * @tc.desc: Test WorkQueue RemoveUnReady.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, RemoveUnReady_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_.SetElement(bundleName, abilityName);
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workStatus->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workQueue_->Push(workStatus);
    workQueue_->RemoveUnReady();
    EXPECT_TRUE(workQueue_->GetWorkList().size() == 0);
}

/**
 * @tc.name: GetRunningCount_001
 * @tc.desc: Test WorkQueue GetRunningCount.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, GetRunningCount_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_.SetElement(bundleName, abilityName);
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workQueue_->Push(workStatus);
    EXPECT_TRUE(workQueue_->GetRunningCount() == 1);
}

/**
 * @tc.name: GetRunningWorks_001
 * @tc.desc: Test WorkQueue GetRunningWorks.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, GetRunningWorks_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_.SetElement(bundleName, abilityName);
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workQueue_->Push(workStatus);
    EXPECT_TRUE(workQueue_->GetRunningWorks().size() == 1);
}

/**
 * @tc.name: GetDeepIdleWorks_001
 * @tc.desc: Test WorkQueue GetDeepIdleWorks.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, GetDeepIdleWorks_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_.SetElement(bundleName, abilityName);
    workInfo_.RequestDeepIdle(true);
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workQueue_->Push(workStatus);
    EXPECT_TRUE(workQueue_->GetDeepIdleWorks().size() == 1);
}
}
}