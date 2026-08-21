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
#include "work_sched_plugin_mgr.h"
#include "res_data.h"
#include "res_type.h"
#include "background_task_observer_plugin_adapter.h"

using namespace testing::ext;
using namespace OHOS::ResourceSchedule;

namespace OHOS {
namespace WorkScheduler {

class WorkSchedPluginMgrTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: WorkSchedPluginMgrTest_001
 * @tc.desc: test Init method Disable method
 * @tc.type: FUNC
 */
HWTEST_F(WorkSchedPluginMgrTest, WorkSchedPluginMgrTest_001, TestSize.Level2)
{
    auto resData = std::make_shared<ResourceSchedule::ResData>();
    WorkSchedPluginMgr::GetInstance().Init();
    WorkSchedPluginMgr::GetInstance().DispatchResource(resData);
    EXPECT_TRUE(WorkSchedPluginMgr::GetInstance().pluginEnable_.load());
    WorkSchedPluginMgr::GetInstance().Disable();
    WorkSchedPluginMgr::GetInstance().DispatchResource(resData);
    EXPECT_FALSE(WorkSchedPluginMgr::GetInstance().pluginEnable_.load());
}

/**
 * @tc.name: WorkSchedPluginMgrTest_002
 * @tc.desc: test DispatchResource with RES_TYPE_EFFICIENCY_RESOURCES_STATE_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(WorkSchedPluginMgrTest, WorkSchedPluginMgrTest_002, TestSize.Level2)
{
    WorkSchedPluginMgr::GetInstance().Init();
    auto resData = std::make_shared<ResourceSchedule::ResData>();
    resData->resType = 999;
    WorkSchedPluginMgr::GetInstance().DispatchResource(resData);
    resData->resType = ResType::RES_TYPE_EFFICIENCY_RESOURCES_STATE_CHANGED;
    resData->value = 1;
    resData->payload = {{"key", "value"}};

    WorkSchedPluginMgr::GetInstance().DispatchResource(resData);
    auto result =
        BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(resData->payload);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: WorkSchedPluginMgrTest_003
 * @tc.desc: test DispatchResource with valid EfficiencyResourcesStatus
 * @tc.type: FUNC
 */
HWTEST_F(WorkSchedPluginMgrTest, WorkSchedPluginMgrTest_008, TestSize.Level2)
{
    WorkSchedPluginMgr::GetInstance().Init();
    auto resData = std::make_shared<ResourceSchedule::ResData>();
    resData->resType = ResType::RES_TYPE_EFFICIENCY_RESOURCES_STATE_CHANGED;
    resData->value = ResType::EfficiencyResourcesStatus::APP_EFFICIENCY_RESOURCES_RESET;
    resData->payload = {
        {"bundleName", "test.bundle"},
        {"pid", 1234},
        {"uid", 10001},
        {"resourceNumber", 1}
    };
    WorkSchedPluginMgr::GetInstance().DispatchResource(resData);
    auto result =
        BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(resData->payload);
    resData->value = ResType::EfficiencyResourcesStatus::PROC_EFFICIENCY_RESOURCES_APPLY;
    result =
        BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(resData->payload);
    resData->value = ResType::EfficiencyResourcesStatus::PROC_EFFICIENCY_RESOURCES_RESET;
    result =
        BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(resData->payload);
    resData->value = ResType::EfficiencyResourcesStatus::APP_EFFICIENCY_RESOURCES_APPLY;
    result =
        BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(resData->payload);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name: BackgroundTaskObserverPluginAdapter_001
 * @tc.desc: test UnmarshallingResourceCallbackInfo with missing bundleName
 * @tc.type: FUNC
 */
HWTEST_F(WorkSchedPluginMgrTest, BackgroundTaskObserverPluginAdapter_001, TestSize.Level2)
{
    nlohmann::json payload = {
        {"pid", 1234},
        {"uid", 10001},
        {"resourceNumber", 1}
    };
    auto result = BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(payload);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: BackgroundTaskObserverPluginAdapter_002
 * @tc.desc: test UnmarshallingResourceCallbackInfo with missing pid
 * @tc.type: FUNC
 */
HWTEST_F(WorkSchedPluginMgrTest, BackgroundTaskObserverPluginAdapter_002, TestSize.Level2)
{
    nlohmann::json payload = {
        {"bundleName", "test.bundle"},
        {"uid", 10001},
        {"resourceNumber", 1}
    };
    auto result = BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(payload);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: BackgroundTaskObserverPluginAdapter_003
 * @tc.desc: test UnmarshallingResourceCallbackInfo with missing uid
 * @tc.type: FUNC
 */
HWTEST_F(WorkSchedPluginMgrTest, BackgroundTaskObserverPluginAdapter_003, TestSize.Level2)
{
    nlohmann::json payload = {
        {"bundleName", "test.bundle"},
        {"pid", 1234},
        {"resourceNumber", 1}
    };
    auto result = BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(payload);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: BackgroundTaskObserverPluginAdapter_004
 * @tc.desc: test UnmarshallingResourceCallbackInfo with missing resourceNumber
 * @tc.type: FUNC
 */
HWTEST_F(WorkSchedPluginMgrTest, BackgroundTaskObserverPluginAdapter_004, TestSize.Level2)
{
    nlohmann::json payload = {
        {"bundleName", "test.bundle"},
        {"pid", 1234},
        {"uid", 10001}
    };
    auto result = BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(payload);
    EXPECT_EQ(result, nullptr);
}
}  // namespace WorkScheduler
}  // namespace OHOS