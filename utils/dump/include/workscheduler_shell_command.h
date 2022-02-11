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

#ifndef WORK_SCHED_UTILS_WORK_SCHED_DUMP_SHELL_CMD_H
#define WORK_SCHED_UTILS_WORK_SCHED_DUMP_SHELL_CMD_H

#include "shell_command.h"
#include "workscheduler_srv_client.h"

namespace OHOS {
namespace WorkScheduler {
class WorkSchedulerShellCommand : public ShellCommand {
public:
    WorkSchedulerShellCommand(int argc, char *argv[]);

private:
    ErrCode CreateCommandMap() override;
    ErrCode CreateMessageMap() override;
    ErrCode init() override;
    ErrCode RunAsHelpCommand();
    ErrCode RunAsDumpCommand();
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // WORK_SCHED_UTILS_WORK_SCHED_DUMP_SHELL_CMD_H