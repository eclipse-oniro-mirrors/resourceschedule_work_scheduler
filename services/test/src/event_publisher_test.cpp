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
 * @tc.desc: Test EventPublisher PublishEvent.
 * @tc.type: FUNC
 * @tc.require: I8GHCL
 */
HWTEST_F(EventPublisherTest, publishEvent_001, TestSize.Level1)
{
    std::string result;
    std::string eventType;
    std::string eventValue;
    eventPublisher_->PublishEvent(result, eventType, eventValue);
    EXPECT_EQ(result, std::string("dump -d need right params."));
}

HWTEST_F(EventPublisherTest, Dump_001, TestSize.Level1)
{
    std::vector<std::pair<string, string>> infos = {
        {"event", "info"},
        {"network", "wifi"},
        {"network", "disconnect"},
        {"charging", "usb"},
        {"charging", "ac"},
        {"charging", "wireless"},
        {"charging", "none"},
        {"storage", "low"},
        {"storage", "ok"},
        {"batteryStatus", "low"},
        {"batteryStatus", "ok"},
    };
    for (auto it : infos) {
        std::string result;
        std::string eventType = it.first;
        std::string eventValue = it.second;
        eventPublisher_->Dump(result, eventType, eventValue);
        WS_HILOGI("%{public}s", result.c_str());
        EXPECT_EQ(!result.empty(), true);
}
}
}
}