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

#ifndef WORK_SCHED_UTILS_DUMP_SHELL_CMD_H
#define WORK_SCHED_UTILS_DUMP_SHELL_CMD_H

#include <functional>
#include <map>
#include <vector>
#include <string>

#include "errors.h"

namespace OHOS {
namespace WorkScheduler {
class ShellCommand {
public:
    ShellCommand(int argc, char *argv[], std::string name);

    ErrCode OnCommand();
    std::string ExecCommand();
    std::string GetCommandErrorMsg() const;

    virtual ErrCode CreateCommandMap() = 0;
    virtual ErrCode CreateMessageMap() = 0;
    virtual ErrCode init() = 0;

protected:
    static constexpr int MIN_ARGUMENT_NUMBER = 2;

    int argc_;
    char **argv_;

    std::string cmd_;
    std::vector<std::string> argList_;

    std::string name_;
    std::map<std::string, std::function<int()>> commandMap_;
    std::map<int32_t, std::string> messageMap_;

    std::string resultReceiver_ = "";
};
}  // namespace WorkScheduler
}  // namespace OHOS

#endif  // WORK_SCHED_UTILS_DUMP_SHELL_CMD_H