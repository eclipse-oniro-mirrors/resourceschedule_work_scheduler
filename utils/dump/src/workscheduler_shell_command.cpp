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

#include "workscheduler_shell_command.h"

#include <iostream>

#include <getopt.h>

#include "event_publisher.h"
#include "iservice_registry.h"
#include "singleton.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
    auto& client_ = WorkSchedulerSrvClient::GetInstance();

    static const struct option OPTIONS[] = {
        {"help", no_argument, nullptr, 'h'},
        {"all", no_argument, nullptr, 'A'},
    };

    static const std::string DUMP_HELP_MSG =
        "usage: workscheduler dump [<options>]\n"
        "options list:\n"
        "  -h                                   help menu\n"
        "  -A All                               dump all infos\n"
        "  -A WorkQueue                         dump work queue infos\n"
        "  -A WorkPolicy                        dump work policy infos\n"
        "  -A DebugInfo                         dump debug info\n"
        "  -A CheckBundle (bool)                set debug of checking bundle, default false\n"
        "  -A SetMemory (number)                set the available memory\n"
        "  -A SetWatchdogTime (number)          set watch dog time, default 120000\n"
        "  -A SetRepeatCycleTimeMin (number)    set min repeat cycle time, default 1200000\n"
        "  -E help                              show publish common event help menu\n";
} // namespace

WorkSchedulerShellCommand::WorkSchedulerShellCommand(int32_t argc, char *argv[])
    : ShellCommand(argc, argv, "workscheduler")
{}

ErrCode WorkSchedulerShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"dump", std::bind(&WorkSchedulerShellCommand::RunAsDumpCommand, this)},
    };
    return ERR_OK;
}

ErrCode WorkSchedulerShellCommand::CreateMessageMap()
{
    messageMap_ = {};
    return ERR_OK;
}

ErrCode WorkSchedulerShellCommand::init()
{
    return ERR_OK;
}

ErrCode WorkSchedulerShellCommand::RunAsDumpCommand()
{
    int32_t ind = 0;
    int32_t option = getopt_long(argc_, argv_, "hAE", OPTIONS, &ind);
    std::vector<std::string> infos;
    switch (option) {
        case 'h':
            resultReceiver_.append(DUMP_HELP_MSG);
            break;
        case 'A':
            if (!client_.ShellDump(argList_, infos)) {
                resultReceiver_.append("ErrNo: " + std::to_string(ERR_INVALID_VALUE) + "\n");
                return ERR_INVALID_VALUE;
            }
            break;
        case 'E':
            EventPublisher eventPublisher;
            eventPublisher.PublishEvent(argList_, infos);
            break;
        default:
            resultReceiver_.append("please add right options.\n");
            resultReceiver_.append(DUMP_HELP_MSG);
            break;
    }
    for (auto info : infos) {
        resultReceiver_.append(info);
        resultReceiver_.append("\n");
    }
    return ERR_OK;
}
} // namespace WorkScheduler
} // namespace OHOS