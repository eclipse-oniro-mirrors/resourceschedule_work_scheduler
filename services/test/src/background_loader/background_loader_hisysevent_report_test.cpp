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
#include <hisysevent.h>
#include "background_loader/background_loader_hisysevent_report.h"
#include "background_loader/background_loader_mgr.h"

using namespace testing::ext;
namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr int32_t TEST_TASK_ID = 100;
constexpr int32_t TEST_UID = 20001;
constexpr int32_t TEST_PID = 30001;
}

class BackgroundLoaderHiSysEventReportTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

static TaskInfo MakeTaskInfo()
{
    TaskInfo info;
    info.bundleName_ = "com.test.bundle";
    info.abilityName_ = "TestAbility";
    info.appIndex_ = 0;
    info.taskId_ = TEST_TASK_ID;
    info.status_ = TaskStatus::RUNNING;
    info.uid_ = TEST_UID;
    info.pid_ = TEST_PID;
    info.timeoutCount_ = 1;
    return info;
}

/**
 * @tc.name: ReportRecordSysEvent_001
 * @tc.desc: Test ReportRecordSysEvent writes an equivalent event successfully.
 *           Listening/persistence verification belongs to integration tests.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderHiSysEventReportTest, ReportRecordSysEvent_001, TestSize.Level1)
{
    TaskInfo info = MakeTaskInfo();
    BackgroundLoaderHiSysEvent::ReportRecordSysEvent(info, "Register Task", "");

    int ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        "BACKGROUND_LOADER_RECORD", HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "UID", info.uid_, "PID", info.pid_, "BUNDLE_NAME", info.bundleName_, "APP_INDEX",
        info.appIndex_, "REASON", "Register Task", "EXTEND_REASON", "");
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ReportRecordSysEvent_Boundary_001
 * @tc.desc: Test ReportRecordSysEvent handles empty reason and extReason.
 *           Listening/persistence verification belongs to integration tests.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderHiSysEventReportTest, ReportRecordSysEvent_Boundary_001, TestSize.Level1)
{
    TaskInfo info = MakeTaskInfo();
    BackgroundLoaderHiSysEvent::ReportRecordSysEvent(info, "", "");

    int ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        "BACKGROUND_LOADER_RECORD", HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "UID", info.uid_, "PID", info.pid_, "BUNDLE_NAME", info.bundleName_, "APP_INDEX",
        info.appIndex_, "REASON", "", "EXTEND_REASON", "");
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ReportAnomalousSysEvent_001
 * @tc.desc: Test ReportAnomalousSysEvent writes an equivalent event successfully.
 *           Listening/persistence verification belongs to integration tests.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderHiSysEventReportTest, ReportAnomalousSysEvent_001, TestSize.Level1)
{
    TaskInfo info = MakeTaskInfo();
    BackgroundLoaderHiSysEvent::ReportAnomalousSysEvent(info, "4", "");

    int ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::WORK_SCHEDULER,
        "BACKGROUND_LOADER_ANOMALOUS", HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "UID", info.uid_, "PID", info.pid_, "BUNDLE_NAME", info.bundleName_, "REASON", "4",
        "EXTEND_REASON", "");
    EXPECT_EQ(ret, 0);
}
} // namespace WorkScheduler
} // namespace OHOS
