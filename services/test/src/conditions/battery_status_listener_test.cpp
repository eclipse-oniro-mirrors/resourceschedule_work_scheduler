/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
#include <chrono>
#include <functional>
#include <thread>
#include <gtest/gtest.h>

#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"
#include "conditions/battery_status_listener.h"
#include "work_queue_manager.h"
#include "work_scheduler_service.h"
#include "work_event_handler.h"
#include "work_policy_manager.h"
#include "work_conn_manager.h"
#include "work_sched_hilog.h"
#include "work_info.h"
#include "work_status.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class TestBatteryStatusListener : public BatteryStatusListener {
public:
    TestBatteryStatusListener() : BatteryStatusListener(nullptr) {}
    ~TestBatteryStatusListener() override {}
    void OnConditionChanged(WorkCondition::Type conditionType,
        std::shared_ptr<DetectorValue> conditionVal) override
    {
        callCount_++;
        lastType_ = conditionType;
        lastVal_ = conditionVal;
    }
    void Reset()
    {
        callCount_ = 0;
        lastType_ = WorkCondition::Type::UNKNOWN;
        lastVal_ = nullptr;
    }
    int32_t callCount_ = 0;
    WorkCondition::Type lastType_ = WorkCondition::Type::UNKNOWN;
    std::shared_ptr<DetectorValue> lastVal_ = nullptr;
};

class BatteryStatusListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<TestBatteryStatusListener> listener_;
    static std::shared_ptr<BatteryStatusEventSubscriber> subscriber_;
};

std::shared_ptr<TestBatteryStatusListener> BatteryStatusListenerTest::listener_ = nullptr;
std::shared_ptr<BatteryStatusEventSubscriber> BatteryStatusListenerTest::subscriber_ = nullptr;

void BatteryStatusListenerTest::SetUpTestCase()
{
    listener_ = std::make_shared<TestBatteryStatusListener>();
    EventFwk::MatchingSkills skill;
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW);
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY);
    EventFwk::CommonEventSubscribeInfo info(skill);
    subscriber_ = std::make_shared<BatteryStatusEventSubscriber>(info, *listener_);
}

static EventFwk::CommonEventData BuildBatteryStatusEvent(const std::string &action)
{
    EventFwk::Want want;
    want.SetAction(action);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    return data;
}

/**
 * @brief Poll until the work's condition of the given type is published or the timeout elapses.
 *
 * WorkQueueManager::OnConditionChanged() publishes via handler->PostTask(), so the update is
 * asynchronous. A bounded polling wait is used instead of a fixed sleep so a loaded runner cannot
 * cause a spurious failure. Reads are guarded by conditionMapMutex_ to avoid racing the async writer.
 *
 * @return true if the condition was observed before the timeout, false otherwise.
 */
static bool WaitForCondition(const std::shared_ptr<WorkStatus>& workStatus,
    WorkCondition::Type type, int32_t timeoutMs)
{
    const int32_t intervalMs = 10;
    int32_t waitedMs = 0;
    while (true)
    {
        {
            std::lock_guard<frrt::mutex> lock(workStatus->conditionMapMutex_);
            if (workStatus->conditionMap_.count(type) > 0) {
                return true;
            }
        }
        if (waitedMs > timeoutMs) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        waitedMs += intervalMs;
    }
}

