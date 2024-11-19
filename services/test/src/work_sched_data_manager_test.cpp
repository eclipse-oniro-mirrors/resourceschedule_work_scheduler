/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "work_sched_data_manager.h"
#include "work_scheduler_service.h"
#include "work_sched_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class DataManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<DataManager> dataManager_;
};

std::shared_ptr<DataManager> DataManagerTest::dataManager_ = nullptr;

void DataManagerTest::SetUpTestCase()
{
    dataManager_ = DelayedSingleton<DataManager>::GetInstance();
}

/**
 * @tc.name: AddDeviceStandyWhitelist_001
 * @tc.desc: Test DataManager AddDeviceStandyWhitelist.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, AddDeviceStandyWhitelist_001, TestSize.Level1)
{
    dataManager_->ClearDeviceStandyWhitelist();
    std::list<std::string> bundleNames = { "bundleName1", "bundleName2", "bundleName3" };
    dataManager_->AddDeviceStandyWhitelist(bundleNames);
    EXPECT_FALSE(dataManager_->IsDeviceStandyWhitelistEmpty());
}

/**
 * @tc.name: IsInDeviceStandyWhitelist_001
 * @tc.desc: Test DataManager IsInDeviceStandyWhitelist.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, IsInDeviceStandyWhitelist_001, TestSize.Level1)
{
    dataManager_->ClearDeviceStandyWhitelist();
    std::list<std::string> bundleNames = { "bundleName1", "bundleName2", "bundleName3" };
    dataManager_->AddDeviceStandyWhitelist(bundleNames);
    EXPECT_FALSE(dataManager_->IsInDeviceStandyWhitelist("bundleName4"));
}

/**
 * @tc.name: OnDeviceStandyWhitelistChanged_001
 * @tc.desc: Test DataManager OnDeviceStandyWhitelistChanged.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, OnDeviceStandyWhitelistChanged_001, TestSize.Level1)
{
    dataManager_->ClearDeviceStandyWhitelist();
    dataManager_->OnDeviceStandyWhitelistChanged("bundleName1", true);
    EXPECT_FALSE(dataManager_->IsDeviceStandyWhitelistEmpty());
}

/**
 * @tc.name: OnDeviceStandyWhitelistChanged_002
 * @tc.desc: Test DataManager OnDeviceStandyWhitelistChanged.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, OnDeviceStandyWhitelistChanged_002, TestSize.Level1)
{
    dataManager_->ClearDeviceStandyWhitelist();
    dataManager_->OnDeviceStandyWhitelistChanged("bundleName1", true);
    dataManager_->OnDeviceStandyWhitelistChanged("bundleName1", false);
    EXPECT_TRUE(dataManager_->IsDeviceStandyWhitelistEmpty());
}

/**
 * @tc.name: AddGroup_001
 * @tc.desc: Test DataManager AddGroup.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, AddGroup_001, TestSize.Level1)
{
    dataManager_->ClearAllGroup();
    int32_t userId = 202220;
    int32_t appGroup = 20;
    dataManager_->AddGroup("bundleName1", userId, appGroup);
    EXPECT_TRUE(dataManager_->FindGroup("bundleName1", userId, appGroup));
}

/**
 * @tc.name: AddGroup_002
 * @tc.desc: Test DataManager AddGroup.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, AddGroup_002, TestSize.Level1)
{
    dataManager_->ClearAllGroup();
    int32_t userId = 202220;
    int32_t appGroup = 20;
    dataManager_->AddGroup("bundleName1", userId, appGroup);
    dataManager_->ClearGroup("bundleName1", userId);
    EXPECT_FALSE(dataManager_->FindGroup("bundleName1", userId, appGroup));
}

/**
 * @tc.name: SetDeviceSleep_001
 * @tc.desc: Test DataManager SetDeviceSleep.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, SetDeviceSleep_001, TestSize.Level1)
{
    dataManager_->SetDeviceSleep(false);
    EXPECT_FALSE(dataManager_->GetDeviceSleep());
}
}
}