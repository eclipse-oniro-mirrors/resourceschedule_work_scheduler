/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "gmock/gmock.h"

#include "work_scheduler_service.h"
#include "work_policy_manager.h"
#include "work_status.h"
#include "work_sched_errors.h"
#include "work_sched_utils.h"
#include "work_sched_constants.h"
#include "work_sched_data_manager.h"
#include "watchdog.h"


using namespace testing::ext;
using ::testing::Return;

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

class MockPolicyFilter : public IPolicyFilter {
public:
    explicit MockPolicyFilter(std::shared_ptr<WorkPolicyManager> workPolicyManager){};
    ~MockPolicyFilter(){};
    int32_t maxRunningCount;

    int32_t GetPolicyMaxRunning(WorkSchedSystemPolicy& systemPolicy) override
    {
        return maxRunningCount;
    }
};

class MockWorkConnManager : public WorkConnManager {
public:
    bool StopWork(std::shared_ptr<WorkStatus> workStatus, bool isTimeOut)
    {
        return true;
    }
};

std::shared_ptr<WorkPolicyManager> WorkPolicyManagerTest::workPolicyManager_ = nullptr;

void WorkPolicyManagerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
    workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
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

/**
 * @tc.name: RemoveWork_003
 * @tc.desc: Test WorkPolicyManagerTest RemoveWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, RemoveWork_003, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid1 = 10001;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid1);
    workPolicyManager_->AddWork(workStatus, uid1);

    int32_t uid2 = 10002;
    bool ret = workPolicyManager_->RemoveWork(workStatus, uid2);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: PauseRunningWorks_001
 * @tc.desc: Test WorkPolicyManagerTest PauseRunningWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, PauseRunningWorks_001, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    int32_t uid = 10000;
    int32_t ret = workPolicyManager_->PauseRunningWorks(uid);
    EXPECT_EQ(ret, E_UID_NO_MATCHING_WORK_ERR);
}

/**
 * @tc.name: PauseRunningWorks_002
 * @tc.desc: Test WorkPolicyManagerTest PauseRunningWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, PauseRunningWorks_002, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    int32_t ret = workPolicyManager_->PauseRunningWorks(uid);
    EXPECT_EQ(ret, E_UID_NO_MATCHING_WORK_ERR);
}

/**
 * @tc.name: PauseRunningWorks_003
 * @tc.desc: Test WorkPolicyManagerTest PauseRunningWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, PauseRunningWorks_003, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->SetPaused(true);
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    int32_t ret = workPolicyManager_->PauseRunningWorks(uid);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: PauseRunningWorks_004
 * @tc.desc: Test WorkPolicyManagerTest PauseRunningWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, PauseRunningWorks_004, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->SetPaused(false);
    workStatus->SetWorkWatchDogTime(120000);
    workStatus->SetWorkStartTime(WorkSchedUtils::GetCurrentTimeMs());
    std::shared_ptr<WorkSchedulerService> workSchedulerService = DelayedSingleton<WorkSchedulerService>::GetInstance();
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    std::shared_ptr<Watchdog> watchdog_ =
        std::make_shared<Watchdog>(workSchedulerService->GetWorkPolicyManager(), runner);
    workPolicyManager_->watchdog_ = watchdog_;
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    int32_t ret = workPolicyManager_->PauseRunningWorks(uid);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ResumePausedWorks_001
 * @tc.desc: Test WorkPolicyManagerTest ResumePausedWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, ResumePausedWorks_001, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    int32_t uid = 10000;
    int32_t ret = workPolicyManager_->ResumePausedWorks(uid);
    EXPECT_EQ(ret, E_UID_NO_MATCHING_WORK_ERR);
}

/**
 * @tc.name: ResumePausedWorks_002
 * @tc.desc: Test WorkPolicyManagerTest ResumePausedWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, ResumePausedWorks_002, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    int32_t ret = workPolicyManager_->ResumePausedWorks(uid);
    EXPECT_EQ(ret, E_UID_NO_MATCHING_WORK_ERR);
}

/**
 * @tc.name: ResumePausedWorks_003
 * @tc.desc: Test WorkPolicyManagerTest ResumePausedWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, ResumePausedWorks_003, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->SetPaused(false);
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    int32_t ret = workPolicyManager_->ResumePausedWorks(uid);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ResumePausedWorks_004
 * @tc.desc: Test WorkPolicyManagerTest ResumePausedWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, ResumePausedWorks_004, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->SetPaused(true);
    workStatus->SetWorkWatchDogTime(120000);
    workStatus->SetWorkStartTime(WorkSchedUtils::GetCurrentTimeMs());
    std::shared_ptr<WorkSchedulerService> workSchedulerService = DelayedSingleton<WorkSchedulerService>::GetInstance();
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    std::shared_ptr<Watchdog> watchdog_ =
        std::make_shared<Watchdog>(workSchedulerService->GetWorkPolicyManager(), runner);
    workPolicyManager_->watchdog_ = watchdog_;
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    int32_t ret = workPolicyManager_->ResumePausedWorks(uid);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetConditionString_001
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_001, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.name: GetConditionString_002
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_002, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_EQ(ret, "NETWORK");
}

/**
 * @tc.name: GetConditionString_003
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_003, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_EQ(ret, "CHARGER");
}

/**
 * @tc.name: GetConditionString_004
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_004, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_EQ(ret, "BATTERY_STATUS");
}

/**
 * @tc.name: GetConditionString_005
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_005, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryLevel(29);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_EQ(ret, "BATTERY_LEVEL");
}

/**
 * @tc.name: GetConditionString_006
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_006, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_EQ(ret, "STORAGE");
}

/**
 * @tc.name: GetConditionString_007
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_007, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestRepeatCycle(1200000);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_EQ(ret, "TIMER");
}

/**
 * @tc.name: GetConditionString_008
 * @tc.desc: Test WorkPolicyManagerTest GetConditionString.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetConditionString_008, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    std::string ret = workPolicyManager_->GetConditionString(workStatus);
    EXPECT_EQ(ret, "DEEP_IDLE");
}

/**
 * @tc.name: StopWork_001
 * @tc.desc: Test WorkPolicyManagerTest StopWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, StopWork_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    workPolicyManager_->workConnManager_ = std::make_shared<MockWorkConnManager>();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    std::pair<bool, bool> ret = workPolicyManager_->StopWork(workStatus, uid, false, false);
    EXPECT_TRUE(ret.second);
}

/**
 * @tc.name: StopWork_002
 * @tc.desc: Test WorkPolicyManagerTest StopWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, StopWork_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    workPolicyManager_->workConnManager_ = std::make_shared<MockWorkConnManager>();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestRepeatCycle(1200000);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    std::pair<bool, bool> ret = workPolicyManager_->StopWork(workStatus, uid, true, false);
    EXPECT_TRUE(ret.second);
}

/**
 * @tc.name: StopWork_004
 * @tc.desc: Test WorkPolicyManagerTest StopWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, StopWork_004, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    workPolicyManager_->workConnManager_ = std::make_shared<MockWorkConnManager>();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    std::pair<bool, bool> ret = workPolicyManager_->StopWork(workStatus, uid, false, true);
    EXPECT_TRUE(ret.second);
}

/**
 * @tc.name: StopAndClearWorks_001
 * @tc.desc: Test WorkPolicyManagerTest StopAndClearWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, StopAndClearWorks_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workPolicyManager_->AddWork(workStatus, uid);

    bool ret = workPolicyManager_->StopAndClearWorks(uid);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IsLastWorkTimeout_001
 * @tc.desc: Test WorkPolicyManagerTest IsLastWorkTimeout.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, IsLastWorkTimeout_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->lastTimeout_ = true;
    workPolicyManager_->AddWork(workStatus, uid);

    bool ret = false;
    workPolicyManager_->IsLastWorkTimeout(workId, uid, ret);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: GetMaxRunningCount_001
 * @tc.desc: Test WorkPolicyManagerTest GetMaxRunningCount.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetMaxRunningCount_001, TestSize.Level1)
{
    workPolicyManager_->dumpSetMaxRunningCount_ = 2;
    WorkSchedSystemPolicy systemPolicy;
    int32_t ret = workPolicyManager_->GetMaxRunningCount(systemPolicy);
    EXPECT_TRUE(ret == workPolicyManager_->dumpSetMaxRunningCount_);
}

/**
 * @tc.name: GetMaxRunningCount_002
 * @tc.desc: Test WorkPolicyManagerTest GetMaxRunningCount.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetMaxRunningCount_002, TestSize.Level1)
{
    workPolicyManager_->dumpSetMaxRunningCount_ = 0;
    WorkSchedSystemPolicy systemPolicy;
    int32_t ret = workPolicyManager_->GetMaxRunningCount(systemPolicy);
    EXPECT_EQ(ret, MAX_RUNNING_COUNT);
}

/**
 * @tc.name: GetMaxRunningCount_003
 * @tc.desc: Test WorkPolicyManagerTest GetMaxRunningCount.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetMaxRunningCount_003, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetMaxRunningCountByDump(5);
    int32_t result = workPolicyManager_->GetMaxRunningCount(systemPolicy);

    EXPECT_EQ(5, result);
    workPolicyManager_->policyFilters_.clear();
}

/**
 * @tc.name: GetMaxRunningCount_004
 * @tc.desc: Test WorkPolicyManagerTest GetMaxRunningCount.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetMaxRunningCount_004, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetMaxRunningCountByDump(0);

    auto filter1 = std::make_shared<MockPolicyFilter>(workPolicyManager_);
    filter1->maxRunningCount = 2;

    auto filter2 = std::make_shared<MockPolicyFilter>(workPolicyManager_);
    filter2->maxRunningCount = 4;

    workPolicyManager_->AddPolicyFilter(filter1);
    workPolicyManager_->AddPolicyFilter(filter2);

    int32_t result = workPolicyManager_->GetMaxRunningCount(systemPolicy);

    EXPECT_EQ(2, result);
    workPolicyManager_->policyFilters_.clear();
}

/**
 * @tc.name: GetMaxRunningCount_005
 * @tc.desc: Test WorkPolicyManagerTest GetMaxRunningCount.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetMaxRunningCount_005, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetMaxRunningCountByDump(0);

    auto filter1 = std::make_shared<MockPolicyFilter>(workPolicyManager_);
    filter1->maxRunningCount = 5;

    auto filter2 = std::make_shared<MockPolicyFilter>(workPolicyManager_);
    filter2->maxRunningCount = 3;

    auto filter3 = std::make_shared<MockPolicyFilter>(workPolicyManager_);
    filter3->maxRunningCount = 7;

    workPolicyManager_->AddPolicyFilter(filter1);
    workPolicyManager_->AddPolicyFilter(filter2);
    workPolicyManager_->AddPolicyFilter(filter3);

    int32_t result = workPolicyManager_->GetMaxRunningCount(systemPolicy);

    EXPECT_EQ(3, result);
    workPolicyManager_->policyFilters_.clear();
}

/**
 * @tc.name: GetMaxRunningCount_006
 * @tc.desc: Test WorkPolicyManagerTest GetMaxRunningCount.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetMaxRunningCount_006, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetMaxRunningCountByDump(0);

    int32_t result = workPolicyManager_->GetMaxRunningCount(systemPolicy);

    EXPECT_EQ(3, result);
    workPolicyManager_->policyFilters_.clear();
}

/**
 * @tc.name: GetMaxRunningCount_007
 * @tc.desc: Test WorkPolicyManagerTest GetMaxRunningCount.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetMaxRunningCount_007, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetMaxRunningCountByDump(0);

    auto filter1 = std::make_shared<MockPolicyFilter>(workPolicyManager_);
    filter1->maxRunningCount = 10;

    auto filter2 = std::make_shared<MockPolicyFilter>(workPolicyManager_);
    filter2->maxRunningCount = 5;

    auto filter3 = std::make_shared<MockPolicyFilter>(workPolicyManager_);
    filter3->maxRunningCount = 8;

    auto filter4 = std::make_shared<MockPolicyFilter>(workPolicyManager_);
    filter4->maxRunningCount = 3;

    workPolicyManager_->AddPolicyFilter(filter1);
    workPolicyManager_->AddPolicyFilter(filter2);
    workPolicyManager_->AddPolicyFilter(filter3);
    workPolicyManager_->AddPolicyFilter(filter4);

    int32_t result = workPolicyManager_->GetMaxRunningCount(systemPolicy);

    EXPECT_EQ(3, result);
    workPolicyManager_->policyFilters_.clear();
}

/**
 * @tc.name: GetRunningCount_001
 * @tc.desc: Test WorkPolicyManagerTest GetRunningCount.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetRunningCount_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workPolicyManager_->AddWork(workStatus, uid);

    int32_t ret = workPolicyManager_->GetRunningCount();
    EXPECT_TRUE(ret == 1);
}

/**
 * @tc.name: FindWork_001
 * @tc.desc: Test WorkPolicyManagerTest FindWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, FindWork_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workPolicyManager_->AddWork(workStatus, uid);

    bool ret = workPolicyManager_->FindWork(uid);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: FindWork_002
 * @tc.desc: Test WorkPolicyManagerTest FindWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, FindWork_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.bundleName_ = "com.test.demo";
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->userId_ = 100;
    workPolicyManager_->AddWork(workStatus, uid);

    bool ret = workPolicyManager_->FindWork(workStatus->userId_, workStatus->bundleName_);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: FindWork_003
 * @tc.desc: Test WorkPolicyManagerTest FindWork.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, FindWork_003, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.bundleName_ = "com.test.demo";
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->userId_ = 100;

    bool ret = workPolicyManager_->FindWork(workStatus->userId_, workStatus->bundleName_);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetDeepIdleWorks_001
 * @tc.desc: Test WorkPolicyManagerTest GetDeepIdleWorks.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkPolicyManagerTest, GetDeepIdleWorks_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.bundleName_ = "com.test.demo";
    int32_t workId = 10000;
    workinfo.SetWorkId(workId);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->userId_ = 100;
    workPolicyManager_->AddWork(workStatus, uid);

    std::list<std::shared_ptr<WorkStatus>> ret = workPolicyManager_->GetDeepIdleWorks();
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret.front()->bundleName_, "com.test.demo");
}

/**
 * @tc.name: DumpTriggerWork_001
 * @tc.desc: Test WorkPolicyManagerTest DumpTriggerWork.
 * @tc.type: FUNC
 * @tc.require: IC5H77
 */
