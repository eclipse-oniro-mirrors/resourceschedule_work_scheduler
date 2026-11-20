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
#include "work_sched_data_manager.h"

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
    std::shared_ptr<DetectorValue> value = std::make_shared<DetectorValue>(81, 0, false, "");
    std::shared_ptr<Condition> ret = workQueue_->ParseCondition(type, value);
    EXPECT_EQ(ret->intVal, 0);
    EXPECT_EQ(ret->enumVal, 0);
    EXPECT_FALSE(ret->boolVal);
    EXPECT_TRUE(ret->strVal.empty());
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
    std::shared_ptr<DetectorValue> value =
        std::make_shared<DetectorValue>(99, 88, true, "non_default");
    std::shared_ptr<Condition> ret = workQueue_->ParseCondition(type, value);
    EXPECT_EQ(ret->enumVal, 0);
    EXPECT_EQ(ret->intVal, 0);
    EXPECT_FALSE(ret->boolVal);
    EXPECT_TRUE(ret->strVal.empty());
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
 * @tc.name: Push_003
 * @tc.desc: Test WorkQueue Push duplicate workId does not add again.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, Push_003, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo1 = WorkInfo();
    workInfo1.SetWorkId(1);
    workInfo1.SetElement("com.example.dup", "DupAbility");
    auto workStatus1 = std::make_shared<WorkStatus>(workInfo1, 1);
    workQueue_->Push(workStatus1);
    EXPECT_TRUE(workQueue_->GetSize() == 1);
    auto workInfo2 = WorkInfo();
    workInfo2.SetWorkId(1);
    workInfo2.SetElement("com.example.dup", "DupAbility");
    auto workStatus2 = std::make_shared<WorkStatus>(workInfo2, 1);
    workQueue_->Push(workStatus2);
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
    EXPECT_EQ(ret->GetStatus(), WorkStatus::Status::RUNNING);
    EXPECT_EQ(ret->priority_, 10001);
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

