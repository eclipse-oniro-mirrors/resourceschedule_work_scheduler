/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "policy/memory_policy.h"

#include <fstream>
#include <securec.h>
#include "work_sched_hilog.h"

using namespace std;

namespace OHOS {
namespace WorkScheduler {
const int32_t BUF_LIMIT = 128;
const int32_t NAME_SIZE = 128;
const int32_t INVALID_MEM = -1;
const int32_t MEM_CRUCIAL = 1 * 1024 * 1024;
const int32_t MEM_LOW = 2 * 1024 * 1024;
const int32_t COUNT_MEMORY_CRUCIAL = 1;
const int32_t COUNT_MEMORY_LOW = 2;
const int32_t COUNT_MEMORY_NORMAL = 3;

MemoryPolicy::MemoryPolicy(shared_ptr<WorkPolicyManager> workPolicyManager)
{
    workPolicyManager_ = workPolicyManager;
}

MemoryPolicy::~MemoryPolicy()
{
}

int32_t MemoryPolicy::GetMemAvailable()
{
    if (workPolicyManager_ != nullptr) {
        int32_t dumpSetMemory = workPolicyManager_->GetDumpSetMemory();
        if (dumpSetMemory != -1) {
            WS_HILOGD("dump set memory:%{public}d", dumpSetMemory);
            return dumpSetMemory;
        }
    }
    int32_t memAvailable = INVALID_MEM;
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == nullptr) {
        WS_HILOGE("GetMemAvailable file open failed.");
        return memAvailable;
    }
    char buf[BUF_LIMIT];
    int32_t buff_len = BUF_LIMIT;
    const char mem_name[] = "MemAvailable";
    char name[NAME_SIZE];
    int32_t value = -1;
    while (fgets(buf, buff_len, fp) != nullptr) {
        int32_t res = sscanf_s(buf, "%s%d", name, sizeof(name), &value);
        if (res < 0) {
            WS_HILOGE("sscanf_s failed");
            break;
        }
        string sname = name;
        if (sname.find(mem_name) != string::npos) {
            memAvailable = value;
            break;
        }
    }
    fclose(fp);
    fp = nullptr;
    return memAvailable;
}

int32_t MemoryPolicy::GetPolicyMaxRunning()
{
    int32_t memAvailable = GetMemAvailable();
    WS_HILOGI("mem_available: %{public}d", memAvailable);
    if (memAvailable <= MEM_CRUCIAL) {
        return COUNT_MEMORY_CRUCIAL;
    }
    if (memAvailable <= MEM_LOW) {
        return COUNT_MEMORY_LOW;
    }
    WS_HILOGI("memory left normal");
    return COUNT_MEMORY_NORMAL;
}
} // namespace WorkScheduler
} // namespace OHOS