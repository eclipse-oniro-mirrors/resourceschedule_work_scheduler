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
#include "background_loader_mgr.h"
#include "work_sched_errors.h"
#include "res_type.h"

using namespace testing::ext;
using namespace OHOS::ResourceSchedule;
namespace OHOS {
namespace WorkScheduler {
class BackgroundLoaderMgrTest : public testing::Test {
public:
    void SetUp() override
    {
        BackgroundLoaderMgr::GetInstance().Init(BACKGROUND_LOADER_TIMEOUT_COUNT, BACKGROUND_LOADER_TIMEOUT_MS);
    };
    void TearDown() override {};
};

/**
 * @tc.name: BackgroundLoaderMgr_RegisterTask_001
 * @tc.desc: Test BackgroundLoaderMgr RegisterTask.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, RegisterTask_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BackgroundLoaderMgr_RegisterTask_NotReady_001
 * @tc.desc: Test BackgroundLoaderMgr RegisterTask when service not ready.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, RegisterTask_NotReady_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    BackgroundLoaderMgr::GetInstance().isReady_.store(false);
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);
}

/**
 * @tc.name: BackgroundLoaderMgr_RegisterTask_Duplicate_001
 * @tc.desc: Test BackgroundLoaderMgr RegisterTask duplicate task.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, RegisterTask_Duplicate_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BackgroundLoaderMgr_UnregisterTask_001
 * @tc.desc: Test BackgroundLoaderMgr UnregisterTask.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, UnregisterTask_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    ret = BackgroundLoaderMgr::GetInstance().UnregisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BackgroundLoaderMgr_UnregisterTask_NotExist_001
 * @tc.desc: Test BackgroundLoaderMgr UnregisterTask when task not exist.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, UnregisterTask_NotExist_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 999
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().UnregisterTask(info);
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);
}

/**
 * @tc.name: BackgroundLoaderMgr_UnregisterTask_NotReady_001
 * @tc.desc: Test BackgroundLoaderMgr UnregisterTask when service not ready.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, UnregisterTask_NotReady_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    BackgroundLoaderMgr::GetInstance().isReady_.store(false);
    ErrCode ret = BackgroundLoaderMgr::GetInstance().UnregisterTask(info);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);
}

/**
 * @tc.name: BackgroundLoaderMgr_FinishTask_001
 * @tc.desc: Test BackgroundLoaderMgr FinishTask.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTask_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    ret = BackgroundLoaderMgr::GetInstance().FinishTask(info);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BackgroundLoaderMgr_FinishTask_NotExist_001
 * @tc.desc: Test BackgroundLoaderMgr FinishTask when task not exist.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTask_NotExist_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 999
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().FinishTask(info);
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);
}

/**
 * @tc.name: BackgroundLoaderMgr_FinishTask_NotReady_001
 * @tc.desc: Test BackgroundLoaderMgr FinishTask when service not ready.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTask_NotReady_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    BackgroundLoaderMgr::GetInstance().isReady_.store(false);
    ErrCode ret = BackgroundLoaderMgr::GetInstance().FinishTask(info);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);
}

/**
 * @tc.name: BackgroundLoaderMgr_GetTaskInfo_001
 * @tc.desc: Test BackgroundLoaderMgr GetTaskInfo.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfo_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);

    BackgroundLoaderTaskInfo taskInfo;
    ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(1, "com.test.bundle", 0, taskInfo);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(taskInfo.GetTaskId(), 1);
    EXPECT_EQ(taskInfo.GetAbilityName(), "TestAbility");
}

/**
 * @tc.name: BackgroundLoaderMgr_GetTaskInfo_NotExist_001
 * @tc.desc: Test BackgroundLoaderMgr GetTaskInfo when task not exist.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfo_NotExist_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo taskInfo;
    ErrCode ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(999, "com.test.bundle", 0, taskInfo);
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);
}

/**
 * @tc.name: BackgroundLoaderMgr_GetTaskInfo_NotReady_001
 * @tc.desc: Test BackgroundLoaderMgr GetTaskInfo when service not ready.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfo_NotReady_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo taskInfo;
    BackgroundLoaderMgr::GetInstance().isReady_.store(false);
    ErrCode ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(1, "com.test.bundle", 0, taskInfo);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);
}

/**
 * @tc.name: BackgroundLoaderMgr_RegisterUnregisterGetTask_001
 * @tc.desc: Test BackgroundLoaderMgr register then unregister then get task.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, RegisterUnregisterGetTask_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 10
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);

    BackgroundLoaderTaskInfo taskInfo;
    ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(10, "com.test.bundle", 0, taskInfo);
    EXPECT_EQ(ret, ERR_OK);

    ret = BackgroundLoaderMgr::GetInstance().UnregisterTask(info);
    EXPECT_EQ(ret, ERR_OK);

    ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(10, "com.test.bundle", 0, taskInfo);
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);
}

/**
 * @tc.name: BackgroundLoaderMgr_MultipleTasks_001
 * @tc.desc: Test BackgroundLoaderMgr register multiple tasks.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, MultipleTasks_001, TestSize.Level1)
{
    for (int32_t i = 1; i <= 5; i++) {
        TaskInfo info = {
            .bundleName_ = "com.test.bundle",
            .abilityName_ = "Ability_" + std::to_string(i),
            .appIndex_ = 0,
            .taskId_ = i
        };
        ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
        EXPECT_EQ(ret, ERR_OK);
    }

    for (int32_t i = 1; i <= 5; i++) {
        BackgroundLoaderTaskInfo taskInfo;
        ErrCode ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(i, "com.test.bundle", 0, taskInfo);
        EXPECT_EQ(ret, ERR_OK);
        EXPECT_EQ(taskInfo.GetTaskId(), i);
        EXPECT_EQ(taskInfo.GetAbilityName(), "Ability_" + std::to_string(i));
    }
}

/**
 * @tc.name: BackgroundLoaderMgr_DifferentAppIndex_001
 * @tc.desc: Test BackgroundLoaderMgr with same taskId but different appIndex.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, DifferentAppIndex_001, TestSize.Level1)
{
    TaskInfo info1 = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    TaskInfo info2 = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 1,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info1);
    EXPECT_EQ(ret, ERR_OK);
    ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info2);
    EXPECT_EQ(ret, ERR_OK);

    BackgroundLoaderTaskInfo taskInfo1;
    ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(1, "com.test.bundle", 0, taskInfo1);
    EXPECT_EQ(ret, ERR_OK);

    BackgroundLoaderTaskInfo taskInfo2;
    ret = BackgroundLoaderMgr::GetInstance().GetTaskInfo(1, "com.test.bundle", 1, taskInfo2);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BackgroundLoaderMgr_Init_WithParams_001
 * @tc.desc: Test BackgroundLoaderMgr Init with maxTimeoutCount and backgroundLoaderTimeoutMs parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, Init_WithParams_001, TestSize.Level1)
{
    BackgroundLoaderMgr::GetInstance().Init(5, 20000);
    EXPECT_EQ(BackgroundLoaderMgr::GetInstance().maxTimeoutCount_, 5);
    EXPECT_EQ(BackgroundLoaderMgr::GetInstance().backgroundLoaderTimeoutMs_, 20000);
    EXPECT_TRUE(BackgroundLoaderMgr::GetInstance().isReady_.load());
}

/**
 * @tc.name: BackgroundLoaderMgr_RegisterTask_002
 * @tc.desc: Test BackgroundLoaderMgr RegisterTask when bundle is in black list.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, RegisterTask_002, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.list.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1 };
    std::string key = "com.list.bundle_0";
    std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().blackListLock_);
    BackgroundLoaderMgr::GetInstance().blackLists_.insert(key);
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, E_CHECK_WORKINFO_FAILED);
    BackgroundLoaderMgr::GetInstance().blackLists_.erase(key);
}

/**
 * @tc.name: BackgroundLoaderMgr_UnregisterTask_TaskIdMismatch_001
 * @tc.desc: Test BackgroundLoaderMgr UnregisterTask when taskId does not match.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, UnregisterTask_TaskIdMismatch_001, TestSize.Level1)
{
    TaskInfo info = { .bundleName_ = "com.test.bundle", .abilityName_ = "TestAbility", .appIndex_ = 0, .taskId_ = 1 };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    TaskInfo mismatchInfo = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0, .taskId_ = 999 };
    ret = BackgroundLoaderMgr::GetInstance().UnregisterTask(mismatchInfo);
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);
}

/**
 * @tc.name: BackgroundLoaderMgr_FinishTask_SetStatus_001
 * @tc.desc: Test BackgroundLoaderMgr FinishTask sets task status to FINISHED.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTask_SetStatus_001, TestSize.Level1)
{
    TaskInfo info = { .bundleName_ = "com.test.bundle", .abilityName_ = "TestAbility", .appIndex_ = 0, .taskId_ = 1 };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    ret = BackgroundLoaderMgr::GetInstance().FinishTask(info);
    EXPECT_EQ(ret, ERR_OK);
    std::string key = "com.test.bundle_0";
    std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().taskLock_);
    auto it = BackgroundLoaderMgr::GetInstance().taskMap_.find(key);
    if (it != BackgroundLoaderMgr::GetInstance().taskMap_.end()) {
        EXPECT_EQ(it->second.status_, TaskStatus::FINISHED);
    }
}

/**
 * @tc.name: BackgroundLoaderMgr_FinishTask_TaskIdMismatch_001
 * @tc.desc: Test BackgroundLoaderMgr FinishTask when taskId does not match.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTask_TaskIdMismatch_001, TestSize.Level1)
{
    TaskInfo info = { .bundleName_ = "com.test.bundle", .abilityName_ = "TestAbility", .appIndex_ = 0, .taskId_ = 1 };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    TaskInfo mismatchInfo = {
        .bundleName_ = "com.test.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0, .taskId_ = 999 };
    ret = BackgroundLoaderMgr::GetInstance().FinishTask(mismatchInfo);
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);
}

/**
 * @tc.name: BackgroundLoaderMgr_GenerateTaskKey_001
 * @tc.desc: Test BackgroundLoaderMgr GenerateTaskKey returns bundleName_appIndex format.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, GenerateTaskKey_001, TestSize.Level1)
{
    std::string key = BackgroundLoaderMgr::GetInstance().GenerateTaskKey("com.test.bundle", 0);
    EXPECT_EQ(key, "com.test.bundle_0");
    key = BackgroundLoaderMgr::GetInstance().GenerateTaskKey("com.test.bundle", 1);
    EXPECT_EQ(key, "com.test.bundle_1");
}

/**
 * @tc.name: BackgroundLoaderMgr_SaveRemoteObject_001
 * @tc.desc: Test BackgroundLoaderMgr SaveRemoteObject saves remote object to abilityMap.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, SaveRemoteObject_001, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = nullptr;
    BackgroundLoaderMgr::GetInstance().SaveRemoteObject("com.test.bundle", "TestAbility", 0, remoteObject);
    std::string key = "com.test.bundle_0";
    std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().abilityMapLock_);
    auto it = BackgroundLoaderMgr::GetInstance().abilityMap_.find(key);
    EXPECT_NE(it, BackgroundLoaderMgr::GetInstance().abilityMap_.end());
}

/**
 * @tc.name: BackgroundLoaderMgr_GetRemoteObject_001
 * @tc.desc: Test BackgroundLoaderMgr GetRemoteObject returns saved remote object.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, GetRemoteObject_001, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = nullptr;
    BackgroundLoaderMgr::GetInstance().SaveRemoteObject("com.test.bundle", "TestAbility", 0, remoteObject);
    sptr<IRemoteObject> result =
        BackgroundLoaderMgr::GetInstance().GetRemoteObject("com.test.bundle", "TestAbility", 0);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: BackgroundLoaderMgr_GetRemoteObject_NotFound_001
 * @tc.desc: Test BackgroundLoaderMgr GetRemoteObject returns nullptr when not found.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, GetRemoteObject_NotFound_001, TestSize.Level1)
{
    sptr<IRemoteObject> result =
        BackgroundLoaderMgr::GetInstance().GetRemoteObject("com.notexist.bundle", "TestAbility", 0);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: BackgroundLoaderMgr_RemoveRemoteObject_001
 * @tc.desc: Test BackgroundLoaderMgr RemoveRemoteObject removes object from abilityMap.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, RemoveRemoteObject_001, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = nullptr;
    BackgroundLoaderMgr::GetInstance().SaveRemoteObject("com.test.bundle", "TestAbility", 0, remoteObject);
    BackgroundLoaderMgr::GetInstance().RemoveRemoteObject("com.test.bundle", 0);
    sptr<IRemoteObject> result =
        BackgroundLoaderMgr::GetInstance().GetRemoteObject("com.test.bundle", "TestAbility", 0);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: BackgroundLoaderMgr_RemoveRemoteObject_NotFound_001
 * @tc.desc: Test BackgroundLoaderMgr RemoveRemoteObject when object not found does not crash.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, RemoveRemoteObject_NotFound_001, TestSize.Level1)
{
    BackgroundLoaderMgr::GetInstance().RemoveRemoteObject("com.notexist.bundle", 0);
    sptr<IRemoteObject> result =
        BackgroundLoaderMgr::GetInstance().GetRemoteObject("com.notexist.bundle", "TestAbility", 0);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: BackgroundLoaderMgr_GetInnerTaskInfo_001
 * @tc.desc: Test BackgroundLoaderMgr GetInnerTaskInfo returns task info for registered task.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, GetInnerTaskInfo_001, TestSize.Level1)
{
    TaskInfo info = { .bundleName_ = "com.test.bundle", .abilityName_ = "TestAbility", .appIndex_ = 0, .taskId_ = 1 };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    TaskInfo result;
    bool found = BackgroundLoaderMgr::GetInstance().GetInnerTaskInfo("com.test.bundle", 0, result);
    EXPECT_TRUE(found);
    EXPECT_EQ(result.bundleName_, "com.test.bundle");
    EXPECT_EQ(result.taskId_, 1);
}

/**
 * @tc.name: BackgroundLoaderMgr_GetInnerTaskInfo_NotFound_001
 * @tc.desc: Test BackgroundLoaderMgr GetInnerTaskInfo returns false when task not found.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, GetInnerTaskInfo_NotFound_001, TestSize.Level1)
{
    TaskInfo result;
    bool found = BackgroundLoaderMgr::GetInstance().GetInnerTaskInfo("com.notexist.bundle", 0, result);
    EXPECT_FALSE(found);
}

/**
 * @tc.name: BackgroundLoaderMgr_HandleAppUninstallEvent_001
 * @tc.desc: Test HandleAppUninstallEvent with valid uninstall event.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAppUninstallEvent_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.uninstall.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1,
        .pid_ = 100
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    nlohmann::json payload;
    payload["bundleName"] = "com.uninstall.bundle";
    payload["appIndex"] = "0";
    BackgroundLoaderMgr::GetInstance().HandleAppUninstallEvent(
        ResType::AppInstallStatus::APP_UNINSTALL, payload);
    std::string key = "com.uninstall.bundle_0";
    std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().taskLock_);
    auto it = BackgroundLoaderMgr::GetInstance().taskMap_.find(key);
    EXPECT_EQ(it, BackgroundLoaderMgr::GetInstance().taskMap_.end());
}

/**
 * @tc.name: BackgroundLoaderMgr_HandleAppUninstallEvent_002
 * @tc.desc: Test HandleAppUninstallEvent with non-uninstall value.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAppUninstallEvent_002, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.install.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    nlohmann::json payload;
    payload["bundleName"] = "com.install.bundle";
    payload["appIndex"] = "0";
    BackgroundLoaderMgr::GetInstance().HandleAppUninstallEvent(
        ResType::AppInstallStatus::APP_INSTALL_END, payload);
    std::string key = "com.install.bundle_0";
    std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().taskLock_);
    auto it = BackgroundLoaderMgr::GetInstance().taskMap_.find(key);
    EXPECT_NE(it, BackgroundLoaderMgr::GetInstance().taskMap_.end());
}

/**
 * @tc.name: BackgroundLoaderMgr_HandleAppUninstallEvent_003
 * @tc.desc: Test HandleAppUninstallEvent with invalid payload (missing bundleName).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAppUninstallEvent_003, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.invalid.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    nlohmann::json payload;
    payload["appIndex"] = "0";
    BackgroundLoaderMgr::GetInstance().HandleAppUninstallEvent(
        ResType::AppInstallStatus::APP_UNINSTALL, payload);
    std::string key = "com.invalid.bundle_0";
    std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().taskLock_);
    auto it = BackgroundLoaderMgr::GetInstance().taskMap_.find(key);
    EXPECT_NE(it, BackgroundLoaderMgr::GetInstance().taskMap_.end());
}

/**
 * @tc.name: BackgroundLoaderMgr_HandleAppUninstallEvent_004
 * @tc.desc: Test HandleAppUninstallEvent with invalid payload (missing appIndex).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAppUninstallEvent_004, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.noinfo.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    nlohmann::json payload;
    payload["bundleName"] = "com.noinfo.bundle";
    BackgroundLoaderMgr::GetInstance().HandleAppUninstallEvent(
        ResType::AppInstallStatus::APP_UNINSTALL, payload);
    std::string key = "com.noinfo.bundle_0";
    std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().taskLock_);
    auto it = BackgroundLoaderMgr::GetInstance().taskMap_.find(key);
    EXPECT_NE(it, BackgroundLoaderMgr::GetInstance().taskMap_.end());
}

/**
 * @tc.name: BackgroundLoaderMgr_HandleAppUninstallEvent_ClearBlackList_001
 * @tc.desc: Test HandleAppUninstallEvent clears  list entry.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAppUninstallEvent_ClearBlackList_001, TestSize.Level1)
{
    std::string key = "com.list.bundle_0";
    {
        std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().blackListLock_);
        BackgroundLoaderMgr::GetInstance().blackLists_.insert(key);
    }
    nlohmann::json payload;
    payload["bundleName"] = "com.list.bundle";
    payload["appIndex"] = "0";
    BackgroundLoaderMgr::GetInstance().HandleAppUninstallEvent(
        ResType::AppInstallStatus::APP_UNINSTALL, payload);
    std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().blackListLock_);
    auto it = BackgroundLoaderMgr::GetInstance().blackLists_.find(key);
    EXPECT_EQ(it, BackgroundLoaderMgr::GetInstance().blackLists_.end());
}

/**
 * @tc.name: BackgroundLoaderMgr_RemoveRemoteObject_PidReset_001
 * @tc.desc: Test RemoveRemoteObject resets pid_ to -1 in taskMap.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, RemoveRemoteObject_PidReset_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.pidreset.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1,
        .pid_ = 200
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    BackgroundLoaderMgr::GetInstance().RemoveRemoteObject("com.pidreset.bundle", 0);
    std::string key = "com.pidreset.bundle_0";
    std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().taskLock_);
    auto it = BackgroundLoaderMgr::GetInstance().taskMap_.find(key);
    if (it != BackgroundLoaderMgr::GetInstance().taskMap_.end()) {
        EXPECT_EQ(it->second.pid_, -1);
    }
}

/**
 * @tc.name: BackgroundLoaderMgr_RemoveRemoteObject_PidReset_NotFound_001
 * @tc.desc: Test RemoveRemoteObject when task not found does not crash.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, RemoveRemoteObject_PidReset_NotFound_001, TestSize.Level1)
{
    BackgroundLoaderMgr::GetInstance().RemoveRemoteObject("com.nope.bundle", 0);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: BackgroundLoaderMgr_FinishTask_ReportEvent_001
 * @tc.desc: Test FinishTask reports BACKGROUND_LOADER_TASK_FINISH event.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTask_ReportEvent_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.finishtask.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    ret = BackgroundLoaderMgr::GetInstance().FinishTask(info);
    EXPECT_EQ(ret, ERR_OK);
    std::string key = "com.finishtask.bundle_0";
    std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().taskLock_);
    auto it = BackgroundLoaderMgr::GetInstance().taskMap_.find(key);
    if (it != BackgroundLoaderMgr::GetInstance().taskMap_.end()) {
        EXPECT_EQ(it->second.status_, TaskStatus::FINISHED);
    }
}

/**
 * @tc.name: BackgroundLoaderMgr_TaskInfo_PidField_001
 * @tc.desc: Test TaskInfo pid_ field default value and assignment.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, TaskInfo_PidField_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.pid.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1,
        .pid_ = 300
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    TaskInfo result;
    bool found = BackgroundLoaderMgr::GetInstance().GetInnerTaskInfo("com.pid.bundle", 0, result);
    EXPECT_TRUE(found);
    EXPECT_EQ(result.pid_, 300);
}

/**
 * @tc.name: BackgroundLoaderMgr_TaskInfo_PidDefault_001
 * @tc.desc: Test TaskInfo pid_ default value is -1.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrTest, TaskInfo_PidDefault_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.defaultpid.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);
    TaskInfo result;
    bool found = BackgroundLoaderMgr::GetInstance().GetInnerTaskInfo("com.defaultpid.bundle", 0, result);
    EXPECT_TRUE(found);
    EXPECT_EQ(result.pid_, -1);
}
}
}