/**
 * @tc.name: Find_003
 * @tc.desc: Test WorkQueue Find by workId returns workStatus.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, Find_003, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(10);
    workInfo_.SetElement("com.test.find", "FindAbility");
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workQueue_->Push(workStatus);
    auto ret = workQueue_->Find(workStatus->workId_);
    EXPECT_EQ(ret->workId_, workStatus->workId_);
}

/**
 * @tc.name: Find_004
 * @tc.desc: Test WorkQueue Find by workId not found.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, Find_004, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto ret = workQueue_->Find("nonexistent_id");
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: Find_UserIdBundleName_001
 * @tc.desc: Test WorkQueue Find by userId and bundleName returns true.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, Find_UserIdBundleName_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(20);
    workInfo_.SetElement("com.test.findub", "FindAbility");
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workQueue_->Push(workStatus);
    bool ret = workQueue_->Find(workStatus->userId_, "com.test.findub");
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: Find_UserIdBundleName_002
 * @tc.desc: Test WorkQueue Find by userId and bundleName not found.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, Find_UserIdBundleName_002, TestSize.Level1)
{
    workQueue_->ClearAll();
    bool ret = workQueue_->Find(999, "nonexist.bundle");
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CancelWork_001
 * @tc.desc: Test WorkQueue CancelWork removes work from list.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, CancelWork_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(30);
    workInfo_.SetElement("com.test.cancel", "CancelAbility");
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workQueue_->Push(workStatus);
    EXPECT_EQ(workQueue_->GetSize(), 1);
    workQueue_->CancelWork(workStatus);
    EXPECT_EQ(workQueue_->GetSize(), 0);
}

/**
 * @tc.name: SetMinIntervalByDump_001
 * @tc.desc: Test WorkQueue SetMinIntervalByDump updates minInterval for all works.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, SetMinIntervalByDump_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(50);
    workInfo_.SetElement("com.test.interval", "IntervalAbility");
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workQueue_->Push(workStatus);
    workQueue_->SetMinIntervalByDump(5000);
    auto workList = workQueue_->GetWorkList();
    EXPECT_EQ(workList.front()->minInterval_, 5000);
}

/**
 * @tc.name: Dump_001
 * @tc.desc: Test WorkQueue Dump appends work info when workList not empty.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, Dump_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(60);
    workInfo_.SetElement("com.test.dump", "DumpAbility");
    auto workStatus = std::make_shared<WorkStatus>(workInfo_, 1);
    workQueue_->Push(workStatus);
    std::string result;
    workQueue_->Dump(result);
    EXPECT_NE(result.find("ul_60"), std::string::npos);
    EXPECT_NE(result.find("com.test.dump"), std::string::npos);
}

/**
 * @tc.name: GetSize_001
 * @tc.desc: Test WorkQueue GetSize after push and clear.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, GetSize_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    EXPECT_EQ(workQueue_->GetSize(), 0);
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(60);
    workInfo_.SetElement("com.test.size", "SizeAbility");
    workQueue_->Push(std::make_shared<WorkStatus>(workInfo_, 1));
    EXPECT_EQ(workQueue_->GetSize(), 1);
}

/**
 * @tc.name: ClearAll_001
 * @tc.desc: Test WorkQueue ClearAll empties list.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ClearAll_001, TestSize.Level1)
{
    auto workInfo_ = WorkInfo();
    workInfo_.SetWorkId(70);
    workInfo_.SetElement("com.test.clear", "ClearAbility");
    workQueue_->Push(std::make_shared<WorkStatus>(workInfo_, 1));
    workQueue_->ClearAll();
    EXPECT_EQ(workQueue_->GetSize(), 0);
}

/**
 * @tc.name: ParseCondition_Charger_001
 * @tc.desc: Test WorkQueue ParseCondition CHARGER sets enumVal and boolVal.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ParseCondition_Charger_001, TestSize.Level1)
{
    WorkCondition::Type type = WorkCondition::Type::CHARGER;
    auto value = std::make_shared<DetectorValue>(WorkCondition::CHARGING_PLUGGED_AC, 0, true, "");
    auto ret = workQueue_->ParseCondition(type, value);
    EXPECT_EQ(ret->enumVal, WorkCondition::CHARGING_PLUGGED_AC);
    EXPECT_EQ(ret->boolVal, true);
}

/**
 * @tc.name: ParseCondition_BatteryLevel_001
 * @tc.desc: Test WorkQueue ParseCondition BATTERY_LEVEL sets intVal.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ParseCondition_BatteryLevel_001, TestSize.Level1)
{
    WorkCondition::Type type = WorkCondition::Type::BATTERY_LEVEL;
    auto value = std::make_shared<DetectorValue>(85, 0, false, "");
    auto ret = workQueue_->ParseCondition(type, value);
    EXPECT_EQ(ret->intVal, 85);
}

/**
 * @tc.name: ParseCondition_Group_001
 * @tc.desc: Test WorkQueue ParseCondition GROUP sets all fields.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ParseCondition_Group_001, TestSize.Level1)
{
    WorkCondition::Type type = WorkCondition::Type::GROUP;
    auto value = std::make_shared<DetectorValue>(30, 100, true, "com.test.group");
    auto ret = workQueue_->ParseCondition(type, value);
    EXPECT_EQ(ret->enumVal, 30);
    EXPECT_EQ(ret->intVal, 100);
    EXPECT_EQ(ret->boolVal, true);
    EXPECT_EQ(ret->strVal, "com.test.group");
}

/**
 * @tc.name: ParseCondition_Standby_001
 * @tc.desc: Test WorkQueue ParseCondition STANDBY sets boolVal.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, ParseCondition_Standby_001, TestSize.Level1)
{
    WorkCondition::Type type = WorkCondition::Type::STANDBY;
    auto value = std::make_shared<DetectorValue>(0, 0, true, "");
    auto ret = workQueue_->ParseCondition(type, value);
    EXPECT_EQ(ret->boolVal, true);
}

/**
 * @tc.name: FindSA_001
 * @tc.desc: Test WorkQueue FindSA returns nullptr when no SA work found.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, FindSA_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto = ret workQueue_->FindSA(100);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: OnConditionChanged_001
 * @tc.desc: Test WorkQueue OnConditionChanged skips SA work with mismatched saId on DEEP_IDLE.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, OnConditionChanged_001, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo = WorkInfo();
    workInfo.SetWorkId(9301);
    workInfo.SetElement("com.test.deepidle", "DeepIdleAbility");
    workInfo.saId_ = 401;
    auto workStatus = std::make_shared<WorkStatus>(workInfo, 9301);
    workQueue_->Push(workStatus);
    auto conditionVal = std::make_shared<DetectorValue>(402, 0, false, "");
    auto result = workQueue_->OnConditionChanged(WorkCondition::Type::DEEP_IDLE, conditionVal);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: OnConditionChanged_002
 * @tc.desc: Test WorkQueue OnConditionChanged skips work returning E_GROUP_CHANGE_NOT_MATCH_HAP.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, OnConditionChanged_002, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo = WorkInfo();
    workInfo.SetWorkId(9302);
    workInfo.SetElement("com.test.restrict", "RestrictAbility");
    auto workStatus = std::make_shared<WorkStatus>(workInfo, 9302);
    workStatus->bundleName_ = "com.test.restrict";
    workQueue_->Push(workStatus);
    std::list<std::string> restrictList = {"com.test.restrict"};
    DelayedSingleton<DataManager>::GetInstance()->AddDeviceStandyRestrictlist(restrictList);
    auto conditionVal = std::make_shared<DetectorValue>(0, 0, false, "");
    auto result = workQueue_->OnConditionChanged(WorkCondition::Type::STORAGE, conditionVal);
    EXPECT_TRUE(result.empty());
    DelayedSingleton<DataManager>::GetInstance()->ClearDeviceStandyRestrictlist();
}

/**
 * @tc.name: OnConditionChanged_003
 * @tc.desc: Test WorkQueue OnConditionChanged marks WAIT_CONDITION when not ready but IsReadyStatus.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, OnConditionChanged_003, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo = WorkInfo();
    workInfo.SetWorkId(9303);
    workInfo.SetElement("com.test.condready", "CondReadyAbility");
    workInfo.RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    auto workStatus = std::make_shared<WorkStatus>(workInfo, 9303);
    workStatus->bundleName_ = "com.test.condready";
    workStatus->MarkStatus(WorkStatus::Status::CONDITION_READY);
    workQueue_->Push(workStatus);
    DelayedSingleton<DataManager>::GetInstance()->SetDeviceSleep(false);
    auto conditionVal = std::make_shared<DetectorValue>(0, 0, false, "");
    auto result = workQueue_->OnConditionChanged(WorkCondition::Type::STORAGE, conditionVal);
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(workStatus->GetStatus(), WorkStatus::Status::WAIT_CONDITION);
}

/**
 * @tc.name: OnConditionChanged_004
 * @tc.desc: Test WorkQueue OnConditionChanged adds work to result when needRetrigger is true.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueTest, OnConditionChanged_004, TestSize.Level1)
{
    workQueue_->ClearAll();
    auto workInfo = WorkInfo();
    workInfo.SetWorkId(9304);
    workInfo.SetElement("com.test.retrigger", "RetriggerAbility");
    auto workStatus = std::make_shared<WorkStatus>(workInfo, 9304);
    workStatus->bundleName_ = "com.test.retrigger";
    workStatus->s_uid_last_time_map[9304] = workStatus->getOppositeTime();
    workQueue_->Push(workStatus);
    DelayedSingleton<DataManager>::GetInstance()->SetDeviceSleep(false);
    auto conditionVal = std::make_shared<DetectorValue>(0, 0, false, "");
    auto result = workQueue_->OnConditionChanged(WorkCondition::Type::STORAGE, conditionVal);
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result.front, workStatus);
    workStatus->s_uid_last_time_map.erase(9304);
}
}
}