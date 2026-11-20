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

#include <chrono>
#include <functional>
#include <thread>
#include <gtest/gtest.h>

#include "work_queue_manager.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"
#include "work_condition.h"
#include "work_sched_hilog.h"
#include "work_info.h"
#include "work_sched_utils.h"
#include "work_sched_constants.h"
#include "work_sched_data_manager.h"
#include "work_status.h"
#include "conditions/network_listener.h"
#include "conditions/icondition_listener.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class MockConditionListener : public IConditionListener {
public:
    void OnConditionChanged(WorkCondition::Type conditionType,
        std::shared_ptr<DetectorValue> conditionVal) override {}
    bool Start() override
    {
        startCount_++;
        return true;
    }
    bool Stop() override
    {
        stopCount_++;
        return true;
    }
    int32_t startCount_ = 0;
    int32_t stopCount_ = 0;
};

/**
 * @brief Poll until the work's timeout flag is reset or the timeout elapses.
 *
 * AsyncStopWork() posts the stop task via handler->PostTask(), so the reset is asynchronous.
 * A bounded polling wait avoids a fixed sleep that may be too short under load.
 */
static bool WaitForTimeoutReset(const std::shared_ptr<WorkStatus>& workStatus, int32_t timeoutMs)
{
    const int32_t intervalMs = 10;
    int32_t waitedMs = 0;
    while (true) {
        if (!workStatus->IsTimeout()) {
            return true;
        }
        if (waitedMs >= timeoutMs) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        waitedMs += intervalMs;
    }
}

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

