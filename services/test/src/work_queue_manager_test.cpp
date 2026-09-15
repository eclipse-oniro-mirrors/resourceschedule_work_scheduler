/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "work_queue_manager.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"
#include "work_condition.h"
#include "work_sched_hilog.h"
#include "work_info.h"
#include "work_sched_utils.h"
#include "work_status.h"
#include "conditions/network_listener.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class MockWorkPolicyManager : public WorkPolicyManager {
public:
    using WorkPolicyManager::WorkPolicyManager;
    ~MockWorkPolicyManager() = default;

    void CheckWorkToRun(){};
};
class WorkQueueManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
};

std::shared_ptr<WorkQueueManager> WorkQueueManagerTest::workQueueManager_ = nullptr;

void WorkQueueManagerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (workSchedulerService_->handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create("WorkSchedulerService", AppExecFwk::ThreadMode::FFRT);
        workSchedulerService_->eventRunner_ = runner;
        workSchedulerService_->handler_ = std::make_shared<WorkEventHandler>(runner, workSchedulerService_);
    }
    std::shared_ptr<WorkPolicyManager> workPolicyManager_ =
        std::make_shared<MockWorkPolicyManager>(workSchedulerService_);
    workSchedulerService_->workPolicyManager_ = workPolicyManager_;
    if (workSchedulerService_->workPolicyManager_->workConnManager_ == nullptr) {
        workSchedulerService_->workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
    }
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
}

