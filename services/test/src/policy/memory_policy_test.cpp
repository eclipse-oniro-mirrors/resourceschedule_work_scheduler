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

#include "policy/memory_policy.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class MemoryPolicyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkPolicyManager> workPolicyManager_;
    static std::shared_ptr<MemoryPolicy> memoryPolicy_;
};

std::shared_ptr<MemoryPolicy> MemoryPolicyTest::memoryPolicy_ = nullptr;
std::shared_ptr<WorkPolicyManager> MemoryPolicyTest::workPolicyManager_ = nullptr;

void MemoryPolicyTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workSchedulerService_->Start();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
    memoryPolicy_ = std::make_shared<MemoryPolicy>(workPolicyManager_);
}

/**
 * @tc.name: GetPolicyMaxRunning_001
 * @tc.desc: Test MemoryPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(MemoryPolicyTest, GetPolicyMaxRunning_001, TestSize.Level1)
{
    memoryPolicy_->workPolicyManager_->SetMemoryByDump(1 * 1024 * 1024);
    int32_t ret = memoryPolicy_->GetPolicyMaxRunning();
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: GetPolicyMaxRunning_002
 * @tc.desc: Test MemoryPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(MemoryPolicyTest, GetPolicyMaxRunning_002, TestSize.Level1)
{
    memoryPolicy_->workPolicyManager_->SetMemoryByDump(2 * 1024 * 1024);
    int32_t ret = memoryPolicy_->GetPolicyMaxRunning();
    EXPECT_EQ(ret, 2);
}

/**
 * @tc.name: GetPolicyMaxRunning_003
 * @tc.desc: Test MemoryPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(MemoryPolicyTest, GetPolicyMaxRunning_003, TestSize.Level1)
{
    memoryPolicy_->workPolicyManager_->SetMemoryByDump(3 * 1024 * 1024);
    int32_t ret = memoryPolicy_->GetPolicyMaxRunning();
    EXPECT_EQ(ret, 3);
}

/**
 * @tc.name: GetPolicyName_001
 * @tc.desc: Test MemoryPolicy GetPolicyName.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(MemoryPolicyTest, GetPolicyName_001, TestSize.Level1)
{
    std::string ret = memoryPolicy_->GetPolicyName();
    EXPECT_EQ(ret, "MEMORY_POLICY");
}
}
}
