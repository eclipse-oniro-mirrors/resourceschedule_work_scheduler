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
const int32_t COUNT_THERMAL_CRUCIAL = 0;
const int32_t COUNT_THERMAL_LOW = 1;
const int32_t COUNT_THERMAL_MIDDLE = 2;
const int32_t COUNT_THERMAL_NORMAL = 3;
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
#ifdef PC_PLATFORM
    EXPECT_EQ(ret, COUNT_THERMAL_NORMAL);
#else
    EXPECT_EQ(ret, COUNT_THERMAL_CRUCIAL);
#endif
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
#ifdef PC_PLATFORM
    EXPECT_EQ(ret, COUNT_THERMAL_NORMAL);
#else
    EXPECT_EQ(ret, COUNT_THERMAL_LOW);
#endif
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
    EXPECT_EQ(ret, COUNT_THERMAL_NORMAL);
}

/**
 * @tc.name: GetPolicyMaxRunning_004
 * @tc.desc: Test ThermalPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ThermalPolicyTest, GetPolicyMaxRunning_004, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    MockProcess(PowerMgr::ThermalLevel::HOT);
    int32_t ret = thermalPolicy_->GetPolicyMaxRunning(systemPolicy);
#ifdef PC_PLATFORM
    EXPECT_EQ(ret, COUNT_THERMAL_MIDDLE);
#else
    EXPECT_EQ(ret, COUNT_THERMAL_CRUCIAL);
#endif
}

/**
 * @tc.name: GetPolicyMaxRunning_005
 * @tc.desc: Test ThermalPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ThermalPolicyTest, GetPolicyMaxRunning_005, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    MockProcess(PowerMgr::ThermalLevel::OVERHEATED);
    int32_t ret = thermalPolicy_->GetPolicyMaxRunning(systemPolicy);
#ifdef PC_PLATFORM
    EXPECT_EQ(ret, COUNT_THERMAL_LOW);
#else
    EXPECT_EQ(ret, COUNT_THERMAL_CRUCIAL);
#endif
}

/**
 * @tc.name: GetPolicyMaxRunning_006
 * @tc.desc: Test ThermalPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ThermalPolicyTest, GetPolicyMaxRunning_006, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    MockProcess(PowerMgr::ThermalLevel::WARNING);
    int32_t ret = thermalPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(ret, COUNT_THERMAL_CRUCIAL);
}

/**
 * @tc.name: GetPolicyMaxRunning_007
 * @tc.desc: Test ThermalPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ThermalPolicyTest, GetPolicyMaxRunning_007, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetThermalLevelByDump(0);
    int32_t ret = thermalPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(ret, COUNT_THERMAL_NORMAL);
}

/**
 * @tc.name: GetPolicyMaxRunning_008
 * @tc.desc: Test ThermalPolicy GetPolicyMaxRunning.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ThermalPolicyTest, GetPolicyMaxRunning_008, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    MockProcess(PowerMgr::ThermalLevel::COOL);
    workPolicyManager_->SetThermalLevelByDump(-1);
    int32_t ret = thermalPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(ret, COUNT_THERMAL_NORMAL);
}

/**
 * @tc.name: GetPolicyMaxRunning_009
 * @tc.desc: Test ThermalPolicy GetPolicyMaxRunning with dump level exceeding all map entries.
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ThermalPolicyTest, GetPolicyMaxRunning_009, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetThermalLevelByDump(999);
    int32_t ret = thermalPolicy_->GetPolicyMaxRunning(systemPolicy);
    EXPECT_EQ(ret, COUNT_THERMAL_CRUCIAL);
    EXPECT_EQ(systemPolicy.thermalLevel, 999);
}

/**
 * @tc.name: GetPolicyMaxRunning_010
 * @tc.desc: Test ThermalPolicy GetPolicyMaxRunning with dump level=1 (NORMAL on non-PC).
 * @tc.type: FUNC
 * @tc.require: I974IQ
 */
HWTEST_F(ThermalPolicyTest, GetPolicyMaxRunning_010, TestSize.Level1)
{
    WorkSchedSystemPolicy systemPolicy;
    workPolicyManager_->SetThermalLevelByDump(1);
    int32_t ret = thermalPolicy_->GetPolicyMaxRunning(systemPolicy);
#ifdef PC_PLATFORM
    EXPECT_EQ(ret, COUNT_THERMAL_CRUCIAL);
#else
    EXPECT_EQ(ret, COUNT_THERMAL_LOW);
#endif
}
}
}
