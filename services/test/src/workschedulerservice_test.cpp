/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include "timer.h"
#define private public
#include "work_scheduler_service.h"
#include "work_status.h"
#include "work_bundle_group_change_callback.h"
#include "work_scheduler_connection.h"
#include "work_queue_event_handler.h"
#include "work_event_handler.h"
#include "conditions/battery_level_listener.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "battery_info.h"
#include "conditions/battery_status_listener.h"
#include "conditions/charger_listener.h"
#include "event_publisher.h"
#include "accesstoken_kit.h"
#include "token_setproc.h"

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
#include "work_policy_manager.h"
#include "background_loader_task_info.h"
#include "work_sched_constants.h"
#include "frequency_info.h"

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
namespace {
    static const std::string PUSH_SERVICE_NAME = "push_manager_service";
    static const std::string BGTASK_SERVICE_NAME = "resource_schedule_service";
}
class WorkSchedulerServiceTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkSchedulerService> workSchedulerService_;
};

void GetNativeToken(const std::string &name)
{
    auto tokenId = Security::AccessToken::AccessTokenKit::GetNativeTokenId(name);
    SetSelfTokenID(tokenId);
}

std::shared_ptr<WorkSchedulerService> WorkSchedulerServiceTest::workSchedulerService_ =
    DelayedSingleton<WorkSchedulerService>::GetInstance();

class MyWorkSchedulerService : public WorkSchedServiceStub {
    int32_t StartWork(const WorkInfo& workInfo) { return 0; }
    int32_t StopWork(const WorkInfo& workInfo) { return 0; }
    int32_t StopAndCancelWork(const WorkInfo& workInfo)  { return 0; }
    int32_t StopAndClearWorks() { return 0; }
    int32_t IsLastWorkTimeout(int32_t workId, bool &result) { return 0; }
    int32_t ObtainAllWorks(std::vector<WorkInfo>& workInfos) { return 0; }
    int32_t ObtainWorksByUidAndWorkIdForInner(int32_t uid, std::vector<WorkInfo>& workInfos,
        int32_t workId) { return 0; }
    int32_t GetWorkStatus(int32_t workId, WorkInfo& workInfo) { return 0; }
    int32_t GetAllRunningWorks(std::vector<WorkInfo>& workInfos) { return 0; }
    int32_t PauseRunningWorks(int32_t uid) {return 0; }
    int32_t ResumePausedWorks(int32_t uid) {return 0; }
    int32_t SetWorkSchedulerConfig(const std::string &configData, int32_t sourceType) { return 0; }
    int32_t StopWorkForSA(int32_t saId) { return 0; }
    int32_t StartWorkForInner(const WorkInfo& workInfo) { return 0; }
    int32_t StopWorkForInner(const WorkInfo& workInfo, bool needCancel) { return 0; }
    int32_t RegisterTask(const BackgroundLoaderTaskInfo& taskInfo)  { return 0; }
    int32_t UnregisterTask(const BackgroundLoaderTaskInfo& taskInfo)  { return 0; }
    int32_t FinishTask(const BackgroundLoaderTaskInfo& taskInfo) { return 0; }
    int32_t GetTaskInfo(int32_t taskId, BackgroundLoaderTaskInfo& taskInfo)  { return 0; }
    int32_t SetExecFrequency(const FrequencyInfo& frequencyInfo) { return 0; }
    int32_t ResetExecFrequency(const int32_t uid) { return 0; }
};
/**
 * @tc.name: startWork_001
 * @tc.desc: Test WorkSchedulerService startWork.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, startWork_001, TestSize.Level0)
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
HWTEST_F(WorkSchedulerServiceTest, stopWork_001, TestSize.Level0)
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
HWTEST_F(WorkSchedulerServiceTest, StopAndCancelWork_001, TestSize.Level0)
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
HWTEST_F(WorkSchedulerServiceTest, StopAndClearWorks_001, TestSize.Level0)
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
HWTEST_F(WorkSchedulerServiceTest, IsLastWorkTimeout_001, TestSize.Level0)
{
    bool result;
    auto ret = workSchedulerService_->IsLastWorkTimeout(1, result);
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);
}

/**
 * @tc.name: ObtainAllWorks_001
 * @tc.desc: Test WorkSchedulerService ObtainAllWorks.
 * @tc.type: FUNC
 * @tc.require: IA4HTC
 */
HWTEST_F(WorkSchedulerServiceTest, ObtainAllWorks_001, TestSize.Level0)
{
    std::vector<WorkInfo> workInfos;
    auto ret = workSchedulerService_->ObtainAllWorks(workInfos);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: GetWorkStatus_001
 * @tc.desc: Test WorkSchedulerService GetWorkStatus.
 * @tc.type: FUNC
 * @tc.require: IA4HTC
 */
HWTEST_F(WorkSchedulerServiceTest, GetWorkStatus_001, TestSize.Level0)
{
    WorkInfo workInfo;
    int32_t workId = 0;
    auto ret = workSchedulerService_->GetWorkStatus(workId, workInfo);
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);
}

/**
 * @tc.name: ObtainWorksByUidAndWorkIdForInner_001
 * @tc.desc: Test WorkSchedulerService ObtainWorksByUidAndWorkIdForInner.
 * @tc.type: FUNC
 * @tc.require: IA4HTC
 */
HWTEST_F(WorkSchedulerServiceTest, ObtainWorksByUidAndWorkIdForInner_001, TestSize.Level0)
{
    std::vector<WorkInfo> workInfos;
    int32_t uid = 1;
    int32_t workId = 1;
    workSchedulerService_->ready_ = false;
    int32_t ret = -1;
    ret = workSchedulerService_->ObtainWorksByUidAndWorkIdForInner(uid, workInfos, workId);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);

    workSchedulerService_->ready_ = true;
    ret = workSchedulerService_->ObtainWorksByUidAndWorkIdForInner(uid, workInfos, workId);
    EXPECT_EQ(ret, E_WORK_NOT_EXIST_FAILED);

    workId = -1;
    ret = workSchedulerService_->ObtainWorksByUidAndWorkIdForInner(uid, workInfos, workId);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetAllRunningWorks_001
 * @tc.desc: Test WorkSchedulerService GetAllRunningWorks.
 * @tc.type: FUNC
 * @tc.require: I8F08T
 */
HWTEST_F(WorkSchedulerServiceTest, GetAllRunningWorks_001, TestSize.Level0)
{
    std::vector<WorkInfo> workInfos;

    auto ret = workSchedulerService_->GetAllRunningWorks(workInfos);
    EXPECT_EQ(ret, E_INVALID_PROCESS_NAME);
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
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());
    EXPECT_EQ(result.empty(), 0);
    WS_HILOGI("====== test end ====== ");
}

