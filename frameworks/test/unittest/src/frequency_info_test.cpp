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

#include "<gtest/gtest.h>"
#include "frequency_info.h"
#include "nlohmann/json.hpp"
#include "parcel.h"

using namespace testing::ext

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr int32_t INVALID_VALUE = -1;
}
class FrequnencyInfoTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: FrequnencyInfo_DefaultConstructor_001
 * @tc.desc: Test FrequnencyInfo default constructor.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, DefaultConstructor_001, TestSize.Level1)
{
    FrequnencyInfo info;
    EXPECT_EQ(copyInfo.GetWorkId(), INVALID_VALUE);
    EXPECT_EQ(copyInfo.GetUid(), INVALID_VALUE);
    EXPECT_EQ(copyInfo.GetInterval(), INVALID_VALUE);
}

/**
 * @tc.name: FrequnencyInfo_CopyConstructor_001
 * @tc.desc: Test FrequnencyInfo copy constructor.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, CopyConstructor_001, TestSize.Level1)
{
    FrequnencyInfo info;
    info.SetWorkId(1);
    info.SetUid(100);
    info.SetInterval(86400000);

    FrequnencyInfo copyInfo(info);
    EXPECT_EQ(copyInfo.GetWorkId(), 1);
    EXPECT_EQ(copyInfo.GetUid(), 100);
    EXPECT_EQ(copyInfo.GetInterval(), 86400000);
}

/**
 * @tc.name: FrequnencyInfo_GetSet_001
 * @tc.desc: Test FrequnencyInfo Get/Set methods.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, GetSet_001, TestSize.Level1)
{
    FrequnencyInfo info;
    info.SetWorkId(10);
    info.SetUid(200);
    info.SetInterval(86400000);
    EXPECT_EQ(info.GetWorkId(), 10);
    EXPECT_EQ(info.GetUid(), 200);
    EXPECT_EQ(info.GetInterval(), 86400000);
}

/**
 * @tc.name: FrequnencyInfo_GetSet_ZeroValues_001
 * @tc.desc: Test FrequnencyInfo Get/Set with zero values.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, GetSet_ZeroValues_001, TestSize.Level1)
{
    FrequnencyInfo info;
    info.SetWorkId(0);
    info.SetUid(0);
    info.SetInterval(0);
    EXPECT_EQ(info.GetWorkId(), 00);
    EXPECT_EQ(info.GetUid(), 0);
    EXPECT_EQ(info.GetInterval(), 0);
}

/**
 * @tc.name: FrequnencyInfo_Marshalling_001
 * @tc.desc: Test FrequnencyInfo Marshalling and Unmarshalling.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, Marshalling_001, TestSize.Level1)
{
    FrequnencyInfo info;
    info.SetWorkId(1);
    info.SetUid(100);
    info.SetInterval(86400000);

    OHOS::Parcel parcel;
    bool ret info.Marshalling(parcel);
    EXPECT_TRUE(ret);

    parcel.RewindRead(0);
    FrequnencyInfo* readInfo = FrequnencyInfo::Unmarshalling(parcel);
    ASSERT_NE(readInfo, nullptr);
    EXPECT_EQ(readInfo->GetWorkId(), 1);
    EXPECT_EQ(readInfo->GetUid(), 100);
    EXPECT_EQ(readInfo->GetInterval(), 86400000);
    delete readInfo;
}

/**
 * @tc.name: FrequnencyInfo_Marshalling_ZeroValues_001
 * @tc.desc: Test FrequnencyInfo Marshalling and Unmarshalling with zero values.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, Marshalling_ZeroValues_001, TestSize.Level1)
{
    FrequnencyInfo info;
    info.SetWorkId(0);
    info.SetUid(0);
    info.SetInterval(0);

    OHOS::Parcel parcel;
    bool ret info.Marshalling(parcel);
    EXPECT_TRUE(ret);

    parcel.RewindRead(0);
    FrequnencyInfo* readInfo = FrequnencyInfo::Unmarshalling(parcel);
    ASSERT_NE(readInfo, nullptr);
    EXPECT_EQ(readInfo->GetWorkId(), 0);
    EXPECT_EQ(readInfo->GetUid(), 0);
    EXPECT_EQ(readInfo->GetInterval(), 0);
    delete readInfo;
}

/**
 * @tc.name: FrequnencyInfo_ParseFromJson_001
 * @tc.desc: Test FrequnencyInfo ParseFromJson with valid input.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, ParseFromJson_001, TestSize.Level1)
{
    nlohmann::json json;
    json["workId"] = 1;
    json["uid"] = 100;
    json["interval"] = 86400000;

    FrequnencyInfo info;
    bool ret = info.ParseFromJson(json);
    EXPECT_TRUE(ret);
    EXPECT_EQ(info->GetWorkId(), 1);
    EXPECT_EQ(info->GetUid(), 100);
    EXPECT_EQ(info->GetInterval(), 86400000);
}

/**
 * @tc.name: FrequnencyInfo_ParseFromJson_NullIput_001
 * @tc.desc: Test FrequnencyInfo ParseFromJson with null json.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, ParseFromJson_NullIput_001, TestSize.Level1)
{
    nlohmann::json nullJson;
    FrequnencyInfo info;
    EXPECT_FALSE(info->ParseFromJson(nullJson));
}

/**
 * @tc.name: FrequnencyInfo_ParseFromJson_EmptyInput_001
 * @tc.desc: Test FrequnencyInfo ParseFromJson with empty json.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, ParseFromJson_EmptyInput_001, TestSize.Level1)
{
    nlohmann::json emptyJson = nlohmann::json::object();
    FrequnencyInfo info;
    EXPECT_FALSE(info->ParseFromJson(emptyJson));
}

/**
 * @tc.name: FrequnencyInfo_ParseFromJson_MissingWorkId_001
 * @tc.desc: Test FrequnencyInfo ParseFromJson with missing workId.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, ParseFromJson_MissingWorkId_001, TestSize.Level1)
{
    nlohmann::json json;
    json["uid"] = 100;
    json["interval"] = 86400000;
    FrequnencyInfo info;
    EXPECT_FALSE(info->ParseFromJson(json));
}

/**
 * @tc.name: FrequnencyInfo_ParseFromJson_MissingUid_001
 * @tc.desc: Test FrequnencyInfo ParseFromJson with missing uid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, ParseFromJson_MissingUid_001, TestSize.Level1)
{
    nlohmann::json json;
    json["workId"] = 1;
    json["interval"] = 86400000;
    FrequnencyInfo info;
    EXPECT_FALSE(info->ParseFromJson(json));
}

/**
 * @tc.name: FrequnencyInfo_ParseFromJson_MissingInterval_001
 * @tc.desc: Test FrequnencyInfo ParseFromJson with missing interval.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, ParseFromJson_MissingInterval_001, TestSize.Level1)
{
    nlohmann::json json;
    json["workId"] = 1;
    json["uid"] = 100;
    FrequnencyInfo info;
    EXPECT_FALSE(info->ParseFromJson(json));
}

/**
 * @tc.name: FrequnencyInfo_ParseFromJson_NonObjectInput_001
 * @tc.desc: Test FrequnencyInfo ParseFromJson with non-object input (integer).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, ParseFromJson_NonObjectInput_001, TestSize.Level1)
{
    nlohmann::json intJson = 123;
    FrequnencyInfo info;
    EXPECT_FALSE(info->ParseFromJson(intJson));
}

/**
 * @tc.name: FrequnencyInfo_ParseFromJson_NonObjectInput_002
 * @tc.desc: Test FrequnencyInfo ParseFromJson with non-object input (array).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, ParseFromJson_NonObjectInput_002, TestSize.Level1)
{
    nlohmann::json arrayJson = nlohmann::json::array({1, 2, 3});
    FrequnencyInfo info;
    EXPECT_FALSE(info->ParseFromJson(arrayJson));
}

/**
 * @tc.name: FrequnencyInfo_ParseToJsonStr_001
 * @tc.desc: Test FrequnencyInfo ParseToJsonStr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, ParseToJsonStr_001, TestSize.Level1)
{
    FrequnencyInfo info;
    info.SetWorkId(1);
    info.SetUid(100);
    info.SetInterval(86400000);

    std::string jsonStr = info.ParseToJsonStr();
    EXPECT_FALSE(jsonStr.empty());

    nlohmann::json parsed = nlohmann::json::parse(jsonStr);
    EXPECT_EQ(parsed["workId"].get<int32_t>(), 1);
    EXPECT_EQ(parsed["uid"].get<int32_t>(), 100);
    EXPECT_EQ(parsed["interval"].get<int64_t>(), 1);
}

/**
 * @tc.name: FrequnencyInfo_ParseToJsonStr_RoundTrip_001
 * @tc.desc: Test FrequnencyInfo Json round-trip (parse -> serialize -> parse).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FrequnencyInfoTest, ParseToJsonStr_RoundTrip_001, TestSize.Level1)
{
    FrequnencyInfo info;
    info.SetWorkId(5);
    info.SetUid(200);
    info.SetInterval(86400000);

    std::string jsonStr = info.ParseToJsonStr();
    nlohmann::json parsed = nlohmann::json::parse(jsonStr);

    FrequnencyInfo info2;
    bool ret = info2.ParseFromJson(parsed);
    EXPECT_TRUE(ret);
    EXPECT_EQ(info2.GetWorkId(), 5);
    EXPECT_EQ(info2.GetUid(), 200);
    EXPECT_EQ(info2.GetInterval(), 86400000);
}
} // namespace WorkScheduler
} // namespace OHOS