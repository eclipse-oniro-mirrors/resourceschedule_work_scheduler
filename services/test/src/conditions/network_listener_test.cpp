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
#include "net_supplier_info.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
const int32_t DEFAULT_VALUE = -1;
const int32_t BEARER_CELLULAR = 0;
const int32_t BEARER_WIFI = 1;
const int32_t BEARER_BLUETOOTH = 2;
const int32_t BEARER_ETHERNET = 3;
const int32_t BEARER_WIFI_AWARE = 5;

class NetworkListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkSchedulerService> workSchedulerService_;
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
    static std::shared_ptr<NetworkListenerTest> networkListenerTest_;
}

std::shared_ptr<WorkSchedulerService> NetworkListenerTest::workSchedulerService_ = nullptr;
std::shared_ptr<WorkQueueManager> NetworkListenerTest::workQueueManager_ = nullptr;
std::shared_ptr<NetworkListenerTest> NetworkListenerTest::networkListenerTest_ = nullptr;

void NetworkListenerTest::SetUpTestCase()
{
    workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
    networkListenerTest_ = std::make_shared<NetworkListener>(workQueueManager_, workSchedulerService_);
}

/**
 * @tc.name: OnConditionChanged_001
 * @tc.desc: Test etworkListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ScreenListenerTest, OnConditionChanged_001, TestSize.Level1)
{
    networkListenerTest_->Start();
    EventFwk::CommonEventData data;
    networkListenerTest_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = networkListenerTest_->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_002
 * @tc.desc: Test etworkListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ScreenListenerTest, OnConditionChanged_002, TestSize.Level1)
{
    networkListenerTest_->Start();
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
    want.SetParam("NetType", BEARER_CELLULAR);
    data.SetWant(want);
    data.SetCode(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED);
    EventFwk::CommonEventManager::PublishCommonEvent(data);
    networkListenerTest_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = networkListenerTest_->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_003
 * @tc.desc: Test etworkListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ScreenListenerTest, OnConditionChanged_003, TestSize.Level1)
{
    networkListenerTest_->Start();
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
    want.SetParam("NetType", BEARER_WIFI);
    data.SetWant(want);
    data.SetCode(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED);
    EventFwk::CommonEventManager::PublishCommonEvent(data);
    networkListenerTest_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = networkListenerTest_->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_004
 * @tc.desc: Test etworkListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ScreenListenerTest, OnConditionChanged_004, TestSize.Level1)
{
    networkListenerTest_->Start();
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
    want.SetParam("NetType", BEARER_BLUETOOTH);
    data.SetWant(want);
    data.SetCode(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED);
    EventFwk::CommonEventManager::PublishCommonEvent(data);
    networkListenerTest_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = networkListenerTest_->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_005
 * @tc.desc: Test etworkListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ScreenListenerTest, OnConditionChanged_005, TestSize.Level1)
{
    networkListenerTest_->Start();
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
    want.SetParam("NetType", BEARER_ETHERNET);
    data.SetWant(want);
    data.SetCode(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED);
    EventFwk::CommonEventManager::PublishCommonEvent(data);
    networkListenerTest_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = networkListenerTest_->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_006
 * @tc.desc: Test etworkListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ScreenListenerTest, OnConditionChanged_006, TestSize.Level1)
{
    networkListenerTest_->Start();
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
    want.SetParam("NetType", BEARER_WIFI_AWARE);
    data.SetWant(want);
    data.SetCode(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED);
    EventFwk::CommonEventManager::PublishCommonEvent(data);
    networkListenerTest_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = networkListenerTest_->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_007
 * @tc.desc: Test etworkListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ScreenListenerTest, OnConditionChanged_007, TestSize.Level1)
{
    networkListenerTest_->Start();
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
    want.SetParam("NetType", DEFAULT_VALUE);
    data.SetWant(want);
    data.SetCode(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED);
    EventFwk::CommonEventManager::PublishCommonEvent(data);
    networkListenerTest_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = networkListenerTest_->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_008
 * @tc.desc: Test etworkListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ScreenListenerTest, OnConditionChanged_008, TestSize.Level1)
{
    networkListenerTest_->Start();
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
    data.SetWant(want);
    data.SetCode(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED);
    EventFwk::CommonEventManager::PublishCommonEvent(data);
    networkListenerTest_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = networkListenerTest_->Stop();
    EXPECT_TRUE(ret);
}
}
}