HWTEST_F(WorkSchedulerServiceTest, ListenerStart_001, TestSize.Level1)
{
    WS_HILOGI("====== ListenerStart_001 begin====== ");
    for (auto pair : workSchedulerService_->workQueueManager_->listenerMap_)
    {
        pair.second->Start();
    }
    std::vector<std::pair<string, string>> infos = {
        {"event", "info"},
        {"network", "wifi"},
        {"network", "disconnect"},
        {"network", "invalid"},
        {"charging", "usb"},
        {"charging", "ac"},
        {"charging", "wireless"},
        {"charging", "none"},
        {"charging", "invalid"},
        {"storage", "low"},
        {"storage", "ok"},
        {"storage", "invalid"},
        {"batteryStatus", "low"},
        {"batteryStatus", "ok"},
        {"batteryStatus", "invalid"},
    };
    EventPublisher eventPublisher;
    for (auto it : infos) {
        std::string result;
        std::string eventType = it.first;
        std::string eventValue = it.second;
        eventPublisher.Dump(result, eventType, eventValue);
        WS_HILOGI("%{public}s", result.c_str());
        EXPECT_EQ(!result.empty(), true);
    }
    WS_HILOGI("====== ListenerStart_001 end ====== ");
}

HWTEST_F(WorkSchedulerServiceTest, Dump_001, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_001 begin ====== ");
    std::vector<std::string> argsInStr;
    std::string result;
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());

    argsInStr.clear();
    result.clear();
    argsInStr.push_back("-h");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());

    result.clear();
    argsInStr.clear();
    argsInStr.push_back("-a");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());

    result.clear();
    argsInStr.clear();
    argsInStr.push_back("-x");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());

    result.clear();
    argsInStr.clear();
    argsInStr.push_back("-memory");
    argsInStr.push_back("100");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());

    result.clear();
    argsInStr.clear();
    argsInStr.push_back("-watchdog_time");
    argsInStr.push_back("100");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    EXPECT_FALSE(result.empty());
    WS_HILOGI("%{public}s", result.c_str());
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_001 end ====== ");
}

HWTEST_F(WorkSchedulerServiceTest, Dump_002, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_002 begin ====== ");
    std::vector<std::string> argsInStr;
    std::string result;
    argsInStr.push_back("-repeat_time_min");
    argsInStr.push_back("100");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());

    result.clear();
    argsInStr.clear();
    argsInStr.push_back("-min_interval");
    argsInStr.push_back("100");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());

    result.clear();
    argsInStr.clear();
    argsInStr.push_back("-test");
    argsInStr.push_back("100");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());
    EXPECT_EQ(result.empty(), false);
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_002 end ====== ");
}

HWTEST_F(WorkSchedulerServiceTest, Dump_003, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_003 begin ====== ");
    std::vector<std::string> argsInStr;
    std::string result;

    argsInStr.clear();
    result.clear();
    argsInStr.push_back("-d");
    argsInStr.push_back("storage");
    argsInStr.push_back("ok");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());

    argsInStr.clear();
    result.clear();
    argsInStr.push_back("-t");
    argsInStr.push_back("bundlename");
    argsInStr.push_back("abilityname");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());

    WorkInfo workinfo = WorkInfo();
    result.clear();
    workinfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY);
    workinfo.RefreshUid(2);
    workinfo.SetElement("bundlename", "abilityname");
    workSchedulerService_->AddWorkInner(workinfo);
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    EXPECT_TRUE(result.empty());
    WS_HILOGI("%{public}s", result.c_str());
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_003 end ====== ");
}

HWTEST_F(WorkSchedulerServiceTest, Dump_004, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_004 begin ====== ");
    std::vector<std::string> argsInStr;
    std::string result;
    argsInStr.push_back("-d");
    argsInStr.push_back("storage");
    argsInStr.push_back("ok");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());

    argsInStr.clear();
    argsInStr.push_back("arg0");
    argsInStr.push_back("arg1");
    argsInStr.push_back("arg2");
    argsInStr.push_back("arg3");
    argsInStr.push_back("arg4");
    result.clear();
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());
    EXPECT_EQ(result.empty(), false);
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_004 end ====== ");
}

/**
 * @tc.name: Dump_005
 * @tc.desc: Test WorkSchedulerService Dump.
 * @tc.type: FUNC
 * @tc.require: I9RYLE
 */
HWTEST_F(WorkSchedulerServiceTest, Dump_005, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_005 begin ====== ");
    std::vector<std::string> argsInStr;
    std::string result;
    argsInStr.push_back("-count");
    argsInStr.push_back("1");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());
    EXPECT_EQ(result.empty(), false);
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_005 end ====== ");
}

/**
 * @tc.name: Dump_006
 * @tc.desc: Test WorkSchedulerService Dump.
 * @tc.type: FUNC
 * @tc.require: IAHY0B
 */
HWTEST_F(WorkSchedulerServiceTest, Dump_006, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_006 begin ====== ");
    std::vector<std::string> argsInStr;
    std::string result;
    argsInStr.push_back("-s");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());
    EXPECT_FALSE(result.empty());

    argsInStr.clear();
    result.clear();
    argsInStr.push_back("-s");
    argsInStr.push_back("-1");
    argsInStr.push_back("-1");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());
    EXPECT_FALSE(result.empty());

    argsInStr.clear();
    result.clear();
    argsInStr.push_back("-s");
    argsInStr.push_back("1");
    argsInStr.push_back("200000");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());
    EXPECT_FALSE(result.empty());

    argsInStr.clear();
    result.clear();
    WorkInfo workinfo = WorkInfo();
    workinfo.uid_ = 202000;
    workinfo.workId_ = 1;
    workinfo.saId_ = 1000;
    workinfo.residentSa_ = true;
    workSchedulerService_->AddWorkInner(workinfo);
    argsInStr.push_back("-s");
    argsInStr.push_back("1000");
    argsInStr.push_back("202000");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());
    EXPECT_FALSE(result.empty());
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_006 end ====== ");
}

