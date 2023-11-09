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
#include "work_status.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkSchedulerService> workSchedulerService_;
};

std::shared_ptr<WorkSchedulerService> WorkSchedulerServiceTest::workSchedulerService_ = nullptr;

void WorkSchedulerServiceTest::SetUpTestCase()
{
    workSchedulerService_ = std::make_shared<WorkSchedulerService>();
}

/**
 * @tc.name: onStart_001
 * @tc.desc: Test WorkSchedulerService OnStart.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, onStart_001, TestSize.Level1)
{
    workSchedulerService_->ready_ = true;
    workSchedulerService_->OnStart();
    EXPECT_NE(workSchedulerService_, nullptr);
}

/**
 * @tc.name: startWork_001
 * @tc.desc: Test WorkSchedulerService startWork.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, startWork_001, TestSize.Level1)
{
    WorkInfo workinfo = WorkInfo();
    auto ret = workSchedulerService_->StartWork(workinfo);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: stopWork_001
 * @tc.desc: Test WorkSchedulerService stopWork.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, stopWork_001, TestSize.Level1)
{
    WorkInfo workinfo = WorkInfo();
    auto ret = workSchedulerService_->StopWork(workinfo);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: StopAndCancelWork_001
 * @tc.desc: Test WorkSchedulerService StopAndCancelWork.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, StopAndCancelWork_001, TestSize.Level1)
{
    WorkInfo workinfo = WorkInfo();
    auto ret = workSchedulerService_->StopAndCancelWork(workinfo);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: StopAndClearWorks_001
 * @tc.desc: Test WorkSchedulerService StopAndClearWorks.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, StopAndClearWorks_001, TestSize.Level1)
{
    WorkInfo workinfo = WorkInfo();
    auto ret = workSchedulerService_->StopAndClearWorks();
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: IsLastWorkTimeout_001
 * @tc.desc: Test WorkSchedulerService IsLastWorkTimeout.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, IsLastWorkTimeout_001, TestSize.Level1)
{
    bool result;
    auto ret = workSchedulerService_->IsLastWorkTimeout(1, result);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: ObtainAllWorks_001
 * @tc.desc: Test WorkSchedulerService ObtainAllWorks.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, ObtainAllWorks_001, TestSize.Level1)
{
    std::list<std::shared_ptr<WorkInfo>> workInfos;
    int32_t uid, pid;

    auto ret = workSchedulerService_->ObtainAllWorks(uid, pid, workInfos);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: GetWorkStatus_001
 * @tc.desc: Test WorkSchedulerService GetWorkStatus.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, GetWorkStatus_001, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo = std::make_shared<WorkInfo>();
    int32_t uid, pid;

    auto ret = workSchedulerService_->GetWorkStatus(uid, pid, workInfo);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: GetAllRunningWorks_001
 * @tc.desc: Test WorkSchedulerService GetAllRunningWorks.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, GetAllRunningWorks_001, TestSize.Level1)
{
    std::list<std::shared_ptr<WorkInfo>> workInfos;
 
    auto ret = workSchedulerService_->GetAllRunningWorks(workInfos);
    EXPECT_NE(ret, 0);
}
}
}