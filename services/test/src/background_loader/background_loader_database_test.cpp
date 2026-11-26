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
#include "background_loader/background_loader_database.h"
#include "background_loader/background_loader_mgr.h"
#include "work_sched_errors.h"
#include "res_type.h"

using namespace testing::ext;
using namespace OHOS::ResourceSchedule;
namespace OHOS {
namespace WorkScheduler {

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

/**
 * @tc.name: Init_001
 * @tc.desc: Test Init on a fresh database returns true.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderDatabaseTest, Init_001, TestSize.Level1)
{
    EXPECT_TRUE(db_->Init());
}

/**
 * @tc.name: UpsertTask_001
 * @tc.desc: Test UpsertTask inserts a new row and QueryAllTasks reads it back.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderDatabaseTest, UpsertTask_001, TestSize.Level1)
{
    TaskRow row = MakeTaskRow(1001, "com.test.bundle", "TestAbility", 0, 10);
    ASSERT_TRUE(db_->UpsertTask(row));

    std::vector<TaskRow> rows;
    ASSERT_TRUE(db_->QueryAllTasks(rows));
    ASSERT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0].uid, 1001);
    EXPECT_EQ(rows[0].bundleName, "com.test.bundle");
    EXPECT_EQ(rows[0].abilityName, "TestAbility");
    EXPECT_EQ(rows[0].appIndex, 0);
    EXPECT_EQ(rows[0].taskId, 10);
}

/**
 * @tc.name: UpsertTask_002
 * @tc.desc: Test UpsertTask updates an existing row keyed by uid.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderDatabaseTest, UpsertTask_002, TestSize.Level1)
{
    ASSERT_TRUE(db_->UpsertTask(MakeTaskRow(1002, "com.test.bundle", "OldAbility", 0, 1)));

    TaskRow updated = MakeTaskRow(1002, "com.test.bundle", "NewAbility", 1, 2);
    updated.status = 2;
    updated.timeoutCount = 3;
    updated.pid = 456;
    ASSERT_TRUE(db_->UpsertTask(updated));

    std::vector<TaskRow> rows;
    ASSERT_TRUE(db_->QueryAllTasks(rows));
    ASSERT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0].abilityName, "NewAbility");
    EXPECT_EQ(rows[0].appIndex, 1);
    EXPECT_EQ(rows[0].taskId, 2);
    EXPECT_EQ(rows[0].status, 2);
    EXPECT_EQ(rows[0].timeoutCount, 3);
    EXPECT_EQ(rows[0].pid, 456);
}

/**
 * @tc.name: DeleteTask_001
 * @tc.desc: Test DeleteTask removes an existing row.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderDatabaseTest, DeleteTask_001, TestSize.Level1)
{
    ASSERT_TRUE(db_->UpsertTask(MakeTaskRow(1003, "com.test.bundle", "TestAbility", 0, 10)));
    ASSERT_TRUE(db_->DeleteTask(1003));

    std::vector<TaskRow> rows;
    ASSERT_TRUE(db_->QueryAllTasks(rows));
    EXPECT_TRUE(rows.empty());
}

/**
 * @tc.name: DeleteTask_002
 * @tc.desc: Test DeleteTask on a non-existent uid is a no-op success.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderDatabaseTest, DeleteTask_002, TestSize.Level1)
{
    EXPECT_TRUE(db_->DeleteTask(9999));
}

/**
 * @tc.name: QueryAllTasks_Empty_001
 * @tc.desc: Test QueryAllTasks returns true with an empty result set.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderDatabaseTest, QueryAllTasks_Empty_001, TestSize.Level1)
{
    std::vector<TaskRow> rows;
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    EXPECT_TRUE(rows.empty());
}

/**
 * @tc.name: InsertBlockList_001
 * @tc.desc: Test InsertBlockList inserts a key and QueryAllBlockList reads it back.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderDatabaseTest, InsertBlockList_001, TestSize.Level1)
{
    ASSERT_TRUE(db_->InsertBlockList("com.test.block"));

    std::unordered_set<std::string> keys;
    ASSERT_TRUE(db_->QueryAllBlockList(keys));
    EXPECT_EQ(keys.count("com.test.block"), 1);
}

/**
 * @tc.name: InsertBlockList_002
 * @tc.desc: Test InsertBlockList with a duplicate key is ignored without error.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderDatabaseTest, InsertBlockList_002, TestSize.Level1)
{
    ASSERT_TRUE(db_->InsertBlockList("com.test.block"));
    EXPECT_TRUE(db_->InsertBlockList("com.test.block"));

    std::unordered_set<std::string> keys;
    ASSERT_TRUE(db_->QueryAllBlockList(keys));
    EXPECT_EQ(keys.size(), 1);
}

/**
 * @tc.name: DeleteBlockList_001
 * @tc.desc: Test DeleteBlockList removes an existing key.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderDatabaseTest, DeleteBlockList_001, TestSize.Level1)
{
    ASSERT_TRUE(db_->InsertBlockList("com.test.block"));
    ASSERT_TRUE(db_->DeleteBlockList("com.test.block"));

    std::unordered_set<std::string> keys;
    ASSERT_TRUE(db_->QueryAllBlockList(keys));
    EXPECT_EQ(keys.count("com.test.block"), 0);
}

/**
 * @tc.name: QueryAllBlockList_Empty_001
 * @tc.desc: Test QueryAllBlockList returns true with an empty result set.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderDatabaseTest, QueryAllBlockList_Empty_001, TestSize.Level1)
{
    std::unordered_set<std::string> keys;
    EXPECT_TRUE(db_->QueryAllBlockList(keys));
    EXPECT_TRUE(keys.empty());
}

/**
 * @tc.name: NullDb_001
 * @tc.desc: Test all operations on an un-inited database return false.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderDatabaseTest, NullDb_001, TestSize.Level1)
{
    BackgroundLoaderDatabase emptyDb;
    std::vector<TaskRow> rows;
    std::unordered_set<std::string> keys;
    EXPECT_FALSE(emptyDb.UpsertTask(MakeTaskRow(1, "b", "a", 0, 1)));
    EXPECT_FALSE(emptyDb.QueryAllTasks(rows));
    EXPECT_FALSE(emptyDb.DeleteTask(1));
    EXPECT_FALSE(emptyDb.InsertBlockList("b"));
    EXPECT_FALSE(emptyDb.DeleteBlockList("b"));
    EXPECT_FALSE(emptyDb.QueryAllBlockList(keys));
}

/**
 * @tc.name: DeleteBlockList_Nonexistent_001
 * @tc.desc: Test DeleteBlockList on a non-existent key is a no-op success.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderDatabaseTest, DeleteBlockList_Nonexistent_001, TestSize.Level1)
{
    EXPECT_TRUE(db_->DeleteBlockList("com.nobody"));
}

/**
 * @tc.name: UpsertTask_MultiRows_001
 * @tc.desc: Test UpsertTask keeps multiple rows coexisting.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderDatabaseTest, UpsertTask_MultiRows_001, TestSize.Level1)
{
    ASSERT_TRUE(db_->UpsertTask(MakeTaskRow(1004, "com.test.a", "AbilityA", 0, 1)));
    ASSERT_TRUE(db_->UpsertTask(MakeTaskRow(1005, "com.test.b", "AbilityB", 0, 2)));

    std::vector<TaskRow> rows;
    ASSERT_TRUE(db_->QueryAllTasks(rows));
    ASSERT_EQ(rows.size(), 2);
}
} // namespace WorkScheduler
} // namespace OHOS
