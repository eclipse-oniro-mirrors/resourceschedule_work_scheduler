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

#include "policy/power_mode_policy.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"


using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class PowerModePolicyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkPolicyManager> workPolicyManager_;
    static std::shared_ptr<PowerModePolicy> powerModePolicy_;
};

std::shared_ptr<PowerModePolicy> PowerModePolicyTest::powerModePolicy_ = nullptr;
std::shared_ptr<WorkPolicyManager> PowerModePolicyTest::workPolicyManager_ = nullptr;

void PowerModePolicyTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
    PowerModePolicy_ = std::make_shared<PowerModePolicy>(workPolicyManager_);
}

/**
 * @tc.name: getPolicyMaxRunning_001
 * @tc.desc: Test PowerModePolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(PowerModePolicyTest, getPolicyMaxRunning_001, TestSize.Level1)
{
    workPolicyManager_->SetCpuUsageByDump(15);
    int32_t maxRunning = PowerModePolicy_->GetPolicyMaxRunning();
    EXPECT_EQ(maxRunning, 3);
}

/**
 * @tc.name: getPolicyMaxRunning_001
 * @tc.desc: Test PowerModePolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(PowerModePolicyTest, getPolicyMaxRunning_001, TestSize.Level1)
{
    workPolicyManager_->SetCpuUsageByDump(45);
    int32_t maxRunning = PowerModePolicy_->GetPolicyMaxRunning();
    EXPECT_EQ(maxRunning, 3);
}

/**
 * @tc.name: getPolicyMaxRunning_002
 * @tc.desc: Test PowerModePolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(PowerModePolicyTest, getPolicyMaxRunning_002, TestSize.Level1)
{
    workPolicyManager_->SetCpuUsageByDump(55);
    int32_t maxRunning = PowerModePolicy_->GetPolicyMaxRunning();
    EXPECT_EQ(maxRunning, 1);
}
}
}
