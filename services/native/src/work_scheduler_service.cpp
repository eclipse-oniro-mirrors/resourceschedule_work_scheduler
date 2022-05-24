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
#include "work_scheduler_service.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <dirent.h>
#include <fcntl.h>
#include <file_ex.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <string_ex.h>
#include <system_ability_definition.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bundle_mgr_proxy.h"
#include "conditions/battery_level_listener.h"
#include "conditions/battery_status_listener.h"
#include "conditions/charger_listener.h"
#include "conditions/network_listener.h"
#include "conditions/storage_listener.h"
#include "conditions/timer_listener.h"
#include "json/json.h"
#include "policy/memory_policy.h"
#include "policy/thermal_policy.h"
#include "work_scheduler_connection.h"
#include "work_sched_common.h"
#include "work_sched_utils.h"

using namespace std;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace WorkScheduler {
namespace {
const std::string WORKSCHEDULER_SERVICE_NAME = "WorkSchedulerService";
auto wss = DelayedSingleton<WorkSchedulerService>::GetInstance().get();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(wss);
const string ALL_INFO = "All";
const string WORK_QUEUE_INFO = "WorkQueue";
const string WORK_POLICY_INFO = "WorkPolicy";
const string SET_MEMORY = "SetMemory";
const string SET_REPEAT_CYCLE_TIME_MIN = "SetRepeatCycleTimeMin";
const string SET_WATCHDOG_TIME = "SetWatchdogTime";
const string CHECK_BUNDLE = "CheckBundle";
const string DEBUG_INFO = "DebugInfo";
const int32_t INIT_DELAY = 2 * 1000;
const int32_t MAX_BUFFER = 256;
const int32_t DUMP_PARAM_INDEX = 1;
const int32_t DUMP_VALUE_INDEX = 2;
}

WorkSchedulerService::WorkSchedulerService() : SystemAbility(WORK_SCHEDULE_SERVICE_ID, true) {}

WorkSchedulerService::~WorkSchedulerService() {}

void WorkSchedulerService::OnStart()
{
    WS_HILOGE("OnStart enter.");
    if (ready_) {
        WS_HILOGE("OnStart is ready, nothing to do.");
        return;
    }

    // Init handler.
    if (!eventRunner_) {
        eventRunner_ = AppExecFwk::EventRunner::Create(WORKSCHEDULER_SERVICE_NAME);
    }
    if (eventRunner_ == nullptr) {
        WS_HILOGE("Init failed due to create EventRunner");
        return;
    }
    handler_ = std::make_shared<WorkEventHandler>(eventRunner_, wss);

    // Try to init.
    Init();
    WS_HILOGE("OnStart success.");
}

bool WorkSchedulerService::IsBaseAbilityReady()
{
    sptr<ISystemAbilityManager> systemAbilityManager
        = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr
        || systemAbilityManager->CheckSystemAbility(APP_MGR_SERVICE_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(COMMON_EVENT_SERVICE_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) == nullptr) {
        return false;
    }
    return true;
}

void WorkSchedulerService::InitPersisted()
{
    WS_HILOGE("WorkSchedulerService::InitPersisted");
    list<shared_ptr<WorkInfo>> persistedWorks = ReadPersistedWorks();
    for (auto it : persistedWorks) {
        WS_HILOGE("get persisted work, id: %{public}d", it->GetWorkId());
        InitPersistedWork(*it);
    }
}

list<shared_ptr<WorkInfo>> WorkSchedulerService::ReadPersistedWorks()
{
    list<shared_ptr<WorkInfo>> workInfos;
    ifstream fin;
    std::string realPath;
    if (!WorkSchedUtils::ConvertFullPath(PERSISTED_FILE_PATH, realPath)) {
        WS_HILOGE("Get real path failed");
        return workInfos;
    }
    WS_HILOGD("Read from %{public}s", realPath.c_str());
    fin.open(realPath, ios::in);
    if (!fin.is_open()) {
        WS_HILOGE("cannot open file %{public}s", realPath.c_str());
        return workInfos;
    }
    char buffer[MAX_BUFFER];
    ostringstream os;
    while (!fin.eof()) {
        fin.getline(buffer, MAX_BUFFER);
        os << buffer;
    }
    string data = os.str();
    JSONCPP_STRING errs;
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    const unique_ptr<Json::CharReader> jsonReader(readerBuilder.newCharReader());
    bool res = jsonReader->parse(data.c_str(), data.c_str() + data.length(), &root, &errs);
    fin.close();
    if (!res || !errs.empty()) {
        return workInfos;
    }
    for (auto it : root.getMemberNames()) {
        Json::Value workJson = root[it];
        shared_ptr<WorkInfo> workInfo = make_shared<WorkInfo>();
        if (workInfo->ParseFromJson(workJson)) {
            workInfos.emplace_back(workInfo);
        }
    }
    return workInfos;
}

void WorkSchedulerService::OnStop()
{
    WS_HILOGI("stop service.");
    eventRunner_.reset();
    handler_.reset();
    ready_ = false;
}

bool WorkSchedulerService::Init()
{
    if (!IsBaseAbilityReady()) {
        WS_HILOGD("request system service is not ready yet!");
        GetHandler()->SendEvent(InnerEvent::Get(WorkEventHandler::SERVICE_INIT_MSG, 0), INIT_DELAY);
        return false;
    }
    WorkQueueManagerInit();
    if (!WorkPolicyManagerInit()) {
        WS_HILOGE("init failed due to work policy manager init.");
        return false;
    }
    InitPersisted();
    if (!Publish(DelayedSingleton<WorkSchedulerService>::GetInstance().get())) {
        WS_HILOGE("OnStart register to system ability manager failed!");
        return false;
    }
    checkBundle_ = true;
    ready_ = true;
    WS_HILOGI("init success.");
    return true;
}

void WorkSchedulerService::WorkQueueManagerInit()
{
    WS_HILOGD("WorkQueueManagerInit come in");
    if (workQueueManager_ == nullptr) {
        workQueueManager_ = make_shared<WorkQueueManager>(wss);
    }

    auto networkListener = make_shared<NetworkListener>(workQueueManager_);
    auto chargerListener = make_shared<ChargerListener>(workQueueManager_);
    auto batteryStatusListener = make_shared<BatteryStatusListener>(workQueueManager_);
    auto batteryLevelListener = make_shared<BatteryLevelListener>(workQueueManager_);
    auto storageListener = make_shared<StorageListener>(workQueueManager_);
    auto timerListener = make_shared<TimerListener>(workQueueManager_);

    workQueueManager_->AddListener(WorkCondition::Type::NETWORK, networkListener);
    workQueueManager_->AddListener(WorkCondition::Type::CHARGER, chargerListener);
    workQueueManager_->AddListener(WorkCondition::Type::BATTERY_STATUS, batteryStatusListener);
    workQueueManager_->AddListener(WorkCondition::Type::BATTERY_LEVEL, batteryLevelListener);
    workQueueManager_->AddListener(WorkCondition::Type::STORAGE, storageListener);
    workQueueManager_->AddListener(WorkCondition::Type::TIMER, timerListener);
}

bool WorkSchedulerService::WorkPolicyManagerInit()
{
    WS_HILOGE("WorkPolicyManagerInit come in");
    if (workPolicyManager_ == nullptr) {
        workPolicyManager_ = make_shared<WorkPolicyManager>(wss);
    }
    if (!workPolicyManager_->Init()) {
        WS_HILOGE("work policy manager init failed!");
        return false;
    }

    auto thermalFilter = make_shared<ThermalPolicy>(workPolicyManager_);
    auto memoryFilter = make_shared<MemoryPolicy>(workPolicyManager_);
    workPolicyManager_->AddPolicyFilter(thermalFilter);
    workPolicyManager_->AddPolicyFilter(memoryFilter);

    auto appRemoveListener = make_shared<AppRemovedListener>(workPolicyManager_);
    workPolicyManager_->AddAppRemoveListener(appRemoveListener);

    auto appDataClearListener = make_shared<AppDataClearListener>(workPolicyManager_);
    workPolicyManager_->AddAppDataClearListener(appDataClearListener);

    WS_HILOGI("work policy manager init success.");
    return true;
}

bool WorkSchedulerService::CheckWorkInfo(WorkInfo &workInfo, int32_t &uid)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        WS_HILOGE("fail to get system ability mgr.");
        return false;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        WS_HILOGE("fail to get bundle manager proxy.");
        return false;
    }
    sptr<IBundleMgr> bundleMgr =  iface_cast<IBundleMgr>(remoteObject);
    BundleInfo bundleInfo;
    int32_t currentAccountId = WorkSchedUtils::GetCurrentAccountId();
    std::string bundleName = workInfo.GetBundleName();
    WS_HILOGD("check work info currentAccountId : %{public}d, bundleName : %{public}s.",
        currentAccountId, bundleName.c_str());
    if (bundleMgr->GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_WITH_ABILITIES,
        bundleInfo, currentAccountId)) {
        WS_HILOGD("bundleUid : %{public}d , uid : %{public}d.", bundleInfo.uid, uid);
        return bundleInfo.uid == uid;
    }
    WS_HILOGD("Get bundle info failed.");
    return false;
}

