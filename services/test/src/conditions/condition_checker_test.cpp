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

#include <chrono>
#include <functional>
#include <thread>
#include <gtest/gtest.h>

#include "conditions/condition_checker.h"
#include "work_conn_manager.h"
#include "work_policy_manager.h"
#include "work_queue_manager.h"
#include "work_scheduler_service.h"
#include "work_status.h"
#include "work_info.h"
#include "work_sched_hilog.h"
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
#include "battery_srv_client.h"
#endif

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class ConditionCheckerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp();
    void TearDown() {}
    static std::shared_ptr<WorkSchedulerService> service_;
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
};

std::shared_ptr<WorkSchedulerService> ConditionCheckerTest::service_ = nullptr;
std::shared_ptr<WorkQueueManager> ConditionCheckerTest::workQueueManager_ = nullptr;

void ConditionCheckerTest::SetUpTestCase()
{
    service_ = DelayedSingleton<WorkSchedulerService>::GetInstance();
}

void ConditionCheckerTest::SetUp()
{
    if (service_ != nullptr) {
        if (service_->handler_ == nullptr) {
            if (!service_->eventRunner_) {
                service_->eventRunner_ = AppExecFwk::EventRunner::Create(
                    "WorkSchedulerService", AppExecFwk::ThreadMode::FFRT);
            }
            if (service_->eventRunner_ != nullptr) {
                service_->handler_ = std::make_shared<WorkEventHandler>(service_->eventRunner_, service_);
            }
        }
        if (service_->workPolicyManager_ == nullptr) {
            service_->workPolicyManager_ = std::make_shared<WorkPolicyManager>(service_);
        }
        if (service_->workPolicyManager_->workConnManager_ == nullptr) {
            service_->workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
        }
        if (service_->workQueueManager_ == nullptr) {
            service_->workQueueManager_ = std::make_shared<WorkQueueManager>(service_);
        }
        workQueueManager_ = service_->workQueueManager_;
    }
}

/**
 * @tc.name: Constructor_001
 * @tc.desc: Test ConditionChecker constructor stores workQueueManager.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ConditionCheckerTest, Constructor_001, TestSize.Level1)
{
    auto checker = std::make_shared<ConditionChecker>(workQueueManager_);
    EXPECT_EQ(checker->workQueueManager_, workQueueManager_);
}

/**
 * @tc.name: CheckAllStatus_001
 * @tc.desc: Test ConditionChecker CheckAllStatus executes all sub-checks.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ConditionCheckerTest, CheckAllStatus_001, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(9901);
    workInfo->SetElement("com.example.checker", "CheckerAbility");
    workInfo->RequestPersisted(false);
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    workInfo->RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
#endif
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 10001);
    workQueueManager_->AddWork(workStatus);

    auto checker = std::make_shared<ConditionChecker>(workQueueManager_);
    checker->CheckAllStatus();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_EQ(workQueueManager_->queueMap_.count(WorkCondition::Type::BATTERY_STATUS), 1);
    workQueueManager_->RemoveWork(workStatus);
}

#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
/**
 * @tc.name: CheckBatteryStatus_001
 * @tc.desc: Test ConditionChecker CheckBatteryStatus updates conditionMap_ based on real capacity.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ConditionCheckerTest, CheckBatteryStatus_001, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(9902);
    workInfo->SetElement("com.example.checker", "CheckerAbility");
    workInfo->RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 10002);
    workQueueManager_->AddWork(workStatus);

    auto capacity = PowerMgr::BatterySrvClient::GetInstance().GetCapacity();
    auto checker = std::make_shared<ConditionChecker>(workQueueManager_);
    checker->CheckBatteryStatus();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    if (capacity == -1) {
        EXPECT_EQ(workStatus->conditionMap_.count(WorkCondition::Type::BATTERY_STATUS), 0);
    } else if (capacity < 20) {
        EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::BATTERY_STATUS)->enumVal,
            WorkCondition::BATTERY_STATUS_LOW);
    } else {
        EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::BATTERY_STATUS)->enumVal,
            WorkCondition::BATTERY_STATUS_OKAY);
    }
    workQueueManager_->RemoveWork(workStatus);
}

/**
 * @tc.name: CheckChargerStatus_001
 * @tc.desc: Test ConditionChecker CheckChargerStatus updates conditionMap_ based on real plugged type.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ConditionCheckerTest, CheckChargerStatus_001, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(9903);
    workInfo->SetElement("com.example.checker", "CheckerAbility");
    workInfo->RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_ANY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 10003);
    workQueueManager_->AddWork(workStatus);

    auto pluggedType = PowerMgr::BatterySrvClient::GetInstance().GetPluggedType();
    auto checker = std::make_shared<ConditionChecker>(workQueueManager_);
    checker->CheckChargerStatus();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    if (pluggedType == PowerMgr::BatteryPluggedType::PLUGGED_TYPE_AC) {
        EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::CHARGER)->enumVal,
            WorkCondition::CHARGING_PLUGGED_AC);
        EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::CHARGER)->boolVal, true);
    } else if (pluggedType == PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB) {
        EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::CHARGER)->enumVal,
            WorkCondition::CHARGING_PLUGGED_USB);
        EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::CHARGER)->boolVal, true);
    } else if (pluggedType == PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS) {
        EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::CHARGER)->enumVal,
            WorkCondition::CHARGING_PLUGGED_WIRELESS);
        EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::CHARGER)->boolVal, true);
    } else if (pluggedType == PowerMgr::BatteryPluggedType::PLUGGED_TYPE_NONE ||
        pluggedType == PowerMgr::BatteryPluggedType::PLUGGED_TYPE_BUTT) {
        EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::CHARGER)->enumVal,
            WorkCondition::CHARGING_UNPLUGGED);
        EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::CHARGER)->boolVal, false);
    }
    workQueueManager_->RemoveWork(workStatus);
}
#endif // POWERMGR_BATTERY_MANAGER_ENABLE
} // namespace WorkScheduler
} // namespace OHOS
