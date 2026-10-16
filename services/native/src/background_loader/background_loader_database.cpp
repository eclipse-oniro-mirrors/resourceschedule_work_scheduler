/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "background_loader/background_loader_database.h"

#include <sys/stat.h>
#include "work_sched_hilog.h"
#include "work_sched_utils.h"

namespace OHOS {
namespace WorkScheduler {

namespace {
const char* PERSISTED_PATH = "/data/service/el1/public/WorkScheduler";

const char* SQL_CREATE_TASK_TABLE =
    "CREATE TABLE IF NOT EXISTS BackgroundLoaderTask ("
    "taskKey TEXT PRIMARY KEY NOT NULL, "
    "bundleName TEXT NOT NULL, "
    "abilityName TEXT NOT NULL, "
    "appIndex INTEGER NOT NULL, "
    "taskId INTEGER NOT NULL, "
    "status INTEGER NOT NULL DEFAULT 0, "
    "uid INTEGER NOT NULL DEFAULT 0, "
    "timeoutCount INTEGER NOT NULL DEFAULT 0, "
    "pid INTEGER NOT NULL DEFAULT -1);";

const char* SQL_CREATE_TASK_INDEX =
    "CREATE INDEX IF NOT EXISTS BackgroundLoaderTaskIndex "
    "ON BackgroundLoaderTask (bundleName, appIndex);";

const char* SQL_CREATE_BLOCKLIST_TABLE =
    "CREATE TABLE IF NOT EXISTS BlockList ("
    "taskKey TEXT PRIMARY KEY NOT NULL);";

const char* SQL_DELETE_TASK =
    "DELETE FROM BackgroundLoaderTask WHERE taskKey = ?;";

const char* SQL_UPSERT_TASK =
    "INSERT INTO BackgroundLoaderTask "
    "(taskKey, bundleName, abilityName, appIndex, taskId, status, uid, timeoutCount, pid) "
    "VALUES (?,?,?,?,?,?,?,?,?) "
    "ON CONFLICT(taskKey) DO UPDATE SET "
    "bundleName = excluded.bundleName, abilityName = excluded.abilityName, "
    "appIndex = excluded.appIndex, taskId = excluded.taskId, "
    "status = excluded.status, uid = excluded.uid, "
    "timeoutCount = excluded.timeoutCount, pid = excluded.pid;";

const char* SQL_QUERY_ALL_TASKS =
    "SELECT taskKey, bundleName, abilityName, appIndex, taskId, status, uid, timeoutCount, pid "
    "FROM BackgroundLoaderTask;";

const char* SQL_INSERT_BLOCKLIST =
    "INSERT OR IGNORE INTO BlockList (taskKey) VALUES (?);";

const char* SQL_DELETE_BLOCKLIST =
    "DELETE FROM BlockList WHERE taskKey = ?;";

const char* SQL_QUERY_ALL_BLOCKLIST =
    "SELECT taskKey FROM BlockList;";
}

BackgroundLoaderDatabase::~BackgroundLoaderDatabase()
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool BackgroundLoaderDatabase::Init()
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (!OpenDatabase()) {
        return false;
    }
    if (!CheckAndMigrateDatabase()) {
        WS_HILOGE("check and migrate database failed.");
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }
    if (!CreateTables()) {
        WS_HILOGE("create tables failed.");
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }
    return true;
}

bool BackgroundLoaderDatabase::OpenDatabase()
{
    if (db_ != nullptr) {
        return true;
    }
    if (mkdir(PERSISTED_PATH, S_IRWXU) != 0 && errno != EEXIST) {
        WS_HILOGE("crete db dir failed, errno=%{public}d", errno);
        return false;
    }
    std::string realPath;
    if (!WorkSchedUtils::ConvertFullPath(PERSISTED_PATH, realPath)) {
        WS_HILOGE("get real dir path failed.");
        return false;
    }
    std::string dbPath = realPath + BACKGROUND_LOADER_DB_FILE;
    int32_t ret = sqlite3_open_v2(dbPath.c_str(), &db_,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
    if (ret != SQLITE_OK) {
        const char* msg = db_ != nullptr ? sqlite3_errmsg(db_) : "unknown";
        WS_HILOGE("open db failed, ret=%{public}d, msg=%{public}s", ret, msg);
        if (db_ != nullptr) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
        return false;
    }
    chmod(dbPath.c_str(), S_IRUSR | S_IWUSR);
    WS_HILOGI("open db success, path=%{public}s", dbPath.c_str());
    return true;
}

bool BackgroundLoaderDatabase::CreateTables()
{
    if (db_ == nullptr) {
        return false;
    }
    if (!ExecuteSql(SQL_CREATE_TASK_TABLE)) {
        WS_HILOGE("create task table failed.");
        return false;
    }
    if (!ExecuteSql(SQL_CREATE_TASK_INDEX)) {
        WS_HILOGE("create task index failed.");
        return false;
    }
    if (!ExecuteSql(SQL_CREATE_BLOCKLIST_TABLE)) {
        WS_HILOGE("create blocklist table failed.");
        return false;
    }
    return true;
}

bool BackgroundLoaderDatabase::CheckAndMigrateDatabase()
{
    if (db_ == nullptr) {
        return false;
    }
    sqlite3_stmt* stmt = nullptr;
    int32_t ret = sqlite3_prepare_v2(db_, "PRAGMA user_version;", -1, &stmt, nullptr);
    if (ret != SQLITE_OK) {
        WS_HILOGE("prepare pragma user_version failed, ret=%{public}d", ret);
        return false;
    }
    int32_t currentVersion = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        currentVersion = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (currentVersion < BACKGROUND_LOADER_DB_VERSION) {
        std::string sql = "PRAGMA user_version = " + std::to_string(BACKGROUND_LOADER_DB_VERSION) + ";";
        if (!ExecuteSql(sql)) {
            WS_HILOGE("set user_version failed");
            return false;
        }
        WS_HILOGI("database upgrade from %{public}d to %{public}d", currentVersion, BACKGROUND_LOADER_DB_VERSION);
    }
    return true;
}

bool BackgroundLoaderDatabase::ExecuteSql(const std::string &sql)
{
    if (db_ == nullptr) {
        return false;
    }
    char* errMsg = nullptr;
    int32_t ret = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (ret != SQLITE_OK) {
        WS_HILOGE("execute sql failed, ret=%{public}d, msg=%{public}s", ret, errMsg != nullptr ? errMsg : "null");
        if (errMsg != nullptr) {
            sqlite3_free(errMsg);
        }
        return false;
    }
    if (errMsg != nullptr) {
        sqlite3_free(errMsg);
    }
    return true;
}

bool BackgroundLoaderDatabase::ExecuteSingleTextSql(const char* sql, const std::string& param)
{
    if (db_ == nullptr) {
        return false;
    }
    sqlite3_stmt* stmt = nullptr;
    int32_t ret = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (ret != SQLITE_OK) {
        WS_HILOGE("prepare sql failed, ret=%{public}d", ret);
        return false;
    }
    sqlite3_bind_text(stmt, 1, param.c_str(), -1, SQLITE_TRANSIENT);
    ret = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (ret != SQLITE_DONE) {
        WS_HILOGE("step failed, ret=%{public}d", ret);
        return false;
    }
    return true;
}

bool BackgroundLoaderDatabase::DeleteTask(const std::string &taskKey)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    return ExecuteSingleTextSql(SQL_DELETE_TASK, taskKey);
}

bool BackgroundLoaderDatabase::UpsertTask(const TaskRow& row)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (db_ == nullptr) {
        return false;
    }
    sqlite3_stmt* stmt = nullptr;
    int32_t ret = sqlite3_prepare_v2(db_, SQL_UPSERT_TASK, -1, &stmt, nullptr);
    if (ret != SQLITE_OK) {
        WS_HILOGE("prepare upsert failed, ret=%{public}d", ret);
        return false;
    }
    sqlite3_bind_text(stmt, BIND_TASK_KEY, row.taskKey.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, BIND_BUNDLE_NAME, row.bundleName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, BIND_ABILITY_NAME, row.abilityName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, BIND_APP_INDEX, row.appIndex);
    sqlite3_bind_int(stmt, BIND_TASK_ID, row.taskId);
    sqlite3_bind_int(stmt, BIND_STATUS, row.status);
    sqlite3_bind_int(stmt, BIND_UID, row.uid);
    sqlite3_bind_int(stmt, BIND_TIMEOUT_COUNT, row.timeoutCount);
    sqlite3_bind_int(stmt, BIND_PID, row.pid);
    ret = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (ret != SQLITE_DONE) {
        WS_HILOGE("upsert step failed, ret=%{public}d", ret);
        return false;
    }
    return true;
}