bool WorkSchedulerService::CheckCondition(WorkInfo& workInfo)
{
    if (workInfo.GetConditionMap()->size() > 1) {
        return true;
    }
    if (workInfo.GetConditionMap()->count(WorkCondition::Type::TIMER) > 0) {
        uint32_t time = workInfo.GetConditionMap()->at(WorkCondition::Type::TIMER)->uintVal;
        if (time < workQueueManager_->GetTimeCycle()) {
            WS_HILOGE("fail, set time:%{public}u must more than %{public}u", time,
                workQueueManager_->GetTimeCycle());
            return false;
        }
    }
    return true;
}

bool WorkSchedulerService::StartWork(WorkInfo& workInfo)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return false;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (checkBundle_ && !CheckWorkInfo(workInfo, uid)) {
        return false;
    }
    if (!CheckCondition(workInfo)) {
        return false;
    }
    WS_HILOGD("WorkSchedulerService::StartWork workInfo %{public}s/%{public}s ID: %{public}d, uid: %{public}d",
        workInfo.GetBundleName().c_str(), workInfo.GetAbilityName().c_str(), workInfo.GetWorkId(), uid);
    shared_ptr<WorkStatus> workStatus = make_shared<WorkStatus>(workInfo, uid);
    bool ret = false;
    if (workPolicyManager_->AddWork(workStatus, uid)) {
        workQueueManager_->AddWork(workStatus);
        if (workInfo.IsPersisted()) {
            std::lock_guard<std::mutex> lock(mutex_);
            std::shared_ptr<WorkInfo> persistedInfo = make_shared<WorkInfo>(workInfo);
            persistedInfo->RefreshUid(uid);
            persistedMap_.emplace(workStatus->workId_, persistedInfo);
            RefreshPersistedWorks();
        }
        ret = true;
    } else {
        WS_HILOGE("WorkPolicyManager->AddWork return false");
    }
    return ret;
}

