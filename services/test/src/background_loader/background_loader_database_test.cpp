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
#include <chrono>
#include <future>
#include <thread>
#include "background_loader/background_loader_database.h"
#include "background_loader/background_loader_mgr.h"
#include "work_sched_errors.h"
#include "res_type.h"

using namespace testing::ext;
using namespace OHOS::ResourceSchedule;
namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr int32_t TEST_SLEEP_TIME = 10;

void FlushDbQueue()
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    if (mgr.dbQueue_ == nullptr) {
        return;
    }
    std::promise<void> promise;
    auto future = promise.get_future();
    mgr.dbQueue_->submit([&promise]() { promise.set_value(); });
    future.wait();
}
}

static TaskRow MakeTaskRow(int32_t uid, const std::string& bundle, const std::string& ability,
    int32_t appIndex, int32_t taskId)
{
    TaskRow row;
    row.uid = uid;
    row.bundleName = bundle;
    row.abilityName = ability;
    row.appIndex = appIndex;
    row.taskId = taskId;
    row.status = 0;
    row.timeoutCount = 0;
    row.pid = -1;
    return row;
}

class BackgroundLoaderDatabaseTest : public testing::Test {
public:
    void SetUp() override
    {
        db_ = std::make_unique<BackgroundLoaderDatabase>();
        ASSERT_TRUE(db_->Init());
        ASSERT_TRUE(db_->ExecuteSql("DELETE FROM BackgroundLoaderTask;"));
        ASSERT_TRUE(db_->ExecuteSql("DELETE FROM BlockList;"));
    }
    void TearDown() override
    {
        db_.reset();
    }
    std::unique_ptr<BackgroundLoaderDatabase> db_;
};
} // namespace WorkScheduler
} // namespace OHOS