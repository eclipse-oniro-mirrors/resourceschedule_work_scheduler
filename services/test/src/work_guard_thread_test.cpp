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
#include "gmock/gmock.h"

#include "work_guard_thread.h"
#include "work_scheduler_service.h"
#include "work_policy_manager.h"
#include "work_conn_manager.h"
#include "work_status.h"
#include "work_info.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class WorkGuardThreadTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/* ======================== IsWorkInExtensionInfos ======================== */

/**
 * @tc.name: IsWorkInExtensionInfos_001
 * @tc.desc: Test IsWorkInExtensionInfos when work matches an extension.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, IsWorkInExtensionInfos_001, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    auto guardThread = std::make_shared<WorkGuardThread>(service);

    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.SetElement("com.test.demo", "WorkExt");
    int32_t uid = 10000;
    auto workStatus = std::make_shared<WorkStatus>(workinfo, uid);

    AppExecFwk::ExtensionRunningInfo extInfo;
    extInfo.extension.SetBundleName("com.test.demo");
    extInfo.extension.SetAbilityName("WorkExt");
    extInfo.uid = 10000;

    std::vector<AppExecFwk::ExtensionRunningInfo> extensionInfos = {extInfo};
    EXPECT_TRUE(guardThread->IsWorkInExtensionInfos(workStatus, extensionInfos));
}

/**
 * @tc.name: IsWorkInExtensionInfos_002
 * @tc.desc: Test IsWorkInExtensionInfos when work does not match any extension.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, IsWorkInExtensionInfos_002, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    auto guardThread = std::make_shared<WorkGuardThread>(service);

    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.SetElement("com.test.demo", "WorkExt");
    int32_t uid = 10000;
    auto workStatus = std::make_shared<WorkStatus>(workinfo, uid);

    AppExecFwk::ExtensionRunningInfo extInfo;
    extInfo.extension.SetBundleName("com.other.demo");
    extInfo.extension.SetAbilityName("OtherExt");
    extInfo.uid = 20000;

    std::vector<AppExecFwk::ExtensionRunningInfo> extensionInfos = {extInfo};
    EXPECT_FALSE(guardThread->IsWorkInExtensionInfos(workStatus, extensionInfos));
}

/**
 * @tc.name: IsWorkInExtensionInfos_003
 * @tc.desc: Test IsWorkInExtensionInfos with null workStatus.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, IsWorkInExtensionInfos_003, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    auto guardThread = std::make_shared<WorkGuardThread>(service);

    std::vector<AppExecFwk::ExtensionRunningInfo> extensionInfos;
    EXPECT_FALSE(guardThread->IsWorkInExtensionInfos(nullptr, extensionInfos));
}

/* ======================== IsExtensionInRunningWorks ======================== */

/**
 * @tc.name: IsExtensionInRunningWorks_001
 * @tc.desc: Test IsExtensionInRunningWorks when extension matches a running work.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, IsExtensionInRunningWorks_001, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    auto guardThread = std::make_shared<WorkGuardThread>(service);

    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.SetElement("com.test.demo", "WorkExt");
    int32_t uid = 10000;
    auto workStatus = std::make_shared<WorkStatus>(workinfo, uid);

    AppExecFwk::ExtensionRunningInfo extInfo;
    extInfo.extension.SetBundleName("com.test.demo");
    extInfo.extension.SetAbilityName("WorkExt");
    extInfo.uid = 10000;

    std::vector<std::shared_ptr<WorkStatus>> runningWorks = {workStatus};
    EXPECT_TRUE(guardThread->IsExtensionInRunningWorks(extInfo, runningWorks));
}

/**
 * @tc.name: IsExtensionInRunningWorks_002
 * @tc.desc: Test IsExtensionInRunningWorks when extension does not match any running work.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, IsExtensionInRunningWorks_002, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    auto guardThread = std::make_shared<WorkGuardThread>(service);

    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.SetElement("com.test.demo", "WorkExt");
    int32_t uid = 10000;
    auto workStatus = std::make_shared<WorkStatus>(workinfo, uid);

    AppExecFwk::ExtensionRunningInfo extInfo;
    extInfo.extension.SetBundleName("com.other.demo");
    extInfo.extension.SetAbilityName("OtherExt");
    extInfo.uid = 20000;

    std::vector<std::shared_ptr<WorkStatus>> runningWorks = {workStatus};
    EXPECT_FALSE(guardThread->IsExtensionInRunningWorks(extInfo, runningWorks));
}

/* ======================== Start / Stop ======================== */