HWTEST_F(WorkPolicyManagerTest, DumpTriggerWork_001, TestSize.Level1)
{
    int32_t uId = 1;
    int32_t workId = 1;
    std::string result;
    workPolicyManager_->DumpTriggerWork(uId, workId, result);
    EXPECT_EQ(result, "the work is not exist\n");
}

/**
 * @tc.name: DumpTriggerWork_002
 * @tc.desc: Test WorkPolicyManagerTest DumpTriggerWork.
 * @tc.type: FUNC
 * @tc.require: IC5H77
 */
HWTEST_F(WorkPolicyManagerTest, DumpTriggerWork_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    int32_t uId = 1;
    int32_t workId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(workId);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uId);
    workStatus->MarkStatus(WorkStatus::RUNNING);
    int32_t ret = workPolicyManager_->AddWork(workStatus, uId);
    std::string result;
    workPolicyManager_->DumpTriggerWork(uId, workId, result);
    EXPECT_EQ(result, "the work is running\n");
}

/**
 * @tc.name: DumpTriggerWork_003
 * @tc.desc: Test WorkPolicyManagerTest DumpTriggerWork.
 * @tc.type: FUNC
 * @tc.require: IC5H77
 */
HWTEST_F(WorkPolicyManagerTest, DumpTriggerWork_003, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    int32_t uId = 1;
    int32_t workId = 1;
    WorkInfo workinfo;
    workinfo.SetWorkId(workId);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uId);
    workStatus->MarkStatus(WorkStatus::WAIT_CONDITION);
    int32_t ret = workPolicyManager_->AddWork(workStatus, uId);
    std::string result;
    workPolicyManager_->DumpTriggerWork(uId, workId, result);
    EXPECT_EQ(result, "the work trigger error\n");
}