/**
 * @tc.name: Dump_007
 * @tc.desc: Test WorkSchedulerService Dump.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, Dump_007, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_007 begin ====== ");
    std::vector<std::string> argsInStr;
    std::string result;
    argsInStr.push_back("-x");
    argsInStr.push_back("1");
    argsInStr.push_back("1");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());
    EXPECT_EQ(result.empty(), false);

    argsInStr.clear();
    result.clear();
    argsInStr.push_back("-x");
    argsInStr.push_back("1");
    argsInStr.push_back("p");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());
    EXPECT_EQ(result.empty(), false);

    argsInStr.clear();
    result.clear();
    argsInStr.push_back("-x");
    argsInStr.push_back("1");
    argsInStr.push_back("r");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());
    EXPECT_EQ(result.empty(), false);

    argsInStr.clear();
    result.clear();
    argsInStr.push_back("-cpu");
    argsInStr.push_back("1");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());
    EXPECT_EQ(result.empty(), false);
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_007 end ====== ");
}

/**
 * @tc.name: Dump_008
 * @tc.desc: Test WorkSchedulerService Dump.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, Dump_008, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_008 begin ====== ");
    std::vector<std::string> argsInStr;
    std::string result;
    argsInStr.push_back("-t");
    argsInStr.push_back("bundlename");
    argsInStr.push_back("abilityname");
    workSchedulerService_->DumpProcessForUserMode(argsInStr, result);
    EXPECT_EQ(result.empty(), true);
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_008 end ====== ");
}

/**
 * @tc.name: Dump_009
 * @tc.desc: Test WorkSchedulerService Dump.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, Dump_009, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_009 begin ====== ");
    std::vector<std::string> argsInStr;
    std::string result;
    argsInStr.push_back("-r");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);

    argsInStr.clear();
    result.clear();
    argsInStr.push_back("-thermalLevel");
    argsInStr.push_back("1");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    EXPECT_EQ(result.empty(), false);
    WS_HILOGI("====== WorkSchedulerServiceTest.Dump_009 end ====== ");
}

HWTEST_F(WorkSchedulerServiceTest, WorkSchedulerConnection_001, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.WorkSchedulerConnection_001 begin ====== ");
    auto workinfo = std::make_shared<WorkInfo>();
    WorkSchedulerConnection conection(workinfo);
    AppExecFwk::ElementName element;
    conection.StopWork();
    conection.OnAbilityDisconnectDone(element, 0);
    EXPECT_EQ(conection.proxy_, nullptr);
    WS_HILOGI("====== WorkSchedulerServiceTest.WorkSchedulerConnection_001 end ====== ");
}

HWTEST_F(WorkSchedulerServiceTest, SchedulerBgTaskSubscriber_001, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.SchedulerBgTaskSubscriber_001 begin ====== ");
    SchedulerBgTaskSubscriber subscriber;
    subscriber.OnProcEfficiencyResourcesApply(nullptr);
    subscriber.OnProcEfficiencyResourcesReset(nullptr);
    subscriber.OnAppEfficiencyResourcesApply(nullptr);
    subscriber.OnAppEfficiencyResourcesReset(nullptr);

    auto resourceInfo = std::make_shared<BackgroundTaskMgr::ResourceCallbackInfo>(0, 0, 0xFFFF, "name");
    subscriber.OnProcEfficiencyResourcesApply(resourceInfo);
    subscriber.OnProcEfficiencyResourcesReset(resourceInfo);
    subscriber.OnAppEfficiencyResourcesApply(resourceInfo);
    subscriber.OnAppEfficiencyResourcesReset(resourceInfo);
    EXPECT_EQ(resourceInfo->GetBundleName(), "name");
    WS_HILOGI("====== WorkSchedulerServiceTest.SchedulerBgTaskSubscriber_001 end ====== ");
}

HWTEST_F(WorkSchedulerServiceTest, ListenerStop_001, TestSize.Level1)
{
    WS_HILOGI("====== ListenerStop_001 begin====== ");
    for (auto pair : workSchedulerService_->workQueueManager_->listenerMap_)
    {
        pair.second->Stop();
    }
    workSchedulerService_->workQueueManager_->listenerMap_.clear();
    EXPECT_TRUE(workSchedulerService_->workQueueManager_->listenerMap_.size() == 0);
    WS_HILOGI("====== ListenerStop_001 end ====== ");
}

/**
 * @tc.name: GetJsonFromFile_001
 * @tc.desc: Test WorkSchedulerService GetJsonFromFile.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkSchedulerServiceTest, GetJsonFromFile_001, TestSize.Level1)
{
    nlohmann::json root;
    const char* path = "/a/b/c";
    bool ret = workSchedulerService_->GetJsonFromFile(path, root);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetAppIndexAndBundleNameByUid_001
 * @tc.desc: Test WorkSchedulerService GetAppIndexAndBundleNameByUid.
 * @tc.type: FUNC
 * @tc.require: I9TL53
 */
