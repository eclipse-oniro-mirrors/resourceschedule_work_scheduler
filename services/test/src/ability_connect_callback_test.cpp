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
#include "ability_connect_callback.h"
#include "background_loader_mgr.h"

using namespace testing::ext;
namespace OHOS {
namespace WorkScheduler {
class AbilityConnectCallbackTest : public testing::Test {
public:
    void SetUp() override
    {
        BackgroundLoaderMgr::GetInstance().Init(BACKGROUND_LOADER_TIMEOUT_COUNT, BACKGROUND_LOADER_TIMEOUT_MS);
    };
    void TearDown() override {};
};

HWTEST_F(AbilityConnectCallbackTest, OnAbilityConnectDone_ResultCodeError_001, TestSize.Level1)
{
    std::string bundleName = "com.test.bundle";
    std::string abilityName = "TestAbility";
    int32_t appIndex = 0;
    sptr<AbilityConnectCallback> callback = new AbilityConnectCallback(bundleName, abilityName, appIndex);
    AppExecFwk::ElementName element;
    element.SetBundleName(bundleName);
    element.SetAbilityName(abilityName);
    sptr<IRemoteObject> remoteObject = nullptr;
    callback->OnAbilityConnectDone(element, remoteObject, -1);
    sptr<IRemoteObject> result = BackgroundLoaderMgr::GetInstance().GetRemoteObject(bundleName, abilityName, appIndex);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(AbilityConnectCallbackTest, OnAbilityConnectDone_NullRemoteObject_001, TestSize.Level1)
{
    std::string bundleName = "com.test.bundle2";
    std::string abilityName = "TestAbility2";
    int32_t appIndex = 0;
    sptr<AbilityConnectCallback> callback = new AbilityConnectCallback(bundleName, abilityName, appIndex);
    AppExecFwk::ElementName element;
    element.SetBundleName(bundleName);
    element.SetAbilityName(abilityName);
    sptr<IRemoteObject> remoteObject = nullptr;
    callback->OnAbilityConnectDone(element, remoteObject, ERR_OK);
    sptr<IRemoteObject> result = BackgroundLoaderMgr::GetInstance().GetRemoteObject(bundleName, abilityName, appIndex);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(AbilityConnectCallbackTest, OnAbilityDisconnectDone_001, TestSize.Level1)
{
    std::string bundleName = "com.test.bundle";
    std::string abilityName = "TestAbility";
    int32_t appIndex = 0;
    sptr<AbilityConnectCallback> callback = new AbilityConnectCallback(bundleName, abilityName, appIndex);
    AppExecFwk::ElementName element;
    element.SetBundleName(bundleName);
    element.SetAbilityName(abilityName);
    callback->OnAbilityDisconnectDone(element, ERR_OK);
    sptr<IRemoteObject> result = BackgroundLoaderMgr::GetInstance().GetRemoteObject(bundleName, abilityName, appIndex);
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(AbilityConnectCallbackTest, Constructor_001, TestSize.Level1)
{
    std::string bundleName = "com.test.bundle";
    std::string abilityName = "TestAbility";
    int32_t appIndex = 0;
    sptr<AbilityConnectCallback> callback = new AbilityConnectCallback(bundleName, abilityName, appIndex);
    EXPECT_NE(callback, nullptr);
}

HWTEST_F(AbilityConnectCallbackTest, Constructor_DifferentAppIndex_001, TestSize.Level1)
{
    sptr<AbilityConnectCallback> callback0 = new AbilityConnectCallback("com.test.bundle", "Ability0", 0);
    sptr<AbilityConnectCallback> callback1 = new AbilityConnectCallback("com.test.bundle", "Ability1", 1);
    EXPECT_NE(callback0, nullptr);
    EXPECT_NE(callback1, nullptr);
}
}
}