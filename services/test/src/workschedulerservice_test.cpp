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

#include "work_scheduler_service.h"
#include "work_status.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerServiceTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};
/**
 * @tc.name: onStart_001
 * @tc.desc: Test BackgroundTaskMgrService service ready.
 * @tc.type: FUNC
 * @tc.require: SR000GGTET SR000GMUG8 AR000GH86O AR000GH86Q AR000GMUIA AR000GMUHN
 */
HWTEST_F(WorkSchedulerServiceTest, onStart_001, TestSize.Level1)
{
    
}
}  // namespace BackgroundTaskMgr
}  // namespace OHOS