bool BackgroundLoaderDatabase::QueryAllTasks(std::vector<TaskRow> &rows)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (db_ == nullptr) {
        return false;
    }
    rows.clear();
    sqlite3_stmt* stmt = nullptr;
    int32_t ret = sqlite3_prepare_v2(db_, SQL_QUERY_ALL_TASKS, -1, &stmt, nullptr);
    if (ret != SQLITE_OK) {
        WS_HILOGE("prepare query all tasks failed, ret=%{public}d", ret);
        return false;
    }
    int32_t stepRet = sqlite3_step(stmt);
    while (stepRet == SQLITE_ROW) {
        TaskRow row;
        const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, IDX_TASK_KEY));
        row.taskKey = text != nullptr ? text : "";
        text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, IDX_BUNDLE_NAME));
        row.bundleName = text != nullptr ? text : "";
        text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, IDX_ABILITY_NAME));
        row.abilityName = text != nullptr ? text : "";
        row.appIndex = sqlite3_column_int(stmt, IDX_APP_INDEX);
        row.taskId = sqlite3_column_int(stmt, IDX_TASK_ID);
        row.status = sqlite3_column_int(stmt, IDX_STATUS);
        row.uid = sqlite3_column_int(stmt, IDX_UID);
        row.timeoutCount = sqlite3_column_int(stmt, IDX_TIMEOUT_COUNT);
        row.pid = sqlite3_column_int(stmt, IDX_PID);
        rows.push_back(row);
        stepRet = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    if (stepRet != SQLITE_DONE) {
        WS_HILOGE("query all tasks step failed, ret=%{public}d", stepRet);
        return false;
    }
    return true;
}

