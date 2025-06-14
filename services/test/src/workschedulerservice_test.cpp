/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "json/json.h"

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

class MyWorkSchedulerService : public WorkSchedServiceStub {
    int32_t StartWork(const WorkInfo& workInfo) { return 0; }
    int32_t StopWork(const WorkInfo& workInfo) { return 0; };
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
};
/**
 * @tc.name: onStart_001
 * @tc.desc: Test WorkSchedulerService OnStart.
 * @tc.type: FUNC
 * @tc.require: I8ZDJI
 */
HWTEST_F(WorkSchedulerServiceTest, onStart_001, TestSize.Level0)
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
    argsInStr.push_back("600000");
    workSchedulerService_->DumpProcessForEngMode(argsInStr, result);
    WS_HILOGI("%{public}s", result.c_str());

    result.clear();
    argsInStr.clear();
    argsInStr.push_back("-min_interval");
    argsInStr.push_back("7200000");
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

HWTEST_F(WorkSchedulerServiceTest, WorkStandbyStateChangeCallbackTest_001, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.WorkStandbyStateChangeCallbackTest_001 begin ====== ");
    workSchedulerService_->standbyStateObserver_->OnDeviceIdleMode(true, false);
    workSchedulerService_->standbyStateObserver_->OnDeviceIdleMode(true, true);
    workSchedulerService_->standbyStateObserver_->OnDeviceIdleMode(false, true);
    workSchedulerService_->standbyStateObserver_->OnAllowListChanged(0, "bundlename", 0, true);
    EXPECT_NE(workSchedulerService_, nullptr);
    WS_HILOGI("====== WorkSchedulerServiceTest.WorkStandbyStateChangeCallbackTest_001 end ====== ");
}

HWTEST_F(WorkSchedulerServiceTest, WorkBundleGroupChangeCallback_001, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.WorkBundleGroupChangeCallback_001 begin ====== ");
    OHOS::DeviceUsageStats::AppGroupCallbackInfo appGroupCallbackInfo1(0, 1, 2, 0, "bundlename");
    workSchedulerService_->groupObserver_->OnAppGroupChanged(appGroupCallbackInfo1);
    OHOS::DeviceUsageStats::AppGroupCallbackInfo appGroupCallbackInfo2(0, 2, 1, 0, "bundlename");
    workSchedulerService_->groupObserver_->OnAppGroupChanged(appGroupCallbackInfo2);
    EXPECT_NE(workSchedulerService_, nullptr);
    WS_HILOGI("====== WorkSchedulerServiceTest.WorkBundleGroupChangeCallback_001 end ====== ");
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

HWTEST_F(WorkSchedulerServiceTest, WorkQueueEventHandler_001, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.WorkQueueEventHandler_001 begin ====== ");
    WorkQueueEventHandler handler(nullptr, nullptr);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(0);
    handler.ProcessEvent(event);
    event = AppExecFwk::InnerEvent::Get(1);
    handler.ProcessEvent(event);
    event = AppExecFwk::InnerEvent::Get(2);
    handler.ProcessEvent(event);
    EXPECT_TRUE(handler.manager_ == nullptr);
    WS_HILOGI("====== WorkSchedulerServiceTest.WorkQueueEventHandler_001 end ====== ");
}

HWTEST_F(WorkSchedulerServiceTest, WorkEventHandler_001, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.WorkEventHandler_001 begin ====== ");
    WorkEventHandler handler(workSchedulerService_->eventRunner_, workSchedulerService_);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(0);
    handler.ProcessEvent(event);
    event = AppExecFwk::InnerEvent::Get(1);
    handler.ProcessEvent(event);
    event = AppExecFwk::InnerEvent::Get(2);
    handler.ProcessEvent(event);
    event = AppExecFwk::InnerEvent::Get(3);
    handler.ProcessEvent(event);
    event = AppExecFwk::InnerEvent::Get(4);
    handler.ProcessEvent(event);
    event = AppExecFwk::InnerEvent::Get(5);
    handler.ProcessEvent(event);
    EXPECT_TRUE(workSchedulerService_->eventRunner_ != nullptr);
    WS_HILOGI("====== WorkSchedulerServiceTest.WorkEventHandler_001 end ====== ");
}

HWTEST_F(WorkSchedulerServiceTest, BatteryLevelListener_001, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.BatteryLevelListener_001 begin ====== ");

    BatteryLevelListener batteryLevelListener(workSchedulerService_->workQueueManager_, workSchedulerService_);

    batteryLevelListener.Start();
    EXPECT_NE(batteryLevelListener.commonEventSubscriber, nullptr);

    EventFwk::CommonEventData data;
    batteryLevelListener.commonEventSubscriber->OnReceiveEvent(data);

    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    data.SetWant(want);
    batteryLevelListener.commonEventSubscriber->OnReceiveEvent(data);

    want.SetParam(PowerMgr::BatteryInfo::COMMON_EVENT_KEY_CAPACITY, 20);
    data.SetWant(want);
    batteryLevelListener.commonEventSubscriber->OnReceiveEvent(data);
    batteryLevelListener.Stop();

    WS_HILOGI("====== WorkSchedulerServiceTest.BatteryLevelListener_001 end ====== ");
}

HWTEST_F(WorkSchedulerServiceTest, BatteryStatusListener_001, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.BatteryStatusListener_001 begin ====== ");
    BatteryStatusListener batteryStatusListener(workSchedulerService_->workQueueManager_);

    batteryStatusListener.Start();
    EXPECT_NE(batteryStatusListener.commonEventSubscriber, nullptr);

    EventFwk::CommonEventData data;
    batteryStatusListener.commonEventSubscriber->OnReceiveEvent(data);

    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW);
    data.SetWant(want);
    batteryStatusListener.commonEventSubscriber->OnReceiveEvent(data);

    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY);
    data.SetWant(want);
    batteryStatusListener.commonEventSubscriber->OnReceiveEvent(data);
    batteryStatusListener.Stop();

    WS_HILOGI("====== WorkSchedulerServiceTest.BatteryStatusListener_001 end ====== ");
}