/**
 * @tc.name: Init_001
 * @tc.desc: Test WorkQueueManager Init returns true.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, Init_001, TestSize.Level1)
{
    EXPECT_EQ(workQueueManager_->Init(), true);
}

/**
 * @tc.name: AddListener_001
 * @tc.desc: Test WorkQueueManager AddListener returns true for new type.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, AddListener_001, TestSize.Level1)
{
    auto listener = std::make_shared<NetworkListener>(workQueueManager_);
    bool ret = workQueueManager_->AddListener(WorkCondition::Type::NETWORK, listener);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(workQueueManager_->listenerMap_.count(WorkCondition::Type::NETWORK), 1);
    workQueueManager_->listenerMap_.erase(WorkCondition::Type::NETWORK);
}

/**
 * @tc.name: AddListener_002
 * @tc.desc: Test WorkQueueManager AddListener returns false for duplicate type.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, AddListener_002, TestSize.Level1)
{
    auto listener1 = std::make_shared<NetworkListener>(workQueueManager_);
    workQueueManager_->AddListener(WorkCondition::Type::NETWORK, listener1);
    auto listener2 = std::make_shared<NetworkListener>(workQueueManager_);
    bool ret = workQueueManager_->AddListener(WorkCondition::Type::NETWORK, listener2);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(workQueueManager_->listenerMap_.count(WorkCondition::Type::NETWORK), 1);
    workQueueManager_->listenerMap_.erase(WorkCondition::Type::NETWORK);
}

/**
 * @tc.name: AddWork_001
 * @tc.desc: Test WorkQueueManager AddWork returns false for null workStatus.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, AddWork_001, TestSize.Level1)
{
    bool ret = workQueueManager_->AddWork(nullptr);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: AddWork_002
 * @tc.desc: Test WorkQueueManager AddWork adds work to queueMap_.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, AddWork_002, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(5001);
    workInfo->SetElement("com.test.wqm", "TestAbility");
    workInfo->RequestPersisted(false);
    workInfo->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 20001);
    bool ret = workQueueManager_->AddWork(workStatus);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(workQueueManager_->queueMap_.count(WorkCondition::Type::NETWORK), 1);
    workQueueManager_->RemoveWork(workStatus);
}

/**
 * @tc.name: SetTimeCycle_001
 * @tc.desc: Test WorkQueueManager SetTimeCycle and GetTimeCycle.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, SetTimeCycle_001, TestSize.Level1)
{
    workQueueManager_->SetTimeCycle(300000);
    EXPECT_EQ(workQueueManager_->GetTimeCycle(), 300000);
}

/**
 * @tc.name: SetTimeRetrigger_001
 * @tc.desc: Test WorkQueueManager SetTimeRetrigger and GetTimeRetrigger.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, SetTimeRetrigger_001, TestSize.Level1)
{
    workQueueManager_->SetTimeRetrigger(600);
    EXPECT_EQ(workQueueManager_->GetTimeRetrigger(), 600);
}

/**
 * @tc.name: Dump_001
 * @tc.desc: Test WorkQueueManager Dump produces output with queue info.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, Dump_001, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(5002);
    workInfo->SetElement("com.test.dump", "DumpAbility");
    workInfo->RequestPersisted(false);
    workInfo->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 20002);
    workQueueManager_->AddWork(workStatus);

    std::string result;
    workQueueManager_->Dump(result);
    EXPECT_EQ(result.find("network"), 0);
    workQueueManager_->RemoveWork(workStatus);
}

/**
 * @tc.name: StopAndClearWorks_001
 * @tc.desc: Test WorkQueueManager StopAndClearWorks returns true.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, StopAndClearWorks_001, TestSize.Level1)
{
    std::list<std::shared_ptr<WorkStatus>> workList;
    bool ret = workQueueManager_->StopAndClearWorks(workList);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: SetMinIntervalByDump_001
 * @tc.desc: Test WorkQueueManager SetMinIntervalByDump updates all queues.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, SetMinIntervalByDump_001, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(5003);
    workInfo->SetElement("com.test.interval", "IntervalAbility");
    workInfo->RequestPersisted(false);
    workInfo->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 20003);
    workQueueManager_->AddWork(workStatus);

    workQueueManager_->SetMinIntervalByDump(5000);
    auto workList = workQueueManager_->queueMap_.at(WorkCondition::Type::NETWORK)->GetWorkList();
    EXPECT_EQ(workList.size(), 1);
    EXPECT_EQ(workList.front()->minInterval_, 5000);
    workQueueManager_->RemoveWork(workStatus);
}

/**
 * @tc.name: StartListener_001
 * @tc.desc: Test WorkQueueManager StartListener for existing type calls Start.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, StartListener_001, TestSize.Level1)
{
    auto listener = std::make_shared<NetworkListener>(workQueueManager_);
    workQueueManager_->AddListener(WorkCondition::Type::NETWORK, listener);
    workQueueManager_->StartListener(WorkCondition::Type::NETWORK);
    EXPECT_EQ(workQueueManager_->listenerMap_.count(WorkCondition::Type::NETWORK), 1);
    workQueueManager_->listenerMap_.erase(WorkCondition::Type::NETWORK);
}

/**
 * @tc.name: StopListener_001
 * @tc.desc: Test WorkQueueManager StopListener for existing type calls Stop.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, StopListener_001, TestSize.Level1)
{
    auto listener = std::make_shared<NetworkListener>(workQueueManager_);
    workQueueManager_->AddListener(WorkCondition::Type::NETWORK, listener);
    workQueueManager_->StopListener(WorkCondition::Type::NETWORK);
    EXPECT_EQ(workQueueManager_->listenerMap_.count(WorkCondition::Type::NETWORK), 1);
    workQueueManager_->listenerMap_.erase(WorkCondition::Type::NETWORK);
}

/**
 * @tc.name: CancelWork_001
 * @tc.desc: Test WorkQueueManager CancelWork removes work from all queues.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, CancelWork_001, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(5004);
    workInfo->SetElement("com.test.cancel", "CancelAbility");
    workInfo->RequestPersisted(false);
    workInfo->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 20004);
    workQueueManager_->AddWork(workStatus);
    EXPECT_EQ(workQueueManager_->queueMap_.count(WorkCondition::Type::NETWORK), 1);
    workQueueManager_->CancelWork(workStatus);
    auto workList = workQueueManager_->queueMap_.at(WorkCondition::Type::NETWORK)->GetWorkList();
    EXPECT_EQ(workList.size(), 0);
}
} // namespace WorkScheduler
} // namespace OHOS
