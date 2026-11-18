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
    BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(resData->payload);
    EXPECT_FALSE(resData->payload.contains("bundleName"));
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
    BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(payload);
    EXPECT_FALSE(payload.contains("bundleName"));
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
    BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(payload);
    EXPECT_FALSE(payload.contains("pid"));
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
    BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(payload);
    EXPECT_FALSE(payload.contains("uid"));
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
    BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(payload);
    EXPECT_FALSE(payload.contains("resourceNumber"));
}

/**
 * @tc.name: OnEfficiencyResourcesStateChanged_001
 * @tc.desc: Test OnEfficiencyResourcesStateChanged returns when observer is null.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedPluginMgrTest, OnEfficiencyResourcesStateChanged_001, TestSize.Level2)
{
    BackgroundTaskObserverPluginAdapter::GetInstance().UnInit();
    nlohmann::json payload = {
        {"bundleName", "test.bundle"},
        {"pid", 1234},
        {"uid", 10001},
        {"resourceNumber", 1}
    };
    BackgroundTaskObserverPluginAdapter::GetInstance().OnEfficiencyResourcesStateChanged(
        ResType::EfficiencyResourcesStatus::APP_EFFICIENCY_RESOURCES_APPLY, payload);
    EXPECT_EQ(BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(payload)->GetUid(), 10001);
    BackgroundTaskObserverPluginAdapter::GetInstance().Init();
}

/**
 * @tc.name: OnEfficiencyResourcesStateChanged_002
 * @tc.desc: Test OnEfficiencyResourcesStateChanged APP_EFFICIENCY_RESOURCES_APPLY.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedPluginMgrTest, OnEfficiencyResourcesStateChanged_002, TestSize.Level2)
{
    nlohmann::json payload = {
        {"bundleName", "test.bundle"},
        {"pid", 1234},
        {"uid", 10001},
        {"resourceNumber", 1}
    };
    BackgroundTaskObserverPluginAdapter::GetInstance().OnEfficiencyResourcesStateChanged(
        ResType::EfficiencyResourcesStatus::APP_EFFICIENCY_RESOURCES_APPLY, payload);
    EXPECT_EQ(BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(payload)->GetUid(), 10001);
}

/**
 * @tc.name: OnEfficiencyResourcesStateChanged_003
 * @tc.desc: Test OnEfficiencyResourcesStateChanged PROC_EFFICIENCY_RESOURCES_APPLY and RESET.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedPluginMgrTest, OnEfficiencyResourcesStateChanged_003, TestSize.Level2)
{
    nlohmann::json payload = {
        {"bundleName", "test.bundle"},
        {"pid", 1234},
        {"uid", 10001},
        {"resourceNumber", 1}
    };
    BackgroundTaskObserverPluginAdapter::GetInstance().OnEfficiencyResourcesStateChanged(
        ResType::EfficiencyResourcesStatus::PROC_EFFICIENCY_RESOURCES_APPLY, payload);
    BackgroundTaskObserverPluginAdapter::GetInstance().OnEfficiencyResourcesStateChanged(
        ResType::EfficiencyResourcesStatus::PROC_EFFICIENCY_RESOURCES_RESET, payload);
    EXPECT_EQ(BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(payload)->GetUid(), 10001);
}

/**
 * @tc.name: OnEfficiencyResourcesStateChanged_004
 * @tc.desc: Test OnEfficiencyResourcesStateChanged unknown stateType default case.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedPluginMgrTest, OnEfficiencyResourcesStateChanged_004, TestSize.Level2)
{
    nlohmann::json payload = {
        {"bundleName", "test.bundle"},
        {"pid", 1234},
        {"uid", 10001},
        {"resourceNumber", 1}
    };
    BackgroundTaskObserverPluginAdapter::GetInstance().OnEfficiencyResourcesStateChanged(9999, payload);
    EXPECT_EQ(BackgroundTaskObserverPluginAdapter::GetInstance().UnmarshallingResourceCallbackInfo(payload)->GetUid(), 10001);
}

/**
 * @tc.name: HandleCloudConfigUpdateEvent_001
 * @tc.desc: Test HandleCloudConfigUpdateEvent with SUSPEND_MANAGER_SYSTEM_ABILITY_ID.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedPluginMgrTest, HandleCloudConfigUpdateEvent_001, TestSize.Level2)
{
    nlohmann::json payload = nlohmann::json::object();
    BackgroundTaskObserverPluginAdapter::GetInstance().HandleCloudConfigUpdateEvent(4701, payload);
    EXPECT_FALSE(DelayedSingleton<WorkSchedulerConfig>::GetInstance()->IsInActiveGroupWhitelist("nonexistent"));
}

/**
 * @tc.name: HandleCloudConfigUpdateEvent_002
 * @tc.desc: Test HandleCloudConfigUpdateEvent with BACKGROUND_TASK_MANAGER_SERVICE_ID.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedPluginMgrTest, HandleCloudConfigUpdateEvent_002, TestSize.Level2)
{
    nlohmann::json payload;
    nlohmann::json params = nlohmann::json::object();
    nlohmann::json workScheduler = nlohmann::json::object();
    params["work_scheduler"] = workScheduler;
    payload["params"] = params;
    BackgroundTaskObserverPluginAdapter::GetInstance().HandleCloudConfigUpdateEvent(4705, payload);
    EXPECT_FALSE(DelayedSingleton<WorkSchedulerConfig>::GetInstance()->IsInActiveGroupWhitelist("nonexistent"));
}

/**
 * @tc.name: HandleCloudConfigUpdateEvent_003
 * @tc.desc: Test HandleCloudConfigUpdateEvent with unknown stateType.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedPluginMgrTest, HandleCloudConfigUpdateEvent_003, TestSize.Level2)
{
    nlohmann::json payload = nlohmann::json::object();
    BackgroundTaskObserverPluginAdapter::GetInstance().HandleCloudConfigUpdateEvent(9999, payload);
    EXPECT_FALSE(DelayedSingleton<WorkSchedulerConfig>::GetInstance()->IsInActiveGroupWhitelist("nonexistent"));
}

/**
 * @tc.name: OnPluginInit_001
 * @tc.desc: Test extern C OnPluginInit with matching lib name.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedPluginMgrTest, OnPluginInit_001, TestSize.Level2)
{
    WorkSchedPluginMgr::GetInstance().Disable();
    std::string libName = "libworkschedservice.z.so";
    bool ret = OnPluginInit(libName);
    EXPECT_TRUE(ret);
    WorkSchedPluginMgr::GetInstance().Disable();
}

/**
 * @tc.name: OnPluginInit_002
 * @tc.desc: Test extern C OnPluginInit with non-matching lib name.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedPluginMgrTest, OnPluginInit_002, TestSize.Level2)
{
    std::string libName = "libinvalid.z.so";
    bool ret = OnPluginInit(libName);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: OnPluginDisable_001
 * @tc.desc: Test extern C OnPluginDisable.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedPluginMgrTest, OnPluginDisable_001, TestSize.Level2)
{
    WorkSchedPluginMgr::GetInstance().Init();
    OnPluginDisable();
    EXPECT_FALSE(WorkSchedPluginMgr::GetInstance().pluginEnable_.load());
}

/**
 * @tc.name: OnDispatchResource_001
 * @tc.desc: Test extern C OnDispatchResource with null data.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedPluginMgrTest, OnDispatchResource_001, TestSize.Level2)
{
    WorkSchedPluginMgr::GetInstance().Init();
    OnDispatchResource(nullptr);
    EXPECT_TRUE(WorkSchedPluginMgr::GetInstance().pluginEnable_.load());
    WorkSchedPluginMgr::GetInstance().Disable();
}

/**
 * @tc.name: OnDispatchResource_002
 * @tc.desc: Test extern C OnDispatchResource with RSS_CLOUD_CONFIG_UPDATE.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(WorkSchedPluginMgrTest, OnDispatchResource_002, TestSize.Level2)
{
    WorkSchedPluginMgr::GetInstance().Init();
    auto resData = std::make_shared<ResourceSchedule::ResData>();
    resData->resType = ResType::RES_TYPE_RSS_CLOUD_CONFIG_UPDATE;
    resData->value = 4705;
    resData->payload = nlohmann::json::object();
    OnDispatchResource(resData);
    EXPECT_TRUE(WorkSchedPluginMgr::GetInstance().pluginEnable_.load());
    WorkSchedPluginMgr::GetInstance().Disable();
}
} // namespace WorkScheduler
} // namespace OHOS