bool BackgroundLoaderDatabase::InsertBlockList(const std::string &taskKey)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    return ExecuteSingleTextSql(SQL_INSERT_BLOCKLIST, taskKey);
}

bool BackgroundLoaderDatabase::DeleteBlockList(const std::string &taskKey)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    return ExecuteSingleTextSql(SQL_DELETE_BLOCKLIST, taskKey);
}

bool BackgroundLoaderDatabase::QueryAllBlockList(std::unordered_set<std::string> &keys)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    if (db_ == nullptr) {
        return false;
    }
    sqlite3_stmt* stmt = nullptr;
    int32_t ret = sqlite3_prepare_v2(db_, SQL_QUERY_ALL_BLOCKLIST, -1, &stmt, nullptr);
    if (ret != SQLITE_OK) {
        WS_HILOGE("prepare query all blocklist failed, ret=%{public}d", ret);
        return false;
    }
    int32_t stepRet = sqlite3_step(stmt);
    while (stepRet == SQLITE_ROW) {
        const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, IDX_BLOCK_LIST_KEY));
        if (text != nullptr) {
            keys.insert(text);
        }
        stepRet = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    if (stepRet != SQLITE_DONE) {
        WS_HILOGE("query all blocklist step failed, ret=%{public}d", stepRet);
        return false;
    }
    return true;
}

} // WorkScheduler
} // OHOS