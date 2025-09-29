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

#include "work_status.h"
#include "work_condition.h"
#include "work_sched_data_manager.h"
#include "work_sched_hilog.h"
#include "work_info.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr int64_t INVALID_VALUE = -1;
constexpr int64_t ONE_MINUTE = 60 * 1000LL;
constexpr int64_t TWENTY_MINUTE = 20 * ONE_MINUTE;
constexpr int64_t THIRTY_MINUTE = 30 * ONE_MINUTE;
constexpr int64_t TWO_HOUR = 4 * THIRTY_MINUTE;
constexpr int64_t FOUR_HOUR = 2 * TWO_HOUR;
constexpr int64_t TWELVE_HOUR = 6 * TWO_HOUR;
constexpr int64_t TWENTY_FOUR_HOUR = 12 * TWO_HOUR;
constexpr int64_t FOURTY_EIGHT_HOUR = 24 * TWO_HOUR;
const std::string MAIL_APP_NAME = "com.netease.ohmail";
}

class WorkStatusTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkStatus> workStatus_;
    static std::shared_ptr<WorkInfo> workInfo_;
};

std::shared_ptr<WorkStatus> WorkStatusTest::workStatus_ = nullptr;
std::shared_ptr<WorkInfo> WorkStatusTest::workInfo_ = nullptr;

