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

#include "conditions/group_listener.h"
#include "work_scheduler_service.h"
#include "work_queue_manager.h"

using namespace OHOS::AppExecFwk;
using namespace testing::ext;

using namespace OHOS {
namespace WorkScheduler {

const std::string WORKSCHEDULER_SERVICE_NAME = "WorkSchedulerService";

class TimerListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
    static std::shared_ptr<TimerListener> timerListener_;
};

std::shared_ptr<WorkQueueManager> TimerListenerTest::workQueueManager_ = nullptr;
std::shared_ptr<TimerListener> TimerListenerTest::timerListener_ = nullptr;

void TimerListenerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_ = AppExecFwk::EventRunner::Create(WORKSCHEDULER_SERVICE_NAME,
        AppExecFwk::ThreadMode::FFRT);
    timerListener_ = std::make_shared<TimerListener>(workQueueManager_);
}

/**
 * @tc.name: OnConditionChanged_001
 * @tc.desc: Test TimerListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: IB7RQR
 */
HWTEST_F(TimerListenerTest, OnConditionChanged_001, TestSize.Level1)
{
    timerListener_->Start();
    bool ret = timerListener_->Stop();
    EXPECT_TRUE(ret);
}
}
}