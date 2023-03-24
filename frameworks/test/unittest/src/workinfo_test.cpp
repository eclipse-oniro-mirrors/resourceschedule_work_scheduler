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
#include "workinfo_test.h"
#include <message_parcel.h>

#include "work_info.h"
#include "work_sched_common.h"
#include "work_sched_utils.h"
#include "string_wrapper.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
const int32_t INVALID_VALUE = -1;
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
    EXPECT_EQ(workInfo.GetChargerType(), WorkCondition::Charger::CHARGING_UNKNOWN);
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
    EXPECT_EQ(workInfo.GetBatteryStatus(), WorkCondition::BatteryStatus::BATTERY_UNKNOWN);
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
    EXPECT_EQ(workInfo.GetStorageLevel(), WorkCondition::Storage::STORAGE_UNKNOWN);
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
    EXPECT_EQ(workInfo.GetNetworkType(), WorkCondition::Network::NETWORK_UNKNOWN);
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
    EXPECT_EQ(workInfo.GetTimeInterval(), 0);
    EXPECT_EQ(workInfo.GetCycleCount(), INVALID_VALUE);
    workInfo.RequestRepeatCycle(timeInterval, 3);
    EXPECT_EQ(workInfo.GetTimeInterval(), timeInterval);
    EXPECT_EQ(workInfo.IsRepeat(), false);
    EXPECT_EQ(workInfo.GetCycleCount(), 3);
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
    EXPECT_EQ(workInfo.IsRepeat(), false);
    workInfo.RequestRepeatCycle(timeInterval);
    EXPECT_EQ(workInfo.GetTimeInterval(), timeInterval);
    EXPECT_EQ(workInfo.IsRepeat(), true);
    EXPECT_EQ(workInfo.GetCycleCount(), INVALID_VALUE);
}

