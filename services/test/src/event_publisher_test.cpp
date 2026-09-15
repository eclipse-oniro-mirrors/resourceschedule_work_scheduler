/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "event_publisher.h"
#include "work_sched_hilog.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class EventPublisherTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<EventPublisher> eventPublisher_;
};

std::shared_ptr<EventPublisher> EventPublisherTest::eventPublisher_ = nullptr;

void EventPublisherTest::SetUpTestCase()
{
    eventPublisher_ = std::make_shared<EventPublisher>();
}

/**
 * @tc.name: publishEvent_001
 * @tc.desc: Test EventPublisher PublishEvent with empty params.
 * @tc.type: FUNC
 * @tc.require: I8GHCL
 */
HWTEST_F(EventPublisherTest, publishEvent_001, TestSize.Level3)
{
    std::string result;
    std::string eventType;
    std::string eventValue;
    eventPublisher_->PublishEvent(result, eventType, eventValue);
    EXPECT_EQ(result, std::string("dump -d need right params."));
}

/**
 * @tc.name: Dump_001
 * @tc.desc: Test EventPublisher Dump with event info.
 * @tc.type: FUNC
 * @tc.require: I8GHCL
 */
HWTEST_F(EventPublisherTest, Dump_001, TestSize.Level3)
{
    std::string result;
    std::string eventType = "event";
    std::string eventValue = "info";
    eventPublisher_->Dump(result, eventType, eventValue);
    EXPECT_EQ(result.find("event info"), 0);
    EXPECT_GT(result.find("network wifi"), 0);
    EXPECT_GT(result.find("batteryStatus ok"), 0);
}

/**
 * @tc.name: Dump_002
 * @tc.desc: Test EventPublisher Dump dispatches to PublishEvent for network.
 * @tc.type: FUNC
 * @tc.require: I8GHCL
 */
HWTEST_F(EventPublisherTest, Dump_002, TestSize.Level3)
{
    std::string result;
    std::string eventType = "network";
    std::string eventValue = "invalid";
    eventPublisher_->Dump(result, eventType, eventValue);
    EXPECT_NE(result.find("dump need right param"), std::string::npos);
}

/**
 * @tc.name: PublishNetworkEvent_001
 * @tc.desc: Test EventPublisher PublishNetworkEvent with invalid param.
 * @tc.type: FUNC
 * @tc.require: I8GHCL
 */
HWTEST_F(EventPublisherTest, PublishNetworkEvent_001, TestSize.Level3)
{
    std::string result;
    std::string param = "invalid";
    eventPublisher_->PublishNetworkEvent(result, param);
    EXPECT_EQ(result, std::string("dump need right param."));
}

/**
 * @tc.name: PublishChargingEvent_001
 * @tc.desc: Test EventPublisher PublishChargingEvent with invalid param.
 * @tc.type: FUNC
 * @tc.require: I8GHCL
 */
HWTEST_F(EventPublisherTest, PublishChargingEvent_001, TestSize.Level3)
{
    std::string result;
    std::string param = "invalid";
    eventPublisher_->PublishChargingEvent(result, param);
    EXPECT_EQ(result, std::string("dump need right param."));
}

/**
 * @tc.name: PublishStorageEvent_001
 * @tc.desc: Test EventPublisher PublishStorageEvent with invalid param.
 * @tc.type: FUNC
 * @tc.require: I8GHCL
 */
HWTEST_F(EventPublisherTest, PublishStorageEvent_001, TestSize.Level3)
{
    std::string result;
    std::string param = "invalid";
    eventPublisher_->PublishStorageEvent(result, param);
    EXPECT_EQ(result, std::string("dump need right param."));
}

/**
 * @tc.name: PublishBatteryStatusEvent_001
 * @tc.desc: Test EventPublisher PublishBatteryStatusEvent with invalid param.
 * @tc.type: FUNC
 * @tc.require: I8GHCL
 */
HWTEST_F(EventPublisherTest, PublishBatteryStatusEvent_001, TestSize.Level3)
{
    std::string result;
    std::string param = "invalid";
    eventPublisher_->PublishBatteryStatusEvent(result, param);
    EXPECT_EQ(result, std::string("dump need right param."));
}

/**
 * @tc.name: PublishEvent_Network_001
 * @tc.desc: Test EventPublisher PublishEvent dispatches network.
 * @tc.type: FUNC
 * @tc.require: I8GHCL
 */
HWTEST_F(EventPublisherTest, PublishEvent_Network_001, TestSize.Level3)
{
    std::string result;
    std::string type = "network";
    std::string param = "invalid";
    eventPublisher_->PublishEvent(result, type, param);
    EXPECT_EQ(result, std::string("dump need right param."));
}

/**
 * @tc.name: PublishEvent_Charging_001
 * @tc.desc: Test EventPublisher PublishEvent dispatches charging.
 * @tc.type: FUNC
 * @tc.require: I8GHCL
 */
HWTEST_F(EventPublisherTest, PublishEvent_Charging_001, TestSize.Level3)
{
    std::string result;
    std::string type = "charging";
    std::string param = "invalid";
    eventPublisher_->PublishEvent(result, type, param);
    EXPECT_EQ(result, std::string("dump need right param."));
}

/**
 * @tc.name: PublishEvent_Storage_001
 * @tc.desc: Test EventPublisher PublishEvent dispatches storage.
 * @tc.type: FUNC
 * @tc.require: I8GHCL
 */
HWTEST_F(EventPublisherTest, PublishEvent_Storage_001, TestSize.Level3)
{
    std::string result;
    std::string type = "storage";
    std::string param = "invalid";
    eventPublisher_->PublishEvent(result, type, param);
    EXPECT_EQ(result, std::string("dump need right param."));
}

/**
 * @tc.name: PublishEvent_BatteryStatus_001
 * @tc.desc: Test EventPublisher PublishEvent dispatches batteryStatus.
 * @tc.type: FUNC
 * @tc.require: I8GHCL
 */
HWTEST_F(EventPublisherTest, PublishEvent_BatteryStatus_001, TestSize.Level3)
{
    std::string result;
    std::string type = "batteryStatus";
    std::string param = "invalid";
    eventPublisher_->PublishEvent(result, type, param);
    EXPECT_EQ(result, std::string("dump need right param."));
}
}
}
