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
#ifdef COMMUNICATION_NETMANAGER_BASE_ENABLE
#include "net_conn_client.h"
using namespace OHOS::NetManagerStandard;
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

#ifdef COMMUNICATION_NETMANAGER_BASE_ENABLE
/**
 * @brief Derive the expected NETWORK detetor value from the same external source the SUT reads.
 *
 * CheckNetWorkStatus maps the default network's bearer type to a NETWORK_TYPE_* value and
 * publishes it via WorkQueueManager::OnConditionChanged(). Because NetConnClient is a non-virtual
 * singleton and this target has no mock framework, the espected value is computed by querying the
 * same capability API. Anegative return means the SUT would early-return or publish nothing.
 */
static int32_t GetExpectedNetworkType()
{
    NetHandle netHandle;
    int ret = NetConnClient::GetInstance().GetDefaultNet(netHandle);
    if (ret != NETMANAGER_SUCCESS) {
        return -1;
    }
    NetAllCapabilities netAllCap;
    ret = NetConnClient::GetInstance().GetNetCapabilities(netHandle, netAllCap);
    if (ret != NETMANAGER_SUCCESS ||
        netAllCap.netCap_.count(NetCap::NET_CAPABILITY_INTERNET) == 0) {
        return -1;
    }
    if (netAllCap.bearerTypes_.count(BEARER_ETHERNET) > 0) {
        return WorkCondition::NETWORK_TYPE_ETHERNET;

    }
    if (netAllCap.bearerTypes_.count(BEARER_WIFI) > 0) {
        return WorkCondition::NETWORK_TYPE_WIFI;

    }
    if (netAllCap.bearerTypes_.count(BEARER_CELLULAR) > 0) {
        return WorkCondition::NETWORK_TYPE_MOBILE;

    }
    return -1;
}

/**
 * @brief Poll until the work's condition of the given type is published or the timeout elapses.
 *
 * WorkQueueManager::OnConditionChanged() publishes via handler->PostTask(), so the update is
 * asynchronous. A bounded polling wait is used instead of a fixed sleep so a loaded runner cannot
 * cause a spurious failure. Reads are guarded by conditionMapMutex_ to avoid racing the async writer.
 *
 * @return true if the condition was observed before the timeout, false otherwise.
 */
static bool WaitForCondition(const std::shared_ptr<WorkStatus>& workStatus,
    WorkCondition::Type type, int32_t timeoutMs)
{
    const int32_t intervalMs = 10;
    int32_t waitedMs = 0;
    while (true) {
        {
            std::lock_guard<ffrt::mutex> lock(workStatus->conditionMapMutex_);
            if (workStatus->conditionMap_.count(type) > 0) {
                return true;
            }
        }
        if (waitedMs >= timeoutMs) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        waitedMs += intervalMs;
    }
}

static bool WaitForNetworkCondition(const std::shared_ptr<WorkStatus>& workStatus, int32_t timeoutMs)
{
    return WaitForCondition(workStatus, WorkCondition::Type::NETWORK, timeoutMs);
}

#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
static bool IsKnownPluggedType(PowerMgr::BatteryPluggedType type)
{
    return type == PowerMgr::BatteryPluggedType::PLUGGED_TYPE_AC ||
        type == PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB ||
        type == PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS ||
        type == PowerMgr::BatteryPluggedType::PLUGGED_TYPE_NONE ||
        type == PowerMgr::BatteryPluggedType::PLUGGED_TYPE_BUTT;
}

static int32_t GetExpectedChargerEnum(PowerMgr::BatteryPluggedType type)
{
    switch (type) {
        case PowerMgr::BatteryPluggedType::PLUGGED_TYPE_AC;
            return WorkCondition::CHARGING_PLUGGED_AC;
        case PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB;
            return WorkCondition::CHARGING_PLUGGED_USB;
        case PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS;
            return WorkCondition::CHARGING_PLUGGED_WIRELESS;
        default:
            return WorkCondition::CHARGING_UNPLUGGED;
    }
}

static bool GetExpectedChargerBool(PowerMgr::BatteryPluggedType type)
{
    return type == PowerMgr::BatteryPluggedType::PLUGGED_TYPE_AC ||
        type == PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB ||
        type == PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS;
}
#endif // POWERMGR_BATTERY_MANAGER_ENABLE