/**
 * @tc.name: Start_001
 * @tc.desc: Test Start creates thread and sets running flag.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, Start_001, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    auto guardThread = std::make_shared<WorkGuardThread>(service);
    guardThread->Start();
    EXPECT_TRUE(guardThread->running_.load());
    guardThread->Stop();
}

/**
 * @tc.name: Start_002
 * @tc.desc: Test Start twice, second call is no-op.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, Start_002, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    auto guardThread = std::make_shared<WorkGuardThread>(service);
    guardThread->Start();
    guardThread->Start();
    EXPECT_TRUE(guardThread->running_.load());
    guardThread->Stop();
}

/**
 * @tc.name: Stop_001
 * @tc.desc: Test Stop after Start, running flag cleared and thread joined.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, Stop_001, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    auto guardThread = std::make_shared<WorkGuardThread>(service);
    guardThread->Start();
    guardThread->Stop();
    EXPECT_FALSE(guardThread->running_.load());
    EXPECT_EQ(guardThread->thread_, nullptr);
}

/**
 * @tc.name: Stop_002
 * @tc.desc: Test Stop without Start, should be no-op.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, Stop_002, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    auto guardThread = std::make_shared<WorkGuardThread>(service);
    guardThread->Stop();
    EXPECT_FALSE(guardThread->running_.load());
}

/* ======================== GetAbilityManager ======================== */

/**
 * @tc.name: GetAbilityManager_001
 * @tc.desc: Test GetAbilityManager returns nullptr when system ability manager is unavailable.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, GetAbilityManager_001, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    auto guardThread = std::make_shared<WorkGuardThread>(service);
    auto abilityMgr = guardThread->GetAbilityManager();
    EXPECT_NE(abilityMgr, nullptr);
}

/* ======================== GetRunningExtensionInfos ======================== */

/**
 * @tc.name: GetRunningExtensionInfos_001
 * @tc.desc: Test GetRunningExtensionInfos returns false when ability manager is null.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, GetRunningExtensionInfos_001, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    auto guardThread = std::make_shared<WorkGuardThread>(service);
    std::vector<AppExecFwk::ExtensionRunningInfo> extensionInfos;
    bool ret = guardThread->GetRunningExtensionInfos(extensionInfos);
    EXPECT_TRUE(ret);
}

/* ======================== StopRunningExtension ======================== */

/**
 * @tc.name: StopRunningExtension_001
 * @tc.desc: Test StopRunningExtension returns false when ability manager is null.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, StopRunningExtension_001, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    auto guardThread = std::make_shared<WorkGuardThread>(service);
    bool ret = guardThread->StopRunningExtension("com.test.demo", "WorkExt", 10000);
    EXPECT_FALSE(ret);
}

/* ======================== CheckRunningWorkStatus ======================== */
/**
 * @tc.name: CheckRunningWorkStatus_001
 * @tc.desc: Test CheckRunningWorkStatus with running work but GetRunningExtensionInfos fails,
 *           work should not be cleaned.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, CheckRunningWorkStatus_001, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    service->workPolicyManager_ = std::make_shared<WorkPolicyManager>(service);
    service->workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
    service->workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.SetElement("com.test.demo", "WorkExt");
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    auto workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    service->workPolicyManager_->AddWork(workStatus, uid);

    auto guardThread = std::make_shared<WorkGuardThread>(service);
    guardThread->CheckRunningWorkStatus();
    EXPECT_EQ(workStatus->GetStatus(), WorkStatus::Status::REMOVED);
}

/* ======================== CheckRunningExtensions ======================== */
/**
 * @tc.name: CheckRunningExtensions_001
 * @tc.desc: Test CheckRunningExtensions with policyManager but GetRunningExtensionInfos fails,
 *           safe return.
 * @tc.type: FUNC
 */
HWTEST_F(WorkGuardThreadTest, CheckRunningExtensions_001, TestSize.Level1)
{
    auto service = std::make_shared<WorkSchedulerService>();
    service->workPolicyManager_ = std::make_shared<WorkPolicyManager>(service);
    service->workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
    service->workPolicyManager_->uidQueueMap_.clear();
    WorkInfo workinfo;
    workinfo.SetWorkId(10000);
    workinfo.SetElement("com.test.demo", "WorkExt");
    workinfo.RequestDeepIdle(true);
    int32_t uid = 10000;
    auto workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    workStatus->MarkStatus(WorkStatus::Status::RUNNING);
    service->workPolicyManager_->AddWork(workStatus, uid);

    auto guardThread = std::make_shared<WorkGuardThread>(service);
    guardThread->CheckRunningExtensions();
    EXPECT_EQ(workStatus->GetStatus(), WorkStatus::Status::RUNNING);
}
} // namespace WorkScheduler
} // namespace OHOS
