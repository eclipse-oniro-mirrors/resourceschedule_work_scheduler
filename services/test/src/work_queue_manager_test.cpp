/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "work_queue_manager.h"
#include "work_policy_manager.h"
#include "work_scheduler_service.h"
#include "work_condition.h"
#include "work_sched_hilog.h"
#include "work_info.h"
#include "work_sched_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class MockWorkPolicyManager : public WorkPolicyManager {
public:
    using WorkPolicyManager::WorkPolicyManager;
    ~MockWorkPolicyManager() = default;

    void CheckWorkToRun(){};
};
class WorkQueueManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
};

std::shared_ptr<WorkQueueManager> WorkQueueManagerTest::workQueueManager_ = nullptr;

void WorkQueueManagerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = DelayedSingleton<WorkSchedulerService>::GetInstance();
    std::shared_ptr<WorkPolicyManager> workPolicyManager_ =
        std::make_shared<MockWorkPolicyManager>(workSchedulerService_);
    workSchedulerService_->workPolicyManager_ = workPolicyManager_;
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
}
}
}