/**
 * @tc.name: UpdateWatchdogTime_001
 * @tc.desc: Test WorkPolicyManagerTest UpdateWatchdogTime.
 * @tc.type: FUNC
 * @tc.require: https://gitee.com/openharmony/resourceschedule_work_scheduler/issues/ICBI5I
 */
HWTEST_F(WorkPolicyManagerTest, UpdateWatchdogTime_001, TestSize.Level1)
{
    WorkInfo workInfo;
    int32_t workId = 10000;
    int32_t uid = 20200;
    workInfo.SetWorkId(workId);
    workInfo.RequestDeepIdle(true);
    workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<WorkStatus> topWork = std::make_shared<WorkStatus>(workInfo, uid);
    std::shared_ptr<WorkSchedulerService> workSchedulerService = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workPolicyManager_->UpdateWatchdogTime(workSchedulerService, topWork);
    EXPECT_EQ(workPolicyManager_->GetWatchdogTime(), LONG_WATCHDOG_TIME);
}

/**
 * @tc.name: UpdateWatchdogTime_002
 * @tc.desc: Test WorkPolicyManagerTest UpdateWatchdogTime.
 * @tc.type: FUNC
 * @tc.require: https://gitee.com/openharmony/resourceschedule_work_scheduler/issues/ICBI5I
 */
HWTEST_F(WorkPolicyManagerTest, UpdateWatchdogTime_002, TestSize.Level1)
{
    WorkInfo workInfo;
    int32_t workId = 10000;
    int32_t uid = 20200;
    workInfo.SetWorkId(workId);
    workInfo.RequestDeepIdle(false);
    workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_UNKNOWN);
    std::shared_ptr<WorkStatus> topWork = std::make_shared<WorkStatus>(workInfo, uid);
    std::shared_ptr<WorkSchedulerService> workSchedulerService = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workSchedulerService->UpdateEffiResApplyInfo(uid, false);

    workPolicyManager_->UpdateWatchdogTime(workSchedulerService, topWork);
    EXPECT_EQ(workPolicyManager_->GetWatchdogTime(), g_lastWatchdogTime);
}

