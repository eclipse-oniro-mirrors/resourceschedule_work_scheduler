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

#include "work_queue_manager.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"
#include "work_condition.h"
#include "work_sched_hilog.h"
#include "work_info.h"

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
    std::shared_ptr<WorkPolicyManager> workPolicyManager_ =
        std::make_shared<MockWorkPolicyManager>(workSchedulerService_);
    workSchedulerService_->workPolicyManager_ = workPolicyManager_;
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
}

/**
 * @tc.name: ClearTimeOutWorkStatus_001
 * @tc.desc: Test WorkQueueManagerTest ClearTimeOutWorkStatus.
 * @tc.type: FUNC
 * @tc.require: https://gitee.com/openharmony/resourceschedule_work_scheduler/issues/IC8IR5
 */
HWTEST_F(WorkQueueManagerTest, ClearTimeOutWorkStatus_001, TestSize.Level1)
{
    workQueueManager_->queueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workQueueManager_->AddWork(workStatus);
    workQueueManager_->ClearTimeOutWorkStatus();
    EXPECT_EQ(workQueueManager_->queueMap_.size(), 2);
}

/**
 * @tc.name: ClearTimeOutWorkStatus_002
 * @tc.desc: Test WorkQueueManagerTest ClearTimeOutWorkStatus.
 * @tc.type: FUNC
 * @tc.require: https://gitee.com/openharmony/resourceschedule_work_scheduler/issues/IC8IR5
 */
HWTEST_F(WorkQueueManagerTest, ClearTimeOutWorkStatus_002, TestSize.Level1)
{
    workQueueManager_->queueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    workinfo.RequestDeepIdle(WorkCondition::DeepIdle::DEEP_IDLE_OUT);
    workinfo.RequestBatteryLevel(80);
    int32_t uid = 10000;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->SetTimeout(true);
    workQueueManager_->AddWork(workStatus);
    workQueueManager_->ClearTimeOutWorkStatus();
    EXPECT_EQ(workQueueManager_->queueMap_.size(), 3);
}
}
}