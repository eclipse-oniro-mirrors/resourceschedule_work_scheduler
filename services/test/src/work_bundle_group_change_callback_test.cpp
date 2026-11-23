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

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
#include <functional>
#include <gtest/gtest.h>

#include "work_bundle_group_change_callback.h"
#include "work_queue_manager.h"
#include "work_scheduler_service.h"
#include "work_sched_data_manager.h"
#include "work_policy_manager.h"
#include "work_sched_hilog.h"
#include "app_group_callback_info.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class WorkBundleGroupChangeCallbackTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkSchedulerService> service_;
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
    static std::shared_ptr<WorkBundleGroupChangeCallback> callback_;
};

std::shared_ptr<WorkSchedulerService> WorkBundleGroupChangeCallbackTest::service_ = nullptr;
std::shared_ptr<WorkQueueManager> WorkBundleGroupChangeCallbackTest::workQueueManager_ = nullptr;
std::shared_ptr<WorkBundleGroupChangeCallback> WorkBundleGroupChangeCallbackTest::callback_ = nullptr;

void WorkBundleGroupChangeCallbackTest::SetUpTestCase()
{
    service_ = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (service_->handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create("WorkSchedulerService", AppExecFwk::ThreadMode::FFRT);
        service_->eventRunner_ = runner;
        service_->handler_ = std::make_shared<WorkEventHandler>(runner, service_);
    }
    if (service_->workPolicyManager_ == nullptr) {
        service_->workPolicyManager_ = std::make_shared<WorkPolicyManager>(service_);
    }
    if (service_->workQueueManager_ == nullptr) {
        service_->workQueueManager_ = std::make_shared<WorkQueueManager>(service_);
    }
    workQueueManager_ = service_->workQueueManager_;
    callback_ = std::make_shared<WorkBundleGroupChangeCallback>(workQueueManager_);
}

static DeviceUsageStats::AppGroupCallbackInfo BuildCallbackInfo(int32_t newGroup, int32_t oldGroup,
    int32_t userId, const std::string &bundleName)
{
    return DeviceUsageStats::AppGroupCallbackInfo(userId, oldGroup, newGroup, 0, bundleName);
}

/**
 * @tc.name: OnAppGroupChanged_GroupUpgrade_001
 * @tc.desc: Test OnAppGroupChanged with newGroup >= oldGroup (upgrade, no trigger).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkBundleGroupChangeCallbackTest, OnAppGroupChanged_GroupUpgrade_001, TestSize.Level1)
{
    DelayedSingleton<DataManager>::GetInstance()->ClearAllGroup();
    size_t sizeBefore = workQueueManager_->queueMap_.size();
    auto info = BuildCallbackInfo(50, 30, 100, "com.test.upgrade");
    ErrCode ret = callback_->OnAppGroupChanged(info);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(workQueueManager_->queueMap_.size(), sizeBefore);
}

/**
 * @tc.name: OnAppGroupChanged_GroupDowngrade_001
 * @tc.desc: Test OnAppGroupChanged with newGroup < oldGroup and no work found (no trigger).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkBundleGroupChangeCallbackTest, OnAppGroupChanged_GroupDowngrade_001, TestSize.Level1)
{
    DelayedSingleton<DataManager>::GetInstance()->ClearAllGroup();
    size_t sizeBefore = workQueueManager_->queueMap_.size();
    auto info = BuildCallbackInfo(10, 50, 101, "com.test.downgrade");
    ErrCode ret = callback_->OnAppGroupChanged(info);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(workQueueManager_->queueMap_.size(), sizeBefore);
}

/**
 * @tc.name: OnAppGroupChanged_SameGroup_001
 * @tc.desc: Test OnAppGroupChanged with newGroup == oldGroup (no change, no trigger).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkBundleGroupChangeCallbackTest, OnAppGroupChanged_SameGroup_001, TestSize.Level1)
{
    DelayedSingleton<DataManager>::GetInstance()->ClearAllGroup();
    size_t sizeBefore = workQueueManager_->queueMap_.size();
    auto info = BuildCallbackInfo(30, 30, 102, "com.test.same");
    ErrCode ret = callback_->OnAppGroupChanged(info);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(workQueueManager_->queueMap_.size(), sizeBefore);
}

/**
 * @tc.name: OnAppGroupChanged_AddGroup_001
 * @tc.desc: Test OnAppGroupChanged always calls DataManager AddGroup.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkBundleGroupChangeCallbackTest, OnAppGroupChanged_AddGroup_001, TestSize.Level1)
{
    DelayedSingleton<DataManager>::GetInstance()->ClearAllGroup();
    auto info = BuildCallbackInfo(40, 20, 103, "com.test.addgroup");
    callback_->OnAppGroupChanged(info);
    int32_t appGroup = 0;
    EXPECT_EQ(DelayedSingleton<DataManager>::GetInstance()->FindGroup("com.test.addgroup", 103, appGroup), true);
    EXPECT_EQ(appGroup, 40);
}

/**
 * @tc.name: Constructor_001
 * @tc.desc: Test WorkBundleGroupChangeCallback constructor stores workQueueManager_.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkBundleGroupChangeCallbackTest, Constructor_001, TestSize.Level1)
{
    auto cb = std::make_shared<WorkBundleGroupChangeCallback>(workQueueManager_);
    EXPECT_EQ(cb->workQueueManager_.get(), workQueueManager_.get());
}

/**
 * @tc.name: Constructor_NullManager_001
 * @tc.desc: Test WorkBundleGroupChangeCallback constructor with null manager.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkBundleGroupChangeCallbackTest, Constructor_NullManager_001, TestSize.Level1)
{
    auto cb = std::make_shared<WorkBundleGroupChangeCallback>(nullptr);
    auto info = BuildCallbackInfo(10, 50, 104, "com.test.nullmgr");
    ErrCode ret = cb->OnAppGroupChanged(info);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(cb->workQueueManager_.use_count(), 0);
}
} // namespace WorkScheduler
} // namespace OHOS
#endif // DEVICE_USAGE_STATISTICS_ENABLE
