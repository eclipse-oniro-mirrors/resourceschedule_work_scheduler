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
#include "background_loader/ability_connect_callback.h"
#include "background_loader/background_loader_mgr.h"
#include "element_name.h"

using namespace testing::ext;
namespace OHOS {
namespace WorkScheduler {
class AbilityConnectCallbackTest : public testing::Test {
public:
    void SetUp() override
    {
        BackgroundLoaderMgr::GetInstance().Init(BACKGROUND_LOADER_TIMEOUT_COUNT, BACKGROUND_LOADER_TIMEOUT_US);
    };
    void TearDown() override
    {
        auto& mgr = BackgroundLoaderMgr::GetInstance();
        {
            std::lock_guard<ffrt::mutex> lock(mgr.abilityMapLock_);
            mgr.abilityMap_.clear();
        }
        {
            std::lock_guard<ffrt::mutex> lock(mgr.taskLock_);
            mgr.taskMap_.clear();
        }
    };

    void ClearAbilityMap()
    {
        auto& mgr = BackgroundLoaderMgr::GetInstance();
        std::lock_guard<ffrt::mutex> lock(mgr.abilityMapLock_);
        mgr.abilityMap_.clear();
    }

    void ClearTaskMap()
    {
        auto& mgr = BackgroundLoaderMgr::GetInstance();
        std::lock_guard<ffrt::mutex> lock(mgr.taskLock_);
        mgr.taskMap_.clear();
    }
};

/**
 * @tc.name: OnAbilityConnectDone_NullRemote_001
 * @tc.desc: Test OnAbilityConnectDone returns early when remoteObject is null.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectCallbackTest, OnAbilityConnectDone_NullRemote_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    ClearAbilityMap();

    AbilityConnectCallback callback("com.test.bundle", "TestAbility", 0, 1001);
    AppExecFwk::ElementName element;
    callback.OnAbilityConnectDone(element, nullptr, 0);
    EXPECT_TRUE(mgr.abilityMap_.empty());
}

/**
 * @tc.name: OnAbilityDisconnectDone_001
 * @tc.desc: Test OnAbilityDisconnectDone removes the cached remote object.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectCallbackTest, OnAbilityDisconnectDone_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    ClearAbilityMap();
    const std::string bundleName = "com.test.bundle";
    sptr<IRemoteObject> remoteObject = new AbilityConnectCallback(bundleName, "TestAbility", 0, 1001);
    mgr.SaveRemoteObjectLocked(bundleName, "TestAbility", 0, remoteObject);
    EXPECT_EQ(mgr.abilityMap_.count("com.test.bundle_0"), 1);

    AbilityConnectCallback callback(bundleName, "TestAbility", 0, 1001);
    AppExecFwk::ElementName element;
    callback.OnAbilityDisconnectDone(element, 0);
    EXPECT_EQ(mgr.abilityMap_.count("com.test.bundle_0"), 0);
}

/**
 * @tc.name: OnAbilityConnectDone_ValidRemote_001
 * @tc.desc: Test OnAbilityConnectDone keeps the cached object when pid is invalid.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectCallbackTest, OnAbilityConnectDone_ValidRemote_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    ClearAbilityMap();
    ClearTaskMap();

    TaskInfo task;
    task.bundleName_ = "com.test.bundle";
    task.abilityName_ = "TestAbility";
    task.appIndex_ = 0;
    task.taskId_ = 1;
    task.status_ = TaskStatus::RUNNING;
    task.uid_ = 9001;
    {
        std::lock_guard<ffrt::mutex> lock(mgr.taskLock_);
        mgr.taskMap_[9001] = task;
    }

    sptr<IRemoteObject> cached = new AbilityConnectCallback("com.test.bundle", "TestAbility", 0, 9001);
    mgr.SaveRemoteObjectLocked("com.test.bundle", "TestAbility", 0, cached);

    AbilityConnectCallback callback("com.test.bundle", "TestAbility", 0, 9001);
    AppExecFwk::ElementName element;
    element.SetBundleName("com.test.bundle");
    element.SetAbilityName("TestAbility");
    sptr<IRemoteObject> remoteObject = new AbilityConnectCallback("com.test.bundle", "TestAbility", 0, 9001);
    callback.OnAbilityConnectDone(element, remoteObject, 0);

    EXPECT_EQ(mgr.GetRemoteObjectLocked("com.test.bundle", "TestAbility", 0).GetRefPtr(), cached.GetRefPtr());
}

/**
 * @tc.name: OnAbilityDisconnectDone_NoEntry_001
 * @tc.desc: Test OnAbilityDisconnectDone removes by key and keeps other entries.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectCallbackTest, OnAbilityDisconnectDone_NoEntry_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    ClearAbilityMap();
    sptr<IRemoteObject> other = new AbilityConnectCallback("com.other.bundle", "TestAbility", 0, 1001);
    mgr.SaveRemoteObjectLocked("com.other.bundle", "TestAbility", 0, other);

    AbilityConnectCallback callback("com.nobody", "TestAbility", 0, 1001);
    AppExecFwk::ElementName element;
    callback.OnAbilityDisconnectDone(element, 0);

    EXPECT_EQ(mgr.abilityMap_.count("com.other.bundle_0"), 1);
}

/**
 * @tc.name: OnAbilityDisconnectDone_NonZeroResultCode_001
 * @tc.desc: Test OnAbilityDisconnectDone removes the entry regardless of resultCode.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectCallbackTest, OnAbilityDisconnectDone_NonZeroResultCode_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    ClearAbilityMap();
    sptr<IRemoteObject> remoteObject = new AbilityConnectCallback("com.test.bundle", "TestAbility", 0, 1001);
    mgr.SaveRemoteObjectLocked("com.test.bundle", "TestAbility", 0, remoteObject);

    AbilityConnectCallback callback("com.test.bundle", "TestAbility", 0, 1001);
    AppExecFwk::ElementName element;
    callback.OnAbilityDisconnectDone(element, 7);

    EXPECT_EQ(mgr.abilityMap_.count("com.test.bundle_0"), 0);
}
} // namespace WorkScheduler
} // namespace OHOS