void WorkSchedulerService::InitPersistedWork(WorkInfo& workInfo)
{
    WS_HILOGD("come in");
    if (workInfo.GetUid() > 0) {
        shared_ptr<WorkStatus> workStatus = make_shared<WorkStatus>(workInfo, workInfo.GetUid());
        if (workPolicyManager_->AddWork(workStatus, workInfo.GetUid())) {
            workQueueManager_->AddWork(workStatus);
        }
    } else {
        WS_HILOGD("uid is invalid : %{public}d", workInfo.GetUid());
    }
    WS_HILOGD("come out");
}

bool WorkSchedulerService::StopWork(WorkInfo& workInfo)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return false;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (checkBundle_ && !CheckWorkInfo(workInfo, uid)) {
        return false;
    }
    shared_ptr<WorkStatus> workStatus = workPolicyManager_->FindWorkStatus(workInfo, uid);
    if (workStatus == nullptr) {
        WS_HILOGD("StopWorkInner, workStatus is nullptr");
        return false;
    }
    return StopWorkInner(workStatus, uid, false, false);
}

bool WorkSchedulerService::StopAndCancelWork(WorkInfo& workInfo)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return false;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (checkBundle_ && !CheckWorkInfo(workInfo, uid)) {
        return false;
    }
    shared_ptr<WorkStatus> workStatus = workPolicyManager_->FindWorkStatus(workInfo, uid);
    if (workStatus == nullptr) {
        WS_HILOGD("StopWorkInner, workStatus is nullptr");
        return false;
    }
    StopWorkInner(workStatus, uid, true, false);
    if (workStatus->persisted_) {
        std::lock_guard<std::mutex> lock(mutex_);
        persistedMap_.erase(workStatus->workId_);
        RefreshPersistedWorks();
    }
    return true;
}

bool WorkSchedulerService::StopWorkInner(std::shared_ptr<WorkStatus> workStatus, int32_t uid,
    const bool needCancel, bool isTimeOut)
{
    if (workPolicyManager_->StopWork(workStatus, uid, needCancel, isTimeOut)) {
        workQueueManager_->CancelWork(workStatus);
    }
    return true;
}