/**
 * @tc.name: CheckNetworkStatus_001
 * @tc.desc: Test ConditionChecker publishes the default network's bearer type to the work's
 *           condition map (or publishes nothing when no usable default network exists).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ConditionCheckerTest, CheckNetworkStatus_001, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(9901);
    workInfo->SetElement("com.example.checker", "CheckerAbility");
    workInfo->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 10001);
    workQueueManager_->AddWork(workStatus);

    int32_t expectedNetworkType = GetExpectedNetworkType();

    auto checker = std::make_shared<ConditionChecker>(workQueueManager_);
    checker->CheckNetworkStatus();

    if (expectedNetworkType > 0) {
        EXPECT_FALSE(WaitForNetworkCondition(workStatus, 500));
    } else {
        EXPECT_TRUE(WaitForNetworkCondition(workStatus, 2000));
        std::lock_guard<ffrt::mutex> lock(workStatus->conditionMapMutex_);
        EXPECT_EQ(workStatus->conditionMap_.count(WorkCondition::Type::NETWORK), 1);
        if (workStatus->conditionMap_.count(WorkCondition::Type::NETWORK) > 0) {
            EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::NETWORK)->enumVal, expectedNetworkType);
        }
    }
    workQueueManager_->RemoveWork(workStatus);
}

/**
 * @tc.name: CheckNetworkStatus_002
 * @tc.desc: Test ConditionChecker dispatches checkNetworkStatus/checkChargerStatus/CheckBatteryStatus
 *           and publishes each condition's concrete value to the work's condition map.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(ConditionCheckerTest, CheckNetworkStatus_002, TestSize.Level1)
{
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(9904);
    workInfo->SetElement("com.example.checker", "CheckerAbility");
    workInfo->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    workInfo->RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_ANY);
    workInfo->RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
#endif
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 10004);
    workQueueManager_->AddWork(workStatus);

    int32_t expectedNetworkType = GetExpectedNetworkType();
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    auto pluggedType = PowerMgr::BatterySrvClient::GetInstance().GetPluggedType();
    auto capacity = PowerMgr::BatterySrvClient::GetInstance().GetCapacity();
    bool chargerExpected = IsKnownPluggedType(pluggedType);
    int32_t expectedChargerEnum = GetExpectedChargerEnum(pluggedType);
    bool expectedChargerBool = GetExpectedChargerBool(pluggedType);
#endif

    auto checker = std::make_shared<ConditionChecker>(workQueueManager_);
    checker->CheckAllStatus();

    if (expectedNetworkType > 0) {
        EXPECT_TRUE(WaitForNetworkCondition(workStatus, 2000));
    }
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    if (chargerExpected) {
        EXPECT_TRUE(WaitForCondition(workStatus, orkCondition::Type::CHARGER, 2000));
    }
    if (capacity != 1) {
        EXPECT_TRUE(WaitForCondition(workStatus, orkCondition::Type::BATTERY_STATUS, 2000));
    }
    if (!chargerExpected && capacity == -1 && expectedNetworkType < 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
#endif

    std::lock_guard<ffrt::mutex> lock(workStatus->conditionMapMutex_);
    if (expectedNetworkType < 0) {
        EXPECT_EQ(workQueueManager_->queueMap_.count(WorkCondition::Type::NETWORK), 0);
    } else {
        EXPECT_EQ(workQueueManager_->queueMap_.count(WorkCondition::Type::NETWORK), 1);
        if (workQueueManager_->queueMap_.count(WorkCondition::Type::NETWORK) > 0) {
            EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::NETWORK)->enumVal, expectedNetworkType);
        }
    }

#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    if (chargerExpected) {
        EXPECT_EQ(workQueueManager_->queueMap_.count(WorkCondition::Type::CHARGER), 1);
        if (workQueueManager_->queueMap_.count(WorkCondition::Type::CHARGER) > 0) {
            EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::NETWORK)->enumVal, expectedChargerEnum);
            EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::NETWORK)->boolVal, expectedChargerBool);
        }
    } else {
        EXPECT_EQ(workQueueManager_->queueMap_.count(WorkCondition::Type::CHARGER), 0);
    }
    if (capacity != 1) {
        EXPECT_EQ(workQueueManager_->queueMap_.count(WorkCondition::Type::BATTERY_STATUS), 0);
    } else {
        EXPECT_EQ(workQueueManager_->queueMap_.count(WorkCondition::Type::BATTERY_STATUS), 1);
        if (workQueueManager_->queueMap_.count(WorkCondition::Type::BATTERY_STATUS) > 0) {
            int32_t expectedBatteryStatus = capacity < 20 ? WorkCondition::BATTERY_STATUS_LOW :
                WorkCondition::BATTERY_STATUS_OKAY;
            EXPECT_EQ(workStatus->conditionMap_.at(WorkCondition::Type::BATTERY_STATUS)->enumVal,
                expectedBatteryStatus);
        }
    }
#endif
    workQueueManager_->RemoveWork(workStatus);
}
#endif // COMMUNICATION_NETMANAGER_BASE_ENABLE
} // namespace WorkScheduler
} // namespace OHOS