/**
 * @tc.name: UpdateWatchdogTime_003
 * @tc.desc: Test WorkPolicyManagerTest UpdateWatchdogTime.
 * @tc.type: FUNC
 * @tc.require: https://gitee.com/openharmony/resourceschedule_work_scheduler/issues/ICBI5I
 */
HWTEST_F(WorkPolicyManagerTest, UpdateWatchdogTime_003, TestSize.Level1)
{
    WorkInfo workInfo;
    int32_t workId = 10000;
    int32_t uid = 20200;
    workInfo.SetWorkId(workId);
    workInfo.RequestDeepIdle(false);
    workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_UNKNOWN);
    std::shared_ptr<WorkStatus> topWork = std::make_shared<WorkStatus>(workInfo, uid);

    auto workCondition = std::make_shared<Condition>();
    workCondition->enumVal = static_cast<int32_t>(WorkCondition::Charger::CHARGING_UNKNOWN);
    topWork->conditionMap_.emplace(WorkCondition::Type::CHARGER, workCondition);

    std::shared_ptr<WorkSchedulerService> workSchedulerService = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workSchedulerService->UpdateEffiResApplyInfo(uid, true);

    workPolicyManager_->UpdateWatchdogTime(workSchedulerService, topWork);
    EXPECT_EQ(workPolicyManager_->GetWatchdogTime(), MEDIUM_WATCHDOG_TIME);
}

/**
 * @tc.name: UpdateWatchdogTime_004
 * @tc.desc: Test WorkPolicyManagerTest UpdateWatchdogTime.
 * @tc.type: FUNC
 * @tc.require: https://gitee.com/openharmony/resourceschedule_work_scheduler/issues/ICBI5I
 */
HWTEST_F(WorkPolicyManagerTest, UpdateWatchdogTime_004, TestSize.Level1)
{
    WorkInfo workInfo;
    int32_t workId = 10000;
    int32_t uid = 20200;
    workInfo.SetWorkId(workId);
    workInfo.RequestDeepIdle(false);
    workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<WorkStatus> topWork = std::make_shared<WorkStatus>(workInfo, uid);

    auto workCondition = std::make_shared<Condition>();
    workCondition->enumVal = static_cast<int32_t>(WorkCondition::Charger::CHARGING_PLUGGED_AC);
    topWork->conditionMap_.emplace(WorkCondition::Type::CHARGER, workCondition);

    std::shared_ptr<WorkSchedulerService> workSchedulerService = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workSchedulerService->UpdateEffiResApplyInfo(uid, true);

    workPolicyManager_->UpdateWatchdogTime(workSchedulerService, topWork);
    EXPECT_EQ(workPolicyManager_->GetWatchdogTime(), LONG_WATCHDOG_TIME);
}

/**
 * @tc.name: UpdateWatchdogTime_005
 * @tc.desc: Test UpdateWatchdogTime sets WATCHDOG_TIME for performance sensitive work.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, UpdateWatchdogTime_005, TestSize.Level1)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(10005);
    int32_t uid = 20205;
    workInfo.SetElement("com.test.perfsensitive", "PerfSensitiveAbility");
    workInfo.RequestPerformanceSensitive(true);
    std::shared_ptr<WorkStatus> topWork = std::make_shared<WorkStatus>(workInfo, uid);
    std::shared_ptr<WorkSchedulerService> workSchedulerService = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workPolicyManager_->UpdateWatchdogTime(workSchedulerService, topWork);
    EXPECT_EQ(workPolicyManager_->GetWatchdogTime(), WATCHDOG_TIME);
}

/**
 * @tc.name: UpdateWatchdogTime_006
 * @tc.desc: Test UpdateWatchdogTime sets MEDIUM_WATCHDOG_TIME when no charger in conditionMap.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, UpdateWatchdogTime_006, TestSize.Level1)
{
    WorkInfo workInfo;
    workInfo.SetWorkId(10006);
    int32_t uid = 20206;
    workInfo.SetElement("com.test.nocharger", "NoChargerAbility");
    workInfo.RequestDeepIdle(false);
    workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_UNKNOWN);
    std::shared_ptr<WorkStatus> topWork = std::make_shared<WorkStatus>(workInfo, uid);
    std::shared_ptr<WorkSchedulerService> workSchedulerService = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workSchedulerService->UpdateEffiResApplyInfo(uid, true);
    workPolicyManager_->UpdateWatchdogTime(workSchedulerService, topWork);
    EXPECT_EQ(workPolicyManager_->GetWatchdogTime(), MEDIUM_WATCHDOG_TIME);
}

/**
 * @tc.name: WatchdogTimeOut_001
 * @tc.desc: Test WorkPolicyManagerTest WatchdogTimeOut.
 * @tc.type: FUNC
 * @tc.require: https://gitee.com/openharmony/resourceschedule_work_scheduler/issues/ICBI5I
 */
HWTEST_F(WorkPolicyManagerTest, WatchdogTimeOut_001, TestSize.Level1)
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 1;
    workPolicyManager_->WatchdogTimeOut(watchdogId);
    EXPECT_EQ(workPolicyManager_->watchdogIdMap_.size(), 0);
}

/**
 * @tc.name: WatchdogTimeOut_002
 * @tc.desc: Test WatchdogTimeOut erases work from watchdogIdMap when found.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, WatchdogTimeOut_002, TestSize.Level1)
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService = std::make_shared<WorkSchedulerService>();
    workSchedulerService->workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workSchedulerService->workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 2;
    WorkInfo workinfo;
    workinfo.SetWorkId(60002);
    workinfo.SetElement("com.test.timeout", "TimeoutAbility");
    int32_t uid = 60002;
    auto workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    EXPECT_EQ(workPolicyManager_->watchdogIdMap_.size(), 1);
    workPolicyManager_->WatchdogTimeOut(watchdogId);
    EXPECT_EQ(workPolicyManager_->watchdogIdMap_.size(), 0);
}

/**
 * @tc.name: GetAllRunningWorkStatus_001
 * @tc.desc: Test GetAllRunningWorkStatus with a running work.
 * @tc.type: FUNC
 */
HWTEST_F(WorkPolicyManagerTest, GetAllRunningWorkStatus_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workPolicyManager_->AddWork(workStatus, uid);

    auto result = workPolicyManager_->GetAllRunningWorkStatus();
    EXPECT_EQ(result.size(), 1);
}

