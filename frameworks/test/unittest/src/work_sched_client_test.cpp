/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "workscheduler_srv_client.h"
#include "work_sched_errors.h"
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class WorkSchedClientTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: WorkSchedClientTest_001
 * @tc.desc: Test StartWork
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_001, TestSize.Level0)
{
    WorkInfo workInfo = WorkInfo();
    auto ret = WorkSchedulerSrvClient::GetInstance().StartWork(workInfo);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_002
 * @tc.desc: Test StopWork
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_002, TestSize.Level0)
{
    WorkInfo workInfo = WorkInfo();
    auto ret = WorkSchedulerSrvClient::GetInstance().StopWork(workInfo);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_003
 * @tc.desc: Test StopAndCancelWork
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_003, TestSize.Level0)
{
    WorkInfo workInfo = WorkInfo();
    auto ret = WorkSchedulerSrvClient::GetInstance().StopAndCancelWork(workInfo);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_004
 * @tc.desc: Test StopAndClearWorks
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_004, TestSize.Level0)
{
    auto ret = WorkSchedulerSrvClient::GetInstance().StopAndClearWorks();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_005
 * @tc.desc: Test IsLastWorkTimeout
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_005, TestSize.Level0)
{
    int32_t workId = 1;
    bool result;
    ErrCode ret = WorkSchedulerSrvClient::GetInstance().IsLastWorkTimeout(workId, result);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_006
 * @tc.desc: Test GetWorkStatus
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_006, TestSize.Level0)
{
    int32_t workId = 1;
    std::shared_ptr<WorkInfo> work;
    ErrCode ret = WorkSchedulerSrvClient::GetInstance().GetWorkStatus(workId, work);
    EXPECT_NE(ret, ERR_OK);
    workId = -1;
    ret = WorkSchedulerSrvClient::GetInstance().GetWorkStatus(workId, work);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_007
 * @tc.desc: Test ObtainAllWorks
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_007, TestSize.Level0)
{
    std::list<std::shared_ptr<WorkInfo>> workInfos;
    ErrCode ret = WorkSchedulerSrvClient::GetInstance().ObtainAllWorks(workInfos);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_008
 * @tc.desc: Test OnRemoteDied and ResetProxy
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_008, TestSize.Level0)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(WORK_SCHEDULE_SERVICE_ID);
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = sptr<IRemoteObject::DeathRecipient>
        (new WorkSchedulerSrvClient::WorkSchedulerDeathRecipient(WorkSchedulerSrvClient::GetInstance()));
    deathRecipient_->OnRemoteDied(remoteObject_);
    WorkSchedulerSrvClient::GetInstance().iWorkSchedService_ = nullptr;
    deathRecipient_->OnRemoteDied(remoteObject_);
    EXPECT_NE(remoteObject_, nullptr);
}

/**
 * @tc.name: WorkSchedClientTest_009
 * @tc.desc: Test OnRemoteDied and ResetProxy
 * @tc.type: FUNC
 * @tc.require: issueI5Y6YK
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_009, TestSize.Level0)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(WORK_SCHEDULE_SERVICE_ID);
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = sptr<IRemoteObject::DeathRecipient>
        (new WorkSchedulerSrvClient::WorkSchedulerDeathRecipient(WorkSchedulerSrvClient::GetInstance()));
    deathRecipient_->OnRemoteDied(nullptr);
    EXPECT_NE(remoteObject_, nullptr);
}

/**
 * @tc.name: WorkSchedClientTest_010
 * @tc.desc: Test PauseRunningWorks
 * @tc.type: FUNC
 * @tc.require: issue:#I992IA
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_010, TestSize.Level0)
{
    int32_t uid = -1;
    ErrCode ret = WorkSchedulerSrvClient::GetInstance().PauseRunningWorks(uid);
    EXPECT_EQ(ret, E_PARAM_ERROR);
}

/**
 * @tc.name: WorkSchedClientTest_011
 * @tc.desc: Test PauseRunningWorks
 * @tc.type: FUNC
 * @tc.require: issue:#I992IA
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_011, TestSize.Level0)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    ErrCode ret = WorkSchedulerSrvClient::GetInstance().PauseRunningWorks(uid);
    EXPECT_EQ(ret, E_INVALID_PROCESS_NAME);
}

/**
 * @tc.name: WorkSchedClientTest_012
 * @tc.desc: Test ResumePausedWorks
 * @tc.type: FUNC
 * @tc.require: issue:#I992IA
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_012, TestSize.Level0)
{
    int32_t uid = -1;
    ErrCode ret = WorkSchedulerSrvClient::GetInstance().ResumePausedWorks(uid);
    EXPECT_EQ(ret, E_PARAM_ERROR);
}

/**
 * @tc.name: WorkSchedClientTest_013
 * @tc.desc: Test ResumePausedWorks
 * @tc.type: FUNC
 * @tc.require: issue:#I992IA
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_013, TestSize.Level0)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    ErrCode ret = WorkSchedulerSrvClient::GetInstance().ResumePausedWorks(uid);
    EXPECT_EQ(ret, E_INVALID_PROCESS_NAME);
}

/**
 * @tc.name: WorkSchedClientTest_014
 * @tc.desc: Test GetAllRunningWorks
 * @tc.type: FUNC
 * @tc.require: issue:#I9EKGI
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_014, TestSize.Level0)
{
    std::list<std::shared_ptr<WorkInfo>> workInfos;
    ErrCode ret = WorkSchedulerSrvClient::GetInstance().GetAllRunningWorks(workInfos);
    EXPECT_EQ(ret, E_INVALID_PROCESS_NAME);
}

/**
 * @tc.name: WorkSchedClientTest_015
 * @tc.desc: Test SetWorkSchedulerConfig
 * @tc.type: FUNC
 * @tc.require: issue:#I9EKGI
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_015, TestSize.Level0)
{
    int32_t sourceType = 1;
    std::string configData = "";
    ErrCode ret = WorkSchedulerSrvClient::GetInstance().SetWorkSchedulerConfig(configData, sourceType);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_016
 * @tc.desc: Test ObtainWorksByUidAndWorkIdForInner
 * @tc.type: FUNC
 * @tc.require: issueIC1RS6
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_016, TestSize.Level0)
{
    std::list<std::shared_ptr<WorkInfo>> workInfos;
    int32_t uid = -1;
    ErrCode ret = ERR_OK;
    ret = WorkSchedulerSrvClient::GetInstance().ObtainWorksByUidAndWorkIdForInner(uid, workInfos);
    EXPECT_EQ(ret, E_PARAM_INVAILD_UID);

    uid = 1;
    ret = WorkSchedulerSrvClient::GetInstance().ObtainWorksByUidAndWorkIdForInner(uid, workInfos);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_017
 * @tc.desc: Test StartWorkForInner
 * @tc.type: FUNC
 * @tc.require: issue:#ICBWOI
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_017, TestSize.Level0)
{
    WorkInfo workInfo = WorkInfo();
    auto ret = WorkSchedulerSrvClient::GetInstance().StartWorkForInner(workInfo);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: WorkSchedClientTest_018
 * @tc.desc: Test StopWorkForInner
 * @tc.type: FUNC
 * @tc.require: issue:#ICBWOI
 */
HWTEST_F(WorkSchedClientTest, WorkSchedClientTest_018, TestSize.Level0)
{
    WorkInfo workInfo = WorkInfo();
    auto ret = WorkSchedulerSrvClient::GetInstance().StopWorkForInner(workInfo);
    EXPECT_NE(ret, ERR_OK);
}
}  // namespace WorkScheduler
}  // namespace OHOS