void WorkSchedulerService::WatchdogTimeOut(std::shared_ptr<WorkStatus> workStatus)
{
    StopWorkInner(workStatus, workStatus->uid_, false, true);
}

bool WorkSchedulerService::StopAndClearWorks()
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return false;
    }
    return StopAndClearWorksByUid(IPCSkeleton::GetCallingUid());
}

bool WorkSchedulerService::StopAndClearWorksByUid(int32_t uid)
{
    WS_HILOGD("StopAndClearWorksByUid:%{public}d", uid);
    list<std::shared_ptr<WorkStatus>> allWorks = workPolicyManager_->GetAllWorkStatus(uid);
    list<std::string> workIdList;
    for (auto work : allWorks) {
        workIdList.push_back(work->workId_);
    }
    bool ret = workQueueManager_->StopAndClearWorks(allWorks)
        && workPolicyManager_->StopAndClearWorks(uid);
    if (ret) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto workId : workIdList) {
            if (persistedMap_.count(workId) != 0) {
                persistedMap_.erase(workId);
            }
        }
        RefreshPersistedWorks();
    }
    return ret;
}

bool WorkSchedulerService::IsLastWorkTimeout(int32_t workId)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return false;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    return workPolicyManager_->IsLastWorkTimeout(workId, uid);
}

void WorkSchedulerService::OnConditionReady(shared_ptr<vector<shared_ptr<WorkStatus>>> workStatusVector)
{
    workPolicyManager_->OnConditionReady(workStatusVector);
}

list<shared_ptr<WorkInfo>> WorkSchedulerService::ObtainAllWorks(int32_t &uid, int32_t &pid)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        list<shared_ptr<WorkInfo>> allWorks;
        return allWorks;
    }
    return workPolicyManager_->ObtainAllWorks(uid);
}

shared_ptr<WorkInfo> WorkSchedulerService::GetWorkStatus(int32_t &uid, int32_t &workId)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return nullptr;
    }
    return workPolicyManager_->GetWorkStatus(uid, workId);
}

bool WorkSchedulerService::ShellDump(const vector<string> &dumpOption, vector<string> &dumpInfo)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        dumpInfo.push_back(string("service is not ready."));
        return false;
    }
    if (dumpOption.size() < DUMP_PARAM_INDEX + 1) {
        WS_HILOGI("Dump need at least three params.");
        dumpInfo.push_back(string("dump need at least three params."));
        return false;
    }
    if (dumpOption[DUMP_PARAM_INDEX] == ALL_INFO) {
        DumpAllInfo(dumpInfo);
    } else if (dumpOption[DUMP_PARAM_INDEX] == CHECK_BUNDLE) {
        if (dumpOption.size() < DUMP_VALUE_INDEX + 1) {
            WS_HILOGI("Dump checkbundle need at least three params.");
            dumpInfo.push_back(string("dump checkbundle need at least three params."));
            return false;
        }
        if (dumpOption[DUMP_VALUE_INDEX] == "true") {
            checkBundle_ = true;
        } else if (dumpOption[DUMP_VALUE_INDEX] == "false") {
            checkBundle_ = false;
        }
    } else if (dumpOption[DUMP_PARAM_INDEX] == SET_MEMORY) {
        workPolicyManager_->SetMemoryByDump(std::stoi(dumpOption[DUMP_VALUE_INDEX]));
        return true;
    } else if (dumpOption[DUMP_PARAM_INDEX] == SET_WATCHDOG_TIME) {
        workPolicyManager_->SetWatchdogTime(std::stoi(dumpOption[DUMP_VALUE_INDEX]));
        return true;
    } else if (dumpOption[DUMP_PARAM_INDEX] == SET_REPEAT_CYCLE_TIME_MIN) {
        workQueueManager_->SetTimeCycle(std::stoi(dumpOption[DUMP_VALUE_INDEX]));
        return true;
    } else if (dumpOption[DUMP_PARAM_INDEX] == WORK_QUEUE_INFO) {
        DumpWorkQueueInfo(dumpInfo);
    } else if (dumpOption[DUMP_PARAM_INDEX] == WORK_POLICY_INFO) {
        DumpWorkPolicyInfo(dumpInfo);
    } else if (dumpOption[DUMP_PARAM_INDEX] == DEBUG_INFO) {
        DumpDebugInfo(dumpInfo);
        return true;
    } else {
        WS_HILOGI("Dump need right param.");
        dumpInfo.push_back(string("dump need right param."));
        return false;
    }
    return true;
}

