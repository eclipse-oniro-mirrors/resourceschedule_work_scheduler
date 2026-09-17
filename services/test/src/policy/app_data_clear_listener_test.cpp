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
#include "policy/app_data_clear_listener.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"
#include "common_event_support.h"
#include "common_event_manager.h"
#include "matching_skills.h"
#include "want.h"
#include "policy_type.h"


using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class TestAppDataClearListener : public AppDataClearListener {
public:
    TestAppDataClearListener() : AppDataClearListener(nullptr) {}
    ~TestAppDataClearListener() override {}
    void OnPolicyChanged(PolicyType policyType, std::shared_ptr<DetectorValue> detectorVal) override
    {
        callCount_++;
        lastPolicyType_ = policyType;
        lastVal_ = detectorVal;
    }
    void Reset()
    {
        callCount_ = 0;
        lastPolicyType_ = static_cast<PolicyType>(-1);
        lastVal_ = nullptr;
    }
    int32_t callCount_ = 0;
    PolicyType lastPolicyType_ = static_cast<PolicyType>(-1);
    std::shared_ptr<DetectorValue> lastVal_ = nullptr;
};

class AppDataClearListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkPolicyManager> workPolicyManager_;
    static std::shared_ptr<AppDataClearListener> appDataClearListener_;
    static std::shared_ptr<TestAppDataClearListener> testListener_;
    static std::shared_ptr<AppDataClearSubscriber> subscriber_;
};

std::shared_ptr<WorkPolicyManager> AppDataClearListenerTest::workPolicyManager_ = nullptr;
std::shared_ptr<AppDataClearListener> AppDataClearListenerTest::appDataClearListener_ = nullptr;
std::shared_ptr<TestAppDataClearListener> AppDataClearListenerTest::testListener_ = nullptr;
std::shared_ptr<AppDataClearSubscriber> AppDataClearListenerTest::subscriber_ = nullptr;

void AppDataClearListenerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
    appDataClearListener_ = std::make_shared<AppDataClearListener>(workPolicyManager_);

    testListener_ = std::make_shared<TestAppDataClearListener>();
    MatchingSkills skill;
    skill.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED);
    skill.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    skill.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);
    skill.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    skill.AddEvent(CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    skill.AddEvent(CommonEventSupport::COMMON_EVENT_USER_STARTED);
    CommonEventSubscribeInfo info(skill);
    subscriber_ = std::make_shared<AppDataClearSubscriber>(info, *testListener_);
}

static CommonEventData BuildEvent(const std::string &action, const std::string &bundle, int32_t uid)
{
    AAFwk::Want want;
    want.SetAction(action);
    want.SetBundle(bundle);
    want.SetParam("uid", uid);
    CommonEventData data;
    data.SetWant(want);
    return data;
}

/**
 * @tc.name: OnReceiveEvent_001
 * @tc.desc: Test AppDataClearSubscriber OnReceiveEvent with PACKAGE_DATA_CLEARED.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(AppDataClearListenerTest, OnReceiveEvent_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED, "com.test.app", 10001);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastPolicyType_, PolicyType::APP_DATA_CLEAR);
    EXPECT_EQ(testListener_->lastVal_->intVal, 10001);
    EXPECT_EQ(testListener_->lastVal_->strVal, "com.test.app");
}

/**
 * @tc.name: OnReceiveEvent_002
 * @tc.desc: Test AppDataClearSubscriber OnReceiveEvent with PACKAGE_REMOVED.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(AppDataClearListenerTest, OnReceiveEvent_002, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED, "com.test.removed", 10002);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastPolicyType_, PolicyType::APP_REMOVED);
    EXPECT_EQ(testListener_->lastVal_->intVal, 10002);
}

/**
 * @tc.name: OnReceiveEvent_003
 * @tc.desc: Test AppDataClearSubscriber OnReceiveEvent with PACKAGE_CHANGED.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(AppDataClearListenerTest, OnReceiveEvent_003, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED, "com.test.changed", 10003);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastPolicyType_, PolicyType::APP_CHANGED);
}

/**
 * @tc.name: OnReceiveEvent_004
 * @tc.desc: Test AppDataClearSubscriber OnReceiveEvent with PACKAGE_ADDED.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(AppDataClearListenerTest, OnReceiveEvent_004, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED, "com.test.added", 10004);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastPolicyType_, PolicyType::APP_ADDED);
}

/**
 * @tc.name: OnReceiveEvent_005
 * @tc.desc: Test AppDataClearSubscriber OnReceiveEvent with USER_SWITCHED.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(AppDataClearListenerTest, OnReceiveEvent_005, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildEvent(CommonEventSupport::COMMON_EVENT_USER_SWITCHED, "", 10005);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastPolicyType_, PolicyType::USER_SWITCHED);
}

/**
 * @tc.name: OnReceiveEvent_006
 * @tc.desc: Test AppDataClearSubscriber OnReceiveEvent with USER_STARTED.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(AppDataClearListenerTest, OnReceiveEvent_006, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildEvent(CommonEventSupport::COMMON_EVENT_USER_STARTED, "", 10006);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastPolicyType_, PolicyType::USER_STARTED);
}

/**
 * @tc.name: OnReceiveEvent_007
 * @tc.desc: Test AppDataClearSubscriber OnReceiveEvent with invalid action.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(AppDataClearListenerTest, OnReceiveEvent_007, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildEvent("invalid.action", "com.test.invalid", 10007);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 0);
}
} // namespace WorkScheduler
} // namespace OHOS
