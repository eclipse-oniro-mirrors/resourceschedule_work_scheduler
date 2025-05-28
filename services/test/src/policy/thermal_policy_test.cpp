/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "policy/thermal_policy.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"
#include "thermal_mgr_client.h"

using namespace testing::ext;

namespace {
OHOS::PowerMgr::ThermalLevel thermalLevel_;
void MockProcess(OHOS::PowerMgr::ThermalLevel thermlLevel)
{
    thermalLevel_ = thermlLevel;
}
}

namespace OHOS {
namespace PowerMgr {
ThermalLevel ThermalMgrClient::GetThermalLevel()
{
    return thermalLevel_;
}
}
namespace WorkScheduler {
class ThermalPolicyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkPolicyManager> workPolicyManager_;
    static std::shared_ptr<ThermalPolicy> thermalPolicy_;
};

std::shared_ptr<ThermalPolicy> ThermalPolicyTest::thermalPolicy_ = nullptr;
std::shared_ptr<WorkPolicyManager> ThermalPolicyTest::workPolicyManager_ = nullptr;

void ThermalPolicyTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workSchedulerService_->Start();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
    thermalPolicy_ = std::make_shared<ThermalPolicy>(workPolicyManager_);
}

/**
 * @tc.name: GetPolicyMaxRunning_001
 * @tc.desc: Test ThermalPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ThermalPolicyTest, GetPolicyMaxRunning_001, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    MockProcess(PowerMgr::ThermalLevel::WARM);
    int32_t ret = thermalPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: GetPolicyMaxRunning_002
 * @tc.desc: Test ThermalPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ThermalPolicyTest, GetPolicyMaxRunning_002, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    MockProcess(PowerMgr::ThermalLevel::NORMAL);
    int32_t ret = thermalPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: GetPolicyMaxRunning_003
 * @tc.desc: Test ThermalPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ThermalPolicyTest, GetPolicyMaxRunning_003, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    MockProcess(PowerMgr::ThermalLevel::COOL);
    int32_t ret = thermalPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(ret, 3);
}
}
}
