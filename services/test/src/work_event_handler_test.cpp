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
#include "work_event_handler.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"
#include "work_sched_hilog.h"

using namespace OHOS::AppExecFwk;
using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

const std::string WORKSCHEDULER_SERVICE_NAME = "WorkSchedulerService";

class MockWorkPolicyManagerForHandler : public WorkPolicyManager {
public:
    using WorkPolicyManager::WorkPolicyManager;
    ~MockWorkPolicyManagerForHandler() = default;
    void CheckWorkToRun() override { checkWorkToRunCount_++; }
    void TriggerIdeWork() override { triggerIdeWorkCount_++; }
    int32_t checkWorkToRunCount_ = 0;
    int32_t triggerIdeWorkCount_ = 0;
};

class WorkEventHandlerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp();
    void TearDown() {}
    static std::shared_ptr<WorkSchedulerService> service_;
    static std::shared_ptr<MockWorkPolicyManagerForHandler> mockPolicy_;
    static std::shared_ptr<EventRunner> runner_;
};

std::shared_ptr<WorkSchedulerService> WorkEventHandlerTest::service_ = nullptr;
std::shared_ptr<MockWorkPolicyManagerForHandler> WorkEventHandlerTest::mockPolicy_ = nullptr;
std::shared_ptr<EventRunner> WorkEventHandlerTest::runner_ = nullptr;

void WorkEventHandlerTest::SetUpTestCase()
{
    service_ = DelayedSingleton<WorkSchedulerService>::GetInstance();
    runner_ = EventRunner::Create(WORKSCHEDULER_SERVICE_NAME, ThreadMode::FFRT);
    mockPolicy_ = std::make_shared<MockWorkPolicyManagerForHandler>(service_);
}

void WorkEventHandlerTest::SetUp()
{
    if (service_ != nullptr) {
        if (service_->eventRunner_ == nullptr) {
            service_->eventRunner_ = runner_;
        }
        if (service_->handler_ == nullptr) {
            service_->handler_ = std::make_shared<WorkEventHandler>(runner_, service_);
        }
        service_->workPolicyManager_ = mockPolicy_;
        mockPolicy_->checkWorkToRunCount_ = 0;
        mockPolicy_->triggerIdeWorkCount_ = 0;
    }
}

/**
 * @tc.name: ProcessEvent_Retrigger_001
 * @tc.desc: Test WorkEventHandler ProcessEvent with RETRIGGER_MSG calls CheckWorkToRun.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkEventHandlerTest, ProcessEvent_Retrigger_001, TestSize.Level1)
{
    auto handler = std::make_shared<WorkEventHandler>(runner_, service_);
    auto event = InnerEvent::Get(WorkEventHandler::RETRIGGER_MSG, 0);
    handler->ProcessEvent(event);
    EXPECT_EQ(mockPolicy_->checkWorkToRunCount_, 1);
}

/**
 * @tc.name: ProcessEvent_IdeRetrigger_001
 * @tc.desc: Test WorkEventHandler ProcessEvent with IDE_RETRIGGER_MSG calls TriggerIdeWork.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkEventHandlerTest, ProcessEvent_IdeRetrigger_001, TestSize.Level1)
{
    auto handler = std::make_shared<WorkEventHandler>(runner_, service_);
    auto event = InnerEvent::Get(WorkEventHandler::IDE_RETRIGGER_MSG, 0);
    handler->ProcessEvent(event);
    EXPECT_EQ(mockPolicy_->triggerIdeWorkCount_, 1);
}

/**
 * @tc.name: ProcessEvent_Default_001
 * @tc.desc: Test WorkEventHandler ProcessEvent with unknown event does nothing.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkEventHandlerTest, ProcessEvent_Default_001, TestSize.Level1)
{
    auto handler = std::make_shared<WorkEventHandler>(runner_, service_);
    auto event = InnerEvent::Get(999, 0);
    handler->ProcessEvent(event);
    EXPECT_EQ(mockPolicy_->checkWorkToRunCount_, 0);
    EXPECT_EQ(mockPolicy_->triggerIdeWorkCount_, 0);
}

/**
 * @tc.name: ProcessEvent_ExpiredService_001
 * @tc.desc: Test WorkEventHandler ProcessEvent with expired service returns early.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkEventHandlerTest, ProcessEvent_ExpiredService_001, TestSize.Level1)
{
    std::shared_ptr<WorkSchedulerService> tempService = std::make_shared<WorkSchedulerService>();
    tempService->workPolicyManager_ = mockPolicy_;
    auto handler = std::make_shared<WorkEventHandler>(runner_, tempService);
    tempService.reset();
    auto event = InnerEvent::Get(WorkEventHandler::RETRIGGER_MSG, 0);
    handler->ProcessEvent(event);
    EXPECT_EQ(mockPolicy_->checkWorkToRunCount_, 0);
}

/**
 * @tc.name: ProcessEvent_CheckCondition_001
 * @tc.desc: Test WorkEventHandler ProcessEvent with CHECK_CONDITION_MSG calls TriggerWorkIfConditionReady.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkEventHandlerTest, ProcessEvent_CheckCondition_001, TestSize.Level1)
{
    auto handler = std::make_shared<WorkEventHandler>(runner_, service_);
    auto event = InnerEvent::Get(WorkEventHandler::CHECK_CONDITION_MSG, 0);
    handler->ProcessEvent(event);
    EXPECT_EQ(mockPolicy_->checkWorkToRunCount_, 0);
    EXPECT_EQ(mockPolicy_->triggerIdeWorkCount_, 0);
}
} // namespace WorkScheduler
} // namespace OHOS
