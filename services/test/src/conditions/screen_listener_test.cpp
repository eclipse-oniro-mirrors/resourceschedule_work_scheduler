/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "matching_skills.h"
#include "want.h"
#include "conditions/screen_listener.h"
#include "work_queue_manager.h"
#include "work_scheduler_service.h"
#include "work_sched_hilog.h"
#include "work_sched_constants.h"
#include "work_event_handler.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class TestScreenListener : public ScreenListener {
public:
    explicit TestScreenListener(std::shared_ptr<WorkSchedulerService> service)
        : ScreenListener(nullptr, service) {}
    void OnConditionChanged(WorkCondition::Type conditionType,
        std::shared_ptr<DetectorValue> conditionVal) override
    {
        callCount_++;
        lastType_ = conditionType;
    }
    void Reset()
    {
        callCount_ = 0;
        lastType_ = WorkCondition::Type::UNKNOWN;
    }
    int32_t callCount_ = 0;
    WorkCondition::Type lastType_ = WorkCondition::Type::UNKNOWN;
};

class ScreenListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
    static std::shared_ptr<ScreenListener> screenListener_;
    static std::shared_ptr<WorkSchedulerService> workSchedulerService_;
    static std::shared_ptr<TestScreenListener> testListener_;
    static std::shared_ptr<ScreenEventSubscriber> subscriber_;
};

std::shared_ptr<ScreenListener> ScreenListenerTest::screenListener_ = nullptr;
std::shared_ptr<WorkQueueManager> ScreenListenerTest::workQueueManager_ = nullptr;
std::shared_ptr<WorkSchedulerService> ScreenListenerTest::workSchedulerService_ = nullptr;
std::shared_ptr<TestScreenListener> ScreenListenerTest::testListener_ = nullptr;
std::shared_ptr<ScreenEventSubscriber> ScreenListenerTest::subscriber_ = nullptr;

void ScreenListenerTest::SetUpTestCase()
{
    workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    if (workSchedulerService_->handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create("WorkSchedulerService", AppExecFwk::ThreadMode::FFRT);
        workSchedulerService_->eventRunner_ = runner;
        workSchedulerService_->handler_ = std::make_shared<WorkEventHandler>(runner, workSchedulerService_);
    }
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
    screenListener_ = std::make_shared<ScreenListener>(workQueueManager_, workSchedulerService_);

    testListener_ = std::make_shared<TestScreenListener>(workSchedulerService_);
    EventFwk::MatchingSkills skills;
    skills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
#ifdef CAR_PLATFORM
    skills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
#endif
    skills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventSubscribeInfo info(skills);
    subscriber_ = std::make_shared<ScreenEventSubscriber>(info, *testListener_);
}

static EventFwk::CommonEventData BuildScreenEvent(const std::string &action)
{
    EventFwk::Want want;
    want.SetAction(action);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    return data;
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

/**
 * @tc.name: OnReceiveEvent_ScreenUnlocked_001
 * @tc.desc: Test ScreenEventSubscriber OnReceiveEvent SCREEN_UNLOCKED triggers DEEP_IDLE callback.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ScreenListenerTest, OnReceiveEvent_ScreenUnlocked_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildScreenEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastType_, WorkCondition::Type::DEEP_IDLE);
}

/**
 * @tc.name: OnReceiveEvent_ScreenOn_001
 * @tc.desc: Test ScreenEventSubscriber OnReceiveEvent SCREEN_ON behavior depends on CAR_PLATFORM.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ScreenListenerTest, OnReceiveEvent_ScreenOn_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildScreenEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    subscriber_->OnReceiveEvent(data);
#ifdef CAR_PLATFORM
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastType_, WorkCondition::Type::DEEP_IDLE);
#else
    EXPECT_EQ(testListener_->callCount_, 0);
#endif
}

/**
 * @tc.name: OnReceiveEvent_ScreenOff_001
 * @tc.desc: Test ScreenEventSubscriber OnReceiveEvent SCREEN_OFF does not trigger DEEP_IDLE callback.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ScreenListenerTest, OnReceiveEvent_ScreenOff_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildScreenEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 0);
}

/**
 * @tc.name: OnReceiveEvent_InvalidAction_001
 * @tc.desc: Test ScreenEventSubscriber OnReceiveEvent with invalid action does nothing.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ScreenListenerTest, OnReceiveEvent_InvalidAction_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildScreenEvent("invalid.action");
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 0);
}
} // namespace WorkScheduler
} // namespace OHOS
