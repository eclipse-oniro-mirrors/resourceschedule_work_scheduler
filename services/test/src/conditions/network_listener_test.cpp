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

#include "conditions/network_listener.h"
#include "work_scheduler_service.h"
#include "work_queue_manager.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"
#include "work_sched_hilog.h"
#ifdef COMMUNICATION_NETMANAGER_BASE_ENABLE
#include "net_supplier_info.h"
#endif

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
const int32_t DEFAULT_VALUE = -1;
const int32_t BEARER_CELLULAR = 0;
const int32_t BEARER_WIFI = 1;
const int32_t BEARER_BLUETOOTH = 2;
const int32_t BEARER_ETHERNET = 3;
const int32_t BEARER_WIFI_AWARE = 5;

class TestNetworkListener : public NetworkListener {
public:
    TestNetworkListener() : NetworkListener(nullptr) {}
    ~TestNetworkListener() override {}
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

class NetworkListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
    static std::shared_ptr<NetworkListener> networkListener_;
    static std::shared_ptr<TestNetworkListener> testListener_;
    static std::shared_ptr<NetworkEventSubscriber> subscriber_;
};

std::shared_ptr<WorkQueueManager> NetworkListenerTest::workQueueManager_ = nullptr;
std::shared_ptr<NetworkListener> NetworkListenerTest::networkListener_ = nullptr;
std::shared_ptr<TestNetworkListener> NetworkListenerTest::testListener_ = nullptr;
std::shared_ptr<NetworkEventSubscriber> NetworkListenerTest::subscriber_ = nullptr;

void NetworkListenerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
    networkListener_ = std::make_shared<NetworkListener>(workQueueManager_);

    testListener_ = std::make_shared<TestNetworkListener>();
    EventFwk::MatchingSkills skill;
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
    EventFwk::CommonEventSubscribeInfo info(skill);
    subscriber_ = std::make_shared<NetworkEventSubscriber>(info, *testListener_);
}

#ifdef COMMUNICATION_NETMANAGER_BASE_ENABLE
static EventFwk::CommonEventData BuildNetworkEvent(int32_t code, int32_t netType)
{
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
    want.SetParam("NetType", netType);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(code);
    return data;
}

/**
 * @tc.name: OnReceiveEvent_Connected_Cellular_001
 * @tc.desc: Test NetworkEventSubscriber OnReceiveEvent CONNECTED + CELLULAR.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(NetworkListenerTest, OnReceiveEvent_Connected_Cellular_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildNetworkEvent(
        static_cast<int32_t>(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED), BEARER_CELLULAR);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastVal_->intVal, WorkCondition::NETWORK_TYPE_MOBILE);
}

/**
 * @tc.name: OnReceiveEvent_Connected_Wifi_001
 * @tc.desc: Test NetworkEventSubscriber OnReceiveEvent CONNECTED + WIFI.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(NetworkListenerTest, OnReceiveEvent_Connected_Wifi_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildNetworkEvent(
        static_cast<int32_t>(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED), BEARER_WIFI);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastVal_->intVal, WorkCondition::NETWORK_TYPE_WIFI);
}

/**
 * @tc.name: OnReceiveEvent_Connected_Bluetooth_001
 * @tc.desc: Test NetworkEventSubscriber OnReceiveEvent CONNECTED + BLUETOOTH.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(NetworkListenerTest, OnReceiveEvent_Connected_Bluetooth_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildNetworkEvent(
        static_cast<int32_t>(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED), BEARER_BLUETOOTH);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastVal_->intVal, WorkCondition::NETWORK_TYPE_BLUETOOTH);
}

/**
 * @tc.name: OnReceiveEvent_Connected_Ethernet_001
 * @tc.desc: Test NetworkEventSubscriber OnReceiveEvent CONNECTED + ETHERNET.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(NetworkListenerTest, OnReceiveEvent_Connected_Ethernet_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildNetworkEvent(
        static_cast<int32_t>(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED), BEARER_ETHERNET);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastVal_->intVal, WorkCondition::NETWORK_TYPE_ETHERNET);
}

/**
 * @tc.name: OnReceiveEvent_Connected_WifiAware_001
 * @tc.desc: Test NetworkEventSubscriber OnReceiveEvent CONNECTED + WIFI_AWARE.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(NetworkListenerTest, OnReceiveEvent_Connected_WifiAware_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildNetworkEvent(
        static_cast<int32_t>(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED), BEARER_WIFI_AWARE);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastVal_->intVal, WorkCondition::NETWORK_TYPE_WIFI_P2P);
}

/**
 * @tc.name: OnReceiveEvent_Connected_UnknownNetType_001
 * @tc.desc: Test NetworkEventSubscriber OnReceiveEvent CONNECTED + unknown netType.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(NetworkListenerTest, OnReceiveEvent_Connected_UnknownNetType_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildNetworkEvent(
        static_cast<int32_t>(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED), 999);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastVal_->intVal, WorkCondition::NETWORK_TYPE_ANY);
}

/**
 * @tc.name: OnReceiveEvent_Connected_DefaultNetType_001
 * @tc.desc: Test NetworkEventSubscriber OnReceiveEvent CONNECTED + DEFAULT_VALUE (no trigger).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(NetworkListenerTest, OnReceiveEvent_Connected_DefaultNetType_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildNetworkEvent(
        static_cast<int32_t>(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED), DEFAULT_VALUE);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 0);
}

/**
 * @tc.name: OnReceiveEvent_Disconnected_001
 * @tc.desc: Test NetworkEventSubscriber OnReceiveEvent DISCONNECTED triggers NETWORK_UNKNOWN.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(NetworkListenerTest, OnReceiveEvent_Disconnected_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildNetworkEvent(
        static_cast<int32_t>(NetManagerStandard::NetConnState::NET_CONN_STATE_DISCONNECTED), BEARER_WIFI);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastVal_->intVal, WorkCondition::NETWORK_UNKNOWN);
}
#endif // COMMUNICATION_NETMANAGER_BASE_ENABLE
} // namespace WorkScheduler
} // namespace OHOS