/**
 * @tc.name: CleanOrphanWork_001
 * @tc.desc: Test CleanOrphanWork with non-repeating work, verify status removed and queue cleaned.
 * @tc.type: FUNC
 */
HWTEST_F(WorkPolicyManagerTest, CleanOrphanWork_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    workPolicyManager_->workConnManager_ = std::make_shared<MockWorkConnManager>();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->SetWorkStartTime(1000);
    workStatus->SetWorkWatchDogTime(2000);
    workStatus->SetDuration(500);
    workPolicyManager_->AddWork(workStatus, uid);
    EXPECT_EQ(workPolicyManager_->uidQueueMap_.count(uid), 1);

    workPolicyManager_->CleanOrphanWork(workStatus);
    EXPECT_EQ(workStatus->GetStatus(), WorkStatus::Status::REMOVED);
    EXPECT_EQ(workPolicyManager_->uidQueueMap_.count(uid), 0);
}

/**
 * @tc.name: CleanOrphanWork_002
 * @tc.desc: Test CleanOrphanWork with repeating work, verify status reset and work retained.
 * @tc.type: FUNC
 */
HWTEST_F(WorkPolicyManagerTest, CleanOrphanWork_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    workPolicyManager_->workConnManager_ = std::make_shared<MockWorkConnManager>();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestRepeatCycle(1200000);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->SetWorkStartTime(1000);
    workStatus->SetWorkWatchDogTime(2000);
    workStatus->SetDuration(500);
    workPolicyManager_->AddWork(workStatus, uid);
    EXPECT_EQ(workPolicyManager_->uidQueueMap_.count(uid), 1);

    workPolicyManager_->CleanOrphanWork(workStatus);
    EXPECT_EQ(workStatus->GetStatus(), WorkStatus::Status::WAIT_CONDITION);
    EXPECT_EQ(workStatus->GetWorkStartTime(), 0);
    EXPECT_EQ(workStatus->GetWorkWatchDogTime(), 0);
    EXPECT_EQ(workStatus->GetDuration(), 0);
    EXPECT_EQ(workPolicyManager_->uidQueueMap_.count(uid), 1);
}