void WorkSchedulerService::DumpWorkQueueInfo(vector<string> &dumpInfo)
{
    string workQueueInfo;
    workQueueInfo.append("================Work Queue Infos================\n");
    if (workQueueManager_ != nullptr) {
        workQueueManager_->Dump(workQueueInfo);
        dumpInfo.push_back(workQueueInfo);
    }
}

void WorkSchedulerService::DumpWorkPolicyInfo(vector<string> &dumpInfo)
{
    string workPolicyInfo;
    workPolicyInfo.append("================Work Policy Infos================\n");
    if (workPolicyManager_ != nullptr) {
        workPolicyManager_->Dump(workPolicyInfo);
        dumpInfo.push_back(workPolicyInfo);
    }
}

void WorkSchedulerService::UpdateWorkBeforeRealStart(std::shared_ptr<WorkStatus> work)
{
    if (work == nullptr) {
        return;
    }
    work->UpdateTimerIfNeed();
    if (work->NeedRemove()) {
        workQueueManager_->RemoveWork(work);
    }
}

void WorkSchedulerService::DumpDebugInfo(std::vector<std::string> &dumpInfo)
{
    dumpInfo.push_back("Need check bundle:" + std::to_string(checkBundle_));
    dumpInfo.push_back("Dump set memory:" + std::to_string(workPolicyManager_->GetDumpSetMemory()));
    dumpInfo.push_back("Repeat cycle time min:" + std::to_string(workQueueManager_->GetTimeCycle()));
    dumpInfo.push_back("Watchdog time:" + std::to_string(workPolicyManager_->GetWatchdogTime()));
}

void WorkSchedulerService::DumpAllInfo(vector<string> &dumpInfo)
{
    DumpWorkQueueInfo(dumpInfo);
    DumpWorkPolicyInfo(dumpInfo);
}

void WorkSchedulerService::RefreshPersistedWorks()
{
    Json::Value root;
    for (auto &it : persistedMap_) {
        auto workInfo = it.second;
        string data = workInfo->ParseToJsonStr();
        JSONCPP_STRING errs;
        Json::Value workJson;
        Json::CharReaderBuilder readerBuilder;
        const unique_ptr<Json::CharReader> jsonReader(readerBuilder.newCharReader());
        bool res = jsonReader->parse(data.c_str(), data.c_str() + data.length(), &workJson, &errs);
        if (res && errs.empty()) {
            root[it.first] = workJson;
        }
    }
    Json::StreamWriterBuilder writerBuilder;
    ostringstream os;
    unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
    jsonWriter->write(root, &os);
    string result = os.str();
    WS_HILOGD("Work JSON os result %{public}s", result.c_str());
    CreateNodeDir(PERSISTED_PATH);
    CreateNodeFile(PERSISTED_FILE_PATH);
    ofstream fout;
    std::string realPath;
    if (!WorkSchedUtils::ConvertFullPath(PERSISTED_FILE_PATH, realPath)) {
        WS_HILOGE("Get real path failed");
        return;
    }
    WS_HILOGD("Refresh path %{public}s", realPath.c_str());
    fout.open(realPath, ios::out);
    fout<<result.c_str()<<endl;
    fout.close();
    WS_HILOGD("come out");
}

int32_t WorkSchedulerService::CreateNodeDir(std::string dir)
{
    WS_HILOGD("Enter");
    if (access(dir.c_str(), 0) != ERR_OK) {
        int32_t flag = mkdir(dir.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
        if (flag == ERR_OK) {
            WS_HILOGD("Create directory successfully.");
        } else {
            WS_HILOGE("Fail to create directory, flag: %{public}d", flag);
            return flag;
        }
    } else {
        WS_HILOGD("This directory already exists.");
    }
    return ERR_OK;
}

int32_t WorkSchedulerService::CreateNodeFile(std::string filePath)
{
    int32_t fd = -1;
    if (access(filePath.c_str(), 0) != 0) {
        fd = open(filePath.c_str(), O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if (fd < ERR_OK) {
            WS_HILOGE("Open file fail.");
            return fd;
        } else {
            WS_HILOGE("Open file success.");
            close(fd);
        }
    } else {
        WS_HILOGE("The file already exists.");
    }
    return ERR_OK;
}
} // namespace WorkScheduler
} // namespace OHOS