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
 * @tc.name: OnRemoteDied_001
 * @tc.desc: Test SchedulerBgTaskSubscriber OnRemoteDied.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(SchedulerBgTaskSubscriberTest, OnRemoteDied_001, TestSize.Level1)
{
    schedulerBgTaskSubscriber_->OnRemoteDied(nullptr);
    EXPECT_TRUE(schedulerBgTaskSubscriber_ != nullptr);
}
}
}