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

#include "work_queue_event_handler.h"
#include "work_queue_manager.h"
#include "work_scheduler_service.h"
#include "work_sched_hilog.h"

using namespace OHOS::AppExecFwk;
using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

const std::string WORKSCHEDULER_SERVICE_NAME = "WorkSchedulerService";

class WorkQueueEventHandlerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<EventRunner> runner_;
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
};

std::shared_ptr<EventRunner> WorkQueueEventHandlerTest::runner_ = nullptr;
std::shared_ptr<WorkQueueManager> WorkQueueEventHandlerTest::workQueueManager_ = nullptr;

void WorkQueueEventHandlerTest::SetUpTestCase()
{
    runner_ = EventRunner::Create(WORKSCHEDULER_SERVICE_NAME, ThreadMode::FFRT);
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    workQueueManager_ = std::make_shared<WorkQueueManager>(service);
}

/**
 * @tc.name: ProcessEvent_GroupTick_NullManager_001
 * @tc.desc: Test WorkQueueEventHandler ProcessEvent GROUP_TICK with null manager returns early.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueEventHandlerTest, ProcessEvent_GroupTick_NullManager_001, TestSize.Level1)
{
    auto handler = std::make_shared<WorkQueueEventHandler>(runner_, nullptr);
    auto event = InnerEvent::Get(WorkQueueEventHandler::GROUP_TICK, 0);
    handler->ProcessEvent(event);
    EXPECT_EQ(handler->manager_.use_count(), 0);
}

/**
 * @tc.name: ProcessEvent_TimerTick_001
 * @tc.desc: Test WorkQueueEventHandler ProcessEvent TIMER_TICK falls to default and returns.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueEventHandlerTest, ProcessEvent_TimerTick_001, TestSize.Level1)
{
    auto handler = std::make_shared<WorkQueueEventHandler>(runner_, workQueueManager_);
    auto event = InnerEvent::Get(WorkQueueEventHandler::TIMER_TICK, 0);
    handler->ProcessEvent(event);
    EXPECT_EQ(workQueueManager_->queueMap_.size(), 0);
}

/**
 * @tc.name: ProcessEvent_UnknownEvent_001
 * @tc.desc: Test WorkQueueEventHandler ProcessEvent with unknown event returns early.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueEventHandlerTest, ProcessEvent_UnknownEvent_001, TestSize.Level1)
{
    auto handler = std::make_shared<WorkQueueEventHandler>(runner_, workQueueManager_);
    auto event = InnerEvent::Get(999, 0);
    handler->ProcessEvent(event);
    EXPECT_EQ(workQueueManager_->queueMap_.size(), 0);
}

/**
 * @tc.name: Constructor_001
 * @tc.desc: Test WorkQueueEventHandler constructor stores manager_.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueEventHandlerTest, Constructor_001, TestSize.Level1)
{
    auto handler = std::make_shared<WorkQueueEventHandler>(runner_, workQueueManager_);
    EXPECT_EQ(handler->manager_.get(), workQueueManager_.get());
}

/**
 * @tc.name: ProcessEvent_GroupTick_ValidManager_001
 * @tc.desc: Test WorkQueueEventHandler ProcessEvent GROUP_TICK with valid manager does not crash.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkQueueEventHandlerTest, ProcessEvent_GroupTick_ValidManager_001, TestSize.Level1)
{
    auto handler = std::make_shared<WorkQueueEventHandler>(runner_, workQueueManager_);
    auto event = InnerEvent::Get(WorkQueueEventHandler::GROUP_TICK, 0);
    handler->ProcessEvent(event);
    EXPECT_EQ(handler->manager_.get(), workQueueManager_.get());
}
} // namespace WorkScheduler
} // namespace OHOS
