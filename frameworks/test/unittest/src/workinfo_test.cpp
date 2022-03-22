/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "workinfo_test.h"

#include "work_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
void WorkInfoTest::SetUpTestCase(void)
{
}

void WorkInfoTest::TearDownTestCase(void)
{
}

void WorkInfoTest::SetUp()
{
}

void WorkInfoTest::TearDown()
{
}

/**
 * @tc.name WorkInfoTest001
 * @tc.desc Set WorkId to WorkInfo
 * @tc.type FUNC
 * @tc.require: SR000GGTN6 AR000GH896 AR000GH897 AR000GH898
 */
HWTEST_F (WorkInfoTest, WorkInfoTest001, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.SetWorkId(1);
    EXPECT_EQ(workInfo.GetWorkId(), 1);
}

/**
 * @tc.name WorkInfoTest002
 * @tc.desc Set bundleName and abilityName to WorkInfo
 * @tc.type FUNC
 * @tc.require: SR000GGTN6 AR000GH896 AR000GH897 AR000GH898
 */
HWTEST_F (WorkInfoTest, WorkInfoTest002, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.SetElement("bundle_name", "ability_name");
    EXPECT_EQ(workInfo.GetBundleName(), "bundle_name");
    EXPECT_EQ(workInfo.GetAbilityName(), "ability_name");
}

/**
 * @tc.name WorkInfoTest003
 * @tc.desc Set workInfo persisted
 * @tc.type FUNC
 * @tc.require: SR000GGTN6 AR000GH896 AR000GH897 AR000GH898
 */
HWTEST_F (WorkInfoTest, WorkInfoTest003, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.RequestPersisted(true);
    EXPECT_EQ(workInfo.IsPersisted(), true);
    workInfo.RequestPersisted(false);
    EXPECT_EQ(workInfo.IsPersisted(), false);
}

/**
 * @tc.name WorkInfoTest004
 * @tc.desc Set workInfo charger condition charging
 * @tc.type FUNC
 * @tc.require: SR000GGTN6 AR000GH896 AR000GH897 AR000GH898
 */
HWTEST_F (WorkInfoTest, WorkInfoTest004, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_ANY);
    EXPECT_EQ(workInfo.GetChargerType(), WorkCondition::Charger::CHARGING_PLUGGED_ANY);
}

/**
 * @tc.name WorkInfoTest005
 * @tc.desc Set workInfo charger condiiton discharging
 * @tc.type FUNC
 * @tc.require: SR000GGTN6 AR000GH896 AR000GH897 AR000GH898
 */
HWTEST_F (WorkInfoTest, WorkInfoTest005, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.RequestChargerType(false, WorkCondition::Charger::CHARGING_UNPLUGGED);
    EXPECT_EQ(workInfo.GetChargerType(), WorkCondition::Charger::CHARGING_UNPLUGGED);
}

/**
 * @tc.name WorkInfoTest006
 * @tc.desc Set workInfo battery condition battery changed
 * @tc.type FUNC
 * @tc.require: SR000GGTN6 AR000GH896 AR000GH897 AR000GH898
 */
HWTEST_F (WorkInfoTest, WorkInfoTest006, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
    EXPECT_EQ(workInfo.GetBatteryStatus(), WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
}

/**
 * @tc.name WorkInfoTest007
 * @tc.desc Set workInfo battery condition battery low
 * @tc.type FUNC
 * @tc.require: SR000GGTN6 AR000GH896 AR000GH897 AR000GH898
 */
HWTEST_F (WorkInfoTest, WorkInfoTest007, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
    EXPECT_EQ(workInfo.GetBatteryStatus(), WorkCondition::BatteryStatus::BATTERY_STATUS_LOW);
}

/**
 * @tc.name WorkInfoTest008
 * @tc.desc Set workInfo battery condition battery okey
 * @tc.type FUNC
 * @tc.require: SR000GGTN6 AR000GH896 AR000GH897 AR000GH898
 */
HWTEST_F (WorkInfoTest, WorkInfoTest008, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_OKAY);
    EXPECT_EQ(workInfo.GetBatteryStatus(), WorkCondition::BatteryStatus::BATTERY_STATUS_OKAY);
}

/**
 * @tc.name WorkInfoTest009
 * @tc.desc Set workInfo storage condition storage change
 * @tc.type FUNC
 * @tc.require: SR000GGTN9 AR000GH89M
 */
HWTEST_F (WorkInfoTest, WorkInfoTest009, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY);
    EXPECT_EQ(workInfo.GetStorageLevel(), WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY);
}

/**
 * @tc.name WorkInfoTest010
 * @tc.desc Set workInfo storage condition storage low
 * @tc.type FUNC
 * @tc.require: SR000GGTN8 AR000GH89J AR000GH89K AR000GH89L
 */
HWTEST_F (WorkInfoTest, WorkInfoTest010, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW);
    EXPECT_EQ(workInfo.GetStorageLevel(), WorkCondition::Storage::STORAGE_LEVEL_LOW);
}

/**
 * @tc.name WorkInfoTest011
 * @tc.desc Set workInfo storage condition storage ok
 * @tc.type FUNC
 * @tc.require: SR000GGTNB AR000GH89Q
 */
HWTEST_F (WorkInfoTest, WorkInfoTest011, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_OKAY);
    EXPECT_EQ(workInfo.GetStorageLevel(), WorkCondition::Storage::STORAGE_LEVEL_OKAY);
}

/**
 * @tc.name WorkInfoTest012
 * @tc.desc Set workInfo network condition
 * @tc.type FUNC
 * @tc.require: SR000GGTNA AR000GH89P AR000GH89N AR000GH89O
 */
HWTEST_F (WorkInfoTest, WorkInfoTest012, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    EXPECT_EQ(workInfo.GetNetworkType(), WorkCondition::Network::NETWORK_TYPE_ANY);
}

/**
 * @tc.name WorkInfoTest013
 * @tc.desc Set workInfo repeat time condition repeat count 3
 * @tc.type FUNC
 * @tc.require: SR000GGTNA AR000GH89P AR000GH89N AR000GH89O
 */
HWTEST_F (WorkInfoTest, WorkInfoTest013, Function | MediumTest | Level0)
{
    uint32_t timeInterval = 120;
    WorkInfo workInfo = WorkInfo();
    workInfo.RequestRepeatCycle(timeInterval, 3);
    EXPECT_EQ(workInfo.GetTimeInterval(), timeInterval);
    EXPECT_EQ(workInfo.IsRepeat(), false);
}

/**
 * @tc.name WorkInfoTest014
 * @tc.desc Set workInfo repeat time condition repeat
 * @tc.type FUNC
 * @tc.require: SR000GGTNA AR000GH89P AR000GH89N AR000GH89O
 */
HWTEST_F (WorkInfoTest, WorkInfoTest014, Function | MediumTest | Level0)
{
    uint32_t timeInterval = 120;
    WorkInfo workInfo = WorkInfo();
    workInfo.RequestRepeatCycle(timeInterval);
    EXPECT_EQ(workInfo.GetTimeInterval(), timeInterval);
    EXPECT_EQ(workInfo.IsRepeat(), true);
}
} // namespace WorkScheduler
} // namespace OHOS