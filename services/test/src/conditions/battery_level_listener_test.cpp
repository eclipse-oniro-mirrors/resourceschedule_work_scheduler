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

#include "battery_info.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"
#include "conditions/battery_level_listener.h"
#include "work_queue_manager.h"
#include "work_scheduler_service.h"
#include "work_sched_hilog.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class TestBatteryLevelListener : public BatteryLevelListener {
public:
    TestBatteryLevelListener() : BatteryLevelListener(nullptr, nullptr) {}
    ~TestBatteryLevelListener() override {}
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

class BatteryLevelListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<TestBatteryLevelListener> listener_;
    static std::shared_ptr<BatteryLevelEventSubscriber> subscriber_;
};

std::shared_ptr<TestBatteryLevelListener> BatteryLevelListenerTest::listener_ = nullptr;
std::shared_ptr<BatteryLevelEventSubscriber> BatteryLevelListenerTest::subscriber_ = nullptr;

void BatteryLevelListenerTest::SetUpTestCase()
{
    listener_ = std::make_shared<TestBatteryLevelListener>();
    EventFwk::MatchingSkills skill;
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    EventFwk::CommonEventSubscribeInfo info(skill);
    subscriber_ = std::make_shared<BatteryLevelEventSubscriber>(info, *listener_);
}

static EventFwk::CommonEventData BuildBatteryChangedEvent(int32_t capacity)
{
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam(PowerMgr::BatteryInfo::COMMON_EVENT_KEY_CAPACITY, capacity);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    return data;
}

/**
 * @tc.name: OnReceiveEvent_001
 * @tc.desc: Test BatteryLevelEventSubscriber OnReceiveEvent with valid capacity.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnReceiveEvent_001, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildBatteryChangedEvent(50);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 1);
    EXPECT_EQ(listener_->lastType_, WorkCondition::Type::BATTERY_LEVEL);
    EXPECT_EQ(listener_->lastVal_->intVal, 50);
}

/**
 * @tc.name: OnReceiveEvent_002
 * @tc.desc: Test BatteryLevelEventSubscriber OnReceiveEvent with invalid capacity (-1).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnReceiveEvent_002, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildBatteryChangedEvent(-1);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 0);
}

/**
 * @tc.name: OnReceiveEvent_003
 * @tc.desc: Test BatteryLevelEventSubscriber OnReceiveEvent with same capacity (dedup).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnReceiveEvent_003, TestSize.Level1)
{
    listener_->Reset();
    subscriber_->OnReceiveEvent(BuildBatteryChangedEvent(80));
    EXPECT_EQ(listener_->callCount_, 1);
    subscriber_->OnReceiveEvent(BuildBatteryChangedEvent(80));
    EXPECT_EQ(listener_->callCount_, 1);
}

/**
 * @tc.name: OnReceiveEvent_004
 * @tc.desc: Test BatteryLevelEventSubscriber OnReceiveEvent with different capacity.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnReceiveEvent_004, TestSize.Level1)
{
    listener_->Reset();
    subscriber_->OnReceiveEvent(BuildBatteryChangedEvent(30));
    EXPECT_EQ(listener_->lastVal_->intVal, 30);
    subscriber_->OnReceiveEvent(BuildBatteryChangedEvent(90));
    EXPECT_EQ(listener_->callCount_, 2);
    EXPECT_EQ(listener_->lastVal_->intVal, 90);
}

/**
 * @tc.name: OnReceiveEvent_005
 * @tc.desc: Test BatteryLevelEventSubscriber OnReceiveEvent with invalid action.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnReceiveEvent_005, TestSize.Level1)
{
    listener_->Reset();
    EventFwk::Want want;
    want.SetAction("invalid.action");
    EventFwk::CommonEventData data;
    data.SetWant(want);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 0);
}

/**
 * @tc.name: OnReceiveEvent_006
 * @tc.desc: Test BatteryLevelEventSubscriber OnReceiveEvent capacity transition -1 to valid.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnReceiveEvent_006, TestSize.Level1)
{
    listener_->Reset();
    subscriber_->OnReceiveEvent(BuildBatteryChangedEvent(-1));
    EXPECT_EQ(listener_->callCount_, 0);
    subscriber_->OnReceiveEvent(BuildBatteryChangedEvent(60));
    EXPECT_EQ(listener_->callCount_, 1);
    EXPECT_EQ(listener_->lastVal_->intVal, 60);
}
} // namespace WorkScheduler
} // namespace OHOS
#endif // POWERMGR_BATTERY_MANAGER_ENABLE