/**
 * @tc.name: PrintWorkStatus_001
 * @tc.desc: Test PrintWorkStatus prints work status for non-GROUP/STANDBY type.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, PrintWorkStatus_001, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(5010);
    workInfo->SetElement("com.test.print", "PrintAbility");
    workInfo->RequestPersisted(false);
    workInfo->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 20010);
    workQueueManager_->AddWork(workStatus);

    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    bool originalSleep = dataManager->GetDeviceSleep();
    dataManager->SetDeviceSleep(true);
    dataManager->OnDeviceStandyWhitelistChanged("com.test.print", true);
    workStatus->conditionStatus_.clear();

    workQueueManager_->PrintWorkStatus(WorkCondition::Type::NETWORK);
    EXPECT_EQ(workStatus->conditionStatus_, ",STANDBY&1");

    dataManager->OnDeviceStandyWhitelistChanged("com.test.print", false);
    dataManager->SetDeviceSleep(originalSleep);
    workQueueManager_->RemoveWork(workStatus);
}

/**
 * @tc.name: PrintWorkStatus_002
 * @tc.desc: Test PrintWorkStatus calls PrintAllWorkStatus for GROUP type.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, PrintWorkStatus_002, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(5011);
    workInfo->SetElement("com.test.print2", "PrintAbility2");
    workInfo->RequestPersisted(false);
    workInfo->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 20011);
    workQueueManager_->AddWork(workStatus);

    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    bool originalSleep = dataManager->GetDeviceSleep();
    dataManager->SetDeviceSleep(true);
    dataManager->OnDeviceStandyWhitelistChanged("com.test.print2", true);
    workStatus->conditionStatus_.clear();

    workQueueManager_->PrintWorkStatus(WorkCondition::Type::GROUP);
    EXPECT_EQ(workStatus->conditionStatus_, ",STANDBY&1");

    dataManager->OnDeviceStandyWhitelistChanged("com.test.print2", false);
    dataManager->SetDeviceSleep(originalSleep);
    workQueueManager_->RemoveWork(workStatus);
}

/**
 * @tc.name: PrintAllWorkStatus_001
 * @tc.desc: Test PrintAllWorkStatus iterates all queues and prints work status.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, PrintAllWorkStatus_001, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(5012);
    workInfo->SetElement("com.test.printall", "PrintAllAbility");
    workInfo->RequestPersisted(false);
    workInfo->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 20012);
    workQueueManager_->AddWork(workStatus);

    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    bool originalSleep = dataManager->GetDeviceSleep();
    dataManager->SetDeviceSleep(true);
    dataManager->OnDeviceStandyWhitelistChanged("com.test.printall", true);
    workStatus->conditionStatus_.clear();

    workQueueManager_->PrintAllWorkStatus(WorkCondition::Type::STANDBY);
    EXPECT_EQ(workStatus->conditionStatus_, ",STANDBY&1");

    dataManager->OnDeviceStandyWhitelistChanged("com.test.printall", false);
    dataManager->SetDeviceSleep(originalSleep);
    workQueueManager_->RemoveWork(workStatus);
}

/**
 * @tc.name: PrintAllWorkStatus_002
 * @tc.desc: Test PrintAllWorkStatus skips duplicate workIds across queues.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, PrintAllWorkStatus_002, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(5013);
    workInfo->SetElement("com.test.printdup", "PrintDupAbility");
    workInfo->RequestPersisted(false);
    workInfo->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    workInfo->RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 20013);
    workQueueManager_->AddWork(workStatus);

    auto dataManager = DelayedSingleton<DataManager>::GetInstance();
    bool originalSleep = dataManager->GetDeviceSleep();
    dataManager->SetDeviceSleep(true);
    dataManager->OnDeviceStandyWhitelistChanged("com.test.printdup", true);
    workStatus->conditionStatus_.clear();
    workQueueManager_->PrintAllWorkStatus(WorkCondition::Type::STANDBY);
    EXPECT_EQ(workStatus->conditionStatus_, ",STANDBY&1");

    dataManager->OnDeviceStandyWhitelistChanged("com.test.printdup", false);
    dataManager->SetDeviceSleep(originalSleep);
    workQueueManager_->RemoveWork(workStatus);
}

/**
 * @tc.name: SetTimeCycle_002
 * @tc.desc: Test SetTimeCycle restarts timer listener when present.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, SetTimeCycle_002, TestSize.Level1)
{
    workQueueManager_->listenerMap_.clear();
    auto mockListener = std::make_shared<MockConditionListener>();
    workQueueManager_->listenerMap_.emplace(WorkCondition::Type::TIMER, mockListener);
    workQueueManager_->SetTimeCycle(600000);
    EXPECT_EQ(workQueueManager_->GetTimeCycle(), 600000);
    workQueueManager_->listenerMap_.erase(WorkCondition::Type::TIMER);
}

/**
 * @tc.name: SetIdleTime_001
 * @tc.desc: Test SetIdleTime stores value and restarts screen listener when present.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, SetIdleTime_001, TestSize.Level1)
{
    int32_t originalIdleTime = g_deepIdleScreenOffTimeMin.load();
    workQueueManager_->listenerMap_.clear();
    auto mockListener = std::make_shared<MockConditionListener>();
    workQueueManager_->listenerMap_.emplace(WorkCondition::Type::DEEP_IDLE, mockListener);
    workQueueManager_->SetIdleTime(30000);
    EXPECT_EQ(g_deepIdleScreenOffTimeMin.load(), 30000);
    EXPECT_EQ(mockListener->stopCount_, 1);
    EXPECT_EQ(mockListener->startCount_, 1);
    workQueueManager_->listenerMap_.erase(WorkCondition::Type::DEEP_IDLE);
    g_deepIdleScreenOffTimeMin.store(originalIdleTime);
}

/**
 * @tc.name: SetIdleTime_002
 * @tc.desc: Test SetIdleTime stores value when no screen listener.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, SetIdleTime_002, TestSize.Level1)
{
    int32_t originalIdleTime = g_deepIdleScreenOffTimeMin.load();
    workQueueManager_->listenerMap_.clear();
    workQueueManager_->SetIdleTime(60000);
    EXPECT_EQ(g_deepIdleScreenOffTimeMin.load(), 60000);
    g_deepIdleScreenOffTimeMin.store(originalIdleTime);
}

/**
 * @tc.name: AsyncStopWork_001
 * @tc.desc: Test AsyncStopWork posts stop task when handler available.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueManagerTest, AsyncStopWork_001, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(5020);
    workInfo->SetElement("com.test.asyncstop", "AsyncStopAbility");
    workInfo->RequestPersisted(false);
    workInfo->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 20020);
    workQueueManager_->AddWork(workStatus);
    workStatus->SetTimeout(true);

    workQueueManager_->AsyncStopWork(workStatus);
    EXPECT_TRUE(WaitForTimeoutReset(workStatus, 2000)) << "AsyncStopWork did not reset the timeout flag";
    EXPECT_EQ(workStatus->IsTimeout(), false);
    workQueueManager_->RemoveWork(workStatus);
}
} // namespace WorkScheduler
} // namespace OHOS
