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

#include "work_scheduler_service.h"
#include "work_policy_manager.h"
#include "work_status.h"


using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class WorkPolicyManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkPolicyManager> workPolicyManager_;
};

std::shared_ptr<WorkPolicyManager> WorkPolicyManagerTest::workPolicyManager_ = nullptr;

void WorkPolicyManagerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedService>();
    workPolicyManager_ = std::make_shared<WorkPolicyManager>(workSchedulerService_);
}

/**
 * @tc.name: RealStartWork_001
 * @tc.desc: Test WorkPolicyManagerTest RealStartWork.
 * @tc.type: FUNC
 * @tc.require: I8OLHT
 */
HWTEST_F(WorkPolicyManagerTest, RealStartWork_001, TestSize.Level1)
{
    WorkInfo workinfo;
    int32_t uid;
    std::shared_ptr<WorkStatus> topWork = std::make_shared<WorkStatus>(workinfo, uid);
    workPolicyManager_->RealStartWork(topWork);
}

/**
 * @tc.name: PauseRunningWorks_001
 * @tc.desc: Test WorkPolicyManagerTest RealStartWork.
 * @tc.type: FUNC
 * @tc.require: I992IA
 */
HWTEST_F(WorkPolicyManagerTest, PauseRunningWorks_001, TestSize.Level1)
{
    int32_t uid = 123456;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workInfo, uid);
    workPolicyManager_->AddWatchdogForWork(workStatus);
    workStatus->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    int32_t ret = workPolicyManager_->PauseRunningWorks(uid);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: PauseRunningWorks_002
 * @tc.desc: Test WorkPolicyManagerTest RealStartWork.
 * @tc.type: FUNC
 * @tc.require: I992IA
 */
HWTEST_F(WorkPolicyManagerTest, PauseRunningWorks_002, TestSize.Level1)
{
    int32_t uid = 123456;
    int32_t ret = workPolicyManager_->PauseRunningWorks(uid);
    EXPECT_EQ(ret, E_UID_NO_MATCHING_WORK_ERR);
}

/**
 * @tc.name: PauseRunningWorks_002
 * @tc.desc: Test WorkPolicyManagerTest RealStartWork.
 * @tc.type: FUNC
 * @tc.require: I992IA
 */
HWTEST_F(WorkPolicyManagerTest, PauseRunningWorks_002, TestSize.Level1)
{
    int32_t uid = 123456;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workInfo, uid);
    workPolicyManager_->AddWatchdogForWork(workStatus);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    int32_t ret = workPolicyManager_->PauseRunningWorks(uid);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ResumePausedWorks_001
 * @tc.desc: Test WorkPolicyManagerTest RealStartWork.
 * @tc.type: FUNC
 * @tc.require: I992IA
 */
HWTEST_F(WorkPolicyManagerTest, ResumePausedWorks_001, TestSize.Level1)
{
    int32_t uid = 123456;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workInfo, uid);
    uint32_t watchId = workPolicyManager_->AddWatchdogForWork(workStatus);
    workStatus->MarkStatus(WorkStatus::Status::WAIT_CONDITION);
    int32_t ret = workPolicyManager_->ResumePausedWorks(uid);
    workPolicyManager_->RemoveWatchdogMapWork(watchId);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ResumePausedWorks_002
 * @tc.desc: Test WorkPolicyManagerTest RealStartWork.
 * @tc.type: FUNC
 * @tc.require: I992IA
 */
HWTEST_F(WorkPolicyManagerTest, ResumePausedWorks_002, TestSize.Level1)
{
    int32_t uid = 123456;
    int32_t ret = workPolicyManager_->ResumePausedWorks(uid);
    EXPECT_EQ(ret, E_UID_NO_MATCHING_WORK_ERR);
}

/**
 * @tc.name: ResumePausedWorks_002
 * @tc.desc: Test WorkPolicyManagerTest RealStartWork.
 * @tc.type: FUNC
 * @tc.require: I992IA
 */
HWTEST_F(WorkPolicyManagerTest, ResumePausedWorks_002, TestSize.Level1)
{
    int32_t uid = 123456;
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workInfo, uid);
    uint32_t watchId = workPolicyManager_->AddWatchdogForWork(workStatus);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    workStatus->paused_ = true;
    int32_t ret = workPolicyManager_->ResumePausedWorks(uid);
    workPolicyManager_->RemoveWatchdogMapWork(watchId);
    EXPECT_EQ(ret, ERR_OK);
}
}
}