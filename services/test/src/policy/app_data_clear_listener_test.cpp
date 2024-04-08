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
#define private public
#include "policy/app_data_clear_listener.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"
#include "common_event_support.h"


using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class AppDataClearListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkPolicyManager> workPolicyManager_;
    static std::shared_ptr<AppDataClearListener> appDataClearListener_;
};

std::shared_ptr<WorkPolicyManager> AppDataClearListenerTest::workPolicyManager_ = nullptr;
std::shared_ptr<AppDataClearListener> AppDataClearListenerTest::appDataClearListener_ = nullptr;

void AppDataClearListenerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
    appDataClearListener_ = std::make_shared<AppDataClearListener>(workPolicyManager_);
}

/**
 * @tc.name: OnReceiveEvent_001
 * @tc.desc: Test AppDataClearSubscriber OnReceiveEvent.
 * @tc.type: FUNC
 * @tc.require: I9DP93
 */
HWTEST_F(AppDataClearListenerTest, OnReceiveEvent_001, TestSize.Level1)
{
    bool ret1 = appDataClearListener_->Start();
    EXPECT_TRUE(ret1);

    OHOS::AAFwk::Want want;
    want.SetAction(CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED);
    want.SetBundle("com.test.example");
    want.SetParam("uid", 1);
    CommonEventData event1{want};
    appDataClearListener_->commonEventSubscriber->OnReceiveEvent(event1);
    EXPECT_EQ(event1.GetWant().GetAction(), CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED);
    
    want.SetAction(CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    CommonEventData event2{want};
    appDataClearListener_->commonEventSubscriber->OnReceiveEvent(event2);
    EXPECT_EQ(event2.GetWant().GetAction(), CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);

    want.SetAction(CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);
    CommonEventData event3{want};
    appDataClearListener_->commonEventSubscriber->OnReceiveEvent(event3);
    EXPECT_EQ(event3.GetWant().GetAction(), CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);

    want.SetAction(CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    CommonEventData event4{want};
    appDataClearListener_->commonEventSubscriber->OnReceiveEvent(event4);
    EXPECT_EQ(event4.GetWant().GetAction(), CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);

    bool ret2 = appDataClearListener_->Stop();
    EXPECT_TRUE(ret2);
}

/**
 * @tc.name: Start_001
 * @tc.desc: Test AppDataClearListener Start.
 * @tc.type: FUNC
 * @tc.require: I9DP93
 */
HWTEST_F(AppDataClearListenerTest, Start_001, TestSize.Level1)
{
    bool ret1 = appDataClearListener_->Start();
    EXPECT_TRUE(ret1);

    bool ret2 = appDataClearListener_->Stop();
    EXPECT_TRUE(ret2);
}
}
}
