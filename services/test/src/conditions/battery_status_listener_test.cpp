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

#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
#include <functional>
#include <gtest/gtest.h>

#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"
#include "conditions/battery_status_listener.h"
#include "work_queue_manager.h"
#include "work_scheduler_service.h"
#include "work_sched_hilog.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class TestBatteryStatusListener : public BatteryStatusListener {
public:
    TestBatteryStatusListener() : BatteryStatusListener(nullptr) {}
    ~TestBatteryStatusListener() override {}
    void OnConditionChanged(WorkCondition::Type conditionType,
        std::shared_ptr<DetectorValue> conditionVal) override
    {
        callCount_++;
        lastType_ = conditionType;
        lastVal_ = conditionVal;
    }
    void Reset()
    {
        callCount_ = 0;
        lastType_ = WorkCondition::Type::UNKNOWN;
        lastVal_ = nullptr;
    }
    int32_t callCount_ = 0;
    WorkCondition::Type lastType_ = WorkCondition::Type::UNKNOWN;
    std::shared_ptr<DetectorValue> lastVal_ = nullptr;
};

class BatteryStatusListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<TestBatteryStatusListener> listener_;
    static std::shared_ptr<BatteryStatusEventSubscriber> subscriber_;
};

std::shared_ptr<TestBatteryStatusListener> BatteryStatusListenerTest::listener_ = nullptr;
std::shared_ptr<BatteryStatusEventSubscriber> BatteryStatusListenerTest::subscriber_ = nullptr;

void BatteryStatusListenerTest::SetUpTestCase()
{
    listener_ = std::make_shared<TestBatteryStatusListener>();
    EventFwk::MatchingSkills skill;
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW);
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY);
    EventFwk::CommonEventSubscribeInfo info(skill);
    subscriber_ = std::make_shared<BatteryStatusEventSubscriber>(info, *listener_);
}

static EventFwk::CommonEventData BuildBatteryStatusEvent(const std::string &action)
{
    EventFwk::Want want;
    want.SetAction(action);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    return data;
}

/**
 * @tc.name: OnReceiveEvent_001
 * @tc.desc: Test BatteryStatusEventSubscriber OnReceiveEvent with BATTERY_LOW action.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryStatusListenerTest, OnReceiveEvent_001, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildBatteryStatusEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 1);
    EXPECT_EQ(listener_->lastType_, WorkCondition::Type::BATTERY_STATUS);
    EXPECT_EQ(listener_->lastVal_->intVal, WorkCondition::BATTERY_STATUS_LOW);
}

/**
 * @tc.name: OnReceiveEvent_002
 * @tc.desc: Test BatteryStatusEventSubscriber OnReceiveEvent with BATTERY_OKAY action.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryStatusListenerTest, OnReceiveEvent_002, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildBatteryStatusEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 1);
    EXPECT_EQ(listener_->lastType_, WorkCondition::Type::BATTERY_STATUS);
    EXPECT_EQ(listener_->lastVal_->intVal, WorkCondition::BATTERY_STATUS_OKAY);
}

/**
 * @tc.name: OnReceiveEvent_003
 * @tc.desc: Test BatteryStatusEventSubscriber OnReceiveEvent with invalid action.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryStatusListenerTest, OnReceiveEvent_003, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildBatteryStatusEvent("invalid.action");
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 0);
}

/**
 * @tc.name: OnReceiveEvent_004
 * @tc.desc: Test BatteryStatusEventSubscriber OnReceiveEvent LOW then OKAY sequence.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryStatusListenerTest, OnReceiveEvent_004, TestSize.Level1)
{
    listener_->Reset();
    subscriber_->OnReceiveEvent(BuildBatteryStatusEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW));
    EXPECT_EQ(listener_->lastVal_->intVal, WorkCondition::BATTERY_STATUS_LOW);
    subscriber_->OnReceiveEvent(BuildBatteryStatusEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY));
    EXPECT_EQ(listener_->callCount_, 2);
    EXPECT_EQ(listener_->lastVal_->intVal, WorkCondition::BATTERY_STATUS_OKAY);
}
} // namespace WorkScheduler
} // namespace OHOS
#endif // POWERMGR_BATTERY_MANAGER_ENABLE
