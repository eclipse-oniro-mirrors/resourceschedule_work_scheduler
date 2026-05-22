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

#include "workschedulerdump_fuzzer.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "work_scheduler_service.h"
#include "work_queue_manager.h"
#include "work_policy_manager.h"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr size_t MAX_ARG_COUNT = 8;
constexpr size_t MAX_ARG_LEN = 64;
std::shared_ptr<WorkSchedulerService> g_workSchedulerService;

bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
{
    return true;
}

std::vector<std::string> BuildFuzzArgs(const uint8_t* data, size_t size)
{
    std::vector<std::string> args;
    if (data == nullptr || size == 0) {
        args.push_back("-h");
        return args;
    }
    size_t argCount = std::min(static_cast<size_t>(data[0] % MAX_ARG_COUNT) + 1, MAX_ARG_COUNT);
    size_t offset = 1;
    for (size_t i = 0; i < argCount; ++i) {
        if (offset >= size) {
            break;
        }
        size_t argLen = std::min(static_cast<size_t>(data[offset] % MAX_ARG_LEN) + 1, MAX_ARG_LEN);
        ++offset;
        if (offset + argLen > size) {
            argLen = size - offset;
        }
        if (argLen == 0) {
            args.emplace_back("fuzz");
            continue;
        }
        args.emplace_back(reinterpret_cast<const char*>(data + offset), argLen);
        offset += argLen;
    }
    if (args.empty()) {
        args.push_back("-h");
    }
    return args;
}

void InitService()
{
    g_workSchedulerService = DelayedSingleton<WorkSchedulerService>::GetInstance();
    g_workSchedulerService->OnStart();
    g_workSchedulerService->InitBgTaskSubscriber();
    if (!g_workSchedulerService->ready_) {
        g_workSchedulerService->ready_ = true;
    }
    if (g_workSchedulerService->workQueueManager_ == nullptr) {
        g_workSchedulerService->workQueueManager_ =
            std::make_shared<WorkQueueManager>(g_workSchedulerService);
    }
    if (g_workSchedulerService->workPolicyManager_ == nullptr) {
        g_workSchedulerService->workPolicyManager_ =
            std::make_shared<WorkPolicyManager>(g_workSchedulerService);
        g_workSchedulerService->workPolicyManager_->Init(g_workSchedulerService_->eventRunner_);
    }
    if (g_workSchedulerService->checkBundle_) {
        g_workSchedulerService->checkBundle_ = false;
    }
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    InitService();
    std::vector<std::string> argsInStr = BuildFuzzArgs(data, size);
    std::string result;
    g_workSchedulerService->DumpProcessForEngMode(argsInStr, result);
    g_workSchedulerService->DumpProcessForUserMode(argsInStr, result);
    g_workSchedulerService->OnStop();
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
