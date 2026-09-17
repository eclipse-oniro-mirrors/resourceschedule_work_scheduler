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

#include "policy/cpu_policy.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"


using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class CpuPolicyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkPolicyManager> workPolicyManager_;
    static std::shared_ptr<CpuPolicy> cpuPolicy_;
};

std::shared_ptr<CpuPolicy> CpuPolicyTest::cpuPolicy_ = nullptr;
std::shared_ptr<WorkPolicyManager> CpuPolicyTest::workPolicyManager_ = nullptr;

void CpuPolicyTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
    cpuPolicy_ = std::make_shared<CpuPolicy>(workPolicyManager_);
}

/**
 * @tc.name: getCpuUsage_001
 * @tc.desc: Test CpuPolicy GetCpuUsage.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(CpuPolicyTest, getCpuUsage_001, TestSize.Level1)
{
    workPolicyManager_->SetCpuUsageByDump(15);
    int32_t cpuUsage = cpuPolicy_->GetCpuUsage();
    EXPECT_EQ(cpuUsage, 15);
}

/**
 * @tc.name: getPolicyMaxRunning_001
 * @tc.desc: Test CpuPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(CpuPolicyTest, getPolicyMaxRunning_001, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetCpuUsageByDump(15);
    int32_t maxRunning = cpuPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(maxRunning, 3);
}

/**
 * @tc.name: getPolicyMaxRunning_002
 * @tc.desc: Test CpuPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(CpuPolicyTest, getPolicyMaxRunning_002, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetCpuUsageByDump(45);
    int32_t maxRunning = cpuPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(maxRunning, 2);
}

/**
 * @tc.name: getPolicyMaxRunning_003
 * @tc.desc: Test CpuPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(CpuPolicyTest, getPolicyMaxRunning_003, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetCpuUsageByDump(55);
    int32_t maxRunning = cpuPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(maxRunning, 1);
}

/**
 * @tc.name: getPolicyMaxRunning_004
 * @tc.desc: Test CpuPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(CpuPolicyTest, getPolicyMaxRunning_004, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetCpuUsageByDump(65);
    int32_t maxRunning = cpuPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(maxRunning, 0);
}

/**
 * @tc.name: getPolicyMaxRunning_007
 * @tc.desc: Test CpuPolicy GetPolicyMaxRunning boundary cpu=30 (CPU_LOW).
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(CpuPolicyTest, getPolicyMaxRunning_007, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetCpuUsageByDump(30);
    int32_t maxRunning = cpuPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(maxRunning, 2);
    EXPECT_EQ(systemPolicy.cpuUsage, 30);
}

/**
 * @tc.name: getPolicyMaxRunning_008
 * @tc.desc: Test CpuPolicy GetPolicyMaxRunning boundary cpu=50 (CPU_NORMAL).
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(CpuPolicyTest, getPolicyMaxRunning_008, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetCpuUsageByDump(50);
    int32_t maxRunning = cpuPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(maxRunning, 1);
    EXPECT_EQ(systemPolicy.cpuUsage, 50);
}

/**
 * @tc.name: getPolicyMaxRunning_009
 * @tc.desc: Test CpuPolicy GetPolicyMaxRunning boundary cpu=60 (CPU_HIGH).
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(CpuPolicyTest, getPolicyMaxRunning_009, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetCpuUsageByDump(60);
    int32_t maxRunning = cpuPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(maxRunning, 0);
    EXPECT_EQ(systemPolicy.cpuUsage, 60);
}
}
}