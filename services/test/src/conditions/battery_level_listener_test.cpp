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
#include <gtest/gtest.h>
#include <thread>

#include "battery_info.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"
#include "conditions/battery_level_listener.h"
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

class TestBatteryLevelListener : public BatteryLevelListener {
public:
    TestBatteryLevelListener() : BatteryLevelListener(nullptr, nullptr) {}
    ~TestBatteryLevelListener() override {}
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

class BatteryLevelListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<TestBatteryLevelListener> listener_;
    static std::shared_ptr<BatteryLevelEventSubscriber> subscriber_;
};

std::shared_ptr<TestBatteryLevelListener> BatteryLevelListenerTest::listener_ = nullptr;
std::shared_ptr<BatteryLevelEventSubscriber> BatteryLevelListenerTest::subscriber_ = nullptr;

void BatteryLevelListenerTest::SetUpTestCase()
{
    listener_ = std::make_shared<TestBatteryLevelListener>();
    EventFwk::MatchingSkills skill;
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    EventFwk::CommonEventSubscribeInfo info(skill);
    subscriber_ = std::make_shared<BatteryLevelEventSubscriber>(info, *listener_);
}

static EventFwk::CommonEventData BuildBatteryChangedEvent(int32_t capacity)
{
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    want.SetParam(PowerMgr::BatteryInfo::COMMON_EVENT_KEY_CAPACITY, capacity);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    return data;
}

/**
 * @brief Poll until the work's condition of the given type is published or the timeout elapses.
 *
 * WorkQueueManager::OnConditionChanged() publishes via handler->PostTask(), so the update is
 * asynchronous. A bounded polling wait is used instead of a fixed sleep. Reads are guarded by
 * conditionMapMutex_ to avoid racing the async writer.
 */
static bool WaitForCondition(const std::shared_ptr<WorkStatus>& workStatus,
    WorkCondition::Type type, int32_t timeoutMs)
{
    const int32_t intervalMs = 10;
    int32_t waitedMs = 0;
    while (true) {
        {
            std::lock_guard<ffrt::mutex> lock(workStatus->conditionMapMutex_);
            if (workStatus->conditionMap_.count(type) > 0) {
                return true;
            }
        }
        if (waitedMs >= timeoutMs) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        waitedMs += intervalMs;
    }
}