/**
 * @tc.name: NewWatchdogId_001
 * @tc.desc: Test NewWatchdogId wraps around at MAX_WATCHDOG_ID.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, NewWatchdogId_001, TestSize.Level1)
{
    workPolicyManager_->watchdogId_ = MAX_WATCHDOG_ID;
    uint32_t ret = workPolicyManager_->NewWatchdogId();
    EXPECT_EQ(ret, MAX_WATCHDOG_ID);
    EXPECT_EQ(workPolicyManager_->watchdogId_.load(), INIT_WATCHDOG_ID);
}

/**
 * @tc.name: NewWatchdogId_002
 * @tc.desc: Test NewWatchdogId increments normally.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, NewWatchdogId_002, TestSize.Level1)
{
    workPolicyManager_->watchdogId_ = 5;
    uint32_t ret = workPolicyManager_->NewWatchdogId();
    EXPECT_EQ(ret, 5);
}

/**
 * @tc.name: ObtainAllWorks_001
 * @tc.desc: Test ObtainAllWorks returns works for existing uid.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, ObtainAllWorks_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(20001);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 20001;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->AddWork(workStatus, uid);
    auto ret = workPolicyManager_->ObtainAllWorks(uid);
    EXPECT_EQ(ret.size(), 1);
    workPolicyManager_->uidQueueMap_.clear();
}

/**
 * @tc.name: ObtainAllWorks_002
 * @tc.desc: Test ObtainAllWorks returns empty for non-existing uid.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, ObtainAllWorks_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    int32_t uid = 99999;
    auto ret = workPolicyManager_->ObtainAllWorks(uid);
    EXPECT_EQ(ret.size(), 0);
}

/**
 * @tc.name: GetWorkStatus_001
 * @tc.desc: Test GetWorkStatus returns workInfo for existing work.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, GetWorkStatus_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(30001);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    int32_t uid = 30001;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->AddWork(workStatus, uid);
    int32_t queryUid = uid;
    int32_t queryWorkId = 30001;
    auto ret = workPolicyManager_->GetWorkStatus(queryUid, queryWorkId);
    EXPECT_EQ(ret->GetWorkId(), 30001);
    workPolicyManager_->uidQueueMap_.clear();
}

/**
 * @tc.name: GetWorkStatus_002
 * @tc.desc: Test GetWorkStatus returns nullptr for non-existing work.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, GetWorkStatus_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    int32_t queryUid = 99999;
    int32_t queryWorkId = 99999;
    workPolicyManager_->GetWorkStatus(queryUid, queryWorkId);
    EXPECT_FALSE(workPolicyManager_->FindWork(queryUid));
}

/**
 * @tc.name: FindWorkStatus_001
 * @tc.desc: Test FindWorkStatus by uid and workId returns workStatus.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, FindWorkStatus_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(40001);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    int32_t uid = 40001;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->AddWork(workStatus, uid);
    auto ret = workPolicyManager_->FindWorkStatus(uid, 40001);
    EXPECT_EQ(workPolicyManager_->uidQueueMap_.count(uid), 1);
    EXPECT_EQ(ret.get(), workStatus.get());
    workPolicyManager_->uidQueueMap_.clear();
}

/**
 * @tc.name: FindWorkStatus_002
 * @tc.desc: Test FindWorkStatus by uid and workId returns nullptr when not found.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, FindWorkStatus_002, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    workPolicyManager_->FindWorkStatus(99999, 99999);
    EXPECT_FALSE(workPolicyManager_->FindWork(99999));
}

/**
 * @tc.name: RemoveWatchDog_001
 * @tc.desc: Test RemoveWatchDog removes entry from watchdogIdMap_.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, RemoveWatchDog_001, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 10;
    WorkInfo workinfo;
    workinfo.SetWorkId(60001);
    int32_t uid = 60001;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->workId_ = std::to_string(workinfo.GetWorkId()) + "_" + std::to_string(uid);
    std::shared_ptr<WorkSchedulerService> workSchedulerService = DelayedSingleton<WorkSchedulerService>::GetInstance();
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    std::shared_ptr<Watchdog> watchdog =
        std::make_shared<Watchdog>(workSchedulerService->GetWorkPolicyManager(), runner);
    workPolicyManager_->watchdog_ = watchdog;
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    EXPECT_EQ(workPolicyManager_->watchdogIdMap_.size(), 1);
    workPolicyManager_->RemoveWatchDog(workStatus);
    EXPECT_EQ(workPolicyManager_->watchdogIdMap_.size(), 0);
}

/**
 * @tc.name: RemoveWatchDog_002
 * @tc.desc: Test RemoveWatchDog with null workStatus does nothing.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, RemoveWatchDog_002, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    uint32_t watchdogId = 11;
    WorkInfo workinfo;
    workinfo.SetWorkId(60002);
    int32_t uid = 60002;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->watchdogIdMap_.emplace(watchdogId, workStatus);
    auto before = workPolicyManager_->watchdogIdMap_.size();
    workPolicyManager_->RemoveWatchDog(nullptr);
    EXPECT_EQ(workPolicyManager_->watchdogIdMap_.size(), before);
    workPolicyManager_->watchdogIdMap_.clear();
}

/**
 * @tc.name: SetWatchdogTimeByDump_001
 * @tc.desc: Test SetWatchdogTimeByDump with 0 uses WATCHDOG_TIME default.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, SetWatchdogTimeByDump_001, TestSize.Level1)
{
    workPolicyManager_->SetWatchdogTimeByDump(0);
    EXPECT_EQ(workPolicyManager_->GetWatchdogTime(), WATCHDOG_TIME);
}

/**
 * @tc.name: SetWatchdogTimeByDump_002
 * @tc.desc: Test SetWatchdogTimeByDump with non-zero sets custom time.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, SetWatchdogTimeByDump_002, TestSize.Level1)
{
    workPolicyManager_->SetWatchdogTimeByDump(300000);
    EXPECT_EQ(workPolicyManager_->GetWatchdogTime(), 300000);
    workPolicyManager_->SetWatchdogTimeByDump(0);
}

/**
 * @tc.name: GetAllRunningWorks_001
 * @tc.desc: Test GetAllRunningWorks returns running works.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, GetAllRunningWorks_001, TestSize.Level1)
{
    workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(70001);
    workinfo.RequestDeepIdle(true);
    int32_t uid = 70001;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workPolicyManager_->AddWork(workStatus, uid);
    auto ret = workPolicyManager_->GetAllRunningWorks();
    EXPECT_EQ(ret.size(), 1);
    workPolicyManager_->uidQueueMap_.clear();
}

/**
 * @tc.name: OnPolicyChanged_001
 * @tc.desc: Test OnPolicyChanged with USER_SWITCHED policy.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, OnPolicyChanged_001, TestSize.Level1)
{
    auto detectorVal = std::make_shared<DetectorValue>(0, 0, false, "");
    workPolicyManager_->OnPolicyChanged(PolicyType::USER_SWITCHED, detectorVal);
    EXPECT_EQ(workPolicyManager_->uidQueueMap_.size(), 0);
}

/**
 * @tc.name: OnPolicyChanged_002
 * @tc.desc: Test OnPolicyChanged with USER_STARTED policy.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, OnPolicyChanged_002, TestSize.Level1)
{
    auto detectorVal = std::make_shared<DetectorValue>(0, 0, false, "");
    workPolicyManager_->OnPolicyChanged(PolicyType::USER_STARTED, detectorVal);
    EXPECT_EQ(workPolicyManager_->uidQueueMap_.size(), 0);
}

/**
 * @tc.name: OnPolicyChanged_003
 * @tc.desc: Test OnPolicyChanged with APP_REMOVED policy clears uid maps.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, OnPolicyChanged_003, TestSize.Level1)
{
    int32_t uid = 70101;
    WorkStatus::s_uid_last_time_map[uid] = 100;
    WorkStatus::performanceSensitiveLastTimeMap_[uid] = 100;
    auto detectorVal = std::make_shared<DetectorValue>(uid, 0, false, "com.test.removed");
    auto service = std::make_shared<WorkSchedulerService>();
    auto manager = std::make_shared<WorkPolicyManager>(service);
    service->workPolicyManager_ = manager;
    service->workQueueManager_ = std::make_shared<WorkQueueManager>(service);
    manager->workConnManager_ = std::make_shared<WorkConnManager>();
    manager->OnPolicyChanged(PolicyType::APP_REMOVED, detectorVal);
    EXPECT_EQ(WorkStatus::s_uid_last_time_map.count(uid), 0);
    EXPECT_EQ(WorkStatus::performanceSensitiveLastTimeMap_.count(uid), 0);
}

/**
 * @tc.name: OnPolicyChanged_004
 * @tc.desc: Test OnPolicyChanged with APP_ADDED policy for non-preinstalled bundle.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, OnPolicyChanged_004, TestSize.Level1)
{
    auto detectorVal = std::make_shared<DetectorValue>(0, 0, false, "com.test.nonpreinstalled");
    workPolicyManager_->OnPolicyChanged(PolicyType::APP_ADDED, detectorVal);
    EXPECT_EQ(workPolicyManager_->uidQueueMap_.size(), 0);
}

/**
 * @tc.name: OnPolicyChanged_005
 * @tc.desc: Test OnPolicyChanged with APP_DATA_CLEAR policy falls into default.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, OnPolicyChanged_005, TestSize.Level1)
{
    auto detectorVal = std::make_shared<DetectorValue>(0, 0, false, "");
    workPolicyManager_->OnPolicyChanged(PolicyType::APP_DATA_CLEAR, detectorVal);
    EXPECT_EQ(workPolicyManager_->uidQueueMap_.size(), 0);
}

/**
 * @tc.name: IsSpecialScene_001
 * @tc.desc: Test IsSpecialScene returns false when bundle in restrictlist.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, IsSpecialScene_001, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(70201);
    workinfo.SetElement("com.test.restrict2", "RestrictAbility2");
    auto workStatus = std::make_shared<WorkStatus>(workinfo, 70201);
    std::list<std::string> restrictList = {"com.test.restrict2"};
    DelayedSingleton<DataManager>::GetInstance()->AddDeviceStandyRestrictlist(restrictList);
    bool ret = workPolicyManager_->IsSpecialScene(workStatus, 0);
    EXPECT_FALSE(ret);
    DelayedSingleton<DataManager>::GetInstance()->ClearDeviceStandyRestrictlist();
}

/**
 * @tc.name: IsSpecialScene_002
 * @tc.desc: Test IsSpecialScene returns true when device sleep and bundle in whitelist.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, IsSpecialScene_002, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(70202);
    workinfo.SetElement("com.test.whitelist", "WhitelistAbility");
    auto workStatus = std::make_shared<WorkStatus>(workinfo, 70202);
    DelayedSingleton<DataManager>::GetInstance()->SetDeviceSleep(true);
    std::list<std::string> whitelist = {"com.test.whitelist"};
    DelayedSingleton<DataManager>::GetInstance()->AddDeviceStandyWhitelist(whitelist);
    auto service = std::make_shared<WorkSchedulerService>();
    auto manager = std::make_shared<WorkPolicyManager>(service);
    bool ret = manager->IsSpecialScene(workStatus, 0);
    EXPECT_TRUE(ret);
    DelayedSingleton<DataManager>::GetInstance()->ClearDeviceStandyWhitelist();
    DelayedSingleton<DataManager>::GetInstance()->SetDeviceSleep(false);
}

/**
 * @tc.name: IsSpecialScene_003
 * @tc.desc: Test IsSpecialScene returns false when no special condition met.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, IsSpecialScene_003, TestSize.Level1)
{
    WorkInfo workinfo;
    workinfo.SetWorkId(70203);
    workinfo.SetElement("com.test.normal", "NormalAbility");
    auto workStatus = std::make_shared<WorkStatus>(workinfo, 70203);
    DelayedSingleton<DataManager>::GetInstance()->SetDeviceSleep(false);
    DelayedSingleton<DataManager>::GetInstance()->ClearDeviceStandyRestrictlist();
    DelayedSingleton<DataManager>::GetInstance()->ClearDeviceStandyWhitelist();
    bool ret = workPolicyManager_->IsSpecialScene(workStatus, 0);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetWorkToRun_001
 * @tc.desc: Test GetWorkToRun returns nullptr when ready queue empty.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, GetWorkToRun_001, TestSize.Level1)
{
    workPolicyManager_->conditionReadyQueue_->ClearAll();
    workPolicyManager_->GetWorkToRun();
    EXPECT_FALSE(workPolicyManager_->FindWork(0));
}

/**
 * @tc.name: GetWorkToRun_002
 * @tc.desc: Test GetWorkToRun returns top work when ready queue has CONDITION_READY work.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, GetWorkToRun_002, TestSize.Level1)
{
    workPolicyManager_->conditionReadyQueue_->ClearAll();
    WorkInfo workinfo;
    workinfo.SetWorkId(70301);
    workinfo.SetElement("com.test.getworktorun", "GetWorkToRunAbility");
    auto workStatus = std::make_shared<WorkStatus>(workinfo, 70301);
    workStatus->MarkStatus(WorkStatus::Status::CONDITION_READY);
    workPolicyManager_->conditionReadyQueue_->Push(workStatus);
    auto ret = workPolicyManager_->GetWorkToRun();
    EXPECT_EQ(ret->GetStatus(), WorkStatus::Status::RUNNING);
    workPolicyManager_->conditionReadyQueue_->ClearAll();
}

/**
 * @tc.name: RealStartSA_001
 * @tc.desc: Test RealStartSA reverts to condition ready when LoadSa fails.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, RealStartSA_001, TestSize.Level1)
{
    workPolicyManager_->conditionReadyQueue_->ClearAll();
    WorkInfo workinfo;
    workinfo.SetWorkId(70310);
    workinfo.SetElement("com.test.realsa", "RealSaAbility");
    workinfo.saId_ = 999;
    auto workStatus = std::make_shared<WorkStatus>(workinfo, 70310);
    workStatus->MarkStatus(WorkStatus::Status::CONDITION_READY);
    workPolicyManager_->conditionReadyQueue_->Push(workStatus);
    workPolicyManager_->RealStartSA(workStatus);
    EXPECT_FALSE(workStatus->IsRunning());
    workPolicyManager_->conditionReadyQueue_->ClearAll();
}

/**
 * @tc.name: RealStartSA_002
 * @tc.desc: Test RealStartSA handles non-repeating work when LoadSa fails.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, RealStartSA_002, TestSize.Level1)
{
    workPolicyManager_->conditionReadyQueue_->ClearAll();
    WorkInfo workinfo;
    workinfo.SetWorkId(70311);
    workinfo.SetElement("com.test.realsa2", "RealSaAbility2");
    workinfo.saId_ = 1000;
    workinfo.RequestRepeatCycle(10000);
    auto workStatus = std::make_shared<WorkStatus>(workinfo, 70311);
    workStatus->MarkStatus(WorkStatus::Status::CONDITION_READY);
    workPolicyManager_->conditionReadyQueue_->Push(workStatus);
    workPolicyManager_->RealStartSA(workStatus);
    EXPECT_FALSE(workStatus->IsRunning());
    workPolicyManager_->conditionReadyQueue_->ClearAll();
}

/**
 * @tc.name: RealStartWork_002
 * @tc.desc: Test RealStartWork handles non-repeating work when StartWork fails.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, RealStartWork_002, TestSize.Level1)
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService = std::make_shared<WorkSchedulerService>();
    workSchedulerService->workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService);
    auto workPolicyManager = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workPolicyManager->workConnManager_ = std::make_shared<WorkConnManager>();
    WorkInfo workinfo;
    workinfo.SetWorkId(70320);
    workinfo.SetElement("com.test.realstart", "RealStartAbility");
    int32_t uid = 70320;
    auto topWork = std::make_shared<WorkStatus>(workinfo, uid);
    topWork->MarkStatus(WorkStatus::Status::CONDITION_READY);
    workPolicyManager->conditionReadyQueue_->Push(topWork);
    workPolicyManager->RealStartWork(topWork);
    EXPECT_FALSE(topWork->IsRunning());
    EXPECT_EQ(topWork->GetStatus(), WorkStatus::Status::REMOVED);
    workPolicyManager->conditionReadyQueue_->ClearAll();
}

/**
 * @tc.name: RealStartWork_003
 * @tc.desc: Test RealStartWork handles repeating work when StartWork fails.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, RealStartWork_003, TestSize.Level1)
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService = std::make_shared<WorkSchedulerService>();
    auto workPolicyManager = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workPolicyManager->workConnManager_ = std::make_shared<WorkConnManager>();
    WorkInfo workinfo;
    workinfo.SetWorkId(70321);
    workinfo.SetElement("com.test.realstart2", "RealStartAbility2");
    workinfo.RequestRepeatCycle(10000);
    int32_t uid = 70321;
    auto topWork = std::make_shared<WorkStatus>(workinfo, uid);
    topWork->MarkStatus(WorkStatus::Status::CONDITION_READY);
    workPolicyManager->conditionReadyQueue_->Push(topWork);
    workPolicyManager->RealStartWork(topWork);
    EXPECT_FALSE(topWork->IsRunning());
    EXPECT_EQ(topWork->GetStatus(), WorkStatus::Status::WAIT_CONDITION);
    workPolicyManager->conditionReadyQueue_->ClearAll();
}

/**
 * @tc.name: AddWatchdogForWork_001
 * @tc.desc: Test AddWatchdogForWork adds watchdog and inserts into watchdogIdMap.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, AddWatchdogForWork_001, TestSize.Level1)
{
    workPolicyManager_->watchdogIdMap_.clear();
    std::shared_ptr<WorkSchedulerService> workSchedulerService = DelayedSingleton<WorkSchedulerService>::GetInstance();
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    std::shared_ptr<Watchdog> watchdog =
        std::make_shared<Watchdog>(workSchedulerService->GetWorkPolicyManager(), runner);
    workPolicyManager_->watchdog_ = watchdog;
    WorkInfo workinfo;
    workinfo.SetWorkId(70330);
    workinfo.SetElement("com.test.addwatchdog", "AddWatchdogAbility");
    int32_t uid = 70330;
    auto workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    size_t beforeSize = workPolicyManager_->watchdogIdMap_.size();
    workPolicyManager_->AddWatchdogForWork(workStatus);
    EXPECT_EQ(workPolicyManager_->watchdogIdMap_.size(), beforeSize + 1);
    EXPECT_NE(workStatus->GetWorkStartTime(), 0);
    workPolicyManager_->watchdogIdMap_.clear();
    workPolicyManager_->watchdog_ = nullptr;
}

/**
 * @tc.name: SendRetrigger_001
 * @tc.desc: Test SendRetrigger returns early when handler is null.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, SendRetrigger_001, TestSize.Level1)
{
    workPolicyManager_->handler_ = nullptr;
    workPolicyManager_->SendRetrigger(1000);
}

/**
 * @tc.name: SendRetrigger_002
 * @tc.desc: Test SendRetrigger sends event when handler is not null.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, SendRetrigger_002, TestSize.Level1)
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService = std::make_shared<WorkSchedulerService>();
    auto runner = AppExecFwk::EventRunner::Create("test_sendretrigger", AppExecFwk::ThreadMode::FFRT);
    workSchedulerService->eventRunner_ = runner;
    workSchedulerService->handler_ = std::make_shared<WorkEventHandler>(runner, workSchedulerService);
    auto workPolicyManager = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workPolicyManager->handler_ = workSchedulerService->handler_;
    workPolicyManager->SendRetrigger(DELAY_TIME_SHORT);
}

/**
 * @tc.name: IsNeedDiscreteScheduled_001
 * @tc.desc: Test IsNeedDiscreteScheduled returns a valid bool based on current time.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, IsNeedDiscreteScheduled_001, TestSize.Level1)
{
    bool ret = workPolicyManager_->IsNeedDiscreteScheduled();
    time_t t;
    (void)time(&t);
    struct tm nowTime;
    (void)localtime_r(&t, &nowTime);
    bool expected = (nowTime.tm_hour >= 6 && nowTime.tm_hour <= 9) &&
        (nowTime.tm_min == 0 || nowTime.tm_min == 30);
    EXPECT_EQ(ret, expected);
}

/**
 * @tc.name: DiscreteScheduled_001
 * @tc.desc: Test DiscreteScheduled returns early when handler is null.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, DiscreteScheduled_001, TestSize.Level1)
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService = std::make_shared<WorkSchedulerService>();
    auto workPolicyManager = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workPolicyManager->handler_ = nullptr;
    WorkInfo workinfo;
    workinfo.SetWorkId(70340);
    workinfo.SetElement("com.test.discrete1", "DiscreteAbility1");
    int32_t uid = 70340;
    auto topWork = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager->DiscreteScheduled(topWork);
    EXPECT_FALSE(topWork->IsRunning());
}

/**
 * @tc.name: DiscreteScheduled_002
 * @tc.desc: Test DiscreteScheduled posts task when handler is not null.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkPolicyManagerTest, DiscreteScheduled_002, TestSize.Level1)
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService = std::make_shared<WorkSchedulerService>();
    auto runner = AppExecFwk::EventRunner::Create("test_discrete", AppExecFwk::ThreadMode::FFRT);
    workSchedulerService->eventRunner_ = runner;
    workSchedulerService->handler_ = std::make_shared<WorkEventHandler>(runner, workSchedulerService);
    auto workPolicyManager = std::make_shared<WorkPolicyManager>(workSchedulerService);
    workPolicyManager->handler_ = workSchedulerService->handler_;
    workPolicyManager->workConnManager_ = std::make_shared<WorkConnManager>();
    WorkInfo workinfo;
    workinfo.SetWorkId(70341);
    workinfo.SetElement("com.test.discrete2", "DiscreteAbility2");
    int32_t uid = 70341;
    auto topWork = std::make_shared<WorkStatus>(workinfo, uid);
    topWork->MarkStatus(WorkStatus::Status::CONDITION_READY);
    workPolicyManager->DiscreteScheduled(topWork);
    EXPECT_FALSE(topWork->IsRunning());
}
}
}