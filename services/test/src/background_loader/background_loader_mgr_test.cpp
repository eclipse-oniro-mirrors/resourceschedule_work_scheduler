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
class BackgroundLoaderMgrTest : public testing::Test {
public:
    void SetUp() override
    {
        BackgroundLoaderMgr::GetInstance().Init(BACKGROUND_LOADER_TIMEOUT_COUNT, BACKGROUND_LOADER_TIMEOUT_US);
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (!BackgroundLoaderMgr::GetInstance().isReady_.load() &&
            std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME));
        }
    };
    void TearDown() override
    {
        FlushDbQueue();
        BackgroundLoaderMgr::GetInstance().database_.ExecuteSql("DELETE FROM BackgroundLoaderTask;");
        BackgroundLoaderMgr::GetInstance().database_.ExecuteSql("DELETE FROM BlockList;");
        {
            std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().taskLock_);
            BackgroundLoaderMgr::GetInstance().taskMap_.clear();
        }
        {
            std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().blockListLock_);
            BackgroundLoaderMgr::GetInstance().blockLists_.clear();
        }
    };
};
}
}