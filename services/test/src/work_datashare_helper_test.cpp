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

#include <functional>
#include <gtest/gtest.h>

#include "work_datashare_helper.h"
#include "work_sched_hilog.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class WorkDatashareHelperTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: GetInstance_001
 * @tc.desc: Test WorkDatashareHelper GetInstance returns same reference (singleton).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkDatashareHelperTest, GetInstance_001, TestSize.Level1)
{
    auto& instance1 = WorkDatashareHelper::GetInstance();
    auto& instance2 = WorkDatashareHelper::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @tc.name: GetStringValue_001
 * @tc.desc: Test WorkDatashareHelper GetStringValue returns false when helper is null.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkDatashareHelperTest, GetStringValue_001, TestSize.Level1)
{
    std::string value;
    bool result = WorkDatashareHelper::GetInstance().GetStringValue("test_key", value);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetStringValue_002
 * @tc.desc: Test WorkDatashareHelper GetStringValue with empty key returns false.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkDatashareHelperTest, GetStringValue_002, TestSize.Level1)
{
    std::string value = "initial";
    bool result = WorkDatashareHelper::GetInstance().GetStringValue("", value);
    EXPECT_EQ(result, false);
    EXPECT_EQ(value, "initial");
}
} // namespace WorkScheduler
} // namespace OHOS
