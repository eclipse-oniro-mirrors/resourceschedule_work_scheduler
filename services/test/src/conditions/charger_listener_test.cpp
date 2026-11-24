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
#include "conditions/charger_listener.h"
#include "work_queue_manager.h"
#include "work_scheduler_service.h"
#include "work_sched_hilog.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class TestChargerListener : public ChargerListener {
public:
    TestChargerListener() : ChargerListener(nullptr) {}
    ~TestChargerListener() override {}
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

class ChargerListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<TestChargerListener> listener_;
    static std::shared_ptr<ChargerEventSubscriber> subscriber_;
};

std::shared_ptr<TestChargerListener> ChargerListenerTest::listener_ = nullptr;
std::shared_ptr<ChargerEventSubscriber> ChargerListenerTest::subscriber_ = nullptr;

void ChargerListenerTest::SetUpTestCase()
{
    listener_ = std::make_shared<TestChargerListener>();
    EventFwk::MatchingSkills skills;
    skills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    skills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    EventFwk::CommonEventSubscribeInfo info(skills);
    subscriber_ = std::make_shared<ChargerEventSubscriber>(info, *listener_);
}

static EventFwk::CommonEventData BuildChargerEvent(const std::string &action, int32_t code)
{
    EventFwk::Want want;
    want.SetAction(action);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(code);
    return data;
}

/**
 * @tc.name: OnReceiveEvent_AC_001
 * @tc.desc: Test ChargerEventSubscriber OnReceiveEvent POWER_CONNECTED AC.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ChargerListenerTest, OnReceiveEvent_AC_001, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildChargerEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED,
        static_cast<int32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_AC));
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 1);
    EXPECT_EQ(listener_->lastType_, WorkCondition::Type::CHARGER);
    EXPECT_EQ(listener_->lastVal_->intVal, WorkCondition::CHARGING_PLUGGED_AC);
    EXPECT_EQ(listener_->lastVal_->boolVal, true);
}

/**
 * @tc.name: OnReceiveEvent_USB_001
 * @tc.desc: Test ChargerEventSubscriber OnReceiveEvent POWER_CONNECTED USB.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ChargerListenerTest, OnReceiveEvent_USB_001, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildChargerEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED,
        static_cast<int32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB));
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 1);
    EXPECT_EQ(listener_->lastVal_->intVal, WorkCondition::CHARGING_PLUGGED_USB);
    EXPECT_EQ(listener_->lastVal_->boolVal, true);
}

/**
 * @tc.name: OnReceiveEvent_WIRELESS_001
 * @tc.desc: Test ChargerEventSubscriber OnReceiveEvent POWER_CONNECTED WIRELESS.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ChargerListenerTest, OnReceiveEvent_WIRELESS_001, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildChargerEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED,
        static_cast<int32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS));
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 1);
    EXPECT_EQ(listener_->lastVal_->intVal, WorkCondition::CHARGING_PLUGGED_WIRELESS);
    EXPECT_EQ(listener_->lastVal_->boolVal, true);
}

/**
 * @tc.name: OnReceiveEvent_Connected_InvalidType_001
 * @tc.desc: Test ChargerEventSubscriber OnReceiveEvent POWER_CONNECTED invalid type.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ChargerListenerTest, OnReceiveEvent_Connected_InvalidType_001, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildChargerEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED, 999);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 0);
}

/**
 * @tc.name: OnReceiveEvent_Disconnected_NONE_001
 * @tc.desc: Test ChargerEventSubscriber OnReceiveEvent POWER_DISCONNECTED NONE.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ChargerListenerTest, OnReceiveEvent_Disconnected_NONE_001, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildChargerEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED,
        static_cast<int32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_NONE));
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 1);
    EXPECT_EQ(listener_->lastVal_->intVal, WorkCondition::CHARGING_UNPLUGGED);
    EXPECT_EQ(listener_->lastVal_->boolVal, false);
}

/**
 * @tc.name: OnReceiveEvent_Disconnected_BUTT_001
 * @tc.desc: Test ChargerEventSubscriber OnReceiveEvent POWER_DISCONNECTED BUTT.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ChargerListenerTest, OnReceiveEvent_Disconnected_BUTT_001, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildChargerEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED,
        static_cast<int32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_BUTT));
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 1);
    EXPECT_EQ(listener_->lastVal_->intVal, WorkCondition::CHARGING_UNPLUGGED);
    EXPECT_EQ(listener_->lastVal_->boolVal, false);
}

/**
 * @tc.name: OnReceiveEvent_Disconnected_InvalidType_001
 * @tc.desc: Test ChargerEventSubscriber OnReceiveEvent POWER_DISCONNECTED invalid type.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ChargerListenerTest, OnReceiveEvent_Disconnected_InvalidType_001, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildChargerEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED, 999);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 0);
}

/**
 * @tc.name: OnReceiveEvent_InvalidAction_001
 * @tc.desc: Test ChargerEventSubscriber OnReceiveEvent with invalid action.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ChargerListenerTest, OnReceiveEvent_InvalidAction_001, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildChargerEvent("invalid.action", 0);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 0);
}
} // namespace WorkScheduler
} // namespace OHOS
#endif // POWERMGR_BATTERY_MANAGER_ENABLE