/**
 * @tc.name WorkInfoTest015
 * @tc.desc Set workInfo battery level
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkInfoTest015, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    EXPECT_EQ(workInfo.GetBatteryLevel(), INVALID_VALUE);
    workInfo.RequestBatteryLevel(1);
    EXPECT_EQ(workInfo.GetBatteryLevel(), 1);
}

/**
 * @tc.name WorkInfoTest016
 * @tc.desc Set workInfo RefreshUid
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkInfoTest016, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.RefreshUid(0);
    EXPECT_EQ(workInfo.GetUid(), 0);
}

/**
 * @tc.name WorkInfoTest017
 * @tc.desc Set workInfo extras
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkInfoTest017, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    AAFwk::WantParams extras;
    workInfo.RequestExtras(extras);
    auto extrasRead = workInfo.GetExtras();
    if (extras == *extrasRead.get()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_FALSE(true);
    }
}

/**
 * @tc.name WorkInfoTest018
 * @tc.desc Set workInfo GetConditionMap
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkInfoTest018, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.GetConditionMap();
    std::string result;
    workInfo.Dump(result);
    EXPECT_NE(result, "");
}

/**
 * @tc.name WorkInfoTestJson001
 * @tc.desc Set workInfo json func
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkInfoTestJson001, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    std::string res = workInfo.ParseToJsonStr();

    workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_OKAY);
    workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_ANY);
    workInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW);
    workInfo.RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    workInfo.RequestBatteryLevel(1);
    uint32_t timeInterval = 120;
    workInfo.RequestRepeatCycle(timeInterval);
    res = workInfo.ParseToJsonStr();

    workInfo.RefreshUid(1);
    workInfo.RequestRepeatCycle(timeInterval, 3);
    AAFwk::WantParams extras;
    extras.SetParam("key1", OHOS::AAFwk::String::Box("value1"));
    workInfo.RequestExtras(extras);
    res = workInfo.ParseToJsonStr();

    std::shared_ptr<Condition> condUnknown = std::make_shared<Condition>();
    condUnknown->boolVal = true;
    workInfo.conditionMap_.emplace(WorkCondition::Type::UNKNOWN, condUnknown);
    res = workInfo.ParseToJsonStr();
    EXPECT_NE(res, "");
}

/**
 * @tc.name WorkInfoTestJson002
 * @tc.desc Set workInfo json func
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkInfoTestJson002, Function | MediumTest | Level0)
{
    using namespace OHOS::WorkScheduler;
    WorkInfo workInfo = WorkInfo();
    Json::Value root;
    bool res = workInfo.ParseFromJson(root);
    EXPECT_FALSE(res);

    root["workId"] = 1;
    res = workInfo.ParseFromJson(root);
    EXPECT_FALSE(res);

    root.clear();
    root["bundleName"] = "bundleName";
    res = workInfo.ParseFromJson(root);
    EXPECT_FALSE(res);

    root.clear();
    root["abilityName"] = "abilityName";
    res = workInfo.ParseFromJson(root);
    EXPECT_FALSE(res);

    root.clear();
    root["workId"] = 1;
    root["bundleName"] = "bundleName";
    res = workInfo.ParseFromJson(root);
    EXPECT_FALSE(res);

    root.clear();
    root["workId"] = 1;
    root["abilityName"] = "abilityName";
    res = workInfo.ParseFromJson(root);
    EXPECT_FALSE(res);

    root.clear();
    root["bundleName"] = "bundleName";
    root["abilityName"] = "abilityName";
    res = workInfo.ParseFromJson(root);
    EXPECT_FALSE(res);

    root["workId"] = 1;
    root["persisted"] = false;
    res = workInfo.ParseFromJson(root);
    EXPECT_TRUE(res);
}

/**
 * @tc.name WorkInfoTestJson003
 * @tc.desc Set workInfo json func
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkInfoTestJson003, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    Json::Value root;
    root["workId"] = 1;
    root["bundleName"] = "bundleName";
    root["abilityName"] = "abilityName";
    root["uid"] = 1;
    Json::Value conditions;
    conditions["network"] = 0;
    conditions["isCharging"] = true;
    conditions["batteryLevel"] = 0;
    conditions["batteryStatus"] = 0;
    conditions["storage"] = 0;
    conditions["timer"] = 0;
    root["conditions"] = conditions;
    bool res = workInfo.ParseFromJson(root);
    EXPECT_TRUE(res);

    conditions.clear();
    conditions["chargerType"] = 0;
    conditions["repeat"] = true;
    root["conditions"] = conditions;
    res = workInfo.ParseFromJson(root);
    EXPECT_TRUE(res);

    conditions.clear();
    conditions["isCharging"] = true;
    conditions["chargerType"] = 0;
    conditions["timer"] = 0;
    conditions["repeat"] = true;
    conditions["cycle"] = 0;
    root["conditions"] = conditions;
    res = workInfo.ParseFromJson(root);
    EXPECT_TRUE(res);

    conditions.clear();
    conditions["timer"] = 0;
    conditions["repeat"] = true;
    root["conditions"] = conditions;
    res = workInfo.ParseFromJson(root);
    EXPECT_TRUE(res);

    conditions.clear();
    conditions["timer"] = 0;
    conditions["repeat"] = false;
    root["conditions"] = conditions;
    res = workInfo.ParseFromJson(root);
    EXPECT_TRUE(res);
}

/**
 * @tc.name WorkInfoTestJson004
 * @tc.desc Set workInfo json func
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkInfoTestJson004, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    Json::Value root;
    root["workId"] = 1;
    root["bundleName"] = "bundleName";
    root["abilityName"] = "abilityName";
    root["uid"] = 1;
    Json::Value extrasJson;
    extrasJson["key1"] = "value1";
    extrasJson["key2"] = "value2";
    root["parameters"] = extrasJson;
    Json::Value extrasType;
    extrasType["key1"] = 9;
    extrasType["key2"] = -1;
    root["parametersType"] = extrasType;
    bool res = workInfo.ParseFromJson(root);
    EXPECT_TRUE(res);
}

/**
 * @tc.name WorkInfoTest019
 * @tc.desc Set workInfo Marshalling and Unmarshalling
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkInfoTest019, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.SetElement("bundle_name", "ability_name");
    workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_OKAY);
    MessageParcel data;
    WRITE_PARCEL_WITHOUT_RET(data, Parcelable, &workInfo);
    sptr<WorkInfo> workInfoRead = data.ReadStrongParcelable<WorkInfo>();
    EXPECT_EQ(workInfo.GetBatteryStatus(), workInfoRead->GetBatteryStatus());

    workInfo.RequestChargerType(true, WorkCondition::Charger::CHARGING_PLUGGED_ANY);
    WRITE_PARCEL_WITHOUT_RET(data, Parcelable, &workInfo);
    workInfoRead = data.ReadStrongParcelable<WorkInfo>();
    EXPECT_EQ(workInfo.GetChargerType(), workInfoRead->GetChargerType());

    workInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW);
    WRITE_PARCEL_WITHOUT_RET(data, Parcelable, &workInfo);
    workInfoRead = data.ReadStrongParcelable<WorkInfo>();
    EXPECT_EQ(workInfo.GetStorageLevel(), workInfoRead->GetStorageLevel());

    workInfo.RequestNetworkType(WorkCondition::Network::NETWORK_TYPE_ANY);
    WRITE_PARCEL_WITHOUT_RET(data, Parcelable, &workInfo);
    workInfoRead = data.ReadStrongParcelable<WorkInfo>();
    EXPECT_EQ(workInfo.GetNetworkType(), workInfoRead->GetNetworkType());

    workInfo.RequestBatteryLevel(1);
    WRITE_PARCEL_WITHOUT_RET(data, Parcelable, &workInfo);
    workInfoRead = data.ReadStrongParcelable<WorkInfo>();
    EXPECT_EQ(workInfo.GetBatteryLevel(), workInfoRead->GetBatteryLevel());

    uint32_t timeInterval = 120;
    workInfo.RequestRepeatCycle(timeInterval);
    WRITE_PARCEL_WITHOUT_RET(data, Parcelable, &workInfo);
    workInfoRead = data.ReadStrongParcelable<WorkInfo>();
    EXPECT_EQ(workInfo.GetTimeInterval(), workInfoRead->GetTimeInterval());

    workInfo.RequestRepeatCycle(timeInterval, 3);
    WRITE_PARCEL_WITHOUT_RET(data, Parcelable, &workInfo);
    workInfoRead = data.ReadStrongParcelable<WorkInfo>();
    EXPECT_EQ(workInfo.GetTimeInterval(), workInfoRead->GetTimeInterval());
}

/**
 * @tc.name WorkInfoTest020
 * @tc.desc Set workInfo Marshalling and Unmarshalling with extras.
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkInfoTest020, Function | MediumTest | Level0)
{
    WorkInfo workInfo = WorkInfo();
    workInfo.SetElement("bundle_name", "ability_name");
    workInfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_OKAY);
    MessageParcel data;
    AAFwk::WantParams extras;
    workInfo.RequestExtras(extras);
    WRITE_PARCEL_WITHOUT_RET(data, Parcelable, &workInfo);
    sptr<WorkInfo> workInfoRead = data.ReadStrongParcelable<WorkInfo>();
    EXPECT_EQ(workInfo.GetBundleName(), workInfoRead->GetBundleName());

    extras.SetParam("key1", OHOS::AAFwk::String::Box("value1"));
    workInfo.RequestExtras(extras);
    WRITE_PARCEL_WITHOUT_RET(data, Parcelable, &workInfo);
    workInfoRead = data.ReadStrongParcelable<WorkInfo>();
    std::string valueRead = workInfoRead->GetExtras()->GetStringParam("key1");
    EXPECT_EQ(valueRead, "value1");

    std::shared_ptr<Condition> condUnknown = std::make_shared<Condition>();
    condUnknown->boolVal = true;
    workInfo.conditionMap_.emplace(WorkCondition::Type::UNKNOWN, condUnknown);
    WRITE_PARCEL_WITHOUT_RET(data, Parcelable, &workInfo);
    workInfoRead = data.ReadStrongParcelable<WorkInfo>();
    auto condRead = workInfoRead->GetConditionMap();
    EXPECT_EQ(workInfoRead->GetConditionMap()->size(), 2);
}

/**
 * @tc.name WorkSchedUtils001
 * @tc.desc test GetCurrentAccountId
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkSchedUtils001, Function | MediumTest | Level0)
{
    int32_t res = WorkSchedUtils::GetCurrentAccountId();
    EXPECT_NE(res, -1000);
}

/**
 * @tc.name WorkSchedUtils002
 * @tc.desc test IsIdActive
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkSchedUtils002, Function | MediumTest | Level0)
{
    bool res = WorkSchedUtils::IsIdActive(0);
    EXPECT_FALSE(res);
}

/**
 * @tc.name WorkSchedUtils003
 * @tc.desc test GetUserIdByUid
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkSchedUtils003, Function | MediumTest | Level0)
{
    int32_t res = WorkSchedUtils::GetUserIdByUid(1);
    EXPECT_EQ(res, 0);
    res = WorkSchedUtils::GetUserIdByUid(INVALID_VALUE);
    EXPECT_EQ(res, INVALID_VALUE);
}

/**
 * @tc.name WorkSchedUtils004
 * @tc.desc test ConvertFullPath
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkSchedUtils004, Function | MediumTest | Level0)
{
    std::string partialPath;
    std::string fullPath;
    bool res = WorkSchedUtils::ConvertFullPath(partialPath, fullPath);
    EXPECT_FALSE(res);

    std::string partialPathLong(4097, 'c');
    res = WorkSchedUtils::ConvertFullPath(partialPathLong, fullPath);
    EXPECT_FALSE(res);

    partialPath = "partialPath";
    res = WorkSchedUtils::ConvertFullPath(partialPath, fullPath);
    EXPECT_FALSE(res);

    partialPath = "/data";
    res = WorkSchedUtils::ConvertFullPath(partialPath, fullPath);
    EXPECT_TRUE(res);
}

/**
 * @tc.name WorkSchedUtils005
 * @tc.desc test WorkSchedHilog
 * @tc.type FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F (WorkInfoTest, WorkSchedUtils005, Function | MediumTest | Level0)
{
    WorkSchedLogLevel curLevel = {WorkSchedLogLevel::INFO};
    WorkSchedHilog::SetLogLevel(curLevel);
    bool res = WorkSchedHilog::JudgeLevel({WorkSchedLogLevel::DEBUG});
    EXPECT_EQ(res, false);
    WorkSchedHilog::GetBriefFileName(nullptr);
}
} // namespace WorkScheduler
} // namespace OHOS