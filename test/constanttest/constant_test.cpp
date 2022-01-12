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

#define private public
#define protected public

#include "work_condition.h"

#undef private
#undef protected

#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class ConstantTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ConstantTest::SetUpTestCase()
{}

void ConstantTest::TearDownTestCase()
{}

void ConstantTest::SetUp()
{}

void ConstantTest::TearDown()
{}

/*
 * @tc.number: ConstantTest_GetNetworkType_001
 * @tc.name: Get Network Type
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_GetNetworkType_001, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::Network::NETWORK_TYPE_ANY, 0);
}

/*
 * @tc.number: ConstantTest_GetNetworkType_002
 * @tc.name: Get Network Type
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_GetNetworkType_002, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::Network::NETWORK_TYPE_MOBILE, 1);
}

/*
 * @tc.number: ConstantTest_GetNetworkType_003
 * @tc.name: Get Network Type
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_GetNetworkType_003, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::Network::NETWORK_TYPE_WIFI, 2);
}

/*
 * @tc.number: ConstantTest_GetNetworkType_004
 * @tc.name: Get Network Type
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_GetNetworkType_004, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::Network::NETWORK_TYPE_BLUETOOTH, 3);
}

/*
 * @tc.number: ConstantTest_GetNetworkType_005
 * @tc.name: Get Network Type
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_GetNetworkType_005, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::Network::NETWORK_TYPE_WIFI_P2P, 4);
}

/*
 * @tc.number: ConstantTest_GetNetworkType_006
 * @tc.name: Get Network Type
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_GetNetworkType_006, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::Network::NETWORK_TYPE_ETHERNET, 5);
}

/*
 * @tc.number: ConstantTest_ChargingType_001
 * @tc.name: Get Battery Status
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_ChargingType_001, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::Charger::CHARGER_PLUGGED_ANY, 0);
}

/*
 * @tc.number: ConstantTest_ChargingType_002
 * @tc.name: Get Battery Status
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_ChargingType_002, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::Charger::CHARGER_PLUGGED_AC, 1);
}

/*
 * @tc.number: ConstantTest_ChargingType_003
 * @tc.name: Get Battery Status
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_CHARGING_PLUGGED_USB_003, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::Charger::CHARGER_PLUGGED_USB, 2);
}

/*
 * @tc.number: ConstantTest_ChargingType_004
 * @tc.name: Get Battery Status
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_CHARGING_PLUGGED_WIRELESS_004, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::Charger::CHARGER_PLUGGED_WIRELESS, 3);
}

/*
 * @tc.number: ConstantTest_BatteryStatus_001
 * @tc.name: Get Battery Status
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_BatteryStatus_001, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW, 0);
}

/*
 * @tc.number: ConstantTest_BatteryStatus_002
 * @tc.name: Get Battery Status
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_BatteryStatus_002, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::BatteryStatus::BATTERY_STATUS_OKAY, 1);
}

/*
 * @tc.number: ConstantTest_BatteryStatus_003
 * @tc.name: Get Battery Status
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_BatteryStatus_003, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY, 2);
}

/*
 * @tc.number: ConstantTest_StorageRequest_001
 * @tc.name: Get Storage Request
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_StorageRequest_001, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::Storage::STORAGE_LEVEL_LOW, 0);
}

/*
 * @tc.number: ConstantTest_StorageRequest_002
 * @tc.name: Get Storage Request
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_StorageRequest_002, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::Storage::STORAGE_LEVEL_OKAY, 1);
}

/*
 * @tc.number: ConstantTest_StorageRequest_003
 * @tc.name: Get Storage Request
 * @tc.desc:
 */
HWTEST_F(ConstantTest, ConstantTest_StorageRequest_003, Function | MediumTest | Level0)
{
    EXPECT_EQ(WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY, 2);
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS