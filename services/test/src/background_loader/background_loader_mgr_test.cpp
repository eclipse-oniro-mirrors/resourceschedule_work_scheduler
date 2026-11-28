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
#include "background_loader/ability_connect_callback.h"
#include "background_loader_task_info.h"
#include "background_loader/background_loader_errors.h"
#include "res_type.h"
#include "ipc_skeleton.h"
#include "ability_state.h"
#include "nlohmann/json.hpp"

using namespace testing::ext;
using namespace OHOS::ResourceSchedule;
namespace OHOS {
namespace WorkScheduler {
using namespace BackgroundLoader;
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

TaskInfo MakeTask(int32_t uid, const std::string& bundle, const std::string& ability,
    int32_t appIndex, int32_t taskId)
{
    TaskInfo info;
    info.bundleName_ = bundle;
    info.abilityName_ = ability;
    info.appIndex_ = appIndex;
    info.taskId_ = taskId;
    info.status_ = TaskStatus::NOT_STARTED;
    info.uid_ = uid;
    info.timeoutCount_ = 0;
    info.pid_ = -1;
    return info;
}

TaskRow MakeTaskRow(int32_t uid, const std::string& bundle, const std::string& ability,
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
        auto& mgr = BackgroundLoaderMgr::GetInstance();
        FlushDbQueue();
        mgr.database_.ExecuteSql("DELETE FROM BackgroundLoaderTask;");
        mgr.database_.ExecuteSql("DELETE FROM BlockList;");
        {
            std::lock_guard<ffrt::mutex> lock(mgr.taskLock_);
            mgr.taskMap_.clear();
        }
        {
            std::lock_guard<ffrt::mutex> lock(mgr.blockListLock_);
            mgr.blockLists_.clear();
        }
        {
            std::lock_guard<ffrt::mutex> lock(mgr.abilityMapLock_);
            mgr.abilityMap_.clear();
        }
        {
            std::lock_guard<ffrt::mutex> lock(mgr.timeoutTaskLock_);
            mgr.timeoutTaskMap_.clear();
        }
        {
            std::lock_guard<ffrt::mutex> lock(mgr.delayFinishLock_);
            mgr.delayFinishTaskMap_.clear();
        }
        mgr.isReady_.store(false);
        mgr.retryCount_.store(0);
    };

    void InsertTask(int32_t uid, const TaskInfo& info)
    {
        auto& mgr = BackgroundLoaderMgr::GetInstance();
        std::lock_guard<ffrt::mutex> lock(mgr.taskLock_);
        mgr.taskMap_[uid] = info;
    }

    int32_t GetCallingUid()
    {
        return IPCSkeleton::GetCallingAppUid();
    }
};

/**
 * @tc.name: Init_001
 * @tc.desc: Test Init creates the ffrt and db queues and marks the manager ready.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, Init_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    EXPECT_TRUE(mgr.isReady_.load());
    EXPECT_NE(mgr.ffrtQueue_, nullptr);
    EXPECT_NE(mgr.dbQueue_, nullptr);
    EXPECT_EQ(mgr.maxTimeoutCount_, BACKGROUND_LOADER_TIMEOUT_COUNT);
    EXPECT_EQ(mgr.backgroundLoaderTimeoutUs_, BACKGROUND_LOADER_TIMEOUT_US);
}

/**
 * @tc.name: GenerateTaskKey_001
 * @tc.desc: Test GenerateTaskKey concatenates bundleName and appIndex.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, GenerateTaskKey_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    EXPECT_EQ(mgr.GenerateTaskKey("com.test.bundle", 3), "com.test.bundle_3");
}

/**
 * @tc.name: ToTaskRow_001
 * @tc.desc: Test ToTaskRow maps every TaskInfo field into a TaskRow.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, ToTaskRow_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 7);
    info.status_ = TaskStatus::RUNNING;
    info.timeoutCount_ = 2;
    info.pid_ = 999;

    TaskRow row = mgr.ToTaskRow(info);
    EXPECT_EQ(row.uid, 1001);
    EXPECT_EQ(row.bundleName, "com.test.bundle");
    EXPECT_EQ(row.abilityName, "TestAbility");
    EXPECT_EQ(row.appIndex, 0);
    EXPECT_EQ(row.taskId, 7);
    EXPECT_EQ(row.status, static_cast<int32_t>(TaskStatus::RUNNING));
    EXPECT_EQ(row.timeoutCount, 2);
    EXPECT_EQ(row.pid, 999);
}

/**
 * @tc.name: BlockList_001
 * @tc.desc: Test AddToBlockListLocked, IsInBlockListLocked and RemoveFromBlockListLocked.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, BlockList_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.AddToBlockListLocked("com.test.block");
    EXPECT_TRUE(mgr.IsInBlockListLocked("com.test.block"));
    EXPECT_FALSE(mgr.IsInBlockListLocked("com.test.other"));
    EXPECT_TRUE(mgr.RemoveFromBlockListLocked("com.test.block"));
    EXPECT_FALSE(mgr.IsInBlockListLocked("com.test.block"));
    EXPECT_FALSE(mgr.RemoveFromBlockListLocked("com.test.block"));
}

/**
 * @tc.name: AddNewTaskLocked_001
 * @tc.desc: Test AddNewTaskLocked inserts a task and IsTaskExistLocked reports it.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, AddNewTaskLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    EXPECT_TRUE(mgr.AddNewTaskLocked(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1)));
    EXPECT_TRUE(mgr.IsTaskExistLocked(1001));
    EXPECT_FALSE(mgr.IsTaskExistLocked(1002));
}

/**
 * @tc.name: AddNewTaskLocked_002
 * @tc.desc: Test AddNewTaskLocked rejects a task when the map reaches the upper limit.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, AddNewTaskLocked_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    for (int32_t uid = 1; uid <= 1000; ++uid) {
        mgr.AddNewTaskLocked(uid, MakeTask(uid, "com.test.bundle", "TestAbility", 0, uid));
    }
    EXPECT_FALSE(mgr.AddNewTaskLocked(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1001)));
}

/**
 * @tc.name: EraseTaskByKeyLocked_001
 * @tc.desc: Test EraseTaskByKeyLocked removes a task and returns false when absent.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, EraseTaskByKeyLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.AddNewTaskLocked(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    EXPECT_TRUE(mgr.EraseTaskByKeyLocked(1001));
    EXPECT_FALSE(mgr.IsTaskExistLocked(1001));
    EXPECT_FALSE(mgr.EraseTaskByKeyLocked(1001));
}

/**
 * @tc.name: UpdateIsBgLocked_001
 * @tc.desc: Test UpdateIsBgLocked flips the isBg_ flag of an existing task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, UpdateIsBgLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    mgr.UpdateIsBgLocked(1001, true);
    EXPECT_TRUE(mgr.taskMap_.at(1001).isBg_);
    mgr.UpdateIsBgLocked(1001, false);
    EXPECT_FALSE(mgr.taskMap_.at(1001).isBg_);
    mgr.UpdateIsBgLocked(9999, true);
    EXPECT_TRUE(mgr.taskMap_.find(9999) == mgr.taskMap_.end());
}

/**
 * @tc.name: SetAudioStreamLocked_001
 * @tc.desc: Test SetAudioStreamLocked updates the hasAudioStream_ flag.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, SetAudioStreamLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    EXPECT_FALSE(mgr.SetAudioStreamLocked(9999, true));
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    EXPECT_TRUE(mgr.SetAudioStreamLocked(1001, true));
    EXPECT_TRUE(mgr.taskMap_.at(1001).hasAudioStream_);
    EXPECT_TRUE(mgr.SetAudioStreamLocked(1001, false));
    EXPECT_FALSE(mgr.taskMap_.at(1001).hasAudioStream_);
}

/**
 * @tc.name: SetShortTaskMarkLocked_001
 * @tc.desc: Test SetShortTaskMarkLocked updates the hasShortTask_ flag.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, SetShortTaskMarkLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    EXPECT_FALSE(mgr.SetShortTaskMarkLocked(9999, true));
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    EXPECT_TRUE(mgr.SetShortTaskMarkLocked(1001, true));
    EXPECT_TRUE(mgr.taskMap_.at(1001).hasShortTask_);
}

/**
 * @tc.name: SetLongTaskMarkLocked_001
 * @tc.desc: Test SetLongTaskMarkLocked updates the hasLongTask_ flag.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, SetLongTaskMarkLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    EXPECT_FALSE(mgr.SetLongTaskMarkLocked(9999, true));
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    EXPECT_TRUE(mgr.SetLongTaskMarkLocked(1001, true));
    EXPECT_TRUE(mgr.taskMap_.at(1001).hasLongTask_);
}

/**
 * @tc.name: MarkTaskRunningLocked_001
 * @tc.desc: Test MarkTaskRunningLocked transitions a task to RUNNING.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, MarkTaskRunningLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo copy;
    EXPECT_FALSE(mgr.MarkTaskRunningLocked(9999, copy));
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    EXPECT_TRUE(mgr.MarkTaskRunningLocked(1001, copy));
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::RUNNING);
    EXPECT_EQ(copy.status_, TaskStatus::RUNNING);
}

/**
 * @tc.name: MarkTaskFinishedLocked_001
 * @tc.desc: Test MarkTaskFinishedLocked transitions a RUNNING task to FINISHED.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, MarkTaskFinishedLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo copy;
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    EXPECT_FALSE(mgr.MarkTaskFinishedLocked(1001, copy));
    mgr.taskMap_.at(1001).status_ = TaskStatus::RUNNING;
    EXPECT_TRUE(mgr.MarkTaskFinishedLocked(1001, copy));
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::FINISHED);
}

/**
 * @tc.name: MarkTaskBlockListLocked_001
 * @tc.desc: Test MarkTaskBlockListLocked transitions a task to IN_BLOCK_LIST.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, MarkTaskBlockListLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo copy;
    EXPECT_FALSE(mgr.MarkTaskBlockListLocked(9999, copy));
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    EXPECT_TRUE(mgr.MarkTaskBlockListLocked(1001, copy));
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::IN_BLOCK_LIST);
}

/**
 * @tc.name: FindUidLocked_001
 * @tc.desc: Test FindUidLocked finds a task by bundleName and appIndex.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, FindUidLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    int32_t uid = -1;
    EXPECT_FALSE(mgr.FindUidLocked("com.test.bundle", 0, uid));
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    EXPECT_TRUE(mgr.FindUidLocked("com.test.bundle", 0, uid));
    EXPECT_EQ(uid, 1001);
    EXPECT_FALSE(mgr.FindUidLocked("com.test.bundle", 1, uid));
}

/**
 * @tc.name: CanBackGroundLoadLocked_001
 * @tc.desc: Test CanBackGroundLoadLocked returns false when the task is not found.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CanBackGroundLoadLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    int32_t uid = -1;
    EXPECT_FALSE(mgr.CanBackGroundLoadLocked("com.test.bundle", 0, uid));
}

/**
 * @tc.name: CanBackGroundLoadLocked_002
 * @tc.desc: Test CanBackGroundLoadLocked returns false when the task is RUNNING.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CanBackGroundLoadLocked_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.isBg_ = true;
    info.hasShortTask_ = true;
    info.hasLongTask_ = true;
    info.hasAudioStream_ = true;
    InsertTask(1001, info);
    int32_t uid = -1;
    EXPECT_FALSE(mgr.CanBackGroundLoadLocked("com.test.bundle", 0, uid));
}

/**
 * @tc.name: CanBackGroundLoadLocked_003
 * @tc.desc: Test CanBackGroundLoadLocked returns false when the task is in foreground.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CanBackGroundLoadLocked_003, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.isBg_ = false;
    InsertTask(1001, info);
    int32_t uid = -1;
    EXPECT_FALSE(mgr.CanBackGroundLoadLocked("com.test.bundle", 0, uid));
}

/**
 * @tc.name: CanBackGroundLoadLocked_004
 * @tc.desc: Test CanBackGroundLoadLocked returns false when the task has a short task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CanBackGroundLoadLocked_004, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.isBg_ = true;
    info.hasShortTask_ = true;
    InsertTask(1001, info);
    int32_t uid = -1;
    EXPECT_FALSE(mgr.CanBackGroundLoadLocked("com.test.bundle", 0, uid));
}

/**
 * @tc.name: CanBackGroundLoadLocked_005
 * @tc.desc: Test CanBackGroundLoadLocked returns false when the task has a long task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CanBackGroundLoadLocked_005, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.isBg_ = true;
    info.hasLongTask_ = true;
    InsertTask(1001, info);
    int32_t uid = -1;
    EXPECT_FALSE(mgr.CanBackGroundLoadLocked("com.test.bundle", 0, uid));
}

/**
 * @tc.name: CanBackGroundLoadLocked_006
 * @tc.desc: Test CanBackGroundLoadLocked returns false when the task has an audio stream.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CanBackGroundLoadLocked_006, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.isBg_ = true;
    info.hasAudioStream_ = true;
    InsertTask(1001, info);
    int32_t uid = -1;
    EXPECT_FALSE(mgr.CanBackGroundLoadLocked("com.test.bundle", 0, uid));
}

/**
 * @tc.name: CanBackGroundLoadLocked_007
 * @tc.desc: Test CanBackGroundLoadLocked returns true for a ready NOT_STARTED background task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CanBackGroundLoadLocked_007, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.isBg_ = true;
    InsertTask(1001, info);
    int32_t uid = -1;
    EXPECT_TRUE(mgr.CanBackGroundLoadLocked("com.test.bundle", 0, uid));
    EXPECT_EQ(uid, 1001);
}

/**
 * @tc.name: CanBackGroundLoadLocked_008
 * @tc.desc: Test CanBackGroundLoadLocked returns true for a ready FINISHED background task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CanBackGroundLoadLocked_008, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::FINISHED;
    info.isBg_ = true;
    InsertTask(1001, info);
    int32_t uid = -1;
    EXPECT_TRUE(mgr.CanBackGroundLoadLocked("com.test.bundle", 0, uid));
}

/**
 * @tc.name: RemoveTaskLocked_001
 * @tc.desc: Test RemoveTaskLocked returns E_TASK_ID_NOT_EXIST when the task is absent.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, RemoveTaskLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.RemoveTaskLocked(taskInfo, 1001, copy), E_TASK_ID_NOT_EXIST);
}

/**
 * @tc.name: RemoveTaskLocked_002
 * @tc.desc: Test RemoveTaskLocked returns E_TASK_ID_NOT_EXIST on taskId mismatch.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, RemoveTaskLocked_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    BackgroundLoaderTaskInfo taskInfo(2, "TestAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.RemoveTaskLocked(taskInfo, 1001, copy), E_TASK_ID_NOT_EXIST);
}

/**
 * @tc.name: RemoveTaskLocked_003
 * @tc.desc: Test RemoveTaskLocked returns E_TASK_IS_RUNNING for a RUNNING task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, RemoveTaskLocked_003, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    InsertTask(1001, info);
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.RemoveTaskLocked(taskInfo, 1001, copy), E_TASK_IS_RUNNING);
}

/**
 * @tc.name: RemoveTaskLocked_004
 * @tc.desc: Test RemoveTaskLocked returns E_BACKGROUND_LOADER_PROHIBITED for an IN_BLOCK_LIST task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, RemoveTaskLocked_004, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::IN_BLOCK_LIST;
    InsertTask(1001, info);
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.RemoveTaskLocked(taskInfo, 1001, copy), E_BACKGROUND_LOADER_PROHIBITED);
}

/**
 * @tc.name: RemoveTaskLocked_005
 * @tc.desc: Test RemoveTaskLocked removes a FINISHED task and returns ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, RemoveTaskLocked_005, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::FINISHED;
    InsertTask(1001, info);
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.RemoveTaskLocked(taskInfo, 1001, copy), ERR_OK);
    EXPECT_FALSE(mgr.IsTaskExistLocked(1001));
}

/**
 * @tc.name: FinishTaskLocked_001
 * @tc.desc: Test FinishTaskLocked returns E_TASK_NOT_EXIST when absent.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTaskLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.FinishTaskLocked(taskInfo, 1001, copy), E_TASK_NOT_EXIST);
}

/**
 * @tc.name: FinishTaskLocked_002
 * @tc.desc: Test FinishTaskLocked returns E_TASK_ID_NOT_EXIST on taskId mismatch.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTaskLocked_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    BackgroundLoaderTaskInfo taskInfo(2, "TestAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.FinishTaskLocked(taskInfo, 1001, copy), E_TASK_ID_NOT_EXIST);
}

/**
 * @tc.name: FinishTaskLocked_003
 * @tc.desc: Test FinishTaskLocked returns E_BACKGROUND_LOADER_FAILED when not RUNNING.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTaskLocked_003, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.FinishTaskLocked(taskInfo, 1001, copy), E_BACKGROUND_LOADER_FAILED);
}

/**
 * @tc.name: FinishTaskLocked_004
 * @tc.desc: Test FinishTaskLocked transitions a RUNNING task to FINISHED.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTaskLocked_004, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    InsertTask(1001, info);
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.FinishTaskLocked(taskInfo, 1001, copy), ERR_OK);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::FINISHED);
    EXPECT_EQ(copy.status_, TaskStatus::FINISHED);
}

/**
 * @tc.name: GetTaskInfoLocked_001
 * @tc.desc: Test GetTaskInfoLocked returns E_TASK_ID_NOT_EXIST when absent.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfoLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    BackgroundLoaderTaskInfo taskInfo;
    std::string bundleName;
    EXPECT_EQ(mgr.GetTaskInfoLocked(1001, 1, taskInfo, bundleName), E_TASK_ID_NOT_EXIST);
}

/**
 * @tc.name: GetTaskInfoLocked_002
 * @tc.desc: Test GetTaskInfoLocked returns E_BACKGROUND_LOADER_PROHIBITED for an IN_BLOCK_LIST task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfoLocked_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 5);
    info.status_ = TaskStatus::IN_BLOCK_LIST;
    InsertTask(1001, info);
    BackgroundLoaderTaskInfo taskInfo;
    std::string bundleName;
    EXPECT_EQ(mgr.GetTaskInfoLocked(1001, 5, taskInfo, bundleName), E_BACKGROUND_LOADER_PROHIBITED);
}

/**
 * @tc.name: GetTaskInfoLocked_003
 * @tc.desc: Test GetTaskInfoLocked returns the task info for a valid task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfoLocked_003, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 5));
    BackgroundLoaderTaskInfo taskInfo;
    std::string bundleName;
    EXPECT_EQ(mgr.GetTaskInfoLocked(1001, 5, taskInfo, bundleName), ERR_OK);
    EXPECT_EQ(taskInfo.GetTaskId(), 5);
    EXPECT_EQ(taskInfo.GetAbilityName(), "TestAbility");
    EXPECT_EQ(bundleName, "com.test.bundle");
}

/**
 * @tc.name: GetTaskInfoLocked_004
 * @tc.desc: Test GetTaskInfoLocked returns E_TASK_ID_NOT_EXIST on taskId mismatch.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfoLocked_004, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 5));
    BackgroundLoaderTaskInfo taskInfo;
    std::string bundleName;
    EXPECT_EQ(mgr.GetTaskInfoLocked(1001, 6, taskInfo, bundleName), E_TASK_ID_NOT_EXIST);
}

/**
 * @tc.name: IsTimeoutTriggeredLocked_001
 * @tc.desc: Test IsTimeoutTriggeredLocked returns false when the task is absent.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, IsTimeoutTriggeredLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo copy;
    bool shouldAdd = false;
    EXPECT_FALSE(mgr.IsTimeoutTriggeredLocked(1001, copy, shouldAdd));
}

/**
 * @tc.name: IsTimeoutTriggeredLocked_002
 * @tc.desc: Test IsTimeoutTriggeredLocked returns false when the task is not RUNNING.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, IsTimeoutTriggeredLocked_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    TaskInfo copy;
    bool shouldAdd = false;
    EXPECT_FALSE(mgr.IsTimeoutTriggeredLocked(1001, copy, shouldAdd));
}

/**
 * @tc.name: IsTimeoutTriggeredLocked_003
 * @tc.desc: Test IsTimeoutTriggeredLocked increments the counter without blocking.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, IsTimeoutTriggeredLocked_003, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    InsertTask(1001, info);
    TaskInfo copy;
    bool shouldAdd = true;
    EXPECT_TRUE(mgr.IsTimeoutTriggeredLocked(1001, copy, shouldAdd));
    EXPECT_EQ(mgr.taskMap_.at(1001).timeoutCount_, 1);
    EXPECT_FALSE(shouldAdd);
}

/**
 * @tc.name: IsTimeoutTriggeredLocked_004
 * @tc.desc: Test IsTimeoutTriggeredLocked requests block listing at the limit.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, IsTimeoutTriggeredLocked_004, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.timeoutCount_ = 2;
    InsertTask(1001, info);
    TaskInfo copy;
    bool shouldAdd = false;
    EXPECT_TRUE(mgr.IsTimeoutTriggeredLocked(1001, copy, shouldAdd));
    EXPECT_TRUE(shouldAdd);
}

/**
 * @tc.name: MarkTimeOutStatusLocked_001
 * @tc.desc: Test MarkTimeOutStatusLocked is a no-op when the task is absent.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, MarkTimeOutStatusLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    TaskInfo copy;
    mgr.MarkTimeOutStatusLocked(9999, false, copy);
    EXPECT_TRUE(mgr.IsTaskExistLocked(1001));
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::NOT_STARTED);
}

/**
 * @tc.name: MarkTimeOutStatusLocked_002
 * @tc.desc: Test MarkTimeOutStatusLocked keeps an IN_BLOCK_LIST task unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, MarkTimeOutStatusLocked_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::IN_BLOCK_LIST;
    InsertTask(1001, info);
    TaskInfo copy;
    mgr.MarkTimeOutStatusLocked(1001, false, copy);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::IN_BLOCK_LIST);
    EXPECT_EQ(copy.status_, TaskStatus::IN_BLOCK_LIST);
}

/**
 * @tc.name: MarkTimeOutStatusLocked_003
 * @tc.desc: Test MarkTimeOutStatusLocked transitions to IN_BLOCK_LIST when requested.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, MarkTimeOutStatusLocked_003, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    TaskInfo copy;
    mgr.MarkTimeOutStatusLocked(1001, true, copy);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::IN_BLOCK_LIST);
}

/**
 * @tc.name: MarkTimeOutStatusLocked_004
 * @tc.desc: Test MarkTimeOutStatusLocked transitions to FINISHED when not blocking.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, MarkTimeOutStatusLocked_004, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    TaskInfo copy;
    mgr.MarkTimeOutStatusLocked(1001, false, copy);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::FINISHED);
}

/**
 * @tc.name: HandleProcessDiedLocked_001
 * @tc.desc: Test HandleProcessDiedLocked returns NOT_REGISTERED when absent.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleProcessDiedLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    int32_t appIndex = -1;
    TaskInfo copy;
    EXPECT_EQ(mgr.HandleProcessDiedLocked(1001, 500, false, appIndex, copy), ProcessDiedResult::NOT_REGISTERED);
}

/**
 * @tc.name: HandleProcessDiedLocked_002
 * @tc.desc: Test HandleProcessDiedLocked returns NOT_REGISTERED on pid mismatch.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleProcessDiedLocked_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.pid_ = 500;
    InsertTask(1001, info);
    int32_t appIndex = -1;
    TaskInfo copy;
    EXPECT_EQ(mgr.HandleProcessDiedLocked(1001, 501, false, appIndex, copy), ProcessDiedResult::NOT_REGISTERED);
}

/**
 * @tc.name: HandleProcessDiedLocked_003
 * @tc.desc: Test HandleProcessDiedLocked resets flags when the task is not RUNNING.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleProcessDiedLocked_003, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::FINISHED;
    info.pid_ = 500;
    info.isBg_ = false;
    info.hasShortTask_ = true;
    info.hasLongTask_ = true;
    info.hasAudioStream_ = true;
    InsertTask(1001, info);
    int32_t appIndex = -1;
    TaskInfo copy;
    EXPECT_EQ(mgr.HandleProcessDiedLocked(1001, 500, false, appIndex, copy), ProcessDiedResult::NOT_RUNNING);
    EXPECT_EQ(appIndex, 0);
    EXPECT_EQ(mgr.taskMap_.at(1001).isBg_, true);
    EXPECT_FALSE(mgr.taskMap_.at(1001).hasShortTask_);
}

/**
 * @tc.name: HandleProcessDiedLocked_004
 * @tc.desc: Test HandleProcessDiedLocked blocks the task on abnormal exit.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleProcessDiedLocked_004, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.pid_ = 500;
    InsertTask(1001, info);
    int32_t appIndex = -1;
    TaskInfo copy;
    EXPECT_EQ(mgr.HandleProcessDiedLocked(1001, 500, true, appIndex, copy), ProcessDiedResult::RUNNING_ABNORMAL);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::IN_BLOCK_LIST);
}

/**
 * @tc.name: HandleProcessDiedLocked_005
 * @tc.desc: Test HandleProcessDiedLocked marks a RUNNING task FINISHED on normal exit.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleProcessDiedLocked_005, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.pid_ = 500;
    InsertTask(1001, info);
    int32_t appIndex = -1;
    TaskInfo copy;
    EXPECT_EQ(mgr.HandleProcessDiedLocked(1001, 500, false, appIndex, copy), ProcessDiedResult::RUNNING_NORMAL);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::FINISHED);
}

/**
 * @tc.name: HandleProcessDiedLocked_006
 * @tc.desc: Test HandleProcessDiedLocked blocks a task whose timeout count reaches the limit on normal exit.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleProcessDiedLocked_006, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.pid_ = 500;
    info.timeoutCount_ = BACKGROUND_LOADER_TIMEOUT_COUNT;
    InsertTask(1001, info);
    int32_t appIndex = -1;
    TaskInfo copy;
    EXPECT_EQ(mgr.HandleProcessDiedLocked(1001, 500, false, appIndex, copy), ProcessDiedResult::RUNNING_NORMAL);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::IN_BLOCK_LIST);
}

/**
 * @tc.name: CheckPerceivableInterceptLocked_001
 * @tc.desc: Test CheckPerceivableInterceptLocked returns false when absent.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CheckPerceivableInterceptLocked_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    std::string bundleName;
    int32_t appIndex = -1;
    TaskInfo copy;
    EXPECT_FALSE(mgr.CheckPerceivableInterceptLocked(1001, bundleName, appIndex, copy));
}

/**
 * @tc.name: CheckPerceivableInterceptLocked_002
 * @tc.desc: Test CheckPerceivableInterceptLocked returns false when not RUNNING.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CheckPerceivableInterceptLocked_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    std::string bundleName;
    int32_t appIndex = -1;
    TaskInfo copy;
    EXPECT_FALSE(mgr.CheckPerceivableInterceptLocked(1001, bundleName, appIndex, copy));
}

/**
 * @tc.name: CheckPerceivableInterceptLocked_003
 * @tc.desc: Test CheckPerceivableInterceptLocked returns false when in foreground.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CheckPerceivableInterceptLocked_003, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.isBg_ = false;
    InsertTask(1001, info);
    std::string bundleName;
    int32_t appIndex = -1;
    TaskInfo copy;
    EXPECT_FALSE(mgr.CheckPerceivableInterceptLocked(1001, bundleName, appIndex, copy));
}

/**
 * @tc.name: CheckPerceivableInterceptLocked_004
 * @tc.desc: Test CheckPerceivableInterceptLocked blocks a RUNNING background task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CheckPerceivableInterceptLocked_004, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.isBg_ = true;
    InsertTask(1001, info);
    std::string bundleName;
    int32_t appIndex = -1;
    TaskInfo copy;
    EXPECT_TRUE(mgr.CheckPerceivableInterceptLocked(1001, bundleName, appIndex, copy));
    EXPECT_EQ(bundleName, "com.test.bundle");
    EXPECT_EQ(appIndex, 0);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::IN_BLOCK_LIST);
}

/**
 * @tc.name: AddToBlockList_001
 * @tc.desc: Test AddToBlockList inserts the bundle even without a task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, AddToBlockList_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.AddToBlockList("com.test.block", 1001, static_cast<int32_t>(BlockListReason::TIMEOUT));
    EXPECT_TRUE(mgr.IsInBlockListLocked("com.test.block"));
    EXPECT_FALSE(mgr.IsTaskExistLocked(1001));
}

/**
 * @tc.name: AddToBlockList_002
 * @tc.desc: Test AddToBlockList blocks an existing task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, AddToBlockList_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.block", "TestAbility", 0, 1));
    mgr.AddToBlockList("com.test.block", 1001, static_cast<int32_t>(BlockListReason::TIMEOUT));
    EXPECT_TRUE(mgr.IsInBlockListLocked("com.test.block"));
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::IN_BLOCK_LIST);
}

/**
 * @tc.name: ClearAllByUid_001
 * @tc.desc: Test ClearAllByUid clears block list, ability map and task map.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, ClearAllByUid_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.AddToBlockListLocked("com.test.bundle");
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    mgr.SaveRemoteObjectLocked("com.test.bundle", "TestAbility", 0, new AbilityConnectCallback(
        "com.test.bundle", "TestAbility", 0, 1001));

    mgr.ClearAllByUid(1001, "com.test.bundle", 0);
    EXPECT_FALSE(mgr.IsInBlockListLocked("com.test.bundle"));
    EXPECT_FALSE(mgr.IsTaskExistLocked(1001));
    EXPECT_EQ(mgr.abilityMap_.count("com.test.bundle_0"), 0);
}

/**
 * @tc.name: ClearAllByUid_DelayFinish_001
 * @tc.desc: Test ClearAllByUid cancels the pending delay finish event.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, ClearAllByUid_DelayFinish_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    InsertTask(1001, info);
    mgr.PostDelayFinishEvent(1001, false, info);
    EXPECT_TRUE(mgr.IsDelayFinishPending(1001));

    mgr.ClearAllByUid(1001, "com.test.bundle", 0);
    EXPECT_FALSE(mgr.IsDelayFinishPending(1001));
    EXPECT_FALSE(mgr.IsTaskExistLocked(1001));
}

/**
 * @tc.name: DbQueueNull_001
 * @tc.desc: Test db access helpers return E_SYSTEM_SERVICE_EXCEPTION when the db queue is null.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, DbQueueNull_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.dbQueue_.reset();
    EXPECT_EQ(mgr.UpsertTaskToDb(1001, MakeTask(1001, "b", "a", 0, 1)), E_SYSTEM_SERVICE_EXCEPTION);
    EXPECT_EQ(mgr.DeleteTaskFromDb(1001), E_SYSTEM_SERVICE_EXCEPTION);
    EXPECT_EQ(mgr.DeleteBlockListFromDb("b"), E_SYSTEM_SERVICE_EXCEPTION);
}

/**
 * @tc.name: UpsertDeleteTaskFromDb_001
 * @tc.desc: Test UpsertTaskToDb and DeleteTaskFromDb return ERR_OK with a valid queue.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, UpsertDeleteTaskFromDb_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    EXPECT_EQ(mgr.UpsertTaskToDb(1001, MakeTask(1001, "b", "a", 0, 1)), ERR_OK);
    EXPECT_EQ(mgr.DeleteTaskFromDb(1001), ERR_OK);
    EXPECT_EQ(mgr.DeleteBlockListFromDb("b"), ERR_OK);
}

/**
 * @tc.name: ParseAppInfo_001
 * @tc.desc: Test ParseAppInfo parses a valid payload.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, ParseAppInfo_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    nlohmann::json payload;
    payload["bundleName"] = "com.test.bundle";
    payload["abilityName"] = "TestAbility";
    payload["appIndex"] = 0;
    std::string bundleName;
    std::string abilityName;
    int32_t appIndex = -1;
    EXPECT_TRUE(mgr.ParseAppInfo(payload, bundleName, abilityName, appIndex));
    EXPECT_EQ(bundleName, "com.test.bundle");
    EXPECT_EQ(abilityName, "TestAbility");
    EXPECT_EQ(appIndex, 0);
}

/**
 * @tc.name: ParseAppInfo_002
 * @tc.desc: Test ParseAppInfo fails on missing fields.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, ParseAppInfo_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    nlohmann::json payload;
    std::string bundleName;
    std::string abilityName;
    int32_t appIndex = -1;
    EXPECT_FALSE(mgr.ParseAppInfo(payload, bundleName, abilityName, appIndex));
}

/**
 * @tc.name: ParseAppInfo_003
 * @tc.desc: Test ParseAppInfo fails on an empty bundleName.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, ParseAppInfo_003, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    nlohmann::json payload;
    payload["bundleName"] = "";
    payload["abilityName"] = "TestAbility";
    payload["appIndex"] = 0;
    std::string bundleName;
    std::string abilityName;
    int32_t appIndex = -1;
    EXPECT_FALSE(mgr.ParseAppInfo(payload, bundleName, abilityName, appIndex));
}

/**
 * @tc.name: ParseAppInfo_004
 * @tc.desc: Test ParseAppInfo fails on an over-long bundleName.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, ParseAppInfo_004, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    nlohmann::json payload;
    payload["bundleName"] = std::string(300, 'a');
    payload["abilityName"] = "TestAbility";
    payload["appIndex"] = 0;
    std::string bundleName;
    std::string abilityName;
    int32_t appIndex = -1;
    EXPECT_FALSE(mgr.ParseAppInfo(payload, bundleName, abilityName, appIndex));
}

/**
 * @tc.name: ParseAppInfo_005
 * @tc.desc: Test ParseAppInfo fails on an over-long abilityName.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, ParseAppInfo_005, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    nlohmann::json payload;
    payload["bundleName"] = "com.test.bundle";
    payload["abilityName"] = std::string(300, 'a');
    payload["appIndex"] = 0;
    std::string bundleName;
    std::string abilityName;
    int32_t appIndex = -1;
    EXPECT_FALSE(mgr.ParseAppInfo(payload, bundleName, abilityName, appIndex));
}

/**
 * @tc.name: IsValidUserId_001
 * @tc.desc: Test IsValidUserId returns a valid userId when absent or consistent, false when mismatched.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, IsValidUserId_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    int32_t userId = 0;

    nlohmann::json noUserId;
    EXPECT_TRUE(mgr.IsValidUserId(noUserId, 200000, userId));
    EXPECT_EQ(userId, 1);

    nlohmann::json payload;
    payload["userId"] = 100;
    EXPECT_TRUE(mgr.IsValidUserId(payload, 20000000, userId));
    EXPECT_EQ(userId, 100);

    EXPECT_FALSE(mgr.IsValidUserId(payload, 200000, userId));
}

/**
 * @tc.name: IsValidUserId_002
 * @tc.desc: Test IsValidUserId returns false when the uid cannot be converted to a userId.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, IsValidUserId_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    int32_t userId = 0;
    nlohmann::json payload;
    payload["userId"] = 100;
    EXPECT_FALSE(mgr.IsValidUserId(payload, -1, userId));
}

/**
 * @tc.name: ParseFrameNum_001
 * @tc.desc: Test ParseFrameNum resolves frameNum to default, valid and invalid values.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, ParseFrameNum_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    int32_t frameNum = -1;

    nlohmann::json empty;
    mgr.ParseFrameNum(empty, frameNum);
    EXPECT_EQ(frameNum, 0);

    nlohmann::json payload;
    payload["frameNum"] = 5;
    mgr.ParseFrameNum(payload, frameNum);
    EXPECT_EQ(frameNum, 5);

    nlohmann::json negative;
    negative["frameNum"] = -1;
    mgr.ParseFrameNum(negative, frameNum);
    EXPECT_EQ(frameNum, 0);
}

/**
 * @tc.name: RemoteObject_001
 * @tc.desc: Test Save/Get/Remove remote object lifecycle.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, RemoteObject_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    sptr<IRemoteObject> remoteObject = new AbilityConnectCallback("com.test.bundle", "TestAbility", 0, 1001);
    mgr.SaveRemoteObjectLocked("com.test.bundle", "TestAbility", 0, remoteObject);
    EXPECT_EQ(mgr.GetRemoteObjectLocked("com.test.bundle", "TestAbility", 0), remoteObject);
    EXPECT_EQ(mgr.GetRemoteObjectLocked("com.test.other", "TestAbility", 0), nullptr);
    mgr.RemoveRemoteObjectLocked("com.test.bundle", 0);
    EXPECT_EQ(mgr.GetRemoteObjectLocked("com.test.bundle", "TestAbility", 0), nullptr);
}

/**
 * @tc.name: PostCancelTimeoutTask_001
 * @tc.desc: Test PostTimeoutTask and CancelTimeoutTask manage the timeout map.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, PostCancelTimeoutTask_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.CancelTimeoutTask(1001);
    mgr.PostTimeoutTask(1001);
    EXPECT_EQ(mgr.timeoutTaskMap_.count(1001), 1);
    mgr.PostTimeoutTask(1001);
    EXPECT_EQ(mgr.timeoutTaskMap_.count(1001), 1);
    mgr.CancelTimeoutTask(1001);
    EXPECT_EQ(mgr.timeoutTaskMap_.count(1001), 0);
}

/**
 * @tc.name: SendOnStart_001
 * @tc.desc: Test SendOnStart returns early when the task is not found.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, SendOnStart_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    mgr.SendOnStart(nullptr, "com.test.bundle", 0, 9999);
    EXPECT_TRUE(mgr.IsTaskExistLocked(1001));
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::NOT_STARTED);
}

/**
 * @tc.name: SendOnStop_001
 * @tc.desc: Test SendOnStop does not remove the cached remote object.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, SendOnStop_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    sptr<IRemoteObject> remoteObject = new AbilityConnectCallback("com.test.bundle", "TestAbility", 0, 1001);
    mgr.SaveRemoteObjectLocked("com.test.bundle", "TestAbility", 0, remoteObject);

    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    mgr.SendOnStop(info, static_cast<int32_t>(StopCode::TIMEOUT_ERROR), "timeOut");
    EXPECT_EQ(mgr.abilityMap_.count("com.test.bundle_0"), 1);
}

/**
 * @tc.name: ConnectCachedAbility_001
 * @tc.desc: Test ConnectCachedAbility returns true when a cached remote object exists.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, ConnectCachedAbility_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    sptr<IRemoteObject> remoteObject = new AbilityConnectCallback("com.test.bundle", "TestAbility", 0, 1001);
    mgr.SaveRemoteObjectLocked("com.test.bundle", "TestAbility", 0, remoteObject);
    EXPECT_TRUE(mgr.ConnectCachedAbility("com.test.bundle", "TestAbility", 0, 1001));
    EXPECT_FALSE(mgr.IsTaskExistLocked(1001));
}

/**
 * @tc.name: ConnectCachedAbility_002
 * @tc.desc: Test ConnectCachedAbility returns false when no cached remote object exists.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, ConnectCachedAbility_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    EXPECT_FALSE(mgr.ConnectCachedAbility("com.test.nocache", "TestAbility", 0, 1001));
}

/**
 * @tc.name: HandleExistingTask_001
 * @tc.desc: Test HandleExistingTask rejects a bundle in the block list.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleExistingTask_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.AddToBlockListLocked("com.test.bundle");
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.HandleExistingTask(taskInfo, "com.test.bundle", 1001, copy), E_BACKGROUND_LOADER_PROHIBITED);
}

/**
 * @tc.name: HandleExistingTask_002
 * @tc.desc: Test HandleExistingTask returns E_TASK_NOT_EXIST when absent.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleExistingTask_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.HandleExistingTask(taskInfo, "com.test.bundle", 1001, copy), E_TASK_NOT_EXIST);
}

/**
 * @tc.name: HandleExistingTask_003
 * @tc.desc: Test HandleExistingTask returns E_NOT_SPECIFIED_ABILITY_NAME on abilityName mismatch.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleExistingTask_003, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "OldAbility", 0, 1));
    BackgroundLoaderTaskInfo taskInfo(2, "NewAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.HandleExistingTask(taskInfo, "com.test.bundle", 1001, copy), E_NOT_SPECIFIED_ABILITY_NAME);
}

/**
 * @tc.name: HandleExistingTask_004
 * @tc.desc: Test HandleExistingTask returns E_ADD_REPEAT_TASK_ERR for a duplicate taskId.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleExistingTask_004, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.HandleExistingTask(taskInfo, "com.test.bundle", 1001, copy), E_ADD_REPEAT_TASK_ERR);
}

/**
 * @tc.name: HandleExistingTask_005
 * @tc.desc: Test HandleExistingTask refreshes taskId on success.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleExistingTask_005, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    BackgroundLoaderTaskInfo taskInfo(2, "TestAbility");
    TaskInfo copy;
    EXPECT_EQ(mgr.HandleExistingTask(taskInfo, "com.test.bundle", 1001, copy), ERR_OK);
    EXPECT_EQ(mgr.taskMap_.at(1001).taskId_, 2);
}

/**
 * @tc.name: RegisterNewTask_001
 * @tc.desc: Test RegisterNewTask rejects a bundle in the block list.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, RegisterNewTask_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.AddToBlockListLocked("com.test.bundle");
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    EXPECT_EQ(mgr.RegisterNewTask(taskInfo, "com.test.bundle", 0, 1001), E_BACKGROUND_LOADER_PROHIBITED);
}

/**
 * @tc.name: VerifyAbilityName_001
 * @tc.desc: Test VerifyAbilityName returns false for empty inputs.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, VerifyAbilityName_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    EXPECT_FALSE(mgr.VerifyAbilityName("", "TestAbility", 1001));
    EXPECT_FALSE(mgr.VerifyAbilityName("com.test.bundle", "", 1001));
}

/**
 * @tc.name: ReportEvents_001
 * @tc.desc: Test report helpers do not crash and do not mutate state.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, ReportEvents_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    InsertTask(1001, info);

    mgr.ReportTaskChangeEvent("com.test.bundle", 0, static_cast<int32_t>(ResType::BackgroundLoaderState::ADD));
    mgr.ReportTaskFinishEvent("com.test.bundle", 0, 1, E_BACKGROUND_LOADER_FAILED);
    mgr.ReportTaskStatus(info, 0, false);
    mgr.ReportTaskStatus(info, 1, true);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::RUNNING);
}

/**
 * @tc.name: HandleBackgroundLoaderTask_NotReady_001
 * @tc.desc: Test HandleBackgroundLoaderTask returns early when not ready.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleBackgroundLoaderTask_NotReady_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    mgr.isReady_.store(false);
    mgr.HandleBackgroundLoaderTask(nlohmann::json{});
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::NOT_STARTED);
}

/**
 * @tc.name: HandleBackgroundLoaderTask_InvalidPayload_001
 * @tc.desc: Test HandleBackgroundLoaderTask fails on an invalid payload.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleBackgroundLoaderTask_InvalidPayload_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    mgr.HandleBackgroundLoaderTask(nlohmann::json{});
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::NOT_STARTED);
}

/**
 * @tc.name: HandleBackgroundLoaderTask_BlockList_001
 * @tc.desc: Test HandleBackgroundLoaderTask fails for a block listed bundle.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleBackgroundLoaderTask_BlockList_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    mgr.AddToBlockListLocked("com.test.bundle");
    nlohmann::json payload;
    payload["bundleName"] = "com.test.bundle";
    payload["abilityName"] = "TestAbility";
    payload["appIndex"] = 0;
    mgr.HandleBackgroundLoaderTask(payload);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::NOT_STARTED);
    EXPECT_TRUE(mgr.IsInBlockListLocked("com.test.bundle"));
}

/**
 * @tc.name: HandleBackgroundLoaderTask_NoTask_001
 * @tc.desc: Test HandleBackgroundLoaderTask fails when no matching task exists.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleBackgroundLoaderTask_NoTask_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.other.bundle", "TestAbility", 0, 1));
    nlohmann::json payload;
    payload["bundleName"] = "com.test.bundle";
    payload["abilityName"] = "TestAbility";
    payload["appIndex"] = 0;
    mgr.HandleBackgroundLoaderTask(payload);
    EXPECT_TRUE(mgr.IsTaskExistLocked(1001));
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::NOT_STARTED);
}

/**
 * @tc.name: HandleBackgroundLoaderTask_UserIdMismatch_001
 * @tc.desc: Test HandleBackgroundLoaderTask skips prelaunch when payload userId mismatches uid.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleBackgroundLoaderTask_UserIdMismatch_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    int32_t uid = 200000;
    TaskInfo info = MakeTask(uid, "com.test.bundle", "TestAbility", 0, 1);
    info.isBg_ = true;
    InsertTask(uid, info);

    nlohmann::json payload;
    payload["bundleName"] = "com.test.bundle";
    payload["abilityName"] = "TestAbility";
    payload["appIndex"] = 0;
    payload["userId"] = 999;
    mgr.HandleBackgroundLoaderTask(payload);

    EXPECT_TRUE(mgr.IsTaskExistLocked(uid));
    EXPECT_EQ(mgr.taskMap_.at(uid).status_, TaskStatus::NOT_STARTED);
}

/**
 * @tc.name: HandleAppStateChangeEvent_001
 * @tc.desc: Test HandleAppStateChangeEvent updates isBg_ for foreground and background.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAppStateChangeEvent_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));

    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["bundleName"] = "com.test.bundle";
    mgr.HandleAppStateChangeEvent(static_cast<int64_t>(ResType::ProcessStatus::PROCESS_BACKGROUND), payload);
    EXPECT_TRUE(mgr.taskMap_.at(1001).isBg_);

    mgr.HandleAppStateChangeEvent(static_cast<int64_t>(ResType::ProcessStatus::PROCESS_FOREGROUND), payload);
    EXPECT_FALSE(mgr.taskMap_.at(1001).isBg_);
}

/**
 * @tc.name: HandleAppStateChangeEvent_002
 * @tc.desc: Test HandleAppStateChangeEvent ignores invalid values and bad payloads.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAppStateChangeEvent_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    mgr.HandleAppStateChangeEvent(999, nlohmann::json{});
    mgr.HandleAppStateChangeEvent(static_cast<int64_t>(ResType::ProcessStatus::PROCESS_BACKGROUND), nlohmann::json{});
    EXPECT_FALSE(mgr.taskMap_.at(1001).isBg_);
}

/**
 * @tc.name: HandleAppInstallUninstall_001
 * @tc.desc: Test HandleAppInstallUninstall guards against not-ready, invalid value and bad payload.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAppInstallUninstall_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));

    mgr.isReady_.store(false);
    mgr.HandleAppInstallUninstall(static_cast<int64_t>(ResType::AppInstallStatus::APP_UNINSTALL), nlohmann::json{});

    mgr.isReady_.store(true);
    mgr.HandleAppInstallUninstall(static_cast<int64_t>(ResType::AppInstallStatus::APP_INSTALL_END), nlohmann::json{});
    mgr.HandleAppInstallUninstall(static_cast<int64_t>(ResType::AppInstallStatus::APP_UNINSTALL), nlohmann::json{});

    EXPECT_TRUE(mgr.IsTaskExistLocked(1001));
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::NOT_STARTED);
}

/**
 * @tc.name: HandleAppInstallUninstall_Clear_001
 * @tc.desc: Test HandleAppInstallUninstall clears task, block list and remote object.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAppInstallUninstall_Clear_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.AddToBlockListLocked("com.test.bundle");
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    mgr.SaveRemoteObjectLocked("com.test.bundle", "TestAbility", 0,
        new AbilityConnectCallback("com.test.bundle", "TestAbility", 0, 1001));

    nlohmann::json payload;
    payload["bundleName"] = "com.test.bundle";
    payload["appIndex"] = 0;
    payload["uid"] = 1001;
    mgr.HandleAppInstallUninstall(static_cast<int64_t>(ResType::AppInstallStatus::APP_UNINSTALL), payload);

    EXPECT_FALSE(mgr.IsInBlockListLocked("com.test.bundle"));
    EXPECT_FALSE(mgr.IsTaskExistLocked(1001));
    EXPECT_EQ(mgr.abilityMap_.count("com.test.bundle_0"), 0);
}

/**
 * @tc.name: HandleTransientTaskEvent_001
 * @tc.desc: Test HandleTransientTaskEvent clears the short task mark on end.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleTransientTaskEvent_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.hasShortTask_ = true;
    InsertTask(1001, info);

    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["bundleName"] = "com.test.bundle";
    mgr.HandleTransientTaskEvent(static_cast<int64_t>(ResType::TransientTaskStatus::TRANSIENT_TASK_END), payload);
    EXPECT_FALSE(mgr.taskMap_.at(1001).hasShortTask_);
}

/**
 * @tc.name: HandleTransientTaskEvent_002
 * @tc.desc: Test HandleTransientTaskEvent sets the short task mark without intercepting a non-running task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleTransientTaskEvent_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::NOT_STARTED;
    info.isBg_ = true;
    InsertTask(1001, info);

    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["bundleName"] = "com.test.bundle";
    mgr.HandleTransientTaskEvent(static_cast<int64_t>(ResType::TransientTaskStatus::TRANSIENT_TASK_START), payload);
    EXPECT_TRUE(mgr.taskMap_.at(1001).hasShortTask_);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::NOT_STARTED);
}

/**
 * @tc.name: HandleContinuousTaskEvent_001
 * @tc.desc: Test HandleContinuousTaskEvent clears the long task mark on end.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleContinuousTaskEvent_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.hasLongTask_ = true;
    InsertTask(1001, info);

    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["bundleName"] = "com.test.bundle";
    mgr.HandleContinuousTaskEvent(static_cast<int64_t>(ResType::ContinuousTaskStatus::CONTINUOUS_TASK_END), payload);
    EXPECT_FALSE(mgr.taskMap_.at(1001).hasLongTask_);
}

/**
 * @tc.name: HandleAppAudioRenderStateChangeEvent_001
 * @tc.desc: Test HandleAppAudioRenderStateChangeEvent clears the audio mark on stop.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAppAudioRenderStateChangeEvent_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.hasAudioStream_ = true;
    InsertTask(1001, info);

    nlohmann::json payload;
    payload["uid"] = 1001;
    mgr.HandleAppAudioRenderStateChangeEvent(
        static_cast<int64_t>(ResType::AudioStatus::RENDERER_STOPPED), payload);
    EXPECT_FALSE(mgr.taskMap_.at(1001).hasAudioStream_);
}

/**
 * @tc.name: HandleAppAudioRenderStateChangeEvent_002
 * @tc.desc: Test HandleAppAudioRenderStateChangeEvent sets the audio mark without intercepting a non-running task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAppAudioRenderStateChangeEvent_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::NOT_STARTED;
    info.isBg_ = true;
    InsertTask(1001, info);

    nlohmann::json payload;
    payload["uid"] = 1001;
    mgr.HandleAppAudioRenderStateChangeEvent(
        static_cast<int64_t>(ResType::AudioStatus::RENDERER_RUNNING), payload);
    EXPECT_TRUE(mgr.taskMap_.at(1001).hasAudioStream_);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::NOT_STARTED);
}

/**
 * @tc.name: HandleAppAudioCaptureStateChangeEvent_001
 * @tc.desc: Test HandleAppAudioCaptureStateChangeEvent leaves a non-running task unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAppAudioCaptureStateChangeEvent_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::NOT_STARTED;
    info.isBg_ = true;
    InsertTask(1001, info);

    nlohmann::json payload;
    payload["uid"] = 1001;
    mgr.HandleAppAudioCaptureStateChangeEvent(
        static_cast<int64_t>(ResType::AudioCaptureState::AUDIO_CAPTURE_BEGIN), payload);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::NOT_STARTED);
}

/**
 * @tc.name: HandleAppTorchStateEvent_001
 * @tc.desc: Test HandleAppTorchStateEvent leaves a non-running task unchanged on torch on.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAppTorchStateEvent_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::NOT_STARTED;
    info.isBg_ = true;
    InsertTask(1001, info);

    nlohmann::json payload;
    payload["uid"] = 1001;
    mgr.HandleAppTorchStateEvent(static_cast<int64_t>(ResType::TorchState::TORCH_ON), payload);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::NOT_STARTED);
}

/**
 * @tc.name: HandleProcessDiedEvent_001
 * @tc.desc: Test HandleProcessDiedEvent guards against not-ready, bad value and bad payload.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleProcessDiedEvent_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.pid_ = 555;
    InsertTask(1001, info);

    mgr.isReady_.store(false);
    mgr.HandleProcessDiedEvent(static_cast<int64_t>(ResType::ProcessStatus::PROCESS_DIED), nlohmann::json{});

    mgr.isReady_.store(true);
    mgr.HandleProcessDiedEvent(static_cast<int64_t>(ResType::ProcessStatus::PROCESS_CREATED), nlohmann::json{});
    mgr.HandleProcessDiedEvent(static_cast<int64_t>(ResType::ProcessStatus::PROCESS_DIED), nlohmann::json{});

    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::RUNNING);
    EXPECT_EQ(mgr.taskMap_.at(1001).pid_, 555);
}

/**
 * @tc.name: PostCancelDelayFinishEvent_001
 * @tc.desc: Test PostDelayFinishEvent makes the finish event pending and CancelDelayFinishEvent clears it.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, PostCancelDelayFinishEvent_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);

    EXPECT_FALSE(mgr.IsDelayFinishPending(1001));
    mgr.PostDelayFinishEvent(1001, false, info);
    EXPECT_TRUE(mgr.IsDelayFinishPending(1001));

    EXPECT_TRUE(mgr.CancelDelayFinishEvent(1001));
    EXPECT_FALSE(mgr.IsDelayFinishPending(1001));
    EXPECT_FALSE(mgr.CancelDelayFinishEvent(1001));
}

/**
 * @tc.name: RemoveDelayFinishHandle_001
 * @tc.desc: Test RemoveDelayFinishHandle clears the pending finish event handle.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, RemoveDelayFinishHandle_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    mgr.PostDelayFinishEvent(1001, false, info);
    EXPECT_TRUE(mgr.IsDelayFinishPending(1001));
    mgr.RemoveDelayFinishHandle(1001);
    EXPECT_FALSE(mgr.IsDelayFinishPending(1001));
}

/**
 * @tc.name: ExecuteOnStopFinish_001
 * @tc.desc: Test ExecuteOnStopFinish marks the task FINISHED and clears its handle.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, ExecuteOnStopFinish_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    InsertTask(1001, info);
    mgr.PostDelayFinishEvent(1001, false, info);
    EXPECT_TRUE(mgr.IsDelayFinishPending(1001));

    mgr.ExecuteOnStopFinish(1001, false, info);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::FINISHED);
    EXPECT_FALSE(mgr.IsDelayFinishPending(1001));
}

/**
 * @tc.name: FinishTaskWithCheck_DelayFinish_001
 * @tc.desc: Test FinishTaskWithCheck returns E_BACKGROUND_LOADER_FAILED while the delay finish event is pending.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTaskWithCheck_DelayFinish_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    int32_t uid = GetCallingUid();
    TaskInfo info = MakeTask(uid, "com.test.bundle", "TestAbility", 0, 5);
    info.status_ = TaskStatus::RUNNING;
    InsertTask(uid, info);
    mgr.PostDelayFinishEvent(uid, false, info);

    BackgroundLoaderTaskInfo taskInfo(5, "TestAbility");
    EXPECT_EQ(mgr.FinishTaskWithCheck(taskInfo), E_BACKGROUND_LOADER_FAILED);
    mgr.CancelDelayFinishEvent(uid);
}

/**
 * @tc.name: HandleProcessDiedEvent_DelayFinish_001
 * @tc.desc: Test HandleProcessDiedEvent cancels the delay finish event and marks the task FINISHED normally.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleProcessDiedEvent_DelayFinish_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.pid_ = 500;
    InsertTask(1001, info);
    mgr.PostDelayFinishEvent(1001, false, info);
    EXPECT_TRUE(mgr.IsDelayFinishPending(1001));

    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["bundleName"] = "com.test.bundle";
    payload["pid"] = 500;
    payload["exitReason"] = 1;
    mgr.HandleProcessDiedEvent(static_cast<int64_t>(ResType::ProcessStatus::PROCESS_DIED), payload);

    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::FINISHED);
}

/**
 * @tc.name: HandleProcessDiedEvent_NotRegistered_001
 * @tc.desc: Test HandleProcessDiedEvent cancels the delay finish event for an unregistered task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleProcessDiedEvent_NotRegistered_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    mgr.PostDelayFinishEvent(1001, false, info);
    EXPECT_TRUE(mgr.IsDelayFinishPending(1001));

    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["bundleName"] = "com.test.bundle";
    payload["pid"] = 500;
    payload["exitReason"] = 0;
    mgr.HandleProcessDiedEvent(static_cast<int64_t>(ResType::ProcessStatus::PROCESS_DIED), payload);

    EXPECT_FALSE(mgr.IsDelayFinishPending(1001));
    EXPECT_FALSE(mgr.IsTaskExistLocked(1001));
}

/**
 * @tc.name: HandleProcessDiedEvent_NotRunning_001
 * @tc.desc: Test HandleProcessDiedEvent keeps a non-running task unchanged.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleProcessDiedEvent_NotRunning_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::FINISHED;
    info.pid_ = 500;
    InsertTask(1001, info);

    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["bundleName"] = "com.test.bundle";
    payload["pid"] = 500;
    payload["exitReason"] = 0;
    mgr.HandleProcessDiedEvent(static_cast<int64_t>(ResType::ProcessStatus::PROCESS_DIED), payload);

    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::FINISHED);
}

/**
 * @tc.name: HandleProcessDiedEvent_Abnormal_001
 * @tc.desc: Test HandleProcessDiedEvent blocks the task on abnormal exit.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleProcessDiedEvent_Abnormal_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.pid_ = 500;
    InsertTask(1001, info);

    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["bundleName"] = "com.test.bundle";
    payload["pid"] = 500;
    payload["exitReason"] = 2;
    mgr.HandleProcessDiedEvent(static_cast<int64_t>(ResType::ProcessStatus::PROCESS_DIED), payload);

    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::IN_BLOCK_LIST);
    EXPECT_TRUE(mgr.IsInBlockListLocked("com.test.bundle"));
}

/**
 * @tc.name: RegisterTaskWithCheck_NotReady_001
 * @tc.desc: Test RegisterTaskWithCheck returns E_SYSTEM_SERVICE_NOT_READY when not ready.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, RegisterTaskWithCheck_NotReady_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.isReady_.store(false);
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    EXPECT_EQ(mgr.RegisterTaskWithCheck(taskInfo), E_SYSTEM_SERVICE_NOT_READY);
}

/**
 * @tc.name: UnregisterTaskWithCheck_NotReady_001
 * @tc.desc: Test UnregisterTaskWithCheck returns E_SYSTEM_SERVICE_NOT_READY when not ready.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, UnregisterTaskWithCheck_NotReady_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.isReady_.store(false);
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    EXPECT_EQ(mgr.UnregisterTaskWithCheck(taskInfo), E_SYSTEM_SERVICE_NOT_READY);
}

/**
 * @tc.name: UnregisterTaskWithCheck_NoTask_001
 * @tc.desc: Test UnregisterTaskWithCheck returns E_TASK_ID_NOT_EXIST when absent.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, UnregisterTaskWithCheck_NoTask_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    EXPECT_EQ(mgr.UnregisterTaskWithCheck(taskInfo), E_TASK_ID_NOT_EXIST);
}

/**
 * @tc.name: UnregisterTaskWithCheck_Success_001
 * @tc.desc: Test UnregisterTaskWithCheck removes a FINISHED task successfully.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, UnregisterTaskWithCheck_Success_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    int32_t uid = GetCallingUid();
    TaskInfo info = MakeTask(uid, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::FINISHED;
    InsertTask(uid, info);

    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    EXPECT_EQ(mgr.UnregisterTaskWithCheck(taskInfo), ERR_OK);
    EXPECT_FALSE(mgr.IsTaskExistLocked(uid));
}

/**
 * @tc.name: FinishTaskWithCheck_NotReady_001
 * @tc.desc: Test FinishTaskWithCheck returns E_SYSTEM_SERVICE_NOT_READY when not ready.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTaskWithCheck_NotReady_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.isReady_.store(false);
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    EXPECT_EQ(mgr.FinishTaskWithCheck(taskInfo), E_SYSTEM_SERVICE_NOT_READY);
}

/**
 * @tc.name: FinishTaskWithCheck_NoTask_001
 * @tc.desc: Test FinishTaskWithCheck returns E_TASK_NOT_EXIST when absent.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTaskWithCheck_NoTask_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    BackgroundLoaderTaskInfo taskInfo(1, "TestAbility");
    EXPECT_EQ(mgr.FinishTaskWithCheck(taskInfo), E_TASK_NOT_EXIST);
}

/**
 * @tc.name: FinishTaskWithCheck_Success_001
 * @tc.desc: Test FinishTaskWithCheck finishes a RUNNING task successfully.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, FinishTaskWithCheck_Success_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    int32_t uid = GetCallingUid();
    TaskInfo info = MakeTask(uid, "com.test.bundle", "TestAbility", 0, 5);
    info.status_ = TaskStatus::RUNNING;
    InsertTask(uid, info);

    BackgroundLoaderTaskInfo taskInfo(5, "TestAbility");
    EXPECT_EQ(mgr.FinishTaskWithCheck(taskInfo), ERR_OK);
    EXPECT_EQ(mgr.taskMap_.at(uid).status_, TaskStatus::FINISHED);
}

/**
 * @tc.name: GetTaskInfoWithCheck_NotReady_001
 * @tc.desc: Test GetTaskInfoWithCheck returns E_SYSTEM_SERVICE_NOT_READY when not ready.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfoWithCheck_NotReady_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.isReady_.store(false);
    BackgroundLoaderTaskInfo taskInfo;
    EXPECT_EQ(mgr.GetTaskInfoWithCheck(1, taskInfo), E_SYSTEM_SERVICE_NOT_READY);
}

/**
 * @tc.name: GetTaskInfoWithCheck_NoTask_001
 * @tc.desc: Test GetTaskInfoWithCheck returns E_TASK_ID_NOT_EXIST when absent.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfoWithCheck_NoTask_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    BackgroundLoaderTaskInfo taskInfo;
    EXPECT_EQ(mgr.GetTaskInfoWithCheck(1, taskInfo), E_TASK_ID_NOT_EXIST);
}

/**
 * @tc.name: GetTaskInfoWithCheck_Success_001
 * @tc.desc: Test GetTaskInfoWithCheck returns the task info for a valid task.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfoWithCheck_Success_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    int32_t uid = GetCallingUid();
    InsertTask(uid, MakeTask(uid, "com.test.bundle", "TestAbility", 0, 5));

    BackgroundLoaderTaskInfo taskInfo;
    EXPECT_EQ(mgr.GetTaskInfoWithCheck(5, taskInfo), ERR_OK);
    EXPECT_EQ(taskInfo.GetAbilityName(), "TestAbility");
    EXPECT_EQ(taskInfo.GetTaskId(), 5);
}

/**
 * @tc.name: HandleAbilityDisonnectDone_001
 * @tc.desc: Test HandleAbilityDisonnectDone removes the cached remote object.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAbilityDisonnectDone_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.SaveRemoteObjectLocked("com.test.bundle", "TestAbility", 0,
        new AbilityConnectCallback("com.test.bundle", "TestAbility", 0, 1001));
    EXPECT_EQ(mgr.abilityMap_.count("com.test.bundle_0"), 1);
    mgr.HandleAbilityDisonnectDone("com.test.bundle", 0);
    EXPECT_EQ(mgr.abilityMap_.count("com.test.bundle_0"), 0);
}

/**
 * @tc.name: RestoreTaskMapFromDb_001
 * @tc.desc: Test RestoreTaskMapFromDb skips rows with an invalid status.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, RestoreTaskMapFromDb_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    std::vector<TaskRow> rows;
    TaskRow row = MakeTaskRow(1001, "com.test.bundle", "TestAbility", 0, 1);
    row.status = 99;
    rows.push_back(row);
    mgr.RestoreTaskMapFromDb(rows);
    EXPECT_TRUE(mgr.taskMap_.empty());
}

/**
 * @tc.name: RestoreTaskMapFromDb_002
 * @tc.desc: Test RestoreTaskMapFromDb restores an IN_BLOCK_LIST row without reporting.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, RestoreTaskMapFromDb_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    std::vector<TaskRow> rows;
    TaskRow row = MakeTaskRow(1001, "com.test.bundle", "TestAbility", 0, 1);
    row.status = static_cast<int32_t>(TaskStatus::IN_BLOCK_LIST);
    rows.push_back(row);
    mgr.RestoreTaskMapFromDb(rows);
    EXPECT_EQ(mgr.taskMap_.count(1001), 1);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::IN_BLOCK_LIST);
}

/**
 * @tc.name: RestoreBlockListFromDb_001
 * @tc.desc: Test RestoreBlockListFromDb merges keys from the database.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, RestoreBlockListFromDb_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    ASSERT_TRUE(mgr.database_.InsertBlockList("com.test.block"));
    mgr.RestoreBlockListFromDb();
    EXPECT_TRUE(mgr.IsInBlockListLocked("com.test.block"));
}

/**
 * @tc.name: CheckAndSendOnStop_001
 * @tc.desc: Test CheckAndSendOnStop returns early when the task is absent.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CheckAndSendOnStop_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.timeoutCount_ = 1;
    InsertTask(1001, info);

    mgr.CheckAndSendOnStop(9999);

    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::RUNNING);
    EXPECT_EQ(mgr.taskMap_.at(1001).timeoutCount_, 1);
}

/**
 * @tc.name: HandleAbilityConnectDone_001
 * @tc.desc: Test HandleAbilityConnectDone does not overwrite the cached object when pid is invalid.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleAbilityConnectDone_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    sptr<IRemoteObject> cached = new AbilityConnectCallback("com.test.bundle", "TestAbility", 0, 1001);
    mgr.SaveRemoteObjectLocked("com.test.bundle", "TestAbility", 0, cached);

    sptr<IRemoteObject> remoteObject = new AbilityConnectCallback("com.test.bundle", "TestAbility", 0, 1001);
    mgr.HandleAbilityConnectDone("com.test.bundle", "TestAbility", 0, 1001, remoteObject);

    EXPECT_EQ(mgr.GetRemoteObjectLocked("com.test.bundle", "TestAbility", 0).GetRefPtr(), cached.GetRefPtr());
}

/**
 * @tc.name: CheckAndSendOnStop_Timeout_001
 * @tc.desc: Test CheckAndSendOnStop increments the timeout count without blocking.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CheckAndSendOnStop_Timeout_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.timeoutCount_ = BACKGROUND_LOADER_TIMEOUT_COUNT - 2;
    InsertTask(1001, info);

    mgr.CheckAndSendOnStop(1001);

    EXPECT_EQ(mgr.taskMap_.at(1001).timeoutCount_, BACKGROUND_LOADER_TIMEOUT_COUNT - 1);
    EXPECT_FALSE(mgr.IsInBlockListLocked("com.test.bundle"));
    EXPECT_EQ(mgr.timeoutTaskMap_.count(1001), 0);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::RUNNING);
    mgr.CancelDelayFinishEvent(1001);
}

/**
 * @tc.name: CheckAndSendOnStop_Timeout_002
 * @tc.desc: Test CheckAndSendOnStop blocks the task when the timeout count reaches the limit.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, CheckAndSendOnStop_Timeout_002, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.timeoutCount_ = BACKGROUND_LOADER_TIMEOUT_COUNT - 1;
    InsertTask(1001, info);

    mgr.CheckAndSendOnStop(1001);

    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::IN_BLOCK_LIST);
    EXPECT_TRUE(mgr.IsInBlockListLocked("com.test.bundle"));
    mgr.CancelDelayFinishEvent(1001);
}

/**
 * @tc.name: HandleProcessDiedEvent_Normal_001
 * @tc.desc: Test HandleProcessDiedEvent handles a normal exit fully.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleProcessDiedEvent_Normal_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    TaskInfo info = MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1);
    info.status_ = TaskStatus::RUNNING;
    info.pid_ = 500;
    info.isBg_ = true;
    info.hasShortTask_ = true;
    info.hasLongTask_ = true;
    info.hasAudioStream_ = true;
    InsertTask(1001, info);
    mgr.SaveRemoteObjectLocked("com.test.bundle", "TestAbility", 0,
        new AbilityConnectCallback("com.test.bundle", "TestAbility", 0, 1001));
    mgr.PostTimeoutTask(1001);
    EXPECT_EQ(mgr.timeoutTaskMap_.count(1001), 1);

    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["bundleName"] = "com.test.bundle";
    payload["pid"] = 500;
    payload["exitReason"] = 0;
    mgr.HandleProcessDiedEvent(static_cast<int64_t>(ResType::ProcessStatus::PROCESS_DIED), payload);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::FINISHED);
    EXPECT_EQ(mgr.taskMap_.at(1001).pid_, -1);
    EXPECT_FALSE(mgr.taskMap_.at(1001).hasShortTask_);
    EXPECT_TRUE(mgr.taskMap_.at(1001).isBg_);
    EXPECT_EQ(mgr.abilityMap_.count("com.test.bundle_0"), 0);
    EXPECT_EQ(mgr.timeoutTaskMap_.count(1001), 0);
}

/**
 * @tc.name: HandleTransientTaskEvent_InvalidValue_001
 * @tc.desc: Test HandleTransientTaskEvent returns early for an invalid value.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleTransientTaskEvent_InvalidValue_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));
    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["bundleName"] = "com.test.bundle";
    mgr.HandleTransientTaskEvent(static_cast<int64_t>(999), payload);

    EXPECT_FALSE(mgr.taskMap_.at(1001).hasShortTask_);
}

/**
 * @tc.name: HandleContinuousTaskEvent_Start_001
 * @tc.desc: Test HandleContinuousTaskEvent marks the long task on start.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, HandleContinuousTaskEvent_Start_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    InsertTask(1001, MakeTask(1001, "com.test.bundle", "TestAbility", 0, 1));

    nlohmann::json payload;
    payload["uid"] = 1001;
    payload["bundleName"] = "com.test.bundle";
    mgr.HandleContinuousTaskEvent(static_cast<int64_t>(ResType::ContinuousTaskStatus::CONTINUOUS_TASK_START), payload);

    EXPECT_TRUE(mgr.taskMap_.at(1001).hasLongTask_);
    EXPECT_EQ(mgr.taskMap_.at(1001).status_, TaskStatus::NOT_STARTED);
}

/**
 * @tc.name: GetTaskInfoWithCheck_BlockList_001
 * @tc.desc: Test GetTaskInfoWithCheck returns E_BACKGROUND_LOADER_PROHIBITED for a block listed bundle.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderMgrTest, GetTaskInfoWithCheck_BlockList_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    int32_t uid = GetCallingUid();
    InsertTask(uid, MakeTask(uid, "com.test.bundle", "TestAbility", 0, 5));
    mgr.AddToBlockListLocked("com.test.bundle");

    BackgroundLoaderTaskInfo taskInfo;
    EXPECT_EQ(mgr.GetTaskInfoWithCheck(5, taskInfo), E_BACKGROUND_LOADER_PROHIBITED);
}
} // namespace WorkScheduler
} // namespace OHOS