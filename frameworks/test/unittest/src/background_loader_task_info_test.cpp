/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "background_loader_task_info.h"

using namespace testing::ext;
using namespace OHOS::WorkScheduler;

class BackgroundLoaderTaskInfoTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: BackgroundLoaderTaskInfo_DefaultConstructor_001
 * @tc.desc: Test BackgroundLoaderTaskInfo default constructor.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderTaskInfoTest, DefaultConstructor_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo info;
    EXPECT_EQ(info.GetTaskId(), 0);
    EXPECT_EQ(info.GetAbilityName(), "");
}

/**
 * @tc.name: BackgroundLoaderTaskInfo_ParameterizedConstructor_001
 * @tc.desc: Test BackgroundLoaderTaskInfo parameterized constructor.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderTaskInfoTest, ParameterizedConstructor_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo info(100, "TestAbility");
    EXPECT_EQ(info.GetTaskId(), 100);
    EXPECT_EQ(info.GetAbilityName(), "TestAbility");
}

/**
 * @tc.name: BackgroundLoaderTaskInfo_SetTaskId_001
 * @tc.desc: Test BackgroundLoaderTaskInfo SetTaskId.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderTaskInfoTest, SetTaskId_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo info;
    info.SetTaskId(200);
    EXPECT_EQ(info.GetTaskId(), 200);
}

/**
 * @tc.name: BackgroundLoaderTaskInfo_SetAbilityName_001
 * @tc.desc: Test BackgroundLoaderTaskInfo SetAbilityName.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderTaskInfoTest, SetAbilityName_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo info;
    info.SetAbilityName("MyAbility");
    EXPECT_EQ(info.GetAbilityName(), "MyAbility");
}

/**
 * @tc.name: BackgroundLoaderTaskInfo_SetTaskId_Negative_001
 * @tc.desc: Test BackgroundLoaderTaskInfo SetTaskId with negative value.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderTaskInfoTest, SetTaskId_Negative_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo info;
    info.SetTaskId(-1);
    EXPECT_EQ(info.GetTaskId(), -1);
}

/**
 * @tc.name: BackgroundLoaderTaskInfo_SetAbilityName_Empty_001
 * @tc.desc: Test BackgroundLoaderTaskInfo SetAbilityName with empty string.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderTaskInfoTest, SetAbilityName_Empty_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo info(1, "Ability");
    info.SetAbilityName("");
    EXPECT_EQ(info.GetAbilityName(), "");
}

/**
 * @tc.name: BackgroundLoaderTaskInfo_Marshalling_001
 * @tc.desc: Test BackgroundLoaderTaskInfo Marshalling.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderTaskInfoTest, Marshalling_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo info(100, "TestAbility");
    OHOS::Parcel parcel;
    bool ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: BackgroundLoaderTaskInfo_Marshalling_ReadBack_001
 * @tc.desc: Test BackgroundLoaderTaskInfo Marshalling and read back values.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderTaskInfoTest, Marshalling_ReadBack_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo info(100, "TestAbility");
    OHOS::Parcel parcel;
    bool ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);
    EXPECT_EQ(parcel.ReadInt32(), 100);
    EXPECT_EQ(parcel.ReadString(), "TestAbility");
}

/**
 * @tc.name: BackgroundLoaderTaskInfo_Unmarshalling_001
 * @tc.desc: Test BackgroundLoaderTaskInfo Unmarshalling.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderTaskInfoTest, Unmarshalling_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo info(100, "TestAbility");
    OHOS::Parcel parcel;
    bool ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);

    BackgroundLoaderTaskInfo* result = BackgroundLoaderTaskInfo::Unmarshalling(parcel);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->GetTaskId(), 100);
    EXPECT_EQ(result->GetAbilityName(), "TestAbility");
    delete result;
}

/**
 * @tc.name: BackgroundLoaderTaskInfo_Unmarshalling_ZeroValues_001
 * @tc.desc: Test BackgroundLoaderTaskInfo Unmarshalling with zero taskId and empty abilityName.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderTaskInfoTest, Unmarshalling_ZeroValues_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo info(0, "");
    OHOS::Parcel parcel;
    bool ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);

    BackgroundLoaderTaskInfo* result = BackgroundLoaderTaskInfo::Unmarshalling(parcel);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->GetTaskId(), 0);
    EXPECT_EQ(result->GetAbilityName(), "");
    delete result;
}

/**
 * @tc.name: BackgroundLoaderTaskInfo_MarshallingUnmarshalling_RoundTrip_001
 * @tc.desc: Test BackgroundLoaderTaskInfo Marshalling and Unmarshalling round trip.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderTaskInfoTest, MarshallingUnmarshalling_RoundTrip_001, TestSize.Level1)
{
    for (int32_t i = -100; i <= 100; i += 50) {
        BackgroundLoaderTaskInfo info(i, "Ability_" + std::to_string(i));
        OHOS::Parcel parcel;
        bool ret = info.Marshalling(parcel);
        EXPECT_TRUE(ret);
        BackgroundLoaderTaskInfo* result = BackgroundLoaderTaskInfo::Unmarshalling(parcel);
        EXPECT_NE(result, nullptr);
        EXPECT_EQ(result->GetTaskId(), i);
        EXPECT_EQ(result->GetAbilityName(), "Ability_" + std::to_string(i));
        delete result;
    }
}

/**
 * @tc.name: BackgroundLoaderTaskInfo_Marshalling_LongAbilityName_001
 * @tc.desc: Test BackgroundLoaderTaskInfo Marshalling with long abilityName.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderTaskInfoTest, Marshalling_LongAbilityName_001, TestSize.Level1)
{
    std::string longName(256, 'A');
    BackgroundLoaderTaskInfo info(1, longName);
    OHOS::Parcel parcel;
    bool ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);

    BackgroundLoaderTaskInfo* result = BackgroundLoaderTaskInfo::Unmarshalling(parcel);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->GetAbilityName(), longName);
    delete result;
}