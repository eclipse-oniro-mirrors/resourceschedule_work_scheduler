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

#include "cpu_policy.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"


using namespace testing::ext;
 
namespace OHOS {
namespace WorkScheduler {
class CpuPolicyTest: public testing::Test {
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
    std::shared_ptr<WorkPolicyManager> workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
    cpuPolicy_ = std::make_shared<CpuPolicy>(workPolicyManager_);
}

/**
 * @tc.name: getCpuUsage_001
 * @tc.desc: Test CpuPolicy GetCpuUsage.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(CpuPolicyTest, getCpuUsage_001, TestSize.Level1)
{
    workPolicyManager_->SetCpuUsageByDump(15);
    int32_t cpuUsage = CpuPolicy->GetCpuUsage();
    EXPECT_EQ(cpuUsage, 15);
}

/**
 * @tc.name: getCpuUsage_002
 * @tc.desc: Test CpuPolicy GetCpuUsage.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(CpuPolicyTest, getCpuUsage_002, TestSize.Level1)
{
    workPolicyManager_->SetCpuUsageByDump(120);
    int32_t cpuUsage = CpuPolicy->GetCpuUsage();
    EXPECT_TRUE(cpuUsage >=0 && cpuUsage <= 100);
}

/**
 * @tc.name: getPolicyMaxRunning_001
 * @tc.desc: Test CpuPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(CpuPolicyTest, getPolicyMaxRunning_001, TestSize.Level1)
{
    workPolicyManager_->SetCpuUsageByDump(15);
    int32_t maxRunning = CpuPolicy->GetPolicyMaxRunning();
    EXPECT_EQ(maxRunning, 3);
}

/**
 * @tc.name: getPolicyMaxRunning_002
 * @tc.desc: Test CpuPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(CpuPolicyTest, getPolicyMaxRunning_002, TestSize.Level1)
{
    workPolicyManager_->SetCpuUsageByDump(45);
    int32_t maxRunning = CpuPolicy->GetPolicyMaxRunning();
    EXPECT_EQ(maxRunning, 2);
}

/**
 * @tc.name: getPolicyMaxRunning_003
 * @tc.desc: Test CpuPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(CpuPolicyTest, getPolicyMaxRunning_003, TestSize.Level1)
{
    workPolicyManager_->SetCpuUsageByDump(75);
    int32_t maxRunning = CpuPolicy->GetPolicyMaxRunning();
    EXPECT_EQ(maxRunning, 1);
}

/**
 * @tc.name: getPolicyMaxRunning_004
 * @tc.desc: Test CpuPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(CpuPolicyTest, getPolicyMaxRunning_004, TestSize.Level1)
{
    workPolicyManager_->SetCpuUsageByDump(120);
    int32_t maxRunning = CpuPolicy->GetPolicyMaxRunning();
    EXPECT_TRUE(maxRunning >= 1 && maxRunning <= 2);
}
}
}