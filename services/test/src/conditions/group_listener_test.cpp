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

#include "conditions/group_listener.h"
#include "work_scheduler_service.h"
#include "work_queue_manager.h"

using namespace OHOS::AppExecFwk;
using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {

const std::string WORKSCHEDULER_SERVICE_NAME = "WorkSchedulerService";

class GroupListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
    static std::shared_ptr<GroupListener> groupListener_;
};

std::shared_ptr<WorkQueueManager> GroupListenerTest::workQueueManager_ = nullptr;
std::shared_ptr<GroupListener> GroupListenerTest::groupListener_ = nullptr;

void GroupListenerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_ = AppExecFwk::EventRunner::Create(WORKSCHEDULER_SERVICE_NAME,
        AppExecFwk::ThreadMode::FFRT);
    groupListener_ = std::make_shared<GroupListener>(workQueueManager_, eventRunner_);
}

/**
 * @tc.name: OnConditionChanged_001
 * @tc.desc: Test GroupListener OnConditionChanged.
 * @tc.type: FUNC
 * @tc.require: IB7RQR
 */
HWTEST_F(GroupListenerTest, OnConditionChanged_001, TestSize.Level1)
{
    groupListener_->Start();
    int32_t newGroup = 10;
    int32_t userId = 100;
    std::string bundleName = "com.ohos.sceneboard";
    workQueueManager_->OnConditionChanged(WorkCondition::Type::GROUP,
        std::make_shared<DetectorValue>(newGroup, userId, true, bundleName));
    bool ret = groupListener_->Stop();
    EXPECT_TRUE(ret);
}
}
}