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
};

std::shared_ptr<Watchdog> WatchdogTest::watchdog_ = nullptr;

void WatchdogTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService = DelayedSingleton<WorkSchedulerService>::GetInstance();
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    watchdog_ = std::make_shared<Watchdog>(workSchedulerService->GetWorkPolicyManager(), runner);
}

/**
 * @tc.name: watchdog_001
 * @tc.desc: Test Watchdog AddWatchdog.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WatchdogTest, watchdog_001, TestSize.Level1)
{
    bool result = watchdog_->AddWatchdog(1, 1);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: watchdog_002
 * @tc.desc: Test Watchdog RemoveWatchdog.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WatchdogTest, watchdog_002, TestSize.Level1)
{
    bool result = watchdog_->AddWatchdog(1, 1);
    EXPECT_EQ(result, false);
    watchdog_->RemoveWatchdog(1);
}

/**
 * @tc.name: watchdog_003
 * @tc.desc: Test Watchdog ProcessEvent.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WatchdogTest, watchdog_003, TestSize.Level1)
{
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(0);
    event = nullptr;
    watchdog_->ProcessEvent(event);
    EXPECT_TRUE(event == nullptr);
}

/**
 * @tc.name: watchdog_004
 * @tc.desc: Test Watchdog ProcessEvent.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WatchdogTest, watchdog_004, TestSize.Level1)
{
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(0);
    watchdog_->ProcessEvent(event);
    EXPECT_TRUE(event->GetInnerEventId() == 0);
}

/**
 * @tc.name: watchdog_005
 * @tc.desc: Test Watchdog ProcessEvent.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WatchdogTest, watchdog_005, TestSize.Level1)
{
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(0);
    watchdog_->service_ = nullptr;
    watchdog_->ProcessEvent(event);
    std::shared_ptr<WorkPolicyManager> service;
    watchdog_->service_ = service;
    EXPECT_TRUE(event->GetInnerEventId() == 0);
}

}
}