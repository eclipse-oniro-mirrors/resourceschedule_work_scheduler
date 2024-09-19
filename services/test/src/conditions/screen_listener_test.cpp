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

#include <functional>
#include <gtest/gtest.h>

#include "common_event_manager.h"
#include "common_event_support.h"
#include "conditions/screen_listener.h"
#include "work_queue_manager.h"
#include "work_scheduler_service.h"


using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class ScreenListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
    static std::shared_ptr<ScreenListener> screenListener_;
};

std::shared_ptr<ScreenListener> ScreenListenerTest::screenListener_ = nullptr;
std::shared_ptr<WorkQueueManager> ScreenListenerTest::workQueueManager_ = nullptr;

void ScreenListenerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workSchedulerService_->OnStart();
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
    screenListener_ = std::make_shared<ScreenListener>(workQueueManager_, workSchedulerService_);
}

/**
 * @tc.name: OnConditionChanged_001
 * @tc.desc: Test ScreenListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ScreenListenerTest, OnConditionChanged_001, TestSize.Level1)
{
    screenListener_->Start();
    EventFwk::CommonEventData data;
    screenListener_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = screenListener_->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_002
 * @tc.desc: Test ScreenListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ScreenListenerTest, OnConditionChanged_002, TestSize.Level1)
{
    screenListener_->Start();
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
    data.SetWant(want);
    screenListener_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = screenListener_->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_003
 * @tc.desc: Test ScreenListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ScreenListenerTest, OnConditionChanged_003, TestSize.Level1)
{
    screenListener_->Start();
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    data.SetWant(want);
    screenListener_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = screenListener_->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_004
 * @tc.desc: Test ScreenListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ScreenListenerTest, OnConditionChanged_004, TestSize.Level1)
{
    screenListener_->Start();
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    data.SetWant(want);
    screenListener_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = screenListener_->Stop();
    EXPECT_TRUE(ret);
}
}
}
