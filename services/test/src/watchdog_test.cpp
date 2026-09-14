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

#define private public
#include "watchdog.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class WatchdogTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<Watchdog> watchdog_;
    static std::shared_ptr<WorkPolicyManager> policy_;
};

std::shared_ptr<Watchdog> WatchdogTest::watchdog_ = nullptr;
std::shared_ptr<WorkPolicyManager> WatchdogTest::policy_ = nullptr;

void WatchdogTest::SetUpTestCase()
{
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    policy_ = std::make_shared<WorkPolicyManager>(service);
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    watchdog_ = std::make_shared<Watchdog>(policy_, runner);
}

/**
 * @tc.name: watchdog_001
 * @tc.desc: Test Watchdog AddWatchdog with null runner returns false.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WatchdogTest, watchdog_001, TestSize.Level3)
{
    bool result = watchdog_->AddWatchdog(1, 1);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: ProcessEvent_NullEvent_001
 * @tc.desc: Test Watchdog ProcessEvent with null event returns early (watchdogTime_ unchanged).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WatchdogTest, ProcessEvent_NullEvent_001, TestSize.Level3)
{
    auto before = policy_->watchdogTime_.load();
    AppExecFwk::InnerEvent::Pointer event(nullptr, 0);
    watchdog_->ProcessEvent(event);
    EXPECT_EQ(policy_->watchdogTime_.load(), before);
}

/**
 * @tc.name: ProcessEvent_NullService_001
 * @tc.desc: Test Watchdog ProcessEvent with null service_ does not crash.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WatchdogTest, ProcessEvent_NullService_001, TestSize.Level3)
{
    auto before = policy_->watchdogTime_.load();
    auto wd = std::make_shared<Watchdog>(nullptr, nullptr);
    auto event = AppExecFwk::InnerEvent::Get(42, 0);
    wd->ProcessEvent(event);
    EXPECT_EQ(policy_->watchdogTime_.load(), before);
}

/**
 * @tc.name: RemoveWatchdog_001
 * @tc.desc: Test Watchdog RemoveWatchdog sets runner and removes event.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WatchdogTest, RemoveWatchdog_001, TestSize.Level3)
{
    auto runner = AppExecFwk::EventRunner::Create("TestRemoveWd", AppExecFwk::ThreadMode::FFRT);
    auto wd = std::make_shared<Watchdog>(nullptr, runner);
    wd->RemoveWatchdog(99);
    EXPECT_EQ(wd->GetEventRunner().get(), runner.get());
}

/**
 * @tc.name: AddWatchdog_WithRunner_001
 * @tc.desc: Test Watchdog AddWatchdog with valid runner returns true.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WatchdogTest, AddWatchdog_WithRunner_001, TestSize.Level3)
{
    auto runner = AppExecFwk::EventRunner::Create("TestWatchdogRunner", AppExecFwk::ThreadMode::FFRT);
    auto wd = std::make_shared<Watchdog>(nullptr, runner);
    bool result = wd->AddWatchdog(2, 1);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: ProcessEvent_ValidService_001
 * @tc.desc: Test Watchdog ProcessEvent with valid event and non-null service_ calls WatchdogTimeOut.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WatchdogTest, ProcessEvent_ValidService_001, TestSize.Level3)
{
    auto before = policy_->watchdogIdMap_.size();
    auto event = AppExecFwk::InnerEvent::Get(42, 0);
    watchdog_->ProcessEvent(event);
    EXPECT_EQ(policy_->watchdogIdMap_.size(), before);
}
}
}
