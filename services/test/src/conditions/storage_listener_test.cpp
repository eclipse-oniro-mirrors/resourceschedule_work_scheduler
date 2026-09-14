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

#include <functional>
#include <gtest/gtest.h>

#define private public
#include "conditions/storage_listener.h"
#include "work_scheduler_service.h"
#include "work_queue_manager.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"
#include "work_sched_hilog.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

class TestStorageListener : public StorageListener {
public:
    TestStorageListener() : StorageListener(nullptr) {}
    ~TestStorageListener() override {}
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

class StorageListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
    static std::shared_ptr<StorageListener> storageListener_;
    static std::shared_ptr<TestStorageListener> testListener_;
    static std::shared_ptr<StorageEventSubscriber> subscriber_;
};

std::shared_ptr<WorkQueueManager> StorageListenerTest::workQueueManager_ = nullptr;
std::shared_ptr<StorageListener> StorageListenerTest::storageListener_ = nullptr;
std::shared_ptr<TestStorageListener> StorageListenerTest::testListener_ = nullptr;
std::shared_ptr<StorageEventSubscriber> StorageListenerTest::subscriber_ = nullptr;

void StorageListenerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
    storageListener_ = std::make_shared<StorageListener>(workQueueManager_);

    testListener_ = std::make_shared<TestStorageListener>();
    EventFwk::MatchingSkills skill;
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_LOW);
    skill.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_OK);
    EventFwk::CommonEventSubscribeInfo info(skill);
    subscriber_ = std::make_shared<StorageEventSubscriber>(info, *testListener_);
}

static EventFwk::CommonEventData BuildStorageEvent(const std::string &action)
{
    EventFwk::Want want;
    want.SetAction(action);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    return data;
}

/**
 * @tc.name: OnReceiveEvent_001
 * @tc.desc: Test StorageEventSubscriber OnReceiveEvent with STORAGE_LOW action.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(StorageListenerTest, OnReceiveEvent_001, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildStorageEvent(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_LOW);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastType_, WorkCondition::Type::STORAGE);
    EXPECT_EQ(testListener_->lastVal_->intVal, WorkCondition::STORAGE_LEVEL_LOW);
}

/**
 * @tc.name: OnReceiveEvent_002
 * @tc.desc: Test StorageEventSubscriber OnReceiveEvent with STORAGE_OK action.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(StorageListenerTest, OnReceiveEvent_002, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildStorageEvent(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_OK);
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 1);
    EXPECT_EQ(testListener_->lastVal_->intVal, WorkCondition::STORAGE_LEVEL_OKAY);
}

/**
 * @tc.name: OnReceiveEvent_003
 * @tc.desc: Test StorageEventSubscriber OnReceiveEvent with invalid action.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(StorageListenerTest, OnReceiveEvent_003, TestSize.Level1)
{
    testListener_->Reset();
    auto data = BuildStorageEvent("invalid.action");
    subscriber_->OnReceiveEvent(data);
    EXPECT_EQ(testListener_->callCount_, 0);
}

/**
 * @tc.name: OnReceiveEvent_004
 * @tc.desc: Test StorageEventSubscriber OnReceiveEvent LOW then OK sequence.
 * @tc.type: FUNC
 * @tc.require: I8JBRY
 */
HWTEST_F(StorageListenerTest, OnReceiveEvent_004, TestSize.Level1)
{
    testListener_->Reset();
    subscriber_->OnReceiveEvent(BuildStorageEvent(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_LOW));
    EXPECT_EQ(testListener_->lastVal_->intVal, WorkCondition::STORAGE_LEVEL_LOW);
    subscriber_->OnReceiveEvent(BuildStorageEvent(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_OK));
    EXPECT_EQ(testListener_->callCount_, 2);
    EXPECT_EQ(testListener_->lastVal_->intVal, WorkCondition::STORAGE_LEVEL_OKAY);
}
} // namespace WorkScheduler
} // namespace OHOS
