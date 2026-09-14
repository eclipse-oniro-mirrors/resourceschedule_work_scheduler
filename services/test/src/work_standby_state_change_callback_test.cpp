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
#include "gmock/gmock.h"

#define private public
#include "work_queue_manager.h"
#include "work_scheduler_service.h"
#include "work_standby_state_change_callback.h"
#include "work_sched_data_manager.h"
#include "work_sched_utils.h"
#include "work_policy_manager.h"
#include "work_info.h"
#include "work_status.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class WorkStandbyStateChangeCallbackTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkStandbyStateChangeCallback> callback_;
    static std::shared_ptr<DataManager> dataManager_;
};

std::shared_ptr<WorkStandbyStateChangeCallback> WorkStandbyStateChangeCallbackTest::callback_ = nullptr;
std::shared_ptr<DataManager> WorkStandbyStateChangeCallbackTest::dataManager_ = nullptr;

void WorkStandbyStateChangeCallbackTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    std::shared_ptr<WorkQueueManager> workQueueManger_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
    callback_ = std::make_shared<WorkStandbyStateChangeCallback>(workQueueManger_);

    dataManager_ = DelayedSingleton<DataManager>::GetInstance();
}

/**
 * @tc.name: OnDeviceIdleMode_002
 * @tc.desc: Test WorkStandbyStateChangeCallbackTest OnDeviceIdleMode.
 * @tc.type: FUNC
 * @tc.require: https://gitee.com/openharmony/resourceschedule_work_scheduler/issues/ICB1SP
 */
HWTEST_F(WorkStandbyStateChangeCallbackTest, OnDeviceIdleMode_002, TestSize.Level1)
{
    dataManager_->SetDeviceSleep(false);
    callback_->OnDeviceIdleMode(false, true);
    EXPECT_TRUE(dataManager_->GetDeviceSleep());
}

/**
 * @tc.name: OnDeviceIdleMode_004
 * @tc.desc: Test WorkStandbyStateChangeCallbackTest OnDeviceIdleMode.
 * @tc.type: FUNC
 * @tc.require: https://gitee.com/openharmony/resourceschedule_work_scheduler/issues/ICB1SP
 */
HWTEST_F(WorkStandbyStateChangeCallbackTest, OnDeviceIdleMode_004, TestSize.Level1)
{
    dataManager_->SetDeviceSleep(false);
    callback_->OnDeviceIdleMode(true, true);
    EXPECT_TRUE(dataManager_->GetDeviceSleep());
}

/**
 * @tc.name: OnDeviceIdleMode_005
 * @tc.desc: Test OnDeviceIdleMode with napped=true && !sleeping returns early (nap state).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkStandbyStateChangeCallbackTest, OnDeviceIdleMode_005, TestSize.Level1)
{
    dataManager_->SetDeviceSleep(false);
    callback_->OnDeviceIdleMode(true, false);
    EXPECT_FALSE(dataManager_->GetDeviceSleep());
}

/**
 * @tc.name: OnDeviceIdleMode_006
 * @tc.desc: Test OnDeviceIdleMode with same state returns early (no change).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkStandbyStateChangeCallbackTest, OnDeviceIdleMode_006, TestSize.Level1)
{
    dataManager_->SetDeviceSleep(true);
    callback_->OnDeviceIdleMode(false, true);
    EXPECT_TRUE(dataManager_->GetDeviceSleep());
}

/**
 * @tc.name: OnDeviceIdleMode_007
 * @tc.desc: Test OnDeviceIdleMode exit sleeping state (0,0).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkStandbyStateChangeCallbackTest, OnDeviceIdleMode_007, TestSize.Level1)
{
    dataManager_->SetDeviceSleep(true);
    callback_->OnDeviceIdleMode(false, false);
    EXPECT_FALSE(dataManager_->GetDeviceSleep());
}

/**
 * @tc.name: OnRestrictListChanged_001
 * @tc.desc: Test OnRestrictListChanged with wrong resourceType returns early.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkStandbyStateChangeCallbackTest, OnRestrictListChanged_001, TestSize.Level1)
{
    dataManager_->ClearDeviceStandyRestrictlist();
    callback_->OnRestrictListChanged(1000, "com.test.restrict", 999, true);
    EXPECT_FALSE(dataManager_->IsInDeviceStandyRestrictlist("com.test.restrict"));
}

/**
 * @tc.name: OnRestrictListChanged_002
 * @tc.desc: Test OnRestrictListChanged with correct type adds to restrictlist.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkStandbyStateChangeCallbackTest, OnRestrictListChanged_002, TestSize.Level1)
{
    dataManager_->ClearDeviceStandyRestrictlist();
    callback_->OnRestrictListChanged(1000, "com.test.restrict2", 4, true);
    EXPECT_TRUE(dataManager_->IsInDeviceStandyRestrictlist("com.test.restrict2"));
    dataManager_->ClearDeviceStandyRestrictlist();
}

/**
 * @tc.name: OnAllowListChanged_001
 * @tc.desc: Test OnAllowListChanged with wrong allowType returns early.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkStandbyStateChangeCallbackTest, OnAllowListChanged_001, TestSize.Level1)
{
    dataManager_->ClearDeviceStandyWhitelist();
    callback_->OnAllowListChanged(1000, "com.test.allow", 999, true);
    EXPECT_FALSE(dataManager_->IsInDeviceStandyWhitelist("com.test.allow"));
}

/**
 * @tc.name: OnAllowListChanged_002
 * @tc.desc: Test OnAllowListChanged with correct type but not sleeping returns early after whitelist update.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkStandbyStateChangeCallbackTest, OnAllowListChanged_002, TestSize.Level1)
{
    dataManager_->ClearDeviceStandyWhitelist();
    dataManager_->SetDeviceSleep(false);
    callback_->OnAllowListChanged(1000, "com.test.allow2", 4, true);
    EXPECT_TRUE(dataManager_->IsInDeviceStandyWhitelist("com.test.allow2"));
    dataManager_->ClearDeviceStandyWhitelist();
}
}
}