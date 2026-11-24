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

/**
 * @tc.name: SetDeviceSleep_002
 * @tc.desc: Test DataManager SetDeviceSleep true and GetDeviceSleep.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, SetDeviceSleep_002, TestSize.Level1)
{
    dataManager_->SetDeviceSleep(true);
    EXPECT_TRUE(dataManager_->GetDeviceSleep());
}

/**
 * @tc.name: SetDeepIdle_001
 * @tc.desc: Test DataManager SetDeepIdle and GetDeepIdle.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, SetDeepIdle_001, TestSize.Level1)
{
    dataManager_->SetDeepIdle(true);
    EXPECT_TRUE(dataManager_->GetDeepIdle());
    dataManager_->SetDeepIdle(false);
    EXPECT_FALSE(dataManager_->GetDeepIdle());
}

/**
 * @tc.name: IsInDeviceStandyWhitelist_002
 * @tc.desc: Test DataManager IsInDeviceStandyWhitelist returns true for existing bundle.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, IsInDeviceStandyWhitelist_002, TestSize.Level1)
{
    dataManager_->ClearDeviceStandyWhitelist();
    dataManager_->OnDeviceStandyWhitelistChanged("bundleName1", true);
    EXPECT_TRUE(dataManager_->IsInDeviceStandyWhitelist("bundleName1"));
}

/**
 * @tc.name: AddDeviceStandyRestrictlist_001
 * @tc.desc: Test DataManager AddDeviceStandyRestrictlist.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, AddDeviceStandyRestrictlist_001, TestSize.Level1)
{
    dataManager_->ClearDeviceStandyRestrictlist();
    std::list<std::string> bundleNames = { "restrict1", "restrict2" };
    dataManager_->AddDeviceStandyRestrictlist(bundleNames);
    EXPECT_TRUE(dataManager_->IsInDeviceStandyRestrictlist("restrict1"));
    EXPECT_TRUE(dataManager_->IsInDeviceStandyRestrictlist("restrict2"));
}

/**
 * @tc.name: IsInDeviceStandyRestrictlist_001
 * @tc.desc: Test DataManager IsInDeviceStandyRestrictlist returns false for non-existing.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, IsInDeviceStandyRestrictlist_001, TestSize.Level1)
{
    dataManager_->ClearDeviceStandyRestrictlist();
    EXPECT_FALSE(dataManager_->IsInDeviceStandyRestrictlist("nonexist"));
}

/**
 * @tc.name: OnDeviceStandyRestrictlistChanged_001
 * @tc.desc: Test DataManager OnDeviceStandyRestrictlistChanged add and remove.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, OnDeviceStandyRestrictlistChanged_001, TestSize.Level1)
{
    dataManager_->ClearDeviceStandyRestrictlist();
    dataManager_->OnDeviceStandyRestrictlistChanged("restrict1", true);
    EXPECT_TRUE(dataManager_->IsInDeviceStandyRestrictlist("restrict1"));
    dataManager_->OnDeviceStandyRestrictlistChanged("restrict1", false);
    EXPECT_FALSE(dataManager_->IsInDeviceStandyRestrictlist("restrict1"));
}

/**
 * @tc.name: ClearDeviceStandyRestrictlist_001
 * @tc.desc: Test DataManager ClearDeviceStandyRestrictlist.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, ClearDeviceStandyRestrictlist_001, TestSize.Level1)
{
    std::list<std::string> bundleNames = { "r1", "r2", "r3" };
    dataManager_->AddDeviceStandyRestrictlist(bundleNames);
    dataManager_->ClearDeviceStandyRestrictlist();
    EXPECT_FALSE(dataManager_->IsInDeviceStandyRestrictlist("r1"));
}

/**
 * @tc.name: AddGroup_003
 * @tc.desc: Test DataManager AddGroup updates existing key with new group.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(DataManagerTest, AddGroup_003, TestSize.Level1)
{
    dataManager_->ClearAllGroup();
    int32_t userId = 202221;
    dataManager_->AddGroup("bundleName1", userId, 10);
    int32_t group1 = 0;
    dataManager_->FindGroup("bundleName1", userId, group1);
    EXPECT_EQ(group1, 10);
    dataManager_->AddGroup("bundleName1", userId, 50);
    int32_t group2 = 0;
    dataManager_->FindGroup("bundleName1", userId, group2);
    EXPECT_EQ(group2, 50);
}
}
}