/**
 * @tc.name: OnReceiveEvent_001
 * @tc.desc: Test BatteryLevelEventSubscriber OnReceiveEvent with valid capacity.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnReceiveEvent_001, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildBatteryChangedEvent(50);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 1);
    EXPECT_EQ(listener_->lastType_, WorkCondition::Type::BATTERY_LEVEL);
    EXPECT_EQ(listener_->lastVal_->intVal, 50);
}

/**
 * @tc.name: OnReceiveEvent_002
 * @tc.desc: Test BatteryLevelEventSubscriber OnReceiveEvent with invalid capacity (-1).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnReceiveEvent_002, TestSize.Level1)
{
    listener_->Reset();
    auto data = BuildBatteryChangedEvent(-1);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 0);
}

/**
 * @tc.name: OnReceiveEvent_003
 * @tc.desc: Test BatteryLevelEventSubscriber OnReceiveEvent with same capacity (dedup).
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnReceiveEvent_003, TestSize.Level1)
{
    listener_->Reset();
    subscriber_->OnReceiveEvent(BuildBatteryChangedEvent(80));
    EXPECT_EQ(listener_->callCount_, 1);
    subscriber_->OnReceiveEvent(BuildBatteryChangedEvent(80));
    EXPECT_EQ(listener_->callCount_, 1);
}

/**
 * @tc.name: OnReceiveEvent_004
 * @tc.desc: Test BatteryLevelEventSubscriber OnReceiveEvent with different capacity.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnReceiveEvent_004, TestSize.Level1)
{
    listener_->Reset();
    subscriber_->OnReceiveEvent(BuildBatteryChangedEvent(30));
    EXPECT_EQ(listener_->lastVal_->intVal, 30);
    subscriber_->OnReceiveEvent(BuildBatteryChangedEvent(90));
    EXPECT_EQ(listener_->callCount_, 2);
    EXPECT_EQ(listener_->lastVal_->intVal, 90);
}

/**
 * @tc.name: OnReceiveEvent_005
 * @tc.desc: Test BatteryLevelEventSubscriber OnReceiveEvent with invalid action.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnReceiveEvent_005, TestSize.Level1)
{
    listener_->Reset();
    EventFwk::Want want;
    want.SetAction("invalid.action");
    EventFwk::CommonEventData data;
    data.SetWant(want);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(listener_->callCount_, 0);
}

/**
 * @tc.name: OnReceiveEvent_006
 * @tc.desc: Test BatteryLevelEventSubscriber OnReceiveEvent capacity transition -1 to valid.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnReceiveEvent_006, TestSize.Level1)
{
    listener_->Reset();
    subscriber_->OnReceiveEvent(BuildBatteryChangedEvent(-1));
    EXPECT_EQ(listener_->callCount_, 0);
    subscriber_->OnReceiveEvent(BuildBatteryChangedEvent(60));
    EXPECT_EQ(listener_->callCount_, 1);
    EXPECT_EQ(listener_->lastVal_->intVal, 60);
}

/**
 * @tc.name: Start_001
 * @tc.desc: Test BatteryLevelListener Start creates subscriber and subscribes.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, Start_001, TestSize.Level1)
{
    auto listener = std::make_shared<TestBatteryLevelListener>();
    bool ret = listener->Start();
    EXPECT_TRUE(ret);
    listener->Stop();
}

/**
 * @tc.name: Stop_001
 * @tc.desc: Test BatteryLevelListener Stop when subscriber is null.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, Stop_001, TestSize.Level1)
{
    auto listener = std::make_shared<TestBatteryLevelListener>();
    bool ret = listener->Stop();
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: Stop_002
 * @tc.desc: Test BatteryLevelListener Stop when subscriber is not null.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, Stop_002, TestSize.Level1)
{
    auto listener = std::make_shared<TestBatteryLevelListener>();
    listener->Start();
    bool ret = listener->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_001
 * @tc.desc: Test BatteryLevelListener OnConditionChanged with null workQueueManager does not
 *           propagate the condition, verified against a positive control using a real manager.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnConditionChanged_001, TestSize.Level1)
{
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (service->eventRunner_ == nullptr) {
        service->eventRunner_ = AppExecFwk::EventRunner::Create("BatteryLevelListenerTest",
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
    workInfo->SetWorkId(9921);
    workInfo->SetElement("com.test.batlevel", "BatLevelAbility");
    workInfo->RequestBatteryLevel(40);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 10021);
    service->workQueueManager_->AddWork(workStatus);

    // Negative: the test listener holds a null workQueueManager_, so the guard must not publish.
    auto listener = std::make_shared<TestBatteryLevelListener>();
    auto val = std::make_shared<DetectorValue>(50, 0, 0, std::string());
    listener->BatteryLevelListener::OnConditionChanged(WorkCondition::Type::BATTERY_LEVEL, val);
    {
        std::lock_guard<ffrt::mutex> lock(workStatus->conditionMapMutex_);
        EXPECT_EQ(workStatus->conditionMap_.count(WorkCondition::Type::BATTERY_LEVEL), 0);
    }

    BatteryLevelListener realListener(service->workQueueManager_, service);
    realListener.OnConditionChanged(WorkCondition::Type::BATTERY_LEVEL, val);
    EXPECT_TRUE(WaitForCondition(workStatus, WorkCondition::Type::BATTERY_LEVEL, 2000));
    {
        std::lock_guard<ffrt::mutex> lock(workStatus->conditionMapMutex_);
        EXPECT_EQ(workStatus->conditionMap_.count(WorkCondition::Type::BATTERY_LEVEL), 1);
    }
    service->workQueueManager_->RemoveWork(workStatus);
}

/**
 * @tc.name: OnConditionChanged_002
 * @tc.desc: Test BatteryLevelListener OnConditionChanged with valid workQueueManager.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(BatteryLevelListenerTest, OnConditionChanged_002, TestSize.Level1)
{
    auto service = DelayedSingleton<WorkSchedulerService>::GetInstance();
    if (service->eventRunner_ == nullptr) {
        service->eventRunner_ = AppExecFwk::EventRunner::Create("BatteryLevelListenerTest",
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
    workInfo->SetWorkId(9920);
    workInfo->SetElement("com.test.batlevel", "BatLevelAbility");
    workInfo->RequestBatteryLevel(40);
    auto workStatus = std::make_shared<WorkStatus>(*workInfo, 10020);
    service->workQueueManager_->AddWork(workStatus);

    BatteryLevelListener listener(service->workQueueManager_, service);
    auto val = std::make_shared<DetectorValue>(50, 0, 0, std::string());
    listener.OnConditionChanged(WorkCondition::Type::BATTERY_LEVEL, val);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(workStatus->conditionMap_.count(WorkCondition::Type::BATTERY_LEVEL), 1);
    service->workQueueManager_->RemoveWork(workStatus);
}
} // namespace WorkScheduler
} // namespace OHOS
#endif // POWERMGR_BATTERY_MANAGER_ENABLE