/**
 * @tc.name: OnReceiveEvent_001
 * @tc.desc: Test BatteryStatusEventSubscriber OnReceiveEvent with BATTERY_LOW action.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryStatusListenerTest, OnReceiveEvent_001, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildBatteryStatusEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 1);
    EXPECT_EQ(listener_->lastType_, WorkCondition::Type::BATTERY_STATUS);
    EXPECT_EQ(listener_->lastVal_->intVal, WorkCondition::BATTERY_STATUS_LOW);
}

/**
 * @tc.name: OnReceiveEvent_002
 * @tc.desc: Test BatteryStatusEventSubscriber OnReceiveEvent with BATTERY_OKAY action.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryStatusListenerTest, OnReceiveEvent_002, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildBatteryStatusEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 1);
    EXPECT_EQ(listener_->lastType_, WorkCondition::Type::BATTERY_STATUS);
    EXPECT_EQ(listener_->lastVal_->intVal, WorkCondition::BATTERY_STATUS_OKAY);
}

/**
 * @tc.name: OnReceiveEvent_003
 * @tc.desc: Test BatteryStatusEventSubscriber OnReceiveEvent with invalid action.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryStatusListenerTest, OnReceiveEvent_003, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildBatteryStatusEvent("invalid.action");
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 0);
}

/**
 * @tc.name: OnReceiveEvent_004
 * @tc.desc: Test BatteryStatusEventSubscriber OnReceiveEvent LOW then OKAY sequence.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryStatusListenerTest, OnReceiveEvent_004, TestSize.Level1)
{
    listener_->Reset();
    subscriber_->OnReceiveEvent(BuildBatteryStatusEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW));
    EXPECT_EQ(listener_->lastVal_->intVal, WorkCondition::BATTERY_STATUS_LOW);
    subscriber_->OnReceiveEvent(BuildBatteryStatusEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY));
    EXPECT_EQ(listener_->callCount_, 2);
    EXPECT_EQ(listener_->lastVal_->intVal, WorkCondition::BATTERY_STATUS_OKAY);
}

/**
 * @tc.name: Start_001
 * @tc.desc: Test BatteryStatusListener Start subscribes common event.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryStatusListenerTest, Start_001, TestSize.Level1)
{
    auto listener = std::make_shared<TestBatteryStatusListener>();
    bool ret = listener->Start();
    EXPECT_TRUE(ret);
    listener->Stop();
}

/**
 * @tc.name: Stop_001
 * @tc.desc: Test BatteryStatusListener Stop when subscriber is null.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryStatusListenerTest, Stop_001, TestSize.Level1)
{
    auto listener = std::make_shared<TestBatteryStatusListener>();
    bool ret = listener->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: Stop_002
 * @tc.desc: Test BatteryStatusListener Stop when subscriber is not null.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryStatusListenerTest, Stop_002, TestSize.Level1)
{
    auto listener = std::make_shared<TestBatteryStatusListener>();
    listener->Start();
    bool ret = listener->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_001
 * @tc.desc: Test BatteryStatusListener OnConditionChanged with null workQueueManager.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryStatusListenerTest, OnConditionChanged_001, TestSize.Level1)
{
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (service->eventRunner_ == nullptr) {
        service->eventRunner_ = AppExecFwk::EventRunner::Create("BatteryStatusListenerTest",
            AppExecFwk::ThreadMode::FFRT);
    }
    if (service->handler_ == nullptr) {
        service->handler_ = std::make_shared<WorkEventHandler>(service->eventRunner_, service);
    }
    if (service->workQueueManager_ == nullptr) {
        service->workQueueManager_ = std::make_shared<WorkQueueManager>(service);
    }
    if (service->workPolicyManager_ == nullptr) {
        service->workPolicyManager_ = std::make_shared<WorkPolicyManager>(service);
    }
    if (service->workPolicyManager_->workConnManager_ == nullptr) {
        service->workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
    }
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(9911);
    workInfo->SetElement("com.test.battery", "BatteryAbility");
    workInfo->RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 10011);
    service->workQueueManager_->AddWork(workStatus);

    auto listener = std::make_shared<TestBatteryStatusListener>();
    auto val = std::make_shared<DetectorValue>(WorkCondition::BATTERY_STATUS_LOW, 0, 0, std::string());
    listener->BatteryStatusListener::OnConditionChanged(WorkCondition::Type::BATTERY_STATUS, val);
    {
        std::lock_gurad<frrt::mutex> lock(workStatus->conditionMapMutex_);
        EXPECT_EQ(workStatus->conditionMap_.count(WorkCondition::Type::BATTERY_STATUS), 0);

    }

    BatteryStatusListener realListener(service->workQueueManager_);
    realListener.OnConditionChanged(WorkCondition::Type::BATTERY_STATUS, val)
    EXPECT_TRUE(WaitForCondition(workStatus, WorkCondition::Type::BATTERY_STATUS, 2000));
    {
        std::lock_gurad<frrt::mutex> lock(workStatus->conditionMapMutex_);
        EXPECT_EQ(workStatus->conditionMap_.count(WorkCondition::Type::BATTERY_STATUS), 1);
    }
    service->workQueueManager_->RemoveWork(workStatus);
}

/**
 * @tc.name: OnConditionChanged_002
 * @tc.desc: Test BatteryStatusListener OnConditionChanged with valid workQueueManager.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryStatusListenerTest, OnConditionChanged_002, TestSize.Level1)
{
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (service->eventRunner_ == nullptr) {
        service->eventRunner_ = AppExecFwk::EventRunner::Create("BatteryStatusListenerTest",
            AppExecFwk::ThreadMode::FFRT);
    }
    if (service->handler_ == nullptr) {
        service->handler_ = std::make_shared<WorkEventHandler>(service->eventRunner_, service);
    }
    if (service->workQueueManager_ == nullptr) {
        service->workQueueManager_ = std::make_shared<WorkQueueManager>(service);
    }
    if (service->workPolicyManager_ == nullptr) {
        service->workPolicyManager_ = std::make_shared<WorkPolicyManager>(service);
    }
    if (service->workPolicyManager_->workConnManager_ == nullptr) {
        service->workPolicyManager_->workConnManager_ = std::make_shared<WorkConnManager>();
    }
    auto workInfo = std::make_shared<WorkInfo>();
    workInfo->SetWorkId(9910);
    workInfo->SetElement("com.test.battery", "BatteryAbility");
    workInfo->RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 10010);
    service->workQueueManager_->AddWork(workStatus);

    BatteryStatusListener listener(service->workQueueManager_);
    auto val = std::make_shared<DetectorValue>(WorkCondition::BATTERY_STATUS_LOW, 0, 0, std::string());
    listener.OnConditionChanged(WorkCondition::Type::BATTERY_STATUS, val);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(workStatus->conditionMap_.count(WorkCondition::Type::BATTERY_STATUS), 1);
    service->workQueueManager_->RemoveWork(workStatus);
}
} // namespace WorkScheduler
} // namespace OHOS
#endif // POWERMGR_BATTERY_MANAGER_ENABLE
