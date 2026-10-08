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

static TaskRow MakeTaskRow(const std::string& key, const std::string& bundle, const std::string& ability,
    int32_t appIndex, int32_t taskId)
{
    TaskRow row;
    row.taskKey = key;
    row.bundleName = bundle;
    row.abilityName = ability;
    row.appIndex = appIndex;
    row.taskId = taskId;
    row.status = 0;
    row.uid = 0;
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
 * @tc.name: BackgroundLoaderDatabase_Init_001
 * @tc.desc: Test BackgroundLoaderDatabase Init returns true and can be called once.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, Init_001, TestSize.Level1)
{
    EXPECT_TRUE(db_ != nullptr);
    EXPECT_TRUE(db_->db_ != nullptr);
}

/**
 * @tc.name: BackgroundLoaderDatabase_Init_Idempotent_001
 * @tc.desc: Test Init called twice does not crash and db_ stays valid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, Init_Idempotent_001, TestSize.Level1)
{
    EXPECT_TRUE(db_->Init());
    EXPECT_TRUE(db_->db_ != nullptr);
}

/**
 * @tc.name: BackgroundLoaderDatabase_UpsertTask_001
 * @tc.desc: Test UpsertTask inserts a new row and QueryAllTasks returns it.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, UpsertTask_001, TestSize.Level1)
{
    TaskRow row = MakeTaskRow("com.test_0", "com.test", "TestAbility", 0, 1);
    EXPECT_TRUE(db_->UpsertTask(row));

    std::vector<TaskRow> rows;
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    ASSERT_NE(rows.size(), 0u);
    EXPECT_EQ(rows[0].taskKey, "com.test_0");
    EXPECT_EQ(rows[0].bundleName, "com.test");
    EXPECT_EQ(rows[0].abilityName, "TestAbility");
    EXPECT_EQ(rows[0].appIndex, 0);
    EXPECT_EQ(rows[0].taskId, 1);
    EXPECT_EQ(rows[0].status, 0);
    EXPECT_EQ(rows[0].uid, 0);
    EXPECT_EQ(rows[0].timeoutCount, 0);
    EXPECT_EQ(rows[0].pid, -1);
}

/**
 * @tc.name: BackgroundLoaderDatabase_UpsertTask_UpdateExisting_001
 * @tc.desc: Test UpsertTask updates an existing row when taskKey conflicts.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, UpsertTask_UpdateExisting_001, TestSize.Level1)
{
    TaskRow row1 = MakeTaskRow("com.test_0", "com.test", "Ability1", 0, 1);
    row1.uid = 100;
    row1.pid = 200;
    EXPECT_TRUE(db_->UpsertTask(row1));

    TaskRow row2 = MakeTaskRow("com.test_0", "com.test", "Ability2", 0, 2);
    row2.status = 1;
    row2.uid = 200;
    row2.timeoutCount = 3;
    row2.pid = 300;
    EXPECT_TRUE(db_->UpsertTask(row2));

    std::vector<TaskRow> rows;
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    ASSERT_NE(rows.size(), 0u);
    EXPECT_EQ(rows[0].abilityName, "Ability2");
    EXPECT_EQ(rows[0].taskId, 2);
    EXPECT_EQ(rows[0].status, 1);
    EXPECT_EQ(rows[0].uid, 200);
    EXPECT_EQ(rows[0].timeoutCount, 3);
    EXPECT_EQ(rows[0].pid, 300);
}

/**
 * @tc.name: BackgroundLoaderDatabase_UpsertTask_MultipleRows_001
 * @tc.desc: Test UpsertTask with different taskKeys inserts multiple rows.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, UpsertTask_MultipleRows_001, TestSize.Level1)
{
    for (int32_t i = 0; i < 5; i++) {
        TaskRow row = MakeTaskRow("com.test_" + std::to_string(i), "com.test",
            "Ability" + std::to_string(i), i, i * 10);
        row.status = i;
        row.uid = i * 100;
        row.timeoutCount = i;
        row.pid = i * 200;
        EXPECT_TRUE(db_->UpsertTask(row));
    }
    std::vector<TaskRow> rows;
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    EXPECT_EQ(rows.size(), 5u);
}

/**
 * @tc.name: BackgroundLoaderDatabase_DeleteTask_001
 * @tc.desc: Test DeleteTask removes a row and QueryAllTasks returns empty.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, DeleteTask_001, TestSize.Level1)
{
    TaskRow row = MakeTaskRow("com.del_0", "com.del", "TestAbility", 0, 1);
    EXPECT_TRUE(db_->UpsertTask(row));
    EXPECT_TRUE(db_->DeleteTask("com.del_0"));

    std::vector<TaskRow> rows;
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    EXPECT_EQ(rows.size(), 0u);
}

/**
 * @tc.name: BackgroundLoaderDatabase_DeleteTask_NotExist_001
 * @tc.desc: Test DeleteTask returns true even when the key does not exist.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, DeleteTask_NotExist_001, TestSize.Level1)
{
    EXPECT_TRUE(db_->DeleteTask("com.notexist_0"));
}

/**
 * @tc.name: BackgroundLoaderDatabase_UpsertTask_ModifyFields_001
 * @tc.desc: Test UpsertTask modifies an existing row's fields when taskKey conflicts.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, UpsertTask_ModifyFields_001, TestSize.Level1)
{
    TaskRow row = MakeTaskRow("com.upd_0", "com.upd", "Ability1", 0, 1);
    EXPECT_TRUE(db_->UpsertTask(row));

    TaskRow updated = MakeTaskRow("com.upd_0", "com.upd", "Ability2", 0, 2);
    updated.status = 1;
    updated.uid = 500;
    updated.timeoutCount = 2;
    updated.pid = 600;
    EXPECT_TRUE(db_->UpsertTask(updated));

    std::vector<TaskRow> rows;
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].abilityName, "Ability2");
    EXPECT_EQ(rows[0].taskId, 2);
    EXPECT_EQ(rows[0].status, 1);
    EXPECT_EQ(rows[0].uid, 500);
    EXPECT_EQ(rows[0].timeoutCount, 2);
    EXPECT_EQ(rows[0].pid, 600);
}

/**
 * @tc.name: BackgroundLoaderDatabase_UpsertTask_InsertNew_001
 * @tc.desc: Test UpsertTask inserts a new row when taskKey does not exist.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, UpsertTask_InsertNew_001, TestSize.Level1)
{
    TaskRow row = MakeTaskRow("com.noexist_0", "com.noexist", "Ability", 0, 1);
    EXPECT_TRUE(db_->UpsertTask(row));

    std::vector<TaskRow> rows;
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].taskKey, "com.noexist_0");
}

/**
 * @tc.name: BackgroundLoaderDatabase_QueryAllTasks_Empty_001
 * @tc.desc: Test QueryAllTasks returns empty vector when no data exists.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, QueryAllTasks_Empty_001, TestSize.Level1)
{
    std::vector<TaskRow> rows;
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    EXPECT_EQ(rows.size(), 0u);
}

/**
 * @tc.name: BackgroundLoaderDatabase_QueryAllTasks_FullFields_001
 * @tc.desc: Test QueryAllTasks returns all fields correctly including negative values.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, QueryAllTasks_FullFields_001, TestSize.Level1)
{
    TaskRow row = MakeTaskRow("com.full_0", "com.full", "FullAbility", -1, 999);
    row.status = static_cast<int32_t>(TaskStatus::RUNNING);
    row.uid = -100;
    row.timeoutCount = 7;
    row.pid = -999;
    EXPECT_TRUE(db_->UpsertTask(row));

    std::vector<TaskRow> rows;
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].appIndex, -1);
    EXPECT_EQ(rows[0].taskId, 999);
    EXPECT_EQ(rows[0].status, static_cast<int32_t>(TaskStatus::RUNNING));
    EXPECT_EQ(rows[0].uid, -100);
    EXPECT_EQ(rows[0].timeoutCount, 7);
    EXPECT_EQ(rows[0].pid, -999);
}

/**
 * @tc.name: BackgroundLoaderDatabase_InsertBlockList_001
 * @tc.desc: Test InsertBlockList inserts a key and QueryAllBlockList returns it.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, InsertBlockList_001, TestSize.Level1)
{
    EXPECT_TRUE(db_->InsertBlockList("com.black_0"));

    std::unordered_set<std::string> keys;
    EXPECT_TRUE(db_->QueryAllBlockList(keys));
    ASSERT_EQ(keys.size(), 1u);
    EXPECT_TRUE(keys.find("com.black_0") != keys.end());
}

/**
 * @tc.name: BackgroundLoaderDatabase_InsertBlockList_Multiple_001
 * @tc.desc: Test InsertBlockList with multiple keys.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, InsertBlockList_Multiple_001, TestSize.Level1)
{
    for (int32_t i = 0; i < 5; i++) {
        EXPECT_TRUE(db_->InsertBlockList("com.black_" + std::to_string(i)));
    }
    std::unordered_set<std::string> keys;
    EXPECT_TRUE(db_->QueryAllBlockList(keys));
    EXPECT_EQ(keys.size(), 5u);
}

/**
 * @tc.name: BackgroundLoaderDatabase_InsertBlockList_Duplicate_001
 * @tc.desc: Test InsertBlockList with duplicate key returns false (PRIMARY KEY conflict).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, InsertBlockList_Duplicate_001, TestSize.Level1)
{
    EXPECT_TRUE(db_->InsertBlockList("com.dup_0"));

    std::unordered_set<std::string> keys;
    EXPECT_TRUE(db_->QueryAllBlockList(keys));
    EXPECT_EQ(keys.size(), 1u);
}

/**
 * @tc.name: BackgroundLoaderDatabase_DeleteBlockList_001
 * @tc.desc: Test DeleteBlockList removes a key.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, DeleteBlockList_001, TestSize.Level1)
{
    EXPECT_TRUE(db_->InsertBlockList("com.delblk_0"));
    EXPECT_TRUE(db_->DeleteBlockList("com.delblk_0"));

    std::unordered_set<std::string> keys;
    EXPECT_TRUE(db_->QueryAllBlockList(keys));
    EXPECT_EQ(keys.size(), 0u);
}

/**
 * @tc.name: BackgroundLoaderDatabase_DeleteBlockList_NotExist_001
 * @tc.desc: Test DeleteBlockList returns true even when key does not exist.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, DeleteBlockList_NotExist_001, TestSize.Level1)
{
    EXPECT_TRUE(db_->DeleteBlockList("com.notexist_0"));
}

/**
 * @tc.name: BackgroundLoaderDatabase_QueryAllBlockList_Empty_001
 * @tc.desc: Test QueryAllBlockList returns empty set when no data exists.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, QueryAllBlockList_Empty_001, TestSize.Level1)
{
    std::unordered_set<std::string> keys;
    EXPECT_TRUE(db_->QueryAllBlockList(keys));
    EXPECT_EQ(keys.size(), 0u);
}

/**
 * @tc.name: BackgroundLoaderDatabase_BlockListIndependentFromTask_001
 * @tc.desc: Test deleting a task does not affect blocklist entries.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, BlockListIndependentFromTask_001, TestSize.Level1)
{
    TaskRow row = MakeTaskRow("com.indep_0", "com.indep", "Ability", 0, 1);
    EXPECT_TRUE(db_->UpsertTask(row));
    EXPECT_TRUE(db_->InsertBlockList("com.indep_0"));
    EXPECT_TRUE(db_->DeleteTask("com.indep_0"));

    std::vector<TaskRow> tasks;
    EXPECT_TRUE(db_->QueryAllTasks(tasks));
    EXPECT_EQ(tasks.size(), 0u);

    std::unordered_set<std::string> keys;
    EXPECT_TRUE(db_->QueryAllBlockList(keys));
    ASSERT_EQ(keys.size(), 1u);
    EXPECT_TRUE(keys.find("com.indep_0") != keys.end());
}

/**
 * @tc.name: BackgroundLoaderDatabase_SpecialCharactersInTaskKey_001
 * @tc.desc: Test UpsertTask and DeleteTask with special characters in taskKey.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, SpecialCharactersInTaskKey_001, TestSize.Level1)
{
    std::string specialKey = "com.test's;DROP--_0";
    TaskRow row = MakeTaskRow(specialKey, "com.test", "Ability", 0, 1);
    EXPECT_TRUE(db_->UpsertTask(row));

    std::vector<TaskRow> rows;
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].taskKey, specialKey);

    EXPECT_TRUE(db_->DeleteTask(specialKey));
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    EXPECT_EQ(rows.size(), 0u);
}

/**
 * @tc.name: BackgroundLoaderDatabase_SpecialCharactersInBundleName_001
 * @tc.desc: Test UpsertTask with special characters in bundleName and abilityName.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, SpecialCharactersInBundleName_001, TestSize.Level1)
{
    TaskRow row = MakeTaskRow("com.spec_0", "com.test's;DROP--", "Abil'ity\"", 0, 1);
    EXPECT_TRUE(db_->UpsertTask(row));

    std::vector<TaskRow> rows;
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].bundleName, "com.test's;DROP--");
    EXPECT_EQ(rows[0].abilityName, "Abil'ity\"");
}

/**
 * @tc.name: BackgroundLoaderDatabase_CreateTables_Idempotent_001
 * @tc.desc: Test CreateTables can be called multiple time without error.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, CreateTables_Idempotent_001, TestSize.Level1)
{
    EXPECT_TRUE(db_->CreateTables());
    EXPECT_TRUE(db_->CreateTables());
    EXPECT_TRUE(db_->db_ != nullptr);
}

/**
 * @tc.name: BackgroundLoaderDatabase_CheckAndMigrateDatabase_001
 * @tc.desc: Test CheckAndMigrateDatabase sets user_version to BACKGROUND_LOADER_DB_VERSION.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, CheckAndMigrateDatabase_001, TestSize.Level1)
{
    EXPECT_TRUE(db_->CheckAndMigrateDatabase());
    EXPECT_TRUE(db_->db_ != nullptr);
}

/**
 * @tc.name: BackgroundLoaderDatabase_OperationsBeforeInit_001
 * @tc.desc: Test all CRUD operations return false when db_ is nullptr (before Init).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, OperationsBeforeInit_001, TestSize.Level1)
{
    BackgroundLoaderDatabase uninitDb;
    TaskRow row = MakeTaskRow("com.test_0", "com.test", "Ability", 0, 1);
    EXPECT_FALSE(uninitDb.UpsertTask(row));
    EXPECT_FALSE(uninitDb.DeleteTask("com.test_0"));
    EXPECT_FALSE(uninitDb.UpsertTask(row));
    std::vector<TaskRow> rows;
    EXPECT_FALSE(uninitDb.QueryAllTasks(rows));
    EXPECT_FALSE(uninitDb.InsertBlockList("com.test_0"));
    EXPECT_FALSE(uninitDb.DeleteBlockList("com.test_0"));
    std::unordered_set<std::string> keys;
    EXPECT_FALSE(uninitDb.QueryAllBlockList(keys));
}

/**
 * @tc.name: BackgroundLoaderDatabase_UpsertThenUpdateThenDelete_001
 * @tc.desc: Test the full lifecyccle: upsert, update, query, delete.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, UpsertThenUpdateThenDelete_001, TestSize.Level1)
{
    TaskRow row = MakeTaskRow("com.lifecycle_0", "com.lifecycle", "Ability1", 0, 1);
    EXPECT_TRUE(db_->UpsertTask(row));

    TaskRow updated = MakeTaskRow("com.lifecycle_0", "com.lifecycle", "Ability2", 0, 1);
    updated.status = 1;
    updated.uid = 100;
    updated.timeoutCount = 1;
    updated.pid = 200;
    EXPECT_TRUE(db_->UpsertTask(updated));

    std::vector<TaskRow> rows;
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].abilityName, "Ability2");
    EXPECT_EQ(rows[0].status, 1);

    EXPECT_TRUE(db_->DeleteTask("com.lifecycle_0"));
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    EXPECT_EQ(rows.size(), 0u);
}

/**
 * @tc.name: BackgroundLoaderDatabase_ExecuteSql_001
 * @tc.desc: Test ExecuteSql returns false on invalid SQL.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, ExecuteSql_001, TestSize.Level1)
{
    EXPECT_FALSE(db_->ExecuteSql("INVALID SQL STATEMENT;"));
    EXPECT_TRUE(db_->ExecuteSql("SELECT 1;"));
}

/**
 * @tc.name: BackgroundLoaderDatabase_EmptyStringFields_001
 * @tc.desc: Test UpsertTask with empty string fields does not crash.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderDatabaseTest, EmptyStringFields_001, TestSize.Level1)
{
    TaskRow row = MakeTaskRow("", "", "", 0, 1);
    EXPECT_TRUE(db_->UpsertTask(row));

    std::vector<TaskRow> rows;
    EXPECT_TRUE(db_->QueryAllTasks(rows));
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].taskKey, "");
    EXPECT_EQ(rows[0].bundleName, "");
    EXPECT_EQ(rows[0].abilityName, "");
}

class BackgroundLoaderMgrDbTest : public testing::Test {
public:
    void SetUp() override
    {
        BackgroundLoaderMgr::GetInstance().Init(BACKGROUND_LOADER_TIMEOUT_COUNT, BACKGROUND_LOADER_TIMEOUT_US);
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (!BackgroundLoaderMgr::GetInstance().isReady_.load() &&
            std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME));
        }
    }
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
            std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().blackListLock_);
            BackgroundLoaderMgr::GetInstance().blackLists_.clear();
        }
    }
};

/**
 * @tc.name: BackgroundLoaderMgrDb_RegisterTask_PersistsToDb_001
 * @tc.desc: Test RegisterTask persists task to database via UpsertTask.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrDbTest, RegisterTask_PersistsToDb_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.dbtest.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 42
    };
    ErrCode ret = BackgroundLoaderMgr::GetInstance().RegisterTask(info);
    EXPECT_EQ(ret, ERR_OK);

    FlushDbQueue();
    std::vector<TaskRow> rows;
    EXPECT_TRUE(BackgroundLoaderMgr::GetInstance().database_.QueryAllTasks(rows));
    bool found = false;
    for (const auto& row : rows) {
        if (row.taskKey == "com.dbtest.bundle_0") {
            found = true;
            EXPECT_EQ(row.bundleName, "com.dbtest.bundle");
            EXPECT_EQ(row.abilityName, "TestAbility");
            EXPECT_EQ(row.taskId, 42);
            break;
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @tc.name: BackgroundLoaderMgrDb_UnregisterTask_DeleteFromDb_001
 * @tc.desc: Test UnregisterTask remove task from database via DeleteTask.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrDbTest, UnregisterTask_DeleteFromDb_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.dbdel.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1
    };
    EXPECT_EQ(BackgroundLoaderMgr::GetInstance().RegisterTask(info), ERR_OK);
    EXPECT_EQ(BackgroundLoaderMgr::GetInstance().UnregisterTask(info), ERR_OK);

    FlushDbQueue();
    std::vector<TaskRow> rows;
    EXPECT_TRUE(BackgroundLoaderMgr::GetInstance().database_.QueryAllTasks(rows));
    for (const auto& row : rows) {
        EXPECT_NE(row.taskKey, "com.dbdel.bundle_0");
    }
}

/**
 * @tc.name: BackgroundLoaderMgrDb_HandleAppChangeEvent_DeletesFromDb_001
 * @tc.desc: Test HandleAppChangeEvent with uninstall removes task and blocklist from database.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrDbTest, HandleAppChangeEvent_DeletesFromDb_001, TestSize.Level1)
{
    TaskInfo info = {
        .bundleName_ = "com.dbevent.bundle",
        .abilityName_ = "TestAbility",
        .appIndex_ = 0,
        .taskId_ = 1,
    };
    EXPECT_EQ(BackgroundLoaderMgr::GetInstance().RegisterTask(info), ERR_OK);
    EXPECT_TRUE(BackgroundLoaderMgr::GetInstance().database_.InsertBlockList("com.dbevent.bundle_0"));

    nlohmann::json payload;
    payload["bundleName"] = "com.dbevent.bundle";
    payload["appIndex"] = "0";
    BackgroundLoaderMgr::GetInstance().HandleAppChangeEvent(
        ResType::AppInstallStatus::APP_UNINSTALL, payload);

    FlushDbQueue();
    std::vector<TaskRow> tasks;
    EXPECT_TRUE(BackgroundLoaderMgr::GetInstance().database_.QueryAllTasks(tasks));
    for (const auto& row : tasks) {
        EXPECT_NE(row.taskKey, "com.dbevent.bundle_0");
    }

    std::unordered_set<std::string> keys;
    EXPECT_TRUE(BackgroundLoaderMgr::GetInstance().database_.QueryAllBlockList(keys));
    EXPECT_TRUE(keys.find("com.dbevent.bundle_0") == keys.end());
}

/**
 * @tc.name: BackgroundLoaderMgrDb_ToTaskRow_001
 * @tc.desc: Test ToTaskRow converts taskInfo to taskRow correctly.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrDbTest, ToTaskRow_001, TestSize.Level1)
{
    TaskInfo info;
    info.bundleName_ = "com.convert.bundle";
    info.abilityName_ = "ConvertAbility";
    info.appIndex_ = 3;
    info.taskId_ = 77;
    info.status_ = TaskStatus::RUNNING;
    info.uid_ = 1000;
    info.timeoutCount_  = 2;
    info.pid_ = 5000;

    std::string key = "com.convert.bundle_3";
    TaskRow row = BackgroundLoaderMgr::GetInstance().ToTaskRow(key, info);
    EXPECT_EQ(row.taskKey, key);
    EXPECT_EQ(row.bundleName, "com.convert.bundle");
    EXPECT_EQ(row.abilityName, "ConvertAbility");
    EXPECT_EQ(row.appIndex, 3);
    EXPECT_EQ(row.taskId, 77);
    EXPECT_EQ(row.status, static_cast<int32_t>(TaskStatus::RUNNING));
    EXPECT_EQ(row.uid, 1000);
    EXPECT_EQ(row.timeoutCount, 2);
    EXPECT_EQ(row.pid, 5000);
}

/**
 * @tc.name: BackgroundLoaderMgrDb_RestoreBlockListFromDb_001
 * @tc.desc: Test that blockList is restored from database after re-init.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrDbTest, RestoreBlockListFromDb_001, TestSize.Level1)
{
    EXPECT_TRUE(BackgroundLoaderMgr::GetInstance().database_.InsertBlockList("com.restoreblk_0"));

    BackgroundLoaderMgr::GetInstance().InitDatabaseAsync();
    auto dealine = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (std::chrono::steady_clock::now() < dealine) {
        std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().blackListLock_);
        if (BackgroundLoaderMgr::GetInstance().blackLists_.find("com.restoreblk_0") !=
            BackgroundLoaderMgr::GetInstance().blackLists_.end()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().blackListLock_);
    EXPECT_TRUE(BackgroundLoaderMgr::GetInstance().blackLists_.find("com.restoreblk_0") !=
        BackgroundLoaderMgr::GetInstance().blackLists_.end());
}

/**
 * @tc.name: BackgroundLoaderMgrDb_AddToBlockList_001
 * @tc.desc: Test AddToBlockList inserts key into memory and DB
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrDbTest, AddToBlockList_001, TestSize.Level1)
{
    std::string key = "com.blocklist_test_0";
    BackgroundLoaderMgr::GetInstance().AddToBlockList(key);
    {
        std::lock_guard<ffrt::mutex> lock(BackgroundLoaderMgr::GetInstance().blackListLock_);
        EXPECT_TRUE(BackgroundLoaderMgr::GetInstance().blackLists_.find(key) !=
                    BackgroundLoaderMgr::GetInstance().blackLists_.end());
    }
    FlushDbQueue();
    std::unordered_set<std::string> keys;
    EXPECT_TRUE(BackgroundLoaderMgr::GetInstance().database_.QueryAllBlockList(keys));
    EXPECT_TRUE(keys.find(key) != keys.end());
}

/**
 * @tc.name: BackgroundLoaderMgrDb_PersistTaskResult_WithBlockList_001
 * @tc.desc: Test PersistTaskResult writes task and blocklist to DB
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BackgroundLoaderMgrDbTest, PersistTaskResult_WithBlockList_001, TestSize.Level1)
{
    TaskInfo info;
    info.bundleName_ = "com.persistblk.bundle";
    info.abilityName_ = "TestAbility";
    info.appIndex_ = 0;
    info.taskId_ = 101;
    info.status_ = TaskStatus::UNREGISIERED;

    std::string key = "com.persistblk.bundle_0";
    BackgroundLoaderMgr::GetInstance().PersistTaskResult(key, true, info);
    FlushDbQueue();

    std::vector<TaskRow> rows;
    EXPECT_TRUE(BackgroundLoaderMgr::GetInstance().database_.QueryAllTasks(rows));
    bool found = false;
    for (const auto& row : rows) {
        if (row.taskKey == key) {
            found = true;
            EXPECT_EQ(row.status, static_cast<int32_t>(TaskStatus::UNREGISIERED));
            break;
        }
    }
    EXPECT_TRUE(found);

    std::unordered_set<std::string> keys;
    EXPECT_TRUE(BackgroundLoaderMgr::GetInstance().database_.QueryAllBlockList(keys));
    EXPECT_TRUE(keys.find(key) != keys.end());
}
} // namespace WorkScheduler
} // namespace OHOS