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
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
#include "bundle_active_client.h"
#endif
#include "conditions/battery_level_listener.h"
#include "conditions/battery_status_listener.h"
#include "conditions/charger_listener.h"
#include "conditions/network_listener.h"
#include "conditions/storage_listener.h"
#include "conditions/timer_listener.h"
#include "conditions/group_listener.h"
#include "event_publisher.h"
#include "json/json.h"
#include "policy/app_data_clear_listener.h"
#include "policy/app_removed_listener.h"
#include "policy/memory_policy.h"
#include "policy/thermal_policy.h"
#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
#include "scheduler_bg_task_subscriber.h"
#include "background_task_mgr_helper.h"
#include "resource_type.h"
#endif
#include "work_scheduler_connection.h"
#include "work_bundle_group_change_callback.h"
#include "work_sched_errors.h"
#include "work_sched_hilog.h"
#include "work_sched_utils.h"

using namespace std;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace WorkScheduler {
namespace {
const std::string WORKSCHEDULER_SERVICE_NAME = "WorkSchedulerService";
auto instance = DelayedSpSingleton<WorkSchedulerService>::GetInstance();
auto wss = instance.GetRefPtr();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(wss);
const int32_t INIT_DELAY = 2 * 1000;
const int32_t MAX_BUFFER = 2048;
const int32_t DUMP_OPTION = 0;
const int32_t DUMP_PARAM_INDEX = 1;
const int32_t DUMP_VALUE_INDEX = 2;
const char* g_persistedFilePath = "/data/service/el1/public/WorkScheduler/persisted_work";
const char* g_persistedPath = "/data/service/el1/public/WorkScheduler";
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
static int g_hasGroupObserver = -1;
#endif
}

WorkSchedulerService::WorkSchedulerService() : SystemAbility(WORK_SCHEDULE_SERVICE_ID, true) {}

WorkSchedulerService::~WorkSchedulerService() {}

void WorkSchedulerService::OnStart()
{
    if (ready_) {
        WS_HILOGI("OnStart is ready, nothing to do.");
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
    WS_HILOGD("On start success.");
}

bool WorkSchedulerService::IsBaseAbilityReady()
{
    sptr<ISystemAbilityManager> systemAbilityManager
        = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr
        || systemAbilityManager->CheckSystemAbility(APP_MGR_SERVICE_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(COMMON_EVENT_SERVICE_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) == nullptr
        || systemAbilityManager->CheckSystemAbility(BACKGROUND_TASK_MANAGER_SERVICE_ID) == nullptr) {
        return false;
    }
    return true;
}

void WorkSchedulerService::InitPersisted()
{
    WS_HILOGD("init persisted work");
    list<shared_ptr<WorkInfo>> persistedWorks = ReadPersistedWorks();
    for (auto it : persistedWorks) {
        WS_HILOGD("get persisted work, id: %{public}d", it->GetWorkId());
        InitPersistedWork(*it);
    }
}

list<shared_ptr<WorkInfo>> WorkSchedulerService::ReadPersistedWorks()
{
    list<shared_ptr<WorkInfo>> workInfos;
    ifstream fin;
    std::string realPath;
    if (!WorkSchedUtils::ConvertFullPath(g_persistedFilePath, realPath)) {
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
    while (fin.getline(buffer, MAX_BUFFER)) {
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
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    DeviceUsageStats::BundleActiveClient::GetInstance().UnRegisterAppGroupCallBack(groupObserver_);
    groupObserver_ = nullptr;
    g_hasGroupObserver = -1;
#endif
#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
    ErrCode ret = BackgroundTaskMgr::BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(*subscriber_);
    if (ret != ERR_OK) {
        WS_HILOGE("unscribe bgtask failed.");
    }
    subscriber_.reset();
#endif
    eventRunner_.reset();
    handler_.reset();
    ready_ = false;
}

bool WorkSchedulerService::Init()
{
    if (!IsBaseAbilityReady()) {
        WS_HILOGE("request system service is not ready yet!");
        GetHandler()->SendEvent(InnerEvent::Get(WorkEventHandler::SERVICE_INIT_MSG, 0), INIT_DELAY);
        return false;
    }
    WorkQueueManagerInit();
    if (!WorkPolicyManagerInit()) {
        WS_HILOGE("init failed due to work policy manager init.");
        return false;
    }
    InitPersisted();
    if (!Publish(wss)) {
        WS_HILOGE("OnStart register to system ability manager failed!");
        return false;
    }
    WS_HILOGI("start init background task subscriber!");
    if (!InitBgTaskSubscriber()) {
        WS_HILOGE("subscribe background task failed!");
        return false;
    }
    checkBundle_ = true;
    ready_ = true;
    WS_HILOGI("init success.");
    return true;
}

bool WorkSchedulerService::InitBgTaskSubscriber()
{
#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
    subscriber_ = make_shared<SchedulerBgTaskSubscriber>();
    ErrCode ret = BackgroundTaskMgr::BackgroundTaskMgrHelper::SubscribeBackgroundTask(*subscriber_);
    if (ret != ERR_OK) {
        WS_HILOGE("SubscribeBackgroundTask failed.");
        return false;
    }
    this->QueryResAppliedUid();
    WS_HILOGD("subscribe background TASK success!");
#endif
    return true;
}

ErrCode WorkSchedulerService::QueryResAppliedUid()
{
#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
    std::vector<std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo>> appList;
    std::vector<std::shared_ptr<BackgroundTaskMgr::ResourceCallbackInfo>> procList;
    ErrCode result = BackgroundTaskMgr::BackgroundTaskMgrHelper::GetEfficiencyResourcesInfos(appList, procList);
    if (result != ERR_OK) {
        WS_HILOGE("failed to GetEfficiencyResourcesInfos, errcode: %{public}d", result);
        return result;
    }
    std::lock_guard<std::mutex> lock(whitelistMutex_);
    for (const auto& info : appList) {
        if ((info->GetResourceNumber() & BackgroundTaskMgr::ResourceType::WORK_SCHEDULER) != 0) {
            whitelist_.emplace(info->GetUid());
        }
    }
    for (const auto& info : procList) {
        if ((info->GetResourceNumber() & BackgroundTaskMgr::ResourceType::WORK_SCHEDULER) != 0) {
            whitelist_.emplace(info->GetUid());
        }
    }
    WS_HILOGI("get efficiency resources infos succeed.");
#endif
    return ERR_OK;
}

void WorkSchedulerService::WorkQueueManagerInit()
{
    WS_HILOGD("come in");
    if (workQueueManager_ == nullptr) {
        workQueueManager_ = make_shared<WorkQueueManager>(wss);
    }

    auto networkListener = make_shared<NetworkListener>(workQueueManager_);
    auto chargerListener = make_shared<ChargerListener>(workQueueManager_);
    auto batteryStatusListener = make_shared<BatteryStatusListener>(workQueueManager_);
    auto batteryLevelListener = make_shared<BatteryLevelListener>(workQueueManager_);
    auto storageListener = make_shared<StorageListener>(workQueueManager_);
    auto timerListener = make_shared<TimerListener>(workQueueManager_);
    auto groupListener = make_shared<GroupListener>(workQueueManager_);

    workQueueManager_->AddListener(WorkCondition::Type::NETWORK, networkListener);
    workQueueManager_->AddListener(WorkCondition::Type::CHARGER, chargerListener);
    workQueueManager_->AddListener(WorkCondition::Type::BATTERY_STATUS, batteryStatusListener);
    workQueueManager_->AddListener(WorkCondition::Type::BATTERY_LEVEL, batteryLevelListener);
    workQueueManager_->AddListener(WorkCondition::Type::STORAGE, storageListener);
    workQueueManager_->AddListener(WorkCondition::Type::TIMER, timerListener);
    workQueueManager_->AddListener(WorkCondition::Type::GROUP, groupListener);

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    GroupObserverInit();
#endif
}

bool WorkSchedulerService::WorkPolicyManagerInit()
{
    WS_HILOGD("come in");
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
    WS_HILOGE("Get bundle info failed.");
    return false;
}

bool WorkSchedulerService::CheckCondition(WorkInfo& workInfo)
{
    if (workInfo.GetConditionMap()->size() < 1) {
        return false;
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

int32_t WorkSchedulerService::StartWork(WorkInfo& workInfo)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (checkBundle_ && !CheckWorkInfo(workInfo, uid)) {
        return E_CHECK_WORKINFO_FAILED;
    }
    if (!CheckCondition(workInfo)) {
        return E_REPEAT_CYCLE_TIME_ERR;
    }
    WS_HILOGD("workInfo %{public}s/%{public}s ID: %{public}d, uid: %{public}d",
        workInfo.GetBundleName().c_str(), workInfo.GetAbilityName().c_str(), workInfo.GetWorkId(), uid);
    shared_ptr<WorkStatus> workStatus = make_shared<WorkStatus>(workInfo, uid);
    int32_t ret = workPolicyManager_->AddWork(workStatus, uid);
    if (ret == ERR_OK) {
        workQueueManager_->AddWork(workStatus);
        if (workInfo.IsPersisted()) {
            std::lock_guard<std::mutex> lock(mutex_);
            std::shared_ptr<WorkInfo> persistedInfo = make_shared<WorkInfo>(workInfo);
            persistedInfo->RefreshUid(uid);
            persistedMap_.emplace(workStatus->workId_, persistedInfo);
            RefreshPersistedWorks();
        }
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
        WS_HILOGE("uid is invalid : %{public}d", workInfo.GetUid());
    }
}

int32_t WorkSchedulerService::StopWork(WorkInfo& workInfo)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (checkBundle_ && !CheckWorkInfo(workInfo, uid)) {
        return E_CHECK_WORKINFO_FAILED;
    }
    shared_ptr<WorkStatus> workStatus = workPolicyManager_->FindWorkStatus(workInfo, uid);
    if (workStatus == nullptr) {
        WS_HILOGE("workStatus is nullptr");
        return E_WORK_NOT_EXIST_FAILED;
    }
    StopWorkInner(workStatus, uid, false, false);
    return ERR_OK;
}

int32_t WorkSchedulerService::StopAndCancelWork(WorkInfo& workInfo)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (checkBundle_ && !CheckWorkInfo(workInfo, uid)) {
        return E_CHECK_WORKINFO_FAILED;
    }
    shared_ptr<WorkStatus> workStatus = workPolicyManager_->FindWorkStatus(workInfo, uid);
    if (workStatus == nullptr) {
        WS_HILOGE("workStatus is nullptr");
        return E_WORK_NOT_EXIST_FAILED;
    }
    StopWorkInner(workStatus, uid, true, false);
    if (workStatus->persisted_) {
        std::lock_guard<std::mutex> lock(mutex_);
        persistedMap_.erase(workStatus->workId_);
        RefreshPersistedWorks();
    }
    return ERR_OK;
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

int32_t WorkSchedulerService::StopAndClearWorks()
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    StopAndClearWorksByUid(IPCSkeleton::GetCallingUid());
    return ERR_OK;
}

bool WorkSchedulerService::StopAndClearWorksByUid(int32_t uid)
{
    WS_HILOGD("Stop and clear works by Uid:%{public}d", uid);
    list<std::shared_ptr<WorkStatus>> allWorks = workPolicyManager_->GetAllWorkStatus(uid);
    list<std::string> workIdList;
    std::transform(allWorks.cbegin(), allWorks.cend(), std::back_inserter(workIdList),
        [](std::shared_ptr<WorkStatus> work) { return work->workId_; });
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

int32_t WorkSchedulerService::IsLastWorkTimeout(int32_t workId, bool &result)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    return workPolicyManager_->IsLastWorkTimeout(workId, uid, result);
}

void WorkSchedulerService::OnConditionReady(shared_ptr<vector<shared_ptr<WorkStatus>>> workStatusVector)
{
    workPolicyManager_->OnConditionReady(workStatusVector);
}

int32_t WorkSchedulerService::ObtainAllWorks(int32_t &uid, int32_t &pid,
    std::list<std::shared_ptr<WorkInfo>>& workInfos)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    workInfos = workPolicyManager_->ObtainAllWorks(uid);
    return ERR_OK;
}

int32_t WorkSchedulerService::GetWorkStatus(int32_t &uid, int32_t &workId, std::shared_ptr<WorkInfo>& workInfo)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        workInfo = nullptr;
        return E_SERVICE_NOT_READY;
    }
    workInfo = workPolicyManager_->GetWorkStatus(uid, workId);
    return ERR_OK;
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

int32_t WorkSchedulerService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::string result;
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        result.append("service is not ready.");
        if (!SaveStringToFd(fd, result)) {
            WS_HILOGE("save to fd failed.");
        }
        return ERR_OK;
    }

    std::vector<std::string> argsInStr;
    std::transform(args.begin(), args.end(), std::back_inserter(argsInStr),
        [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    switch (argsInStr.size()) {
        case 0:
            // hidumper -s said '-h'
            DumpUsage(result);
            break;
        case DUMP_OPTION + 1:
            // hidumper -s said '-h' or hidumper -s said '-a'
            if (argsInStr[DUMP_OPTION] == "-h") {
                DumpUsage(result);
            } else if (argsInStr[DUMP_OPTION] == "-a") {
                DumpAllInfo(result);
            } else {
                result.append("Error params.");
            }
            break;
        case DUMP_PARAM_INDEX + 1:
            DumpParamSet(argsInStr[DUMP_OPTION], argsInStr[DUMP_PARAM_INDEX], result);
            break;
        case DUMP_VALUE_INDEX + 1:
            if (argsInStr[DUMP_OPTION] == "-d") {
                EventPublisher eventPublisher;
                eventPublisher.Dump(result, argsInStr[DUMP_PARAM_INDEX], argsInStr[DUMP_VALUE_INDEX]);
            } else {
                result.append("Error params.");
            }
            break;
        default:
            result.append("Error params.");
    }

    if (!SaveStringToFd(fd, result)) {
        WS_HILOGE("save to fd failed.");
    }
    return ERR_OK;
}

void WorkSchedulerService::DumpUsage(std::string &result)
{
    result.append("usage: workscheduler dump [<options>]\n")
        .append("    -h: show the help.\n")
        .append("    -a: show all info.\n")
        .append("    -d event info: show the event info.\n")
        .append("    -d (eventType) (TypeValue): publish the event.\n")
        .append("    -memory (number): set the available memory.\n")
        .append("    -watchdog_time (number): set watch dog time, default 120000.\n")
        .append("    -repeat_time_min (number): set min repeat cycle time, default 1200000.\n")
        .append("    -min_interval (number): set min interval time, set 0 means close test mode.\n");
}

void WorkSchedulerService::DumpAllInfo(std::string &result)
{
    result.append("================Work Queue Infos================\n");
    if (workQueueManager_ != nullptr) {
        workQueueManager_->Dump(result);
    }
    result.append("================Work Policy Infos================\n");
    if (workPolicyManager_ != nullptr) {
        workPolicyManager_->Dump(result);
    }
    result.append("================Other Infos================\n");
    result.append("Need check bundle:" + std::to_string(checkBundle_) + "\n")
        .append("Dump set memory:" + std::to_string(workPolicyManager_->GetDumpSetMemory()) + "\n")
        .append("Repeat cycle time min:" + std::to_string(workQueueManager_->GetTimeCycle()) + "\n")
        .append("Watchdog time:" + std::to_string(workPolicyManager_->GetWatchdogTime()) + "\n")
        .append("whitelist:" + GetEffiResApplyUid());
}

std::string WorkSchedulerService::GetEffiResApplyUid()
{
    std::lock_guard<std::mutex> lock(whitelistMutex_);
    if (whitelist_.empty()) {
        return "empty";
    }
    std::string res {""};
    for (auto &it : whitelist_) {
        res.append(std::to_string(it) + " ");
    }
    WS_HILOGD("GetWhiteList  : %{public}s", res.c_str());
    return res;
}

void WorkSchedulerService::DumpParamSet(std::string &key, std::string &value, std::string &result)
{
    if (key == "-memory") {
        workPolicyManager_->SetMemoryByDump(std::stoi(value));
        result.append("Set memory success.");
    } else if (key == "-watchdog_time") {
        workPolicyManager_->SetWatchdogTimeByDump(std::stoi(value));
        result.append("Set watchdog time success.");
    } else if (key == "-repeat_time_min") {
        workQueueManager_->SetTimeCycle(std::stoi(value));
        result.append("Set repeat time min value success.");
    } else if (key == "-min_interval") {
        workQueueManager_->SetMinIntervalByDump(std::stoi(value));
        result.append("Set min interval value success.");
    } else {
        result.append("Error params.");
    }
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
    CreateNodeDir(g_persistedPath);
    CreateNodeFile(g_persistedFilePath);
    ofstream fout;
    std::string realPath;
    if (!WorkSchedUtils::ConvertFullPath(g_persistedFilePath, realPath)) {
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
    if (access(filePath.c_str(), 0) != 0) {
        int32_t fd = open(filePath.c_str(), O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
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

void WorkSchedulerService::UpdateEffiResApplyInfo(int32_t uid, bool isAdd)
{
    std::lock_guard<std::mutex> lock(whitelistMutex_);
    if (isAdd) {
        whitelist_.emplace(uid);
    } else {
        whitelist_.erase(uid);
    }
}

bool WorkSchedulerService::CheckEffiResApplyInfo(int32_t uid)
{
    std::lock_guard<std::mutex> lock(whitelistMutex_);
    return whitelist_.find(uid) != whitelist_.end();
}

void WorkSchedulerService::SystemAbilityStatusChangeListener::OnAddSystemAbility
    (int32_t systemAbilityId, const std::string& deviceId)
{
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    if (systemAbilityId == DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID) {
        instance->GroupObserverInit();
    }
#endif
}

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
void WorkSchedulerService::GroupObserverInit()
{
    if (!workQueueManager_) {
        return;
    }
    if (!groupObserver_) {
        groupObserver_ = new (std::nothrow) WorkBundleGroupChangeCallback(workQueueManager_);
    }
    if (groupObserver_ && g_hasGroupObserver != ERR_OK) {
        g_hasGroupObserver =
            DeviceUsageStats::BundleActiveClient::GetInstance().RegisterAppGroupCallBack(groupObserver_);
    }
}
#endif
} // namespace WorkScheduler
} // namespace OHOS