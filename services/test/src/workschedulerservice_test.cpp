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
#define private public
#include "work_scheduler_service.h"
#include "work_status.h"

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
#include "bundle_active_client.h"
#endif
#ifdef DEVICE_STANDBY_ENABLE
#include "standby_service_client.h"
#include "allow_type.h"
#endif
#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
#include "scheduler_bg_task_subscriber.h"
#include "background_task_mgr_helper.h"
#include "resource_type.h"
#endif
#include "work_sched_errors.h"
#include "work_sched_hilog.h"

#ifdef DEVICE_STANDBY_ENABLE
namespace OHOS {
namespace DevStandbyMgr {
ErrCode StandbyServiceClient::SubscribeStandbyCallback(const sptr<IStandbyServiceSubscriber>& subscriber)
{
    return ERR_OK;
}
}
}
#endif

#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
namespace OHOS {
namespace BackgroundTaskMgr {
ErrCode BackgroundTaskMgrHelper::SubscribeBackgroundTask(const BackgroundTaskSubscriber &subscriber)
{
    return ERR_OK;
}
}
}
#endif

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
namespace OHOS {
namespace DeviceUsageStats {
ErrCode BundleActiveClient::RegisterAppGroupCallBack(const sptr<IAppGroupCallback> &observer)
{
    return ERR_OK;
}
}
}
#endif

namespace OHOS {
namespace WorkScheduler {
bool WorkSchedulerService::IsBaseAbilityReady()
{
    return true;
}
}
}

void OHOS::RefBase::DecStrongRef(void const* obj) {}

using namespace testing::ext;
namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerServiceTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkSchedulerService> workSchedulerService_;
};

std::shared_ptr<WorkSchedulerService> WorkSchedulerServiceTest::workSchedulerService_ =
    DelayedSingleton<WorkSchedulerService>::GetInstance();

/**
 * @tc.name: onStart_001
 * @tc.desc: Test WorkSchedulerService OnStart.
 * @tc.type: FUNC
 * @tc.require: I8ZDJI
 */
HWTEST_F(WorkSchedulerServiceTest, onStart_001, TestSize.Level1)
{
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
    int32_t ret;

    workSchedulerService_->ready_ = false;
    WorkInfo workinfo = WorkInfo();
    ret = workSchedulerService_->StartWork(workinfo);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);

    workSchedulerService_->ready_ = true;
    ret = workSchedulerService_->StartWork(workinfo);
    EXPECT_EQ(ret, E_CHECK_WORKINFO_FAILED);

    workSchedulerService_->checkBundle_ = false;
    ret = workSchedulerService_->StartWork(workinfo);
    EXPECT_EQ(ret, E_REPEAT_CYCLE_TIME_ERR);

    workinfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY);
    ret = workSchedulerService_->StartWork(workinfo);
    EXPECT_EQ(ret, 0);
    ret = workSchedulerService_->StartWork(workinfo);
    EXPECT_EQ(ret, E_ADD_REPEAT_WORK_ERR);
}

/**
 * @tc.name: stopWork_001
 * @tc.desc: Test WorkSchedulerService stopWork.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, stopWork_001, TestSize.Level1)
{
    int32_t ret;

    WS_HILOGI("WorkSchedulerServiceTest.stopWork_001 begin");
    workSchedulerService_->ready_ = false;
    workSchedulerService_->checkBundle_ = true;
    WorkInfo workinfo = WorkInfo();
    ret = workSchedulerService_->StopWork(workinfo);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);

    workSchedulerService_->ready_ = true;
    ret = workSchedulerService_->StopWork(workinfo);
    EXPECT_EQ(ret, E_CHECK_WORKINFO_FAILED);

    workSchedulerService_->checkBundle_ = false;
    ret = workSchedulerService_->StopWork(workinfo);
    EXPECT_EQ(ret, 0);
    WS_HILOGI("WorkSchedulerServiceTest.stopWork_001 end");
}

/**
 * @tc.name: StopAndCancelWork_001
 * @tc.desc: Test WorkSchedulerService StopAndCancelWork.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, StopAndCancelWork_001, TestSize.Level1)
{
    int32_t ret;

    WS_HILOGI("WorkSchedulerServiceTest.StopAndCancelWork_001 begin");
    workSchedulerService_->ready_ = false;
    workSchedulerService_->checkBundle_ = true;
    WorkInfo workinfo = WorkInfo();
    ret = workSchedulerService_->StopAndCancelWork(workinfo);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);

    workSchedulerService_->ready_ = true;
    ret = workSchedulerService_->StopAndCancelWork(workinfo);
    EXPECT_EQ(ret, E_CHECK_WORKINFO_FAILED);

    workSchedulerService_->checkBundle_ = false;
    ret = workSchedulerService_->StopAndCancelWork(workinfo);
    EXPECT_EQ(ret, 0);
    WS_HILOGI("WorkSchedulerServiceTest.StopAndCancelWork_001 end");
}

/**
 * @tc.name: StopAndClearWorks_001
 * @tc.desc: Test WorkSchedulerService StopAndClearWorks.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, StopAndClearWorks_001, TestSize.Level1)
{
    int32_t ret;

    WS_HILOGI("WorkSchedulerServiceTest.StopAndClearWorks_001 begin");
    workSchedulerService_->ready_ = false;
    workSchedulerService_->checkBundle_ = true;
    ret = workSchedulerService_->StopAndClearWorks();
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);

    workSchedulerService_->ready_ = true;
    ret = workSchedulerService_->StopAndClearWorks();
    EXPECT_EQ(ret, 0);
    WS_HILOGI("WorkSchedulerServiceTest.StopAndClearWorks_001 end");
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
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);
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
    EXPECT_EQ(ret, 0);
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
    EXPECT_EQ(ret, 0);
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
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: Datashare_001
 * @tc.desc: Test Datashare
 * @tc.type: FUNC
 * @tc.require: I8ZDJI
 */
HWTEST_F(WorkSchedulerServiceTest, Datashare_001, TestSize.Level1)
{
    WS_HILOGI("====== test begin ====== ");
    std::vector<std::string> argsInStr;
    argsInStr.push_back("-k");
    argsInStr.push_back("settings.power.suspend_sources");
    std::string result;
    workSchedulerService_->DumpProcess(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());
    EXPECT_EQ(result.empty(), 0);
    WS_HILOGI("====== test end ====== ");
}
}
}