void WorkStatusTest::SetUpTestCase()
{
    workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->SetWorkId(-1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_->SetElement(bundleName, abilityName);
    workInfo_->RequestPersisted(false);
    time_t baseTime;
    (void)time(&baseTime);
    workInfo_->RequestBaseTime(baseTime);
    workStatus_ = std::make_shared<WorkStatus>(*(workInfo_.get()), -1);
}

/**
 * @tc.name: makeWorkId_001
 * @tc.desc: Test WorkStatus MakeWorkId.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, makeWorkId_001, TestSize.Level1)
{
    std::string result = workStatus_->MakeWorkId(1, 1);
    EXPECT_EQ(result, "u1_1");
}

/**
 * @tc.name: isSameUser_001
 * @tc.desc: Test WorkStatus IsSameUser.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isSameUser_001, TestSize.Level1)
{
    workStatus_->uid_ = -1;
    bool result = workStatus_->IsSameUser();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: isSameUser_002
 * @tc.desc: Test WorkStatus IsSameUser.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isSameUser_002, TestSize.Level1)
{
    workStatus_->uid_ = 1;
    bool result = workStatus_->IsSameUser();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: isUriKeySwitchOn_001
 * @tc.desc: Test WorkStatus IsUriKeySwitchOn.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isUriKeySwitchOn_001, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->SetPreinstalled(false);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsUriKeySwitchOn();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: isUriKeySwitchOn_002
 * @tc.desc: Test WorkStatus IsUriKeySwitchOn.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isUriKeySwitchOn_002, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->SetPreinstalled(true);
    workInfo_->workId_ = 1;
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsUriKeySwitchOn();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isUriKeySwitchOn_003
 * @tc.desc: Test WorkStatus IsUriKeySwitchOn.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isUriKeySwitchOn_003, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = 1;
    workInfo_->SetPreinstalled(true);
    workInfo_->uriKey_ = "key";
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsUriKeySwitchOn();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_001
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_001, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_002
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_002, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workInfo_->RequestNetworkType(WorkCondition::Network::NETWORK_UNKNOWN);
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_003
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_003, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_BLUETOOTH);
    std::shared_ptr<Condition> networkCondition = std::make_shared<Condition>();
    networkCondition->enumVal = WorkCondition::Network::NETWORK_TYPE_ETHERNET;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::NETWORK, networkCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_004
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_004, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    std::shared_ptr<Condition> batteryCondition = std::make_shared<Condition>();
    batteryCondition->enumVal = WorkCondition::BatteryStatus::BATTERY_STATUS_OKAY;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::BATTERY_STATUS, batteryCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_005
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_005, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workInfo_->RequestBatteryLevel(80);
    std::shared_ptr<Condition> batteryLevelCondition = std::make_shared<Condition>();
    batteryLevelCondition->intVal = 70;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::BATTERY_LEVEL, batteryLevelCondition);
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_006
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_006, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestBatteryLevel(60);
    workInfo_->SetPreinstalled(true);
    std::shared_ptr<Condition> batteryLevelCondition = std::make_shared<Condition>();
    batteryLevelCondition->intVal = 70;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::BATTERY_LEVEL, batteryLevelCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_007
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_007, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_OKAY);
    std::shared_ptr<Condition> storageCondition = std::make_shared<Condition>();
    storageCondition->enumVal = WorkCondition::Storage::STORAGE_LEVEL_LOW;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::STORAGE, storageCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_008
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_008, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    uint32_t timeInterval = 1200;
    workInfo_->RequestRepeatCycle(timeInterval);
    workInfo_->SetPreinstalled(true);
    workStatus_->workInfo_ = workInfo_;

    std::shared_ptr<Condition> timerCondition = std::make_shared<Condition>();
    timerCondition->boolVal = true;
    timerCondition->uintVal = 7200001;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::TIMER, timerCondition);
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_009
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_009, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_OKAY);
    std::shared_ptr<Condition> storageCondition = std::make_shared<Condition>();
    storageCondition->enumVal = WorkCondition::Storage::STORAGE_LEVEL_OKAY;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::STORAGE, storageCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_0010
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_0010, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_OKAY);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_0011
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_0011, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<Condition> chargingCondition = std::make_shared<Condition>();
    chargingCondition->enumVal = WorkCondition::Charger::CHARGING_PLUGGED_USB;
    chargingCondition->boolVal = false;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::CHARGER, chargingCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_0012
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_0012, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<Condition> chargingCondition = std::make_shared<Condition>();
    chargingCondition->enumVal = WorkCondition::Charger::CHARGING_PLUGGED_USB;
    chargingCondition->boolVal = true;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::CHARGER, chargingCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_0013
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_0013, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestChargerType(false, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<Condition> chargingCondition = std::make_shared<Condition>();
    chargingCondition->enumVal = WorkCondition::Charger::CHARGING_PLUGGED_USB;
    chargingCondition->boolVal = false;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::CHARGER, chargingCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: isReady_0014
 * @tc.desc: Test WorkStatus IsReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isReady_0014, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<Condition> chargingCondition = std::make_shared<Condition>();
    chargingCondition->enumVal = WorkCondition::Charger::CHARGING_PLUGGED_AC;
    chargingCondition->boolVal = true;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::CHARGER, chargingCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: dump_001
 * @tc.desc: Test WorkStatus Dump.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, dump_001, TestSize.Level1)
{
    std::shared_ptr<Condition> chargingCondition = std::make_shared<Condition>();
    chargingCondition->enumVal = WorkCondition::Charger::CHARGING_PLUGGED_AC;
    chargingCondition->boolVal = true;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::CHARGER, chargingCondition);

    std::shared_ptr<Condition> networkCondition = std::make_shared<Condition>();
    networkCondition->enumVal = WorkCondition::Network::NETWORK_TYPE_ANY;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::NETWORK, networkCondition);

    std::shared_ptr<Condition> batteryLevelCondition = std::make_shared<Condition>();
    batteryLevelCondition->intVal = 66;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::BATTERY_LEVEL, batteryLevelCondition);

    std::shared_ptr<Condition> batteryStatusCondition = std::make_shared<Condition>();
    batteryStatusCondition->enumVal = WorkCondition::BatteryStatus::BATTERY_STATUS_OKAY;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::BATTERY_STATUS, batteryStatusCondition);

    std::shared_ptr<Condition> storageCondition = std::make_shared<Condition>();
    storageCondition->enumVal = WorkCondition::Storage::STORAGE_LEVEL_OKAY;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::STORAGE, storageCondition);

    std::shared_ptr<Condition> timerCondition = std::make_shared<Condition>();
    timerCondition->boolVal = false;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::TIMER, timerCondition);

    std::string result;
    workStatus_->Dump(result);
    bool ret = result.empty();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: dump_002
 * @tc.desc: Test WorkStatus Dump.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, dump_002, TestSize.Level1)
{
    workStatus_->workInfo_->saId_ = 1000;
    workStatus_->workInfo_->residentSa_ = true;

    std::string result;
    workStatus_->Dump(result);
    EXPECT_TRUE(workStatus_->workInfo_->IsSA());
}

/**
 * @tc.name: getMinInterval_001
 * @tc.desc: Test WorkStatus GetMinInterval.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, getMinInterval_001, TestSize.Level1)
{
    int64_t interval = 7200000;
    workStatus_->SetMinIntervalByDump(interval);
    int64_t ret = workStatus_->GetMinInterval();
    EXPECT_TRUE(ret == interval);
}

/**
 * @tc.name: isRepeating_001
 * @tc.desc: Test WorkStatus IsRepeating.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, isRepeating_001, TestSize.Level1)
{
    workStatus_->conditionMap_.erase(WorkCondition::Type::TIMER);
    bool ret = workStatus_->IsRepeating();
    EXPECT_FALSE(ret);

    std::shared_ptr<Condition> timerCondition = std::make_shared<Condition>();
    timerCondition->boolVal = true;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::TIMER, timerCondition);
    bool ret1 = workStatus_->IsRepeating();
    EXPECT_TRUE(ret1);

    std::shared_ptr<Condition> timerCondition1 = std::make_shared<Condition>();
    timerCondition1->boolVal = false;
    timerCondition1->intVal = 1200;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::TIMER, timerCondition1);
    bool ret2 = workStatus_->IsRepeating();
    EXPECT_TRUE(ret2);
}

/**
 * @tc.name: IsReadyStatus_001
 * @tc.desc: Test WorkStatus IsReadyStatus.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkStatusTest, IsReadyStatus_001, TestSize.Level1)
{
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    bool ret = workStatus_->IsReadyStatus();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsRemoved_001
 * @tc.desc: Test WorkStatus IsRemoved.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkStatusTest, IsRemoved_001, TestSize.Level1)
{
    workStatus_->MarkStatus(WorkStatus::Status::REMOVED);
    bool ret = workStatus_->IsRemoved();
    EXPECT_TRUE(ret);
    workStatus_->MarkRound();
}

/**
 * @tc.name: NeedRemove_001
 * @tc.desc: Test WorkStatus NeedRemove.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkStatusTest, NeedRemove_001, TestSize.Level1)
{
    workStatus_->conditionMap_.clear();
    bool ret = workStatus_->NeedRemove();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: NeedRemove_002
 * @tc.desc: Test WorkStatus NeedRemove.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkStatusTest, NeedRemove_002, TestSize.Level1)
{
    workStatus_->conditionMap_.clear();
    std::shared_ptr<Condition> repeatCycle = std::make_shared<Condition>();
    repeatCycle->boolVal = true;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::TIMER, repeatCycle);
    bool ret = workStatus_->NeedRemove();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: NeedRemove_003
 * @tc.desc: Test WorkStatus NeedRemove.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkStatusTest, NeedRemove_003, TestSize.Level1)
{
    workStatus_->conditionMap_.clear();
    std::shared_ptr<Condition> repeatCycle = std::make_shared<Condition>();
    repeatCycle->boolVal = false;
    repeatCycle->intVal = 0;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::TIMER, repeatCycle);
    bool ret = workStatus_->NeedRemove();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: NeedRemove_004
 * @tc.desc: Test WorkStatus NeedRemove.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkStatusTest, NeedRemove_004, TestSize.Level1)
{
    workStatus_->conditionMap_.clear();
    std::shared_ptr<Condition> repeatCycle = std::make_shared<Condition>();
    repeatCycle->boolVal = false;
    repeatCycle->intVal = 1;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::TIMER, repeatCycle);
    bool ret = workStatus_->NeedRemove();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateTimerIfNeed_001
 * @tc.desc: Test WorkStatus UpdateTimerIfNeed.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkStatusTest, UpdateTimerIfNeed_001, TestSize.Level1)
{
    workStatus_->conditionMap_.clear();
    workStatus_->UpdateTimerIfNeed();

    std::shared_ptr<Condition> repeatCycle = std::make_shared<Condition>();
    repeatCycle->boolVal = false;
    repeatCycle->intVal = 1;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::TIMER, repeatCycle);
    workStatus_->UpdateTimerIfNeed();
    EXPECT_TRUE(repeatCycle->intVal == 0);
}

/**
 * @tc.name: UpdateTimerIfNeed_002
 * @tc.desc: Test WorkStatus UpdateTimerIfNeed.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkStatusTest, UpdateTimerIfNeed_002, TestSize.Level1)
{
    workStatus_->conditionMap_.clear();
    workStatus_->UpdateTimerIfNeed();

    std::shared_ptr<Condition> repeatCycle = std::make_shared<Condition>();
    repeatCycle->boolVal = true;
    repeatCycle->intVal = 1;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::TIMER, repeatCycle);
    workStatus_->UpdateTimerIfNeed();
    EXPECT_TRUE(repeatCycle->intVal == 1);
}

/**
 * @tc.name: MarkTimeout_001
 * @tc.desc: Test WorkStatus MarkTimeout.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkStatusTest, MarkTimeout_001, TestSize.Level1)
{
    workStatus_->MarkTimeout();
    bool ret = workStatus_->IsLastWorkTimeout();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IsNapReady_001
 * @tc.desc: Test WorkStatus IsNapReady.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkStatusTest, IsNapReady_001, TestSize.Level1)
{
    WorkInfo workInfo = WorkInfo();
    std::shared_ptr<Condition> repeatCycle1 = std::make_shared<Condition>();
    repeatCycle1->boolVal = true;
    workInfo.conditionMap_.emplace(WorkCondition::Type::DEEP_IDLE, repeatCycle1);
    workStatus_->workInfo_ = std::make_shared<WorkInfo>(workInfo);

    std::shared_ptr<Condition> repeatCycle2 = std::make_shared<Condition>();
    repeatCycle2->boolVal = false;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::DEEP_IDLE, repeatCycle2);

    bool ret = workStatus_->IsNapReady(WorkCondition::Type::DEEP_IDLE);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsNapReady_002
 * @tc.desc: Test WorkStatus IsNapReady.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkStatusTest, IsNapReady_002, TestSize.Level1)
{
    WorkInfo workInfo = WorkInfo();
    std::shared_ptr<Condition> repeatCycle1 = std::make_shared<Condition>();
    repeatCycle1->boolVal = true;
    workInfo.conditionMap_.emplace(WorkCondition::Type::DEEP_IDLE, repeatCycle1);
    workStatus_->workInfo_ = std::make_shared<WorkInfo>(workInfo);

    std::shared_ptr<Condition> repeatCycle2 = std::make_shared<Condition>();
    repeatCycle2->boolVal = true;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::DEEP_IDLE, repeatCycle2);

    bool ret = workStatus_->IsNapReady(WorkCondition::Type::DEEP_IDLE);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateUidLastTimeMap_001
 * @tc.desc: Test WorkStatus UpdateUidLastTimeMap.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkStatusTest, UpdateUidLastTimeMap_001, TestSize.Level1)
{
    workStatus_->s_uid_last_time_map.clear();
    workStatus_->uid_ = 1;
    workStatus_->UpdateUidLastTimeMap();
    workStatus_->ClearUidLastTimeMap(1);
    EXPECT_TRUE(workStatus_->s_uid_last_time_map.empty());
}

/**
 * @tc.name: GetStatus_001
 * @tc.desc: Test WorkStatus GetStatus.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkStatusTest, GetStatus_001, TestSize.Level1)
{
    workStatus_->MarkStatus(WorkStatus::Status::RUNNING);
    EXPECT_EQ(workStatus_->GetStatus(), WorkStatus::Status::RUNNING);
}

/**
 * @tc.name: IsSAReady_001
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_001, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsSAReady();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSAReady_002
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_002, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workInfo_->RequestNetworkType(WorkCondition::Network::NETWORK_UNKNOWN);
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsSAReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsSAReady_003
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_003, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_BLUETOOTH);
    std::shared_ptr<Condition> networkCondition = std::make_shared<Condition>();
    networkCondition->enumVal = WorkCondition::Network::NETWORK_TYPE_ETHERNET;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::NETWORK, networkCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsSAReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsSAReady_004
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_004, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    std::shared_ptr<Condition> batteryCondition = std::make_shared<Condition>();
    batteryCondition->enumVal = WorkCondition::BatteryStatus::BATTERY_STATUS_OKAY;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::BATTERY_STATUS, batteryCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsSAReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsSAReady_005
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_005, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workInfo_->RequestBatteryLevel(80);
    std::shared_ptr<Condition> batteryLevelCondition = std::make_shared<Condition>();
    batteryLevelCondition->intVal = 70;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::BATTERY_LEVEL, batteryLevelCondition);
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsSAReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsSAReady_006
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_006, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestBatteryLevel(60);
    workInfo_->SetPreinstalled(true);
    std::shared_ptr<Condition> batteryLevelCondition = std::make_shared<Condition>();
    batteryLevelCondition->intVal = 70;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::BATTERY_LEVEL, batteryLevelCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsSAReady();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSAReady_007
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_007, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_OKAY);
    std::shared_ptr<Condition> storageCondition = std::make_shared<Condition>();
    storageCondition->enumVal = WorkCondition::Storage::STORAGE_LEVEL_LOW;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::STORAGE, storageCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsSAReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsSAReady_008
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_008, TestSize.Level1)
{
    workStatus_->s_uid_last_time_map.clear();
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    uint32_t timeInterval = 1200;
    workInfo_->RequestRepeatCycle(timeInterval);
    workInfo_->SetPreinstalled(true);
    workStatus_->workInfo_ = workInfo_;

    std::shared_ptr<Condition> timerCondition = std::make_shared<Condition>();
    timerCondition->boolVal = true;
    timerCondition->uintVal = 7200001;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::TIMER, timerCondition);
    bool result = workStatus_->IsSAReady();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSAReady_009
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_009, TestSize.Level1)
{
    workStatus_->s_uid_last_time_map.clear();
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_OKAY);
    std::shared_ptr<Condition> storageCondition = std::make_shared<Condition>();
    storageCondition->enumVal = WorkCondition::Storage::STORAGE_LEVEL_OKAY;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::STORAGE, storageCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsSAReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsSAReady_0010
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_0010, TestSize.Level1)
{
    workStatus_->s_uid_last_time_map.clear();
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_OKAY);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsSAReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsSAReady_0011
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_0011, TestSize.Level1)
{
    workStatus_->s_uid_last_time_map.clear();
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<Condition> chargingCondition = std::make_shared<Condition>();
    chargingCondition->enumVal = WorkCondition::Charger::CHARGING_PLUGGED_USB;
    chargingCondition->boolVal = false;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::CHARGER, chargingCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsSAReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsSAReady_0012
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_0012, TestSize.Level1)
{
    workStatus_->s_uid_last_time_map.clear();
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<Condition> chargingCondition = std::make_shared<Condition>();
    chargingCondition->enumVal = WorkCondition::Charger::CHARGING_PLUGGED_USB;
    chargingCondition->boolVal = true;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::CHARGER, chargingCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsSAReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsSAReady_0013
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_0013, TestSize.Level1)
{
    workStatus_->s_uid_last_time_map.clear();
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestChargerType(false, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<Condition> chargingCondition = std::make_shared<Condition>();
    chargingCondition->enumVal = WorkCondition::Charger::CHARGING_PLUGGED_USB;
    chargingCondition->boolVal = false;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::CHARGER, chargingCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsSAReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsSAReady_0014
 * @tc.desc: Test WorkStatus IsSAReady.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsSAReady_0014, TestSize.Level1)
{
    workStatus_->s_uid_last_time_map.clear();
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->workId_ = -1;
    workStatus_->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    workInfo_->RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_AC);
    std::shared_ptr<Condition> chargingCondition = std::make_shared<Condition>();
    chargingCondition->enumVal = WorkCondition::Charger::CHARGING_PLUGGED_AC;
    chargingCondition->boolVal = true;
    workStatus_->conditionMap_.emplace(WorkCondition::Type::CHARGER, chargingCondition);
    workStatus_->workInfo_ = workInfo_;
    bool result = workStatus_->IsSAReady();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ToString_001
 * @tc.desc: Test WorkStatus ToString.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, ToString_001, TestSize.Level1)
{
    workStatus_->conditionStatus_.clear();
    std::shared_ptr<DataManager> dataManager = DelayedSingleton<DataManager>::GetInstance();
    dataManager->SetDeviceSleep(false);
    workStatus_->ToString(WorkCondition::Type::TIMER);
    EXPECT_TRUE(workStatus_->conditionStatus_.empty());
}

/**
 * @tc.name: ToString_002
 * @tc.desc: Test WorkStatus ToString.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, ToString_002, TestSize.Level1)
{
    workStatus_->conditionStatus_.clear();
    std::shared_ptr<DataManager> dataManager = DelayedSingleton<DataManager>::GetInstance();
    dataManager->SetDeviceSleep(false);
    workStatus_->conditionStatus_ = "TIMER&ready";
    workStatus_->workInfo_->saId_ = 1000;
    workStatus_->workInfo_->residentSa_ = true;
    workStatus_->ToString(WorkCondition::Type::TIMER);
    EXPECT_FALSE(workStatus_->conditionStatus_.empty());
}

/**
 * @tc.name: ToString_003
 * @tc.desc: Test WorkStatus ToString.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, ToString_003, TestSize.Level1)
{
    workStatus_->conditionStatus_.clear();
    std::shared_ptr<DataManager> dataManager = DelayedSingleton<DataManager>::GetInstance();
    dataManager->SetDeviceSleep(false);
    workStatus_->conditionStatus_ = "TIMER&ready";
    workStatus_->ToString(WorkCondition::Type::TIMER);
    EXPECT_FALSE(workStatus_->conditionStatus_.empty());
}

/**
 * @tc.name: CheckEarliestStartTime_001
 * @tc.desc: Test WorkStatus CheckEarliestStartTime.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, CheckEarliestStartTime_001, TestSize.Level1)
{
    WorkInfo workInfo;
    WorkStatus workStatus(workInfo, 100);
    auto work = std::move(workStatus.workInfo_);
    EXPECT_FALSE(workStatus.CheckEarliestStartTime());
    work->SetEarliestStartTime(-1);
    workStatus.workInfo_ = work;
    EXPECT_FALSE(workStatus.CheckEarliestStartTime());
    work->SetEarliestStartTime(60 * 1000);
    EXPECT_TRUE(workStatus.CheckEarliestStartTime());
    time_t now;
    time(&now);
    now = static_cast<time_t>(static_cast<int64_t>(now) - 50);
    work->SetEarliestStartTime(10);
    work->createTime_ = static_cast<uint64_t>(now) * 1000;
    EXPECT_FALSE(workStatus.CheckEarliestStartTime());
}

/**
 * @tc.name: IsNeedDiscreteScheduled_001
 * @tc.desc: Test WorkStatus IsNeedDiscreteScheduled.
 * @tc.type: FUNC
 * @tc.require: I95QHG
 */
HWTEST_F(WorkStatusTest, IsNeedDiscreteScheduled_001, TestSize.Level1)
{
    WorkInfo workInfo;
    WorkStatus workStatus(workInfo, 100);
    auto work = std::move(workStatus.workInfo_);
    EXPECT_FALSE(workStatus.IsNeedDiscreteScheduled());
    workStatus.workInfo_ = work;
    EXPECT_FALSE(workStatus.IsNeedDiscreteScheduled());
    work->SetCallBySystemApp(true);
    EXPECT_GE(workStatus.IsNeedDiscreteScheduled(), 0);
}

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
/**
 * @tc.name: IsChargingState_001
 * @tc.desc: Test IsChargingState.
 * @tc.type: FUNC
 * @tc.require: ICVNC0
 */
HWTEST_F(WorkStatusTest, IsChargingState_001, TestSize.Level1)
{
    bool isChargingState = workStatus_->IsChargingState();
    EXPECT_TRUE(isChargingState);
}

/**
 * @tc.name: IsMailApp_001
 * @tc.desc: Test IsMailApp.
 * @tc.type: FUNC
 * @tc.require: ICVNC0
 */
HWTEST_F(WorkStatusTest, IsMailApp_001, TestSize.Level1)
{
    workStatus_->bundleName_ = "test";
    bool isMailApp = workStatus_->IsMailApp();
    EXPECT_FALSE(isMailApp);
}

/**
 * @tc.name: SetMinIntervalWhenCharging_001
 * @tc.desc: 非邮箱充电场景
 * @tc.type: FUNC
 * @tc.require: ICVNC0
 */
HWTEST_F(WorkStatusTest, SetMinIntervalWhenCharging_001, TestSize.Level1)
{
    workStatus_->bundleName_ = "test";
    workStatus_->SetMinIntervalWhenCharging(60);
    EXPECT_EQ(workStatus_->minInterval_, INVALID_VALUE);
    workStatus_->SetMinIntervalWhenCharging(50);
    EXPECT_EQ(workStatus_->minInterval_, FOURTY_EIGHT_HOUR);
    workStatus_->SetMinIntervalWhenCharging(40);
    EXPECT_EQ(workStatus_->minInterval_, TWO_HOUR);
    workStatus_->SetMinIntervalWhenCharging(30);
    EXPECT_EQ(workStatus_->minInterval_, TWO_HOUR);
    workStatus_->SetMinIntervalWhenCharging(20);
    EXPECT_EQ(workStatus_->minInterval_, TWO_HOUR);
    workStatus_->SetMinIntervalWhenCharging(10);
    EXPECT_EQ(workStatus_->minInterval_, TWO_HOUR);
}

/**
 * @tc.name: SetMinIntervalWhenCharging_002
 * @tc.desc: 邮箱充电场景
 * @tc.type: FUNC
 * @tc.require: ICVNC0
 */
HWTEST_F(WorkStatusTest, SetMinIntervalWhenCharging_002, TestSize.Level1)
{
    workStatus_->bundleName_ = MAIL_APP_NAME;
    workStatus_->SetMinIntervalWhenCharging(60);
    EXPECT_EQ(workStatus_->minInterval_, INVALID_VALUE);
    workStatus_->SetMinIntervalWhenCharging(50);
    EXPECT_EQ(workStatus_->minInterval_, FOURTY_EIGHT_HOUR);
    workStatus_->SetMinIntervalWhenCharging(40);
    EXPECT_EQ(workStatus_->minInterval_, TWO_HOUR);
    workStatus_->SetMinIntervalWhenCharging(30);
    EXPECT_EQ(workStatus_->minInterval_, TWO_HOUR);
    workStatus_->SetMinIntervalWhenCharging(20);
    EXPECT_EQ(workStatus_->minInterval_, TWO_HOUR);
}

/**
 * @tc.name: SetMinIntervalWhenNotCharging_001
 * @tc.desc: 邮箱非充电场景
 * @tc.type: FUNC
 * @tc.require: ICVNC0
 */
HWTEST_F(WorkStatusTest, SetMinIntervalWhenNotCharging_001, TestSize.Level1)
{
    workStatus_->bundleName_ = MAIL_APP_NAME;
    workStatus_->SetMinIntervalWhenNotCharging(60);
    EXPECT_EQ(workStatus_->minInterval_, INVALID_VALUE);
}

/**
 * @tc.name: SetMinIntervalWhenNotCharging_002
 * @tc.desc: 非邮箱非充电场景
 * @tc.type: FUNC
 * @tc.require: ICVNC0
 */
HWTEST_F(WorkStatusTest, SetMinIntervalWhenNotCharging_002, TestSize.Level1)
{
    workStatus_->bundleName_ = "test";
    workStatus_->SetMinIntervalWhenNotCharging(60);
    EXPECT_EQ(workStatus_->minInterval_, INVALID_VALUE);
}
#endif
}
}