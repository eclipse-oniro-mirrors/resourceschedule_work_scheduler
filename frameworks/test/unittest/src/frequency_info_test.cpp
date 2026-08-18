/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "frequency_info.h"
#include "nlohmann/json.hpp"
#include "parcel.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr int32_t INVALID_VALUE = -1;
}
class FrequencyInfoTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: FrequencyInfo_DefaultConstructor_001
 * @tc.desc: Test FrequencyInfo default constructor.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, DefaultConstructor_001, TestSize.Level1)
{
    FrequencyInfo info;
    EXPECT_EQ(info.GetWorkId(), INVALID_VALUE);
    EXPECT_EQ(info.GetUid(), INVALID_VALUE);
    EXPECT_EQ(info.GetInterval(), INVALID_VALUE);
}

/**
 * @tc.name: FrequencyInfo_CopyConstructor_001
 * @tc.desc: Test FrequencyInfo copy constructor.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, CopyConstructor_001, TestSize.Level1)
{
    FrequencyInfo info;
    info.SetWorkId(1);
    info.SetUid(100);
    info.SetInterval(86400000);

    FrequencyInfo copyInfo(info);
    EXPECT_EQ(copyInfo.GetWorkId(), 1);
    EXPECT_EQ(copyInfo.GetUid(), 100);
    EXPECT_EQ(copyInfo.GetInterval(), 86400000);
}

/**
 * @tc.name: FrequencyInfo_GetSet_001
 * @tc.desc: Test FrequencyInfo Get/Set methods.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, GetSet_001, TestSize.Level1)
{
    FrequencyInfo info;
    info.SetWorkId(10);
    info.SetUid(200);
    info.SetInterval(86400000);
    EXPECT_EQ(info.GetWorkId(), 10);
    EXPECT_EQ(info.GetUid(), 200);
    EXPECT_EQ(info.GetInterval(), 86400000);
}

/**
 * @tc.name: FrequencyInfo_GetSet_ZeroValues_001
 * @tc.desc: Test FrequencyInfo Get/Set with zero values.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, GetSet_ZeroValues_001, TestSize.Level1)
{
    FrequencyInfo info;
    info.SetWorkId(0);
    info.SetUid(0);
    info.SetInterval(0);
    EXPECT_EQ(info.GetWorkId(), 0);
    EXPECT_EQ(info.GetUid(), 0);
    EXPECT_EQ(info.GetInterval(), 0);
}

/**
 * @tc.name: FrequencyInfo_Marshalling_001
 * @tc.desc: Test FrequencyInfo Marshalling and Unmarshalling.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, Marshalling_001, TestSize.Level1)
{
    FrequencyInfo info;
    info.SetWorkId(1);
    info.SetUid(100);
    info.SetInterval(86400000);

    OHOS::Parcel parcel;
    bool ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);

    parcel.RewindRead(0);
    FrequencyInfo* readInfo = FrequencyInfo::Unmarshalling(parcel);
    ASSERT_NE(readInfo, nullptr);
    EXPECT_EQ(readInfo->GetWorkId(), 1);
    EXPECT_EQ(readInfo->GetUid(), 100);
    EXPECT_EQ(readInfo->GetInterval(), 86400000);
    delete readInfo;
}

/**
 * @tc.name: FrequencyInfo_Marshalling_ZeroValues_001
 * @tc.desc: Test FrequencyInfo Marshalling and Unmarshalling with zero values.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, Marshalling_ZeroValues_001, TestSize.Level1)
{
    FrequencyInfo info;
    info.SetWorkId(0);
    info.SetUid(0);
    info.SetInterval(0);

    OHOS::Parcel parcel;
    bool ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);

    parcel.RewindRead(0);
    FrequencyInfo* readInfo = FrequencyInfo::Unmarshalling(parcel);
    ASSERT_NE(readInfo, nullptr);
    EXPECT_EQ(readInfo->GetWorkId(), 0);
    EXPECT_EQ(readInfo->GetUid(), 0);
    EXPECT_EQ(readInfo->GetInterval(), 0);
    delete readInfo;
}

/**
 * @tc.name: FrequencyInfo_ParseFromJson_001
 * @tc.desc: Test FrequencyInfo ParseFromJson with valid input.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, ParseFromJson_001, TestSize.Level1)
{
    nlohmann::json json;
    json["workId"] = 1;
    json["uid"] = 100;
    json["interval"] = 86400000;

    FrequencyInfo info;
    bool ret = info.ParseFromJson(json);
    EXPECT_TRUE(ret);
    EXPECT_EQ(info.GetWorkId(), 1);
    EXPECT_EQ(info.GetUid(), 100);
    EXPECT_EQ(info.GetInterval(), 86400000);
}

/**
 * @tc.name: FrequencyInfo_ParseFromJson_NullInput_001
 * @tc.desc: Test FrequencyInfo ParseFromJson with null json.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, ParseFromJson_NullInput_001, TestSize.Level1)
{
    nlohmann::json nullJson;
    FrequencyInfo info;
    EXPECT_FALSE(info.ParseFromJson(nullJson));
}

/**
 * @tc.name: FrequencyInfo_ParseFromJson_EmptyInput_001
 * @tc.desc: Test FrequencyInfo ParseFromJson with empty json.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, ParseFromJson_EmptyInput_001, TestSize.Level1)
{
    nlohmann::json emptyJson = nlohmann::json::object();
    FrequencyInfo info;
    EXPECT_FALSE(info.ParseFromJson(emptyJson));
}

/**
 * @tc.name: FrequencyInfo_ParseFromJson_MissingWorkId_001
 * @tc.desc: Test FrequencyInfo ParseFromJson with missing workId.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, ParseFromJson_MissingWorkId_001, TestSize.Level1)
{
    nlohmann::json json;
    json["uid"] = 100;
    json["interval"] = 86400000;
    FrequencyInfo info;
    EXPECT_FALSE(info.ParseFromJson(json));
}

/**
 * @tc.name: FrequencyInfo_ParseFromJson_MissingUid_001
 * @tc.desc: Test FrequencyInfo ParseFromJson with missing uid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, ParseFromJson_MissingUid_001, TestSize.Level1)
{
    nlohmann::json json;
    json["workId"] = 1;
    json["interval"] = 86400000;
    FrequencyInfo info;
    EXPECT_FALSE(info.ParseFromJson(json));
}

/**
 * @tc.name: FrequencyInfo_ParseFromJson_MissingInterval_001
 * @tc.desc: Test FrequencyInfo ParseFromJson with missing interval.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, ParseFromJson_MissingInterval_001, TestSize.Level1)
{
    nlohmann::json json;
    json["workId"] = 1;
    json["uid"] = 100;
    FrequencyInfo info;
    EXPECT_FALSE(info.ParseFromJson(json));
}

/**
 * @tc.name: FrequencyInfo_ParseFromJson_NonObjectInput_001
 * @tc.desc: Test FrequencyInfo ParseFromJson with non-object input (integer).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, ParseFromJson_NonObjectInput_001, TestSize.Level1)
{
    nlohmann::json intJson = 123;
    FrequencyInfo info;
    EXPECT_FALSE(info.ParseFromJson(intJson));
}

/**
 * @tc.name: FrequencyInfo_ParseFromJson_NonObjectInput_002
 * @tc.desc: Test FrequencyInfo ParseFromJson with non-object input (array).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, ParseFromJson_NonObjectInput_002, TestSize.Level1)
{
    nlohmann::json arrayJson = nlohmann::json::array({1, 2, 3});
    FrequencyInfo info;
    EXPECT_FALSE(info.ParseFromJson(arrayJson));
}

/**
 * @tc.name: FrequencyInfo_ParseToJsonStr_001
 * @tc.desc: Test FrequencyInfo ParseToJsonStr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, ParseToJsonStr_001, TestSize.Level1)
{
    FrequencyInfo info;
    info.SetWorkId(1);
    info.SetUid(100);
    info.SetInterval(86400000);

    std::string jsonStr = info.ParseToJsonStr();
    EXPECT_FALSE(jsonStr.empty());

    nlohmann::json parsed = nlohmann::json::parse(jsonStr);
    EXPECT_EQ(parsed["workId"].get<int32_t>(), 1);
    EXPECT_EQ(parsed["uid"].get<int32_t>(), 100);
    EXPECT_EQ(parsed["interval"].get<int64_t>(), 86400000);
}

/**
 * @tc.name: FrequencyInfo_ParseToJsonStr_RoundTrip_001
 * @tc.desc: Test FrequencyInfo JSON round-trip (parse -> serialize -> parse).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequencyInfoTest, ParseToJsonStr_RoundTrip_001, TestSize.Level1)
{
    FrequencyInfo info1;
    info1.SetWorkId(5);
    info1.SetUid(200);
    info1.SetInterval(86400000);

    std::string jsonStr = info1.ParseToJsonStr();
    nlohmann::json parsed = nlohmann::json::parse(jsonStr);

    FrequencyInfo info2;
    bool ret = info2.ParseFromJson(parsed);
    EXPECT_TRUE(ret);
    EXPECT_EQ(info2.GetWorkId(), 5);
    EXPECT_EQ(info2.GetUid(), 200);
    EXPECT_EQ(info2.GetInterval(), 86400000);
}
} // namespace WorkScheduler
} // namespace OHOS