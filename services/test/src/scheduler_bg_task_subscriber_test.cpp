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

#include "scheduler_bg_task_subscriber.h"
#include "work_scheduler_service.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class SchedulerBgTaskSubscriberTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<SchedulerBgTaskSubscriber> schedulerBgTaskSubscriber_;
};

std::shared_ptr<SchedulerBgTaskSubscriber> SchedulerBgTaskSubscriberTest::schedulerBgTaskSubscriber_ = nullptr;

void SchedulerBgTaskSubscriberTest::SetUpTestCase()
{
    schedulerBgTaskSubscriber_ = std::make_shared<SchedulerBgTaskSubscriber>();
}

/**
 * @tc.name: OnProcEfficiencyResourcesApply_001
 * @tc.desc: Test OnProcEfficiencyResourcesApply with null resourceInfo returns early.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(SchedulerBgTaskSubscriberTest, OnProcEfficiencyResourcesApply_001, TestSize.Level1)
{
    auto before = DelayedSingleton<WorkSchedulerService>::GetInstance()->effiResApplyUidSet_.size();
    schedulerBgTaskSubscriber_->OnProcEfficiencyResourcesApply(nullptr);
    EXPECT_EQ(DelayedSingleton<WorkSchedulerService>::GetInstance()->effiResApplyUidSet_.size(), before);
}

/**
 * @tc.name: OnProcEfficiencyResourcesReset_001
 * @tc.desc: Test OnProcEfficiencyResourcesReset with null resourceInfo returns early.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(SchedulerBgTaskSubscriberTest, OnProcEfficiencyResourcesReset_001, TestSize.Level1)
{
    auto before = DelayedSingleton<WorkSchedulerService>::GetInstance()->effiResApplyUidSet_.size();
    schedulerBgTaskSubscriber_->OnProcEfficiencyResourcesReset(nullptr);
    EXPECT_EQ(DelayedSingleton<WorkSchedulerService>::GetInstance()->effiResApplyUidSet_.size(), before);
}

/**
 * @tc.name: OnAppEfficiencyResourcesApply_001
 * @tc.desc: Test OnAppEfficiencyResourcesApply with null resourceInfo returns early.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(SchedulerBgTaskSubscriberTest, OnAppEfficiencyResourcesApply_001, TestSize.Level1)
{
    auto before = DelayedSingleton<WorkSchedulerService>::GetInstance()->effiResApplyUidSet_.size();
    schedulerBgTaskSubscriber_->OnAppEfficiencyResourcesApply(nullptr);
    EXPECT_EQ(DelayedSingleton<WorkSchedulerService>::GetInstance()->effiResApplyUidSet_.size(), before);
}

/**
 * @tc.name: OnAppEfficiencyResourcesReset_001
 * @tc.desc: Test OnAppEfficiencyResourcesReset with null resourceInfo returns early.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(SchedulerBgTaskSubscriberTest, OnAppEfficiencyResourcesReset_001, TestSize.Level1)
{
    auto before = DelayedSingleton<WorkSchedulerService>::GetInstance()->effiResApplyUidSet_.size();
    schedulerBgTaskSubscriber_->OnAppEfficiencyResourcesReset(nullptr);
    EXPECT_EQ(DelayedSingleton<WorkSchedulerService>::GetInstance()->effiResApplyUidSet_.size(), before);
}
}
}