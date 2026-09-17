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
#include "work_sched_hilog.h"
#include "work_sched_constants.h"

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
    static std::shared_ptr<WorkSchedulerService> workSchedulerService_;
};

std::shared_ptr<ScreenListener> ScreenListenerTest::screenListener_ = nullptr;
std::shared_ptr<WorkQueueManager> ScreenListenerTest::workQueueManager_ = nullptr;
std::shared_ptr<WorkSchedulerService> ScreenListenerTest::workSchedulerService_ = nullptr;

void ScreenListenerTest::SetUpTestCase()
{
    workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
    screenListener_ = std::make_shared<ScreenListener>(workQueueManager_, workSchedulerService_);
}

/**
 * @tc.name: Start_001
 * @tc.desc: Test ScreenListener Start populates saIdTimeInfoMap_ with DEFAULT_SA_ID.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ScreenListenerTest, Start_001, TestSize.Level1)
{
    auto listener = std::make_shared<ScreenListener>(workQueueManager_, workSchedulerService_);
    listener->saIdTimeInfoMap_.clear();
    listener->Start();
    EXPECT_EQ(listener->saIdTimeInfoMap_.count(0), 1);
    EXPECT_EQ(listener->saIdTimeInfoMap_.at(0).time_, MIN_DEEP_IDLE_SCREEN_OFF_TIME_MIN);
    listener->Stop();
}

/**
 * @tc.name: Start_002
 * @tc.desc: Test ScreenListener Start with null service_ leaves saIdTimeInfoMap_ empty.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ScreenListenerTest, Start_002, TestSize.Level1)
{
    auto listener = std::make_shared<ScreenListener>(workQueueManager_, nullptr);
    listener->saIdTimeInfoMap_.clear();
    listener->Start();
    EXPECT_EQ(listener->saIdTimeInfoMap_.size(), 0);
}

/**
 * @tc.name: Stop_001
 * @tc.desc: Test ScreenListener Stop with null subscriber clears saIdTimeInfoMap_.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ScreenListenerTest, Stop_001, TestSize.Level1)
{
    auto listener = std::make_shared<ScreenListener>(workQueueManager_, workSchedulerService_);
    listener->commonEventSubscriber = nullptr;
    listener->saIdTimeInfoMap_[100] = ScreenListener::SaTimerInfo(30000, 0);
    listener->Stop();
    EXPECT_EQ(listener->saIdTimeInfoMap_.size(), 0);
}
} // namespace WorkScheduler
} // namespace OHOS