HWTEST_F(WorkSchedulerServiceTest, ChargerListener_001, TestSize.Level1)
{
    WS_HILOGI("====== WorkSchedulerServiceTest.ChargerListener_001 begin ====== ");
    ChargerListener chargerListener(workSchedulerService_->workQueueManager_);

    chargerListener.Start();
    EXPECT_NE(chargerListener.commonEventSubscriber, nullptr);

    EventFwk::CommonEventData data;
    chargerListener.commonEventSubscriber->OnReceiveEvent(data);

    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    data.SetWant(want);
    data.SetCode(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_AC));
    chargerListener.commonEventSubscriber->OnReceiveEvent(data);

    data.SetCode(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB));
    chargerListener.commonEventSubscriber->OnReceiveEvent(data);


    data.SetCode(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS));
    chargerListener.commonEventSubscriber->OnReceiveEvent(data);

    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    data.SetWant(want);
    data.SetCode(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_NONE));
    chargerListener.commonEventSubscriber->OnReceiveEvent(data);
    chargerListener.Stop();

    WS_HILOGI("====== WorkSchedulerServiceTest.ChargerListener_001 end ====== ");
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

HWTEST_F(WorkSchedulerServiceTest, WorkSchedServiceStub_001, TestSize.Level1)
{
    MyWorkSchedulerService s;
    MessageParcel data, reply;
    MessageOption option;
    const int size = 11;
    for (int i = 0; i < size; i++) {
        s.OnRemoteRequest(i, data, reply, option);
        WorkInfo info;
        info.Marshalling(data);
        s.OnRemoteRequest(i, data, reply, option);
    }
    int32_t ret = s.OnRemoteRequest(0, data, reply, option);
    EXPECT_TRUE(ret != ERR_OK);
}

/**
 * @tc.name: SendEvent_001
 * @tc.desc: Test WorkSchedulerService SendEvent.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkSchedulerServiceTest, SendEvent_001, TestSize.Level1)
{
    int32_t initDelay = 2 * 1000;
    workSchedulerService_->GetHandler()->
        SendEvent(AppExecFwk::InnerEvent::Get(WorkEventHandler::SERVICE_INIT_MSG, 0), initDelay);
    EXPECT_TRUE(workSchedulerService_->ready_);
}

/**
 * @tc.name: GetJsonFromFile_001
 * @tc.desc: Test WorkSchedulerService GetJsonFromFile.
 * @tc.type: FUNC
 * @tc.require: I9J0A7
 */
HWTEST_F(WorkSchedulerServiceTest, GetJsonFromFile_001, TestSize.Level1)
{
    Json::Value root;
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
 * @tc.name: CheckExtensionInfos_001
 * @tc.desc: Test WorkSchedulerService CheckExtensionInfos.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, CheckExtensionInfos_001, TestSize.Level1)
{
    int32_t uid = 1;
    WorkInfo workInfo = WorkInfo();
    workInfo.SetWorkId(1);
    workInfo.SetElement("bundleName", "abilityName");
    bool ret = workSchedulerService_->CheckExtensionInfos(workInfo, uid);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: InitBgTaskSubscriber_001
 * @tc.desc: Test WorkSchedulerService InitBgTaskSubscriber.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(WorkSchedulerServiceTest, InitBgTaskSubscriber_001, TestSize.Level1)
{
    bool ret = workSchedulerService_->InitBgTaskSubscriber();
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
    workSchedulerService_->exemptionBundles_.insert(bundleName);
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
    workSchedulerService_->exemptionBundles_.insert("com.demo.bundle1");
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
    workSchedulerService_->preinstalledBundles_.insert(bundleName);
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
    workSchedulerService_->preinstalledBundles_.insert("com.demo.bundle1");
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
 * @tc.require: I9J0A7
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
}
}