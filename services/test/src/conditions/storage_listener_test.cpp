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

#include "conditions/storage_listener.h"
#include "work_scheduler_service.h"
#include "work_queue_manager.h"
#include "common_event_manager.h"
#include "common_event_support.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class StorageListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
    static std::shared_ptr<StorageListener> storageListener_;
};

std::shared_ptr<WorkQueueManager> StorageListenerTest::workQueueManager_ = nullptr;
std::shared_ptr<StorageListener> StorageListenerTest::storageListener_ = nullptr;

void StorageListenerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
    storageListener_ = std::make_shared<StorageListener>(workQueueManager_);
}

/**
 * @tc.name: OnConditionChanged_001
 * @tc.desc: Test StorageListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(StorageListenerTest, OnConditionChanged_001, TestSize.Level1)
{
    storageListener_->Start();
    EventFwk::CommonEventData data;
    storageListener_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = storageListener_->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_002
 * @tc.desc: Test StorageListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(StorageListenerTest, OnConditionChanged_002, TestSize.Level1)
{
    storageListener_->Start();
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_LOW);
    storageListener_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = storageListener_->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_003
 * @tc.desc: Test StorageListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(StorageListenerTest, OnConditionChanged_003, TestSize.Level1)
{
    storageListener_->Start();
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_OK);
    storageListener_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = storageListener_->Stop();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: OnConditionChanged_004
 * @tc.desc: Test StorageListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: IAJSVG
 */
HWTEST_F(StorageListenerTest, OnConditionChanged_004, TestSize.Level1)
{
    storageListener_->Start();
    EventFwk::CommonEventData data;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_FULL);
    storageListener_->commonEventSubscriber->OnReceiveEvent(data);
    bool ret = storageListener_->Stop();
    EXPECT_TRUE(ret);
}
}
}