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

#include "conditions/timer_listener.h"
#include "work_scheduler_service.h"
#include "work_queue_manager.h"
#include "work_sched_hilog.h"

using namespace OHOS::AppExecFwk;
using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

const std::string WORKSCHEDULER_SERVICE_NAME = "WorkSchedulerService";

class TimerListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
    static std::shared_ptr<TimerListener> timerListener_;
};

std::shared_ptr<WorkQueueManager> TimerListenerTest::workQueueManager_ = nullptr;
std::shared_ptr<TimerListener> TimerListenerTest::timerListener_ = nullptr;

void TimerListenerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_ = AppExecFwk::EventRunner::Create(WORKSCHEDULER_SERVICE_NAME,
        AppExecFwk::ThreadMode::FFRT);
    timerListener_ = std::make_shared<TimerListener>(workQueueManager_, eventRunner_);
}

/**
 * @tc.name: Start_001
 * @tc.desc: Test TimerListener Start with null workQueueManager_ leaves timerId_ 0.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(TimerListenerTest, Start_001, TestSize.Level1)
{
    auto runner = AppExecFwk::EventRunner::Create("TestTimerRunner", AppExecFwk::ThreadMode::FFRT);
    auto listener = std::make_shared<TimerListener>(nullptr, runner);
    listener->Start();
    EXPECT_EQ(listener->timerId_, 0);
}

/**
 * @tc.name: Start_002
 * @tc.desc: Test TimerListener Start with valid workQueueManager_, verify timerId_.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(TimerListenerTest, Start_002, TestSize.Level1)
{
    auto runner = AppExecFwk::EventRunner::Create("TestTimerRunner2", AppExecFwk::ThreadMode::FFRT);
    auto listener = std::make_shared<TimerListener>(workQueueManager_, runner);
    bool result = listener->Start();
    if (result) {
        EXPECT_GT(listener->timerId_, 0);
        listener->Stop();
        EXPECT_EQ(listener->timerId_, 0);
    } else {
        EXPECT_EQ(listener->timerId_, 0);
    }
}

/**
 * @tc.name: Stop_001
 * @tc.desc: Test TimerListener Stop with timerId_ 0 does nothing.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(TimerListenerTest, Stop_001, TestSize.Level1)
{
    auto runner = AppExecFwk::EventRunner::Create("TestTimerRunner3", AppExecFwk::ThreadMode::FFRT);
    auto listener = std::make_shared<TimerListener>(workQueueManager_, runner);
    listener->timerId_ = 0;
    listener->Stop();
    EXPECT_EQ(listener->timerId_, 0);
}
} // namespace WorkScheduler
} // namespace OHOS
