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

#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_DATABASE_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_DATABASE_H

#include <string>
#include <unordered_set>
#include <vector>

#include "sqlite3sym.h"
#include "ffrt.h"

namespace OHOS {
namespace WorkScheduler {

inline constexpr int32_t BACKGROUND_LOADER_DB_VERSION = 1;
inline const std::string BACKGROUND_LOADER_DB_FILE = "/background_loader.db";
inline const std::string BACKGROUND_LOADER_TABLE = "BackgroundLoaderTask";
inline const std::string BACKGROUND_LOADER_TABLE_INDEX = "BackgroundLoaderTaskIndex";
inline const std::string BLOCK_LIST_TABLE = "BlockList";

// Table columns
inline const std::string COL_TASK_KEY = "taskKey";
inline const std::string COL_BUNDLE_NAME = "bundleName";
inline const std::string COL_ABILITY_NAME = "abilityName";
inline const std::string COL_APP_INDEX = "appIndex";
inline const std::string COL_TASK_ID = "taskId";
inline const std::string COL_STATUS = "status";
inline const std::string COL_UID = "uid";
inline const std::string COL_TIMEOUT_COUNT = "timeoutCount";
inline const std::string COL_PID = "pid";

// Result column indices for sqlite3_column_* (0-based, matches SELECT column order)
enum : int32_t {
    IDX_TASK_KEY = 0,
    IDX_BUNDLE_NAME,
    IDX_ABILITY_NAME,
    IDX_APP_INDEX,
    IDX_TASK_ID,
    IDX_STATUS,
    IDX_UID,
    IDX_TIMEOUT_COUNT,
    IDX_PID
};

enum : int32_t {
    IDX_BLOCK_LIST_KEY = 0
};

// Bind parameter indices for sqlite3_bind_* (1-based, matches INSERT column order)
enum : int32_t {
    BIND_TASK_KEY = 1,
    BIND_BUNDLE_NAME,
    BIND_ABILITY_NAME,
    BIND_APP_INDEX,
    BIND_TASK_ID,
    BIND_STATUS,
    BIND_UID,
    BIND_TIMEOUT_COUNT,
    BIND_PID
};

struct TaskRow {
    std::string taskKey;
    std::string bundleName;
    std::string abilityName;
    int32_t appIndex {-1};
    int32_t taskId {0};
    int32_t status {0};
    int32_t uid {0};
    int32_t timeoutCount {0};
    int32_t pid {-1};
};

class BackgroundLoaderDatabase {
public:
    BackgroundLoaderDatabase() = default;
    ~BackgroundLoaderDatabase();

    bool Init();

    bool DeleteTask(const std::string &taskKey);
    bool UpsertTask(const TaskRow &row);
    bool QueryAllTasks(std::vector<TaskRow> &rows);

    bool InsertBlockList(const std::string &taskKey);
    bool DeleteBlockList(const std::string &taskKey);
    bool QueryAllBlockList(std::unordered_set<std::string> &keys);

private:
    bool OpenDatabase();
    bool CreateTables();
    bool CheckAndMigrateDatabase();
    bool ExecuteSql(const std::string &sql);
    bool ExecuteSingleTextSql(const char* sql, const std::string& param);

    sqlite3* db_ {nullptr};
    ffrt::mutex mutex_;
};
} // WorkScheduler
} // OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_BACKGROUND_LOADER_DATABASE_H