HWTEST_F(WorkSchedulerServiceTest, GetAppIndexAndBundleNameByUid_001, TestSize.Level1)
{
    int32_t appIndex;
    std::string bundleName;
    int32_t uid = 1;
    bool ret = workSchedulerService_->GetAppIndexAndBundleNameByUid(uid, appIndex, bundleName);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: LoadSa_001
 * @tc.desc: Test WorkSchedulerService LoadSa.
 * @tc.type: FUNC
 * @tc.require: IAHY0B
 */
HWTEST_F(WorkSchedulerServiceTest, LoadSa_001, TestSize.Level1)
{
    std::shared_ptr<WorkInfo> workInfo_ = std::make_shared<WorkInfo>();
    workInfo_->SetWorkId(-1);
    std::string bundleName = "com.example.workStatus";
    std::string abilityName = "workStatusAbility";
    workInfo_->SetElement(bundleName, abilityName);
    workInfo_->RequestPersisted(false);
    time_t baseTime;
    (void)time(&baseTime);
    workInfo_->RequestBaseTime(baseTime);
    std::shared_ptr<WorkStatus> workStatus_ = std::make_shared<WorkStatus>(*(workInfo_.get()), -1);

    workSchedulerService_->ready_ = false;
    workSchedulerService_->LoadSa(workStatus_, "");

    workSchedulerService_->ready_ = true;
    workSchedulerService_->LoadSa(workStatus_, "");

    workInfo_->saId_ = 401;
    workSchedulerService_->LoadSa(workStatus_, "");

    workInfo_->residentSa_ = true;
    workSchedulerService_->LoadSa(workStatus_, "");

    workInfo_->saId_ = 5300;
    workSchedulerService_->LoadSa(workStatus_, "");

    workInfo_->residentSa_ = false;
    bool ret = workSchedulerService_->LoadSa(workStatus_, "");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: InitWorkSchedPluginMgr_001
 * @tc.desc: Test WorkSchedulerService InitWorkSchedPluginMgr.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, InitWorkSchedPluginMgr_001, TestSize.Level1)
{
    bool ret = workSchedulerService_->InitWorkSchedPluginMgr();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: AllowDump_001
 * @tc.desc: Test WorkSchedulerService AllowDump.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, AllowDump_001, TestSize.Level1)
{
    bool ret = workSchedulerService_->AllowDump();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UpdateWorkBeforeRealStart_001
 * @tc.desc: Test WorkSchedulerService UpdateWorkBeforeRealStart.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, UpdateWorkBeforeRealStart_001, TestSize.Level1)
{
    std::shared_ptr<WorkStatus> workStatus = nullptr;
    workSchedulerService_->UpdateWorkBeforeRealStart(workStatus);

    WorkInfo workInfo = WorkInfo();
    workStatus = std::make_shared<WorkStatus>(workInfo, 1);
    workStatus->conditionMap_.clear();
    workSchedulerService_->UpdateWorkBeforeRealStart(workStatus);

    std::shared_ptr<Condition> repeatCycle = std::make_shared<Condition>();
    repeatCycle->boolVal = true;
    workStatus->conditionMap_.emplace(WorkCondition::Type::TIMER, repeatCycle);
    workSchedulerService_->UpdateWorkBeforeRealStart(workStatus);
    EXPECT_TRUE(repeatCycle->intVal == 0);
}

/**
 * @tc.name: CheckEffiResApplyInfo_001
 * @tc.desc: Test WorkSchedulerService CheckEffiResApplyInfo.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, CheckEffiResApplyInfo_001, TestSize.Level1)
{
    bool ret = workSchedulerService_->CheckEffiResApplyInfo(1);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: PauseRunningWorks_001
 * @tc.desc: Test WorkSchedulerService PauseRunningWorks.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, PauseRunningWorks_001, TestSize.Level1)
{
    workSchedulerService_->TriggerWorkIfConditionReady();
    int32_t ret = workSchedulerService_->PauseRunningWorks(1);
    EXPECT_TRUE(ret == E_INVALID_PROCESS_NAME);
    ret = workSchedulerService_->ResumePausedWorks(1);
    EXPECT_TRUE(ret == E_INVALID_PROCESS_NAME);
}

/**
 * @tc.name: OnAddSystemAbility_001
 * @tc.desc: Test WorkSchedulerService OnAddSystemAbility.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, OnAddSystemAbility_001, TestSize.Level1)
{
    std::string deviceId;
    int32_t DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID = 1914;
    int32_t DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID = 1907;
    workSchedulerService_->groupObserver_ = nullptr;
    workSchedulerService_->OnAddSystemAbility(DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID, deviceId);
    EXPECT_TRUE(workSchedulerService_->groupObserver_ != nullptr);
    workSchedulerService_->OnRemoveSystemAbility(DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID, deviceId);
    EXPECT_TRUE(workSchedulerService_->groupObserver_ == nullptr);

    workSchedulerService_->standbyStateObserver_ = nullptr;
    workSchedulerService_->OnAddSystemAbility(DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID, deviceId);
    EXPECT_TRUE(workSchedulerService_->standbyStateObserver_ != nullptr);
    workSchedulerService_->OnRemoveSystemAbility(DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID, deviceId);
    EXPECT_TRUE(workSchedulerService_->standbyStateObserver_ == nullptr);
}

/**
 * @tc.name: IsDebugApp_001
 * @tc.desc: Test WorkSchedulerService IsDebugApp.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, IsDebugApp_001, TestSize.Level1)
{
    bool ret = workSchedulerService_->IsDebugApp("bundleName");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsExemptionBundle_001
 * @tc.desc: Test WorkSchedulerService IsExemptionBundle.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, IsExemptionBundle_001, TestSize.Level1)
{
    workSchedulerService_->exemptionBundles_.clear();
    std::string bundleName = "com.demo.bundle";
    bool ret = workSchedulerService_->IsExemptionBundle(bundleName);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsExemptionBundle_002
 * @tc.desc: Test WorkSchedulerService IsExemptionBundle.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, IsExemptionBundle_002, TestSize.Level1)
{
    std::string bundleName = "com.demo.bundle";
    workSchedulerService_->exemptionBundles_.clear();
    workSchedulerService_->InsertExemptionBundles(bundleName);
    bool ret = workSchedulerService_->IsExemptionBundle(bundleName);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IsExemptionBundle_003
 * @tc.desc: Test WorkSchedulerService IsExemptionBundle.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, IsExemptionBundle_003, TestSize.Level1)
{
    std::string bundleName = "com.demo.bundle";
    workSchedulerService_->exemptionBundles_.clear();
    workSchedulerService_->InsertExemptionBundles("com.demo.bundle1");
    bool ret = workSchedulerService_->IsExemptionBundle(bundleName);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: DumpExemptionBundles_001
 * @tc.desc: Test WorkSchedulerService DumpExemptionBundles.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, DumpExemptionBundles_001, TestSize.Level1)
{
    workSchedulerService_->exemptionBundles_.clear();
    std::string ret = workSchedulerService_->DumpExemptionBundles();
    EXPECT_TRUE(ret == "[]");
}

/**
 * @tc.name: DumpExemptionBundles_002
 * @tc.desc: Test WorkSchedulerService DumpExemptionBundles.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, DumpExemptionBundles_002, TestSize.Level1)
{
    workSchedulerService_->exemptionBundles_.insert("com.demo.bundle1");
    std::string ret = workSchedulerService_->DumpExemptionBundles();
    EXPECT_FALSE(ret.empty());
}

/**
 * @tc.name: LoadExemptionBundlesFromFile_001
 * @tc.desc: Test WorkSchedulerService LoadExemptionBundlesFromFile.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, LoadExemptionBundlesFromFile_001, TestSize.Level1)
{
    workSchedulerService_->exemptionBundles_.clear();
    const char* path = nullptr;
    workSchedulerService_->LoadExemptionBundlesFromFile(path);
    EXPECT_TRUE(workSchedulerService_->exemptionBundles_.empty());
}

/**
 * @tc.name: LoadExemptionBundlesFromFile_002
 * @tc.desc: Test WorkSchedulerService LoadExemptionBundlesFromFile.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, LoadExemptionBundlesFromFile_002, TestSize.Level1)
{
    workSchedulerService_->exemptionBundles_.clear();
    const char *path = "/a/b/c";
    workSchedulerService_->LoadExemptionBundlesFromFile(path);
    EXPECT_TRUE(workSchedulerService_->exemptionBundles_.empty());
}

/**
 * @tc.name: IsPreinstalledBundle_001
 * @tc.desc: Test WorkSchedulerService IsPreinstalledBundle.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, IsPreinstalledBundle_001, TestSize.Level1)
{
    workSchedulerService_->preinstalledBundles_.clear();
    std::string bundleName = "com.demo.bundle";
    bool ret = workSchedulerService_->IsPreinstalledBundle(bundleName);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: IsPreinstalledBundle_002
 * @tc.desc: Test WorkSchedulerService IsPreinstalledBundle.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, IsPreinstalledBundle_002, TestSize.Level1)
{
    workSchedulerService_->preinstalledBundles_.clear();
    std::string bundleName = "com.demo.bundle";
    workSchedulerService_->InsertPreinstalledBundles(bundleName);
    bool ret = workSchedulerService_->IsPreinstalledBundle(bundleName);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: IsPreinstalledBundle_003
 * @tc.desc: Test WorkSchedulerService IsPreinstalledBundle.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, IsPreinstalledBundle_003, TestSize.Level1)
{
    workSchedulerService_->preinstalledBundles_.clear();
    std::string bundleName = "com.demo.bundle";
    workSchedulerService_->InsertPreinstalledBundles("com.demo.bundle1");
    bool ret = workSchedulerService_->IsPreinstalledBundle(bundleName);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: LoadMinRepeatTimeFromFile_001
 * @tc.desc: Test WorkSchedulerService LoadMinRepeatTimeFromFile.
 * @tc.type: FUNC
 * @tc.require: IBB60P
 */
HWTEST_F(WorkSchedulerServiceTest, LoadMinRepeatTimeFromFile_001, TestSize.Level1)
{
    workSchedulerService_->specialMap_.clear();
    const char* path = nullptr;
    workSchedulerService_->LoadMinRepeatTimeFromFile(path);
    EXPECT_TRUE(workSchedulerService_->specialMap_.empty());
}

/**
 * @tc.name: LoadMinRepeatTimeFromFile_002
 * @tc.desc: Test WorkSchedulerService LoadMinRepeatTimeFromFile.
 * @tc.type: FUNC
 * @tc.require: IBB60P
 */
HWTEST_F(WorkSchedulerServiceTest, LoadMinRepeatTimeFromFile_002, TestSize.Level1)
{
    workSchedulerService_->specialMap_.clear();
    const char *path = "/a/b/c";
    workSchedulerService_->LoadMinRepeatTimeFromFile(path);
    EXPECT_TRUE(workSchedulerService_->specialMap_.empty());
}

/**
 * @tc.name: DumpTriggerWork_001
 * @tc.desc: Test WorkSchedulerService DumpTriggerWork.
 * @tc.type: FUNC
 * @tc.require: IC5H77
 */
HWTEST_F(WorkSchedulerServiceTest, DumpTriggerWork_001, TestSize.Level1)
{
    std::string uIdStr = "";
    std::string workIdStr = "123";
    std::string result;
    workSchedulerService_->DumpTriggerWork(uIdStr, workIdStr, result);
    EXPECT_EQ(result, "param invalid\n");
}

/**
 * @tc.name: DumpTriggerWork_002
 * @tc.desc: Test WorkSchedulerService DumpTriggerWork.
 * @tc.type: FUNC
 * @tc.require: IC5H77
 */
HWTEST_F(WorkSchedulerServiceTest, DumpTriggerWork_002, TestSize.Level1)
{
    std::string uIdStr = "0";
    std::string workIdStr = "123";
    std::string result;
    workSchedulerService_->DumpTriggerWork(uIdStr, workIdStr, result);
    EXPECT_EQ(result, "uIdStr param invalid, uIdStr:0\n");
}

/**
 * @tc.name: DumpTriggerWork_003
 * @tc.desc: Test WorkSchedulerService DumpTriggerWork.
 * @tc.type: FUNC
 * @tc.require: IC5H77
 */
HWTEST_F(WorkSchedulerServiceTest, DumpTriggerWork_003, TestSize.Level1)
{
    std::string uIdStr = "123";
    std::string workIdStr = "0";
    std::string result;
    workSchedulerService_->DumpTriggerWork(uIdStr, workIdStr, result);
    EXPECT_EQ(result, "workIdStr param invalid, workIdStr:0\n");
}

/**
 * @tc.name: DumpTriggerWork_004
 * @tc.desc: Test WorkSchedulerService DumpTriggerWork.
 * @tc.type: FUNC
 * @tc.require: IC5H77
 */
HWTEST_F(WorkSchedulerServiceTest, DumpTriggerWork_004, TestSize.Level1)
{
    std::string uIdStr = "123";
    std::string workIdStr = "456";
    std::string result;
    workSchedulerService_->DumpTriggerWork(uIdStr, workIdStr, result);
    EXPECT_EQ(result, "the work is not exist\n");
}

/**
 * @tc.name: StartWorkForInner_001
 * @tc.desc: Test WorkSchedulerService StartWorkForInner.
 * @tc.type: FUNC
 * @tc.require: issue:#ICBWOI
 */
HWTEST_F(WorkSchedulerServiceTest, StartWorkForInner_001, TestSize.Level1)
{
    int32_t ret;
    WorkInfo workinfo = WorkInfo();

    ret = workSchedulerService_->StartWorkForInner(workinfo);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GetNativeToken(PUSH_SERVICE_NAME);
    ret = workSchedulerService_->StartWorkForInner(workinfo);
    EXPECT_TRUE(ret);
    GetNativeToken(BGTASK_SERVICE_NAME);
}

/**
 * @tc.name: StopWorkForInner_001
 * @tc.desc: Test WorkSchedulerService StopWorkForInner.
 * @tc.type: FUNC
 * @tc.require: issue:#ICBWOI
 */
HWTEST_F(WorkSchedulerServiceTest, StopWorkForInner_001, TestSize.Level1)
{
    int32_t ret;
    WS_HILOGI("WorkSchedulerServiceTest.StopWorkForInner_001 begin");
    workSchedulerService_->ready_ = false;
    workSchedulerService_->checkBundle_ = true;
    WorkInfo workinfo = WorkInfo();

    ret = workSchedulerService_->StopWorkForInner(workinfo, false);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);

    GetNativeToken(PUSH_SERVICE_NAME);
    ret = workSchedulerService_->StopWorkForInner(workinfo, false);
    EXPECT_TRUE(ret);

    workSchedulerService_->ready_ = true;
    ret = workSchedulerService_->StopWorkForInner(workinfo, false);
    EXPECT_TRUE(ret);
    GetNativeToken(BGTASK_SERVICE_NAME);
    WS_HILOGI("WorkSchedulerServiceTest.StopWorkForInner_001 end");
}

/**
 * @tc.name: HasDeepIdleTime_001
 * @tc.desc: Test WorkSchedulerService HasDeepIdleTime.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, HasDeepIdleTime_001, TestSize.Level1)
{
    int32_t saId = 1;
    int32_t time = 1000;
    int32_t uid = 2;
    std::map<int32_t, std::pair<int32_t, int32_t>> deepIdleTimeMap = workSchedulerService_->GetDeepIdleTimeMap();
    deepIdleTimeMap.emplace(saId, std::pair<int32_t, int32_t>{time, uid});
    workSchedulerService_->deepIdleTimeMap_ = deepIdleTimeMap;
    bool ret = workSchedulerService_->HasDeepIdleTime();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: HasDeepIdleTime_002
 * @tc.desc: Test WorkSchedulerService HasDeepIdleTime.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, HasDeepIdleTime_002, TestSize.Level1)
{
    int32_t saId = 1;
    int32_t time = 1000;
    int32_t uid = 2;
    workSchedulerService_->deepIdleTimeMap_.clear();
    workSchedulerService_->AddDeepIdleTimeToMap(saId, time, uid);
    bool ret = workSchedulerService_->HasDeepIdleTime();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: NeedCreateTimer_001
 * @tc.desc: Test WorkSchedulerService NeedCreateTimer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, NeedCreateTimer_001, TestSize.Level1)
{
    int32_t saId = 1;
    int32_t time = 1000;
    int32_t uid = 2;
    bool ret = workSchedulerService_->NeedCreateTimer(saId, uid, time);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: NeedCreateTimer_002
 * @tc.desc: Test WorkSchedulerService NeedCreateTimer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, NeedCreateTimer_002, TestSize.Level1)
{
    int32_t saId = 1;
    int32_t time = 20 * 60 * 1000;
    int32_t uid = 1;
    int32_t workId = 1;
    WorkInfo workinfo = WorkInfo();
    workinfo.SetWorkId(workId);
    workinfo.RefreshSaId(saId);
    workinfo.RefreshUid(uid);
    workinfo.RequestDeepIdle(true);
    workinfo.SetDeepIdleTime(time);
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    auto instance = DelayedSingleton<WorkSchedulerService>::GetInstance();
    std::shared_ptr<WorkPolicyManager> workPolicyManager = std::make_shared<WorkPolicyManager>(instance);
    workSchedulerService_->workPolicyManager_ = workPolicyManager;
    int32_t ret = workSchedulerService_->workPolicyManager_->AddWork(workStatus, uid);
    if (ret == ERR_OK) {
        bool result = workSchedulerService_->NeedCreateTimer(saId, uid, time);
        EXPECT_EQ(result, true);
    }
}

/**
 * @tc.name: NeedCreateTimer_003
 * @tc.desc: Test WorkSchedulerService NeedCreateTimer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, NeedCreateTimer_003, TestSize.Level1)
{
    int32_t saId = 2;
    int32_t deepIdleTime = 1 * 60 * 1000;
    int32_t uid = 2;
    int32_t workId = 1;
    uint32_t timeInterval = 7 * 24 * 60 * 60 * 1000;
    time_t baseTime;
    (void)time(&baseTime);
    WorkInfo workinfo = WorkInfo();
    workinfo.SetWorkId(workId);
    workinfo.RefreshSaId(saId);
    workinfo.RefreshUid(uid);
    workinfo.RequestDeepIdle(true);
    workinfo.SetDeepIdleTime(deepIdleTime);
    workinfo.RequestRepeatCycle(timeInterval);
    workinfo.RequestBaseTime(baseTime);
    std::shared_ptr<WorkStatus> workStatus = std::make_shared<WorkStatus>(workinfo, uid);
    auto instance = DelayedSingleton<WorkSchedulerService>::GetInstance();
    std::shared_ptr<WorkPolicyManager> workPolicyManager = std::make_shared<WorkPolicyManager>(instance);
    workSchedulerService_->workPolicyManager_ = workPolicyManager;
    int32_t ret = workSchedulerService_->workPolicyManager_->AddWork(workStatus, uid);
    if (ret == ERR_OK) {
        bool result = workSchedulerService_->NeedCreateTimer(saId, uid, deepIdleTime);
        EXPECT_EQ(result, false);
    }
}

/**
 * @tc.name: CreateNodeFile_001
 * @tc.desc: Test WorkSchedulerService CreateNodeFile.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, CreateNodeFile_001, TestSize.Level1)
{
    bool ret = workSchedulerService_->CreateNodeFile();
    EXPECT_TRUE(ret);
}


HWTEST_F(WorkSchedulerServiceTest, RegisterTask_ServiceNotReady_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    int32_t ret = workSchedulerService_->RegisterTask(taskInfo);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(WorkSchedulerServiceTest, UnregisterTask_ServiceNotReady_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    int32_t ret = workSchedulerService_->UnregisterTask(taskInfo);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(WorkSchedulerServiceTest, FinishTask_ServiceNotReady_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    int32_t ret = workSchedulerService_->FinishTask(taskInfo);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(WorkSchedulerServiceTest, GetTaskInfo_ServiceNotReady_001, TestSize.Level1)
{
    BackgroundLoaderTaskInfo taskInfo;
    int32_t ret = workSchedulerService_->GetTaskInfo(1, taskInfo);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

/**
 * @tc.name: GetMinCheckTime_001
 * @tc.desc: Test WorkSchedulerService GetMinCheckTime.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, GetMinCheckTime_001, TestSize.Level1)
{
    uint32_t minCheckTime = 10 * 60 * 1000;
    workSchedulerService_->SetMinCheckTime(minCheckTime);
    EXPECT_EQ(workSchedulerService_->GetMinCheckTime(), minCheckTime);
}

/**
 * @tc.name: UpdateCloudConfigMinRepeatTime_001
 * @tc.desc: Test WorkSchedulerService UpdateCloudConfigMinRepeatTime.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, UpdateCloudConfigMinRepeatTime_001, TestSize.Level1)
{
    workSchedulerService_->ClearSpecialToMap();
    std::string bundleName = "com.ohos.demo";
    uint32_t time = 2000;
    workSchedulerService_->AddSpecialToMap(bundleName, time);
    workSchedulerService_->ready_ = false;
    auto specialRoot = nlohmann::json::array();
    workSchedulerService_->UpdateCloudConfigMinRepeatTime(specialRoot);
    EXPECT_FALSE(workSchedulerService_->specialMap_.empty());

    workSchedulerService_->ready_ = true;
    auto special = nlohmann::json::array();
    specialRoot.push_back(special);
    workSchedulerService_->UpdateCloudConfigMinRepeatTime(specialRoot);
    EXPECT_TRUE(workSchedulerService_->specialMap_.empty());

    auto specialRoot2 = nlohmann::json::array();
    auto special2 = nlohmann::json::object();
    specialRoot2.push_back(special2);
    workSchedulerService_->UpdateCloudConfigMinRepeatTime(specialRoot2);
    EXPECT_TRUE(workSchedulerService_->specialMap_.empty());

    special2["bundleName"] = 1;
    specialRoot2.push_back(special2);
    workSchedulerService_->UpdateCloudConfigMinRepeatTime(specialRoot2);
    EXPECT_TRUE(workSchedulerService_->specialMap_.empty());

    special2["bundleName"] = bundleName;
    specialRoot2.push_back(special2);
    workSchedulerService_->UpdateCloudConfigMinRepeatTime(specialRoot2);
    EXPECT_TRUE(workSchedulerService_->specialMap_.empty());

    special2["time"] = -1;
    specialRoot2.push_back(special2);
    workSchedulerService_->UpdateCloudConfigMinRepeatTime(specialRoot2);
    EXPECT_TRUE(workSchedulerService_->specialMap_.empty());

    special2["time"] = time;
    specialRoot2.push_back(special2);
    workSchedulerService_->UpdateCloudConfigMinRepeatTime(specialRoot2);
    EXPECT_TRUE(workSchedulerService_->specialMap_.empty());

    special2["time"] = time;
    specialRoot2.push_back(special2);
    workSchedulerService_->UpdateCloudConfigMinRepeatTime(specialRoot2);
    EXPECT_TRUE(workSchedulerService_->specialMap_.empty());

    time = 12000000;
    special2["time"] = time;
    specialRoot2.push_back(special2);
    workSchedulerService_->UpdateCloudConfigMinRepeatTime(specialRoot2);
    EXPECT_FALSE(workSchedulerService_->specialMap_.empty());
}

/**
 * @tc.name: UpdateCloudConfigEngExemptionBundles_001
 * @tc.desc: Test WorkSchedulerService UpdateCloudConfigEngExemptionBundles.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, UpdateCloudConfigEngExemptionBundles_001, TestSize.Level1)
{
    workSchedulerService_->ClearExemptionBundles();
    std::string bundleName = "com.ohos.demo";
    workSchedulerService_->InsertExemptionBundles(bundleName);
    workSchedulerService_->ready_ = false;
    auto root = nlohmann::json::array();
    workSchedulerService_->UpdateCloudConfigEngExemptionBundles(root);
    EXPECT_FALSE(workSchedulerService_->exemptionBundles_.empty());

    workSchedulerService_->ready_ = true;
    workSchedulerService_->UpdateCloudConfigEngExemptionBundles(root);
    EXPECT_TRUE(workSchedulerService_->exemptionBundles_.empty());

    root.push_back(1);
    workSchedulerService_->UpdateCloudConfigEngExemptionBundles(root);
    EXPECT_TRUE(workSchedulerService_->exemptionBundles_.empty());

    auto root2 = nlohmann::json::array();
    root2.push_back(bundleName);
    workSchedulerService_->UpdateCloudConfigEngExemptionBundles(root2);
    EXPECT_FALSE(workSchedulerService_->exemptionBundles_.empty());
}

/**
 * @tc.name: CheckCloudConfigPreinstallDelete_001
 * @tc.desc: Test WorkSchedulerService CheckCloudConfigPreinstallDelete.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, CheckCloudConfigPreinstallDelete_001, TestSize.Level1)
{
    nlohmann::json root;
    EXPECT_FALSE(workSchedulerService_->CheckCloudConfigPreinstallDelete(root));
    
    auto root2 = nlohmann::json::array();
    EXPECT_FALSE(workSchedulerService_->CheckCloudConfigPreinstallDelete(root2));

    auto root3 = nlohmann::json::object();
    EXPECT_FALSE(workSchedulerService_->CheckCloudConfigPreinstallDelete(root3));

    root3["delete"] = true;
    EXPECT_TRUE(workSchedulerService_->CheckCloudConfigPreinstallDelete(root3));
}

/**
 * @tc.name: DeleteSaWork_001
 * @tc.desc: Test WorkSchedulerService DeleteSaWork.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, DeleteSaWork_001, TestSize.Level1)
{
    workSchedulerService_->deepIdleTimeMap_.clear();
    int32_t saId = 9999;
    int32_t deepIdleTime = 300000;
    int32_t uid = 1;
    workSchedulerService_->AddDeepIdleTimeToMap(saId, deepIdleTime, uid);
    workSchedulerService_->DeleteSaWork(nullptr);
    EXPECT_TRUE(workSchedulerService_->HasDeepIdleTime());

    std::shared_ptr<WorkInfo> workInfo = std::make_shared<WorkInfo>();
    workInfo->RefreshSaId(0);
    workSchedulerService_->DeleteSaWork(workInfo);
    EXPECT_TRUE(workSchedulerService_->HasDeepIdleTime());

    workInfo->RefreshSaId(saId);
    workSchedulerService_->DeleteSaWork(workInfo);
    EXPECT_TRUE(workSchedulerService_->HasDeepIdleTime());
}

/**
 * @tc.name: DeleteAppWork_001
 * @tc.desc: Test WorkSchedulerService DeleteAppWork.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, DeleteAppWork_001, TestSize.Level1)
{
    int32_t saId = 9999;
    std::shared_ptr<WorkInfo> workInfo = std::make_shared<WorkInfo>();
    workInfo->RefreshSaId(saId);
    std::string workId = "u" + std::to_string(workInfo->GetUid()) + "_" + std::to_string(workInfo->GetWorkId());
    workSchedulerService_->DeleteAppWork(nullptr);
    EXPECT_FALSE(workSchedulerService_->CheckPreinstalledWorkId(workId));
    
    workSchedulerService_->InsertPreinstalledWorkId(workId);
    workSchedulerService_->DeleteAppWork(workInfo);
    EXPECT_TRUE(workSchedulerService_->CheckPreinstalledWorkId(workId));

    workInfo->RefreshSaId(0);
    workSchedulerService_->DeleteAppWork(workInfo);
    EXPECT_TRUE(workSchedulerService_->CheckPreinstalledWorkId(workId));
}

/**
 * @tc.name: StopCloudConfigWork_001
 * @tc.desc: Test WorkSchedulerService StopCloudConfigWork.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, StopCloudConfigWork_001, TestSize.Level1)
{
    std::string workId = "";
    workSchedulerService_->StopCloudConfigWork(workId, nullptr);
    EXPECT_FALSE(workSchedulerService_->CheckPreinstalledWorkId(workId));

    workId = "u1_1";
    workSchedulerService_->InsertPreinstalledWorkId(workId);
    workSchedulerService_->StopCloudConfigWork(workId, nullptr);
    EXPECT_TRUE(workSchedulerService_->CheckPreinstalledWorkId(workId));

    std::shared_ptr<WorkInfo> workInfo = std::make_shared<WorkInfo>();
    workSchedulerService_->RemovePreinstalledWorkId(workId);
    workSchedulerService_->StopCloudConfigWork(workId, workInfo);
    EXPECT_FALSE(workSchedulerService_->CheckPreinstalledWorkId(workId));

    workSchedulerService_->InsertPreinstalledWorkId(workId);
    workSchedulerService_->StopCloudConfigWork(workId, workInfo);
    EXPECT_FALSE(workSchedulerService_->CheckPreinstalledWorkId(workId));
}

// ======================== SetExecFrequency Tests ========================

/**
 * @tc.name: SetExecFrequency_001
 * @tc.desc: Test SetExecFrequency service not ready.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, SetExecFrequency_001, TestSize.Level1)
{
    workSchedulerService_->ready_ = false;
    FrequencyInfo info;
    int32_t ret = workSchedulerService_->SetExecFrequency(info);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);
    workSchedulerService_->ready_ = true;
}

// ======================== ResetExecFrequency Tests ========================

/**
 * @tc.name: ResetExecFrequency_001
 * @tc.desc: Test ResetExecFrequency service not ready.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, ResetExecFrequency_001, TestSize.Level1)
{
    workSchedulerService_->ready_ = false;
    int32_t uid = 1;
    int32_t ret = workSchedulerService_->ResetExecFrequency(uid);
    EXPECT_EQ(ret, E_SERVICE_NOT_READY);
    workSchedulerService_->ready_ = true;
}

// ======================== GetExecFrequency Tests ========================

/**
 * @tc.name: GetExecFrequency_001
 * @tc.desc: Test GetExecFrequency when no frequency is set.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, GetExecFrequency_001, TestSize.Level1)
{
    workSchedulerService_->ClearExecFrequency();
    int64_t result = workSchedulerService_->GetExecFrequency(100);
    EXPECT_EQ(result, INVALID_VALUE);
}

/**
 * @tc.name: GetExecFrequency_002
 * @tc.desc: Test GetExecFrequency after setting frequency.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, GetExecFrequency_002, TestSize.Level1)
{
    workSchedulerService_->ClearExecFrequency();
    FrequencyInfo freqInfo = FrequencyInfo();
    freqInfo.SetWorkId(1);
    freqInfo.SetUid(100);
    freqInfo.SetInterval(86400000);
    workSchedulerService_->SetExecFrequencyInner(1001, freqInfo);
    int64_t result = workSchedulerService_->GetExecFrequency(100);
    EXPECT_EQ(result, 86400000);
    workSchedulerService_->ClearExecFrequency();
}

// ======================== ResetExecFrequencyByUid Tests ========================

/**
 * @tc.name: ResetExecFrequencyByUid_001
 * @tc.desc: Test ResetExecFrequencyByUid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, ResetExecFrequencyByUid_001, TestSize.Level1)
{
    workSchedulerService_->ClearExecFrequency();
    FrequencyInfo freqInfo = FrequencyInfo();
    freqInfo.SetWorkId(1);
    freqInfo.SetUid(200);
    freqInfo.SetInterval(86400000);
    workSchedulerService_->SetExecFrequencyInner(1001, freqInfo);

    bool ret = workSchedulerService_->ResetExecFrequencyByUid(200);
    EXPECT_TRUE(ret);
    auto result = workSchedulerService_->GetExecFrequency(200);
    EXPECT_EQ(result, INVALID_VALUE);
    workSchedulerService_->ClearExecFrequency();
}

/**
 * @tc.name: ResetExecFrequencyByUid_002
 * @tc.desc: Test ResetExecFrequencyByUid when uid not set.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, ResetExecFrequencyByUid_002, TestSize.Level1)
{
    workSchedulerService_->ClearExecFrequency();
    bool ret = workSchedulerService_->ResetExecFrequencyByUid(999);
    EXPECT_FALSE(ret);
}

// ======================== ClearExecFrequency Tests ========================

/**
 * @tc.name: ClearExecFrequency_001
 * @tc.desc: Test ClearExecFrequency.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, ClearExecFrequency_001, TestSize.Level1)
{
    workSchedulerService_->ClearExecFrequency();
    FrequencyInfo freqInfo = FrequencyInfo();
    freqInfo.SetWorkId(1);
    freqInfo.SetUid(100);
    freqInfo.SetInterval(86400000);
    workSchedulerService_->SetExecFrequencyInner(1001, freqInfo);

    auto result = workSchedulerService_->GetExecFrequency(100);
    EXPECT_NE(result, INVALID_VALUE);

    workSchedulerService_->ClearExecFrequency();
    result = workSchedulerService_->GetExecFrequency(100);
    EXPECT_EQ(result, INVALID_VALUE);
}

// ======================== FrequencyMap JSON Tests ========================

/**
 * @tc.name: FrequencyMap_ParseToJson_001
 * @tc.desc: Test FrequencyMap parse to json.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, FrequencyMap_ParseToJson_001, TestSize.Level1)
{
    workSchedulerService_->ClearExecFrequency();
    FrequencyInfo freqInfo = FrequencyInfo();
    freqInfo.SetWorkId(1);
    freqInfo.SetUid(100);
    freqInfo.SetInterval(86400000);
    workSchedulerService_->SetExecFrequencyInner(1001, freqInfo);

    std::string jsonStr = workSchedulerService_->ParseFrequencyMapToJsonStr();
    EXPECT_FALSE(jsonStr.empty());

    nlohmann::json root = nlohmann::json::parse(jsonStr);
    EXPECT_TRUE(root.contains("frequency_infos"));
    workSchedulerService_->ClearExecFrequency();
}

/**
 * @tc.name: FrequencyMap_ParseToJson_002
 * @tc.desc: Test FrequencyMap parse to json when empty.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, FrequencyMap_ParseToJson_002, TestSize.Level1)
{
    workSchedulerService_->ClearExecFrequency();
    std::string jsonStr = workSchedulerService_->ParseFrequencyMapToJsonStr();
    EXPECT_FALSE(jsonStr.empty());

    nlohmann::json root = nlohmann::json::parse(jsonStr);
    EXPECT_TRUE(root.contains("frequency_infos"));
    EXPECT_TRUE(root["frequency_infos"].empty());
}

// ======================== ResetExecFrequencyWhenAppRemove Tests ========================

/**
 * @tc.name: ResetExecFrequencyWhenAppRemove_001
 * @tc.desc: Test ResetExecFrequencyWhenAppRemove removes frequency for uid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, ResetExecFrequencyWhenAppRemove_001, TestSize.Level1)
{
    workSchedulerService_->ClearExecFrequency();
    FrequencyInfo freqInfo = FrequencyInfo();
    freqInfo.SetWorkId(1);
    freqInfo.SetUid(300);
    freqInfo.SetInterval(86400000);
    workSchedulerService_->SetExecFrequencyInner(1001, freqInfo);

    auto result = workSchedulerService_->GetExecFrequency(300);
    EXPECT_NE(result, INVALID_VALUE);

    workSchedulerService_->ResetExecFrequencyWhenAppRemove(300);

    result = workSchedulerService_->GetExecFrequency(300);
    EXPECT_EQ(result, INVALID_VALUE);
    workSchedulerService_->ClearExecFrequency();
}

// ======================== DumpAppGroup Edge Cases ========================

/**
 * @tc.name: DumpAppGroup_Params_001
 * @tc.desc: Test DumpAppGroup with invalid group (0).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, DumpAppGroup_Params_001, TestSize.Level1)
{
    WorkStatus::ClearDumpAppGroup(1000);
    std::vector<std::string> argsInStr;
    std::string result;
    argsInStr.push_back("-group");
    argsInStr.push_back("1000");
    argsInStr.push_back("0");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(WorkStatus::GetDumpAppGroup(1000), INVALID_VALUE);
}

/**
 * @tc.name: DumpAppGroup_Params_002
 * @tc.desc: Test DumpAppGroup with negative group.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, DumpAppGroup_Params_002, TestSize.Level1)
{
    WorkStatus::ClearDumpAppGroup(1000);
    std::vector<std::string> argsInStr;
    std::string result;
    argsInStr.push_back("-group");
    argsInStr.push_back("1000");
    argsInStr.push_back("-5");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(WorkStatus::GetDumpAppGroup(1000), INVALID_VALUE);
}

/**
 * @tc.name: DumpAppGroup_Params_003
 * @tc.desc: Test DumpAppGroup with INVALID_VALUE group (clears group).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, DumpAppGroup_Params_003, TestSize.Level1)
{
    WorkStatus::AddDumpAppGroup(1000, 20);
    EXPECT_EQ(WorkStatus::GetDumpAppGroup(1000), 20);

    std::vector<std::string> argsInStr;
    std::string result;
    argsInStr.push_back("-group");
    argsInStr.push_back("1000");
    argsInStr.push_back(std::to_string(INVALID_VALUE));
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    EXPECT_EQ(WorkStatus::GetDumpAppGroup(1000), INVALID_VALUE);
    WorkStatus::ClearDumpAppGroup(1000);
}

/**
 * @tc.name: DumpAppGroup_Params_004
 * @tc.desc: Test DumpAppGroup with valid group sets app group.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, DumpAppGroup_Params_004, TestSize.Level1)
{
    WorkStatus::ClearDumpAppGroup(1000);
    std::vector<std::string> argsInStr;
    std::string result;
    argsInStr.push_back("-group");
    argsInStr.push_back("1000");
    argsInStr.push_back("10");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    EXPECT_EQ(WorkStatus::GetDumpAppGroup(1000), 10);
    WorkStatus::ClearDumpAppGroup(1000);
}

// ======================== InitPersistedInfos Tests ========================

/**
 * @tc.name: InitPersistedInfos_002
 * @tc.desc: Test InitPersistedInfos when file exists but has no frequency_infos key.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, InitPersistedInfos_002, TestSize.Level1)
{
    const char* dirPath = "/data/service/el1/public/WorkScheduler";
    const char* filePath = "/data/service/el1/public/WorkScheduler/persisted_info";
    if (mkdir(dirPath, 0755) != 0 && errno != EEXIST) {
        GTEST_SKIP() << "Cannot create directory: " << dirPath;
    }
    FILE* fp = fopen(filePath, "w");
    if (fp == nullptr) {
        GTEST_SKIP() << "Cannot write file: " << filePath;
    }
    const char* jsonContent = "{\"other_key\":[]}";
    fputs(jsonContent, fp);
    fclose(fp);

    workSchedulerService_->ClearExecFrequency();
    workSchedulerService_->InitPersistedInfos();
    auto result = workSchedulerService_->GetExecFrequency(100);
    EXPECT_EQ(result, INVALID_VALUE);
    workSchedulerService_->ClearExecFrequency();
    remove(filePath);
}

/**
 * @tc.name: InitPersistedInfos_003
 * @tc.desc: Test InitPersistedInfos when file exists with empty frequency_infos.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WorkSchedulerServiceTest, InitPersistedInfos_003, TestSize.Level1)
{
    const char* dirPath = "/data/service/el1/public/WorkScheduler";
    const char* filePath = "/data/service/el1/public/WorkScheduler/persisted_info";
    if (mkdir(dirPath, 0755) != 0 && errno != EEXIST) {
        GTEST_SKIP() << "Cannot create directory: " << dirPath;
    }
    FILE* fp = fopen(filePath, "w");
    if (fp == nullptr) {
        GTEST_SKIP() << "Cannot write file: " << filePath;
    }
    const char* jsonContent = "{\"frequency_infos\":{}}";
    fputs(jsonContent, fp);
    fclose(fp);

    workSchedulerService_->ClearExecFrequency();
    workSchedulerService_->InitPersistedInfos();
    auto result = workSchedulerService_->GetExecFrequency(100);
    EXPECT_EQ(result, INVALID_VALUE);
    workSchedulerService_->ClearExecFrequency();
    remove(filePath);
}
}
}