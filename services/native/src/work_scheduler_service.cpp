/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include <climits>          // for PATH_MAX

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

#include "parameters.h"
#include "accesstoken_kit.h"
#include "bundle_mgr_proxy.h"
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
#include "bundle_active_client.h"
#endif
#ifdef DEVICE_STANDBY_ENABLE
#include "standby_service_client.h"
#include "allow_type.h"
#endif
#include "conditions/battery_level_listener.h"
#include "conditions/battery_status_listener.h"
#include "conditions/charger_listener.h"
#include "conditions/condition_checker.h"
#include "conditions/network_listener.h"
#include "conditions/screen_listener.h"
#include "conditions/storage_listener.h"
#include "conditions/timer_listener.h"
#include "conditions/group_listener.h"
#include "config_policy_utils.h"           // for GetOneCfgFile
#include "event_publisher.h"
#include "json/json.h"
#include "policy/app_data_clear_listener.h"
#include "policy/memory_policy.h"
#include "policy/thermal_policy.h"
#include "policy/cpu_policy.h"
#ifdef POWERMGR_POWER_MANAGER_ENABLE
#include "policy/power_mode_policy.h"
#endif
#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
#include "scheduler_bg_task_subscriber.h"
#include "background_task_mgr_helper.h"
#include "resource_type.h"
#endif
#include "work_datashare_helper.h"
#include "work_scheduler_connection.h"
#include "work_bundle_group_change_callback.h"
#include "work_sched_errors.h"
#include "work_sched_hilog.h"
#include "work_sched_utils.h"
#include "hitrace_meter.h"

using namespace std;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace WorkScheduler {
namespace {
const std::string WORKSCHEDULER_SERVICE_NAME = "WorkSchedulerService";
const std::string PRINSTALLED_WORKS_KEY = "work_scheduler_preinstalled_works";
auto instance = DelayedSingleton<WorkSchedulerService>::GetInstance();
auto wss = instance.get();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(wss);
const int32_t UID_TRANSFORM_DIVISOR = 200000;
const int32_t INIT_DELAY = 2 * 1000;
const int32_t CHECK_CONDITION_DELAY = 5 * 1000;
const int32_t MAX_BUFFER = 2048;
const int32_t DUMP_OPTION = 0;
const int32_t DUMP_PARAM_INDEX = 1;
const int32_t DUMP_VALUE_INDEX = 2;
const int32_t TIME_OUT = 4;
const int64_t DEVICE_IDLE = 5;
const char* PERSISTED_FILE_PATH = "/data/service/el1/public/WorkScheduler/persisted_work";
const char* PERSISTED_PATH = "/data/service/el1/public/WorkScheduler";
const char* PREINSTALLED_FILE_PATH = "etc/backgroundtask/config.json";
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
static int g_hasGroupObserver = -1;
#endif
const static std::string STRATEGY_NAME = "WORK_SCHEDULER";
const std::set<std::string> WORK_SCHED_NATIVE_OPERATE_CALLER = {
    "resource_schedule_service",
    "hidumper_service",
};
}

#ifdef WORK_SCHEDULER_TEST
#define WEAK_FUNC __attribute__((weak))
#else
#define WEAK_FUNC
#endif

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
    handler_ = std::make_shared<WorkEventHandler>(eventRunner_, instance);
    if (!handler_) {
        WS_HILOGE("Init failed due to create handler_");
        return;
    }

    // Try to init.
    Init(eventRunner_);
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    AddSystemAbilityListener(DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID);
#endif
#ifdef DEVICE_STANDBY_ENABLE
    AddSystemAbilityListener(DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID);
#endif
    WS_HILOGD("On start success.");
}

WEAK_FUNC bool WorkSchedulerService::IsBaseAbilityReady()
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

void WorkSchedulerService::InitPersistedWork()
{
    WS_HILOGD("init persisted work");
    list<shared_ptr<WorkInfo>> persistedWorks = ReadPersistedWorks();
    for (auto it : persistedWorks) {
        WS_HILOGI("get persisted work, id: %{public}d", it->GetWorkId());
        AddWorkInner(*it);
    }
}

void WorkSchedulerService::InitPreinstalledWork()
{
    WS_HILOGD("init preinstalled work");
    bool needRefresh = false;
    static list<shared_ptr<WorkInfo>> preinstalledWorks = ReadPreinstalledWorks();
    for (auto work : preinstalledWorks) {
        WS_HILOGD("preinstalled workinfo id %{public}d, uid %{public}d", work->GetWorkId(), work->GetUid());
        if (!work->IsPersisted()) {
            time_t baseTime;
            (void)time(&baseTime);
            work->RequestBaseTime(baseTime);
            AddWorkInner(*work);
            continue;
        }
        auto iter = std::find_if(persistedMap_.begin(), persistedMap_.end(), [&](const auto &pair) {
            return (pair.second->GetUid() == work->GetUid()) && (pair.second->GetWorkId() == work->GetWorkId());
        });
        if (iter != persistedMap_.end()) {
            WS_HILOGD("find workid %{public}d in persisted map, ignore", work->GetWorkId());
            continue;
        }
        needRefresh = true;
        time_t baseTime;
        (void)time(&baseTime);
        work->RequestBaseTime(baseTime);
        AddWorkInner(*work);
        string workId = "u" + to_string(work->GetUid()) + "_" + to_string(work->GetWorkId());
        persistedMap_.emplace(workId, work);
    }
    if (needRefresh) {
        RefreshPersistedWorks();
    }
}

void WorkSchedulerService::InitWorkInner()
{
    InitPersistedWork();
    InitPreinstalledWork();
}

list<shared_ptr<WorkInfo>> WorkSchedulerService::ReadPersistedWorks()
{
    list<shared_ptr<WorkInfo>> workInfos;
    Json::Value root;
    if (!GetJsonFromFile(PERSISTED_FILE_PATH, root)) {
        return workInfos;
    }
    for (const auto &it : root.getMemberNames()) {
        Json::Value workJson = root[it];
        shared_ptr<WorkInfo> workInfo = make_shared<WorkInfo>();
        if (workInfo->ParseFromJson(workJson)) {
            workInfos.emplace_back(workInfo);
            WS_HILOGI("find one persisted work %{public}d", workInfo->GetWorkId());
            string workId = "u" + to_string(workInfo->GetUid()) + "_" + to_string(workInfo->GetWorkId());
            persistedMap_.emplace(workId, workInfo);
        }
    }
    return workInfos;
}

void WorkSchedulerService::LoadWorksFromFile(const char *path, list<shared_ptr<WorkInfo>> &workInfos)
{
    if (!path) {
        return;
    }
    Json::Value root;
    if (!GetJsonFromFile(path, root) || root.empty()) {
        WS_HILOGE("file is empty %{private}s", path);
        return;
    }
    if (!root.isMember(PRINSTALLED_WORKS_KEY)) {
        WS_HILOGE("no work_scheduler_preinstalled_works key");
        return;
    }
    Json::Value preinstalledWorksRoot = root[PRINSTALLED_WORKS_KEY];
    if (preinstalledWorksRoot.empty() || !preinstalledWorksRoot.isObject()) {
        WS_HILOGE("work_scheduler_preinstalled_works content is empty");
        return;
    }
    for (const auto &it : preinstalledWorksRoot.getMemberNames()) {
        Json::Value workJson = preinstalledWorksRoot[it];
        shared_ptr<WorkInfo> workinfo = make_shared<WorkInfo>();
        if (workinfo->ParseFromJson(workJson)) {
            if (workinfo->GetSaId() > 0) {
                saMap_.emplace(workinfo->GetSaId(), workinfo->IsResidentSa());
                continue;
            }
            int32_t uid;
            if (!GetUidByBundleName(workinfo->GetBundleName(), uid)) {
                continue;
            }
            workinfo->RefreshUid(uid);
            workinfo->SetPreinstalled(true);
            workInfos.emplace_back(workinfo);
        } else {
            WS_HILOGE("ParseFromJson error");
        }
    }
}

list<shared_ptr<WorkInfo>> WorkSchedulerService::ReadPreinstalledWorks()
{
    list<shared_ptr<WorkInfo>> workInfos;
    CfgFiles *files = GetCfgFiles(PREINSTALLED_FILE_PATH);
    if (!files) {
        WS_HILOGE("GetCfgFiles failed");
        return workInfos;
    }
    // china->base
    for (int i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        LoadWorksFromFile(files->paths[i], workInfos);
    }
    FreeCfgFiles(files);
    return workInfos;
}

bool WorkSchedulerService::GetJsonFromFile(const char *filePath, Json::Value &root)
{
    ifstream fin;
    std::string realPath;
    if (!WorkSchedUtils::ConvertFullPath(filePath, realPath)) {
        WS_HILOGE("Get real path failed %{private}s", filePath);
        return false;
    }
    WS_HILOGD("Read from %{private}s", realPath.c_str());
    fin.open(realPath, ios::in);
    if (!fin.is_open()) {
        WS_HILOGE("cannot open file %{private}s", realPath.c_str());
        return false;
    }
    char buffer[MAX_BUFFER];
    ostringstream os;
    while (fin.getline(buffer, MAX_BUFFER)) {
        os << buffer;
    }
    string data = os.str();
    JSONCPP_STRING errs;
    Json::CharReaderBuilder readerBuilder;
    const unique_ptr<Json::CharReader> jsonReader(readerBuilder.newCharReader());
    bool res = jsonReader->parse(data.c_str(), data.c_str() + data.length(), &root, &errs);
    fin.close();
    if (!res || !errs.empty()) {
        WS_HILOGE("parse %{private}s json error", realPath.c_str());
        return false;
    }
    return true;
}

void WorkSchedulerService::OnStop()
{
    WS_HILOGI("stop service.");
    std::lock_guard<std::mutex> observerLock(observerMutex_);
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    DeviceUsageStats::BundleActiveClient::GetInstance().UnRegisterAppGroupCallBack(groupObserver_);
    groupObserver_ = nullptr;
    g_hasGroupObserver = -1;
#endif
#ifdef DEVICE_STANDBY_ENABLE
    DevStandbyMgr::StandbyServiceClient::GetInstance().UnsubscribeStandbyCallback(standbyStateObserver_);
    standbyStateObserver_ = nullptr;
#endif
#ifdef RESOURCESCHEDULE_BGTASKMGR_ENABLE
    ErrCode ret = BackgroundTaskMgr::BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(*subscriber_);
    if (ret != ERR_OK) {
        WS_HILOGE("unscribe bgtask failed.");
    }
#endif
    eventRunner_.reset();
    handler_.reset();
    ready_ = false;
}

bool WorkSchedulerService::Init(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    if (!IsBaseAbilityReady()) {
        WS_HILOGE("request system service is not ready yet!");
        GetHandler()->SendEvent(InnerEvent::Get(WorkEventHandler::SERVICE_INIT_MSG, 0), INIT_DELAY);
        return false;
    }
    WorkQueueManagerInit(runner);
    if (!WorkPolicyManagerInit(runner)) {
        WS_HILOGE("init failed due to work policy manager init.");
        return false;
    }
    InitWorkInner();
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

void WorkSchedulerService::WorkQueueManagerInit(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    WS_HILOGD("come in");
    if (workQueueManager_ == nullptr) {
        workQueueManager_ = make_shared<WorkQueueManager>(instance);
    }

    auto networkListener = make_shared<NetworkListener>(workQueueManager_);
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    auto chargerListener = make_shared<ChargerListener>(workQueueManager_);
    auto batteryStatusListener = make_shared<BatteryStatusListener>(workQueueManager_);
    auto batteryLevelListener = make_shared<BatteryLevelListener>(workQueueManager_, shared_from_this());
    batteryLevelListener->Start();
#endif // POWERMGR_BATTERY_MANAGER_ENABLE
    auto storageListener = make_shared<StorageListener>(workQueueManager_);
    auto timerListener = make_shared<TimerListener>(workQueueManager_, runner);
    auto groupListener = make_shared<GroupListener>(workQueueManager_, runner);
    auto screenListener = make_shared<ScreenListener>(workQueueManager_, shared_from_this());

    workQueueManager_->AddListener(WorkCondition::Type::NETWORK, networkListener);
#ifdef POWERMGR_BATTERY_MANAGER_ENABLE
    workQueueManager_->AddListener(WorkCondition::Type::CHARGER, chargerListener);
    workQueueManager_->AddListener(WorkCondition::Type::BATTERY_STATUS, batteryStatusListener);
    workQueueManager_->AddListener(WorkCondition::Type::BATTERY_LEVEL, batteryLevelListener);
#endif // POWERMGR_BATTERY_MANAGER_ENABLE
    workQueueManager_->AddListener(WorkCondition::Type::STORAGE, storageListener);
    workQueueManager_->AddListener(WorkCondition::Type::TIMER, timerListener);
    workQueueManager_->AddListener(WorkCondition::Type::GROUP, groupListener);
    workQueueManager_->AddListener(WorkCondition::Type::DEEP_IDLE, screenListener);

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    GroupObserverInit();
#endif
    RegisterStandbyStateObserver();
}

bool WorkSchedulerService::WorkPolicyManagerInit(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    WS_HILOGD("come in");
    if (workPolicyManager_ == nullptr) {
        workPolicyManager_ = make_shared<WorkPolicyManager>(instance);
    }
    if (!workPolicyManager_->Init(runner)) {
        WS_HILOGE("work policy manager init failed!");
        return false;
    }

#ifdef POWERMGR_THERMAL_MANAGER_ENABLE
    auto thermalFilter = make_shared<ThermalPolicy>(workPolicyManager_);
    workPolicyManager_->AddPolicyFilter(thermalFilter);
#endif // POWERMGR_THERMAL_MANAGER_ENABLE
    auto memoryFilter = make_shared<MemoryPolicy>(workPolicyManager_);
    workPolicyManager_->AddPolicyFilter(memoryFilter);

    auto cpuFilter = make_shared<CpuPolicy>(workPolicyManager_);
    workPolicyManager_->AddPolicyFilter(cpuFilter);

#ifdef POWERMGR_POWER_MANAGER_ENABLE
    auto powerModeFilter = make_shared<PowerModePolicy>(workPolicyManager_);
    workPolicyManager_->AddPolicyFilter(powerModeFilter);
#endif

    auto appDataClearListener = make_shared<AppDataClearListener>(workPolicyManager_);
    workPolicyManager_->AddAppDataClearListener(appDataClearListener);

    WS_HILOGI("work policy manager init success.");
    return true;
}

WEAK_FUNC bool WorkSchedulerService::GetUidByBundleName(const string &bundleName, int32_t &uid)
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
    if (bundleMgr->GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_WITH_ABILITIES,
        bundleInfo, currentAccountId)) {
        WS_HILOGD("currentAccountId : %{public}d, bundleName : %{public}s, uid = %{public}d",
            currentAccountId, bundleName.c_str(), bundleInfo.uid);
        uid = bundleInfo.uid;
        return true;
    }
    WS_HILOGE("Get bundle info %{public}s failed.", bundleName.c_str());
    return false;
}

bool WorkSchedulerService::GetAppIndexAndBundleNameByUid(int32_t uid, int32_t &appIndex, std::string &bundleName)
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
    ErrCode ret = bundleMgr->GetNameAndIndexForUid(uid, bundleName, appIndex);
    if (ret == ERR_OK) {
        WS_HILOGD("appIndex = %{public}d", appIndex);
        return true;
    }
    WS_HILOGE("fail to get app index.");
    return false;
}

bool WorkSchedulerService::CheckExtensionInfos(WorkInfo &workInfo, int32_t uid)
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
    if (bundleMgr->GetBundleInfo(workInfo.GetBundleName(),
        BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO,
        bundleInfo, uid / UID_TRANSFORM_DIVISOR)) {
        auto findIter = std::find_if(bundleInfo.extensionInfos.begin(), bundleInfo.extensionInfos.end(),
            [&](const auto &info) {
                WS_HILOGD("%{public}s %{public}s %{public}d", info.bundleName.c_str(), info.name.c_str(), info.type);
                return info.bundleName == workInfo.GetBundleName() &&
                    info.name == workInfo.GetAbilityName() &&
                    info.type == ExtensionAbilityType::WORK_SCHEDULER;
            });
        if (findIter == bundleInfo.extensionInfos.end()) {
            workInfo.RefreshExtension(false);
            WS_HILOGE("extension info is error");
            return false;
        }
    }
    return true;
}

bool WorkSchedulerService::CheckWorkInfo(WorkInfo &workInfo, int32_t &uid)
{
    int32_t appIndex;
    string bundleName;
    if (GetAppIndexAndBundleNameByUid(uid, appIndex, bundleName)) {
        workInfo.RefreshAppIndex(appIndex);
        if (workInfo.GetBundleName() == bundleName) {
            CheckExtensionInfos(workInfo, uid);
            return true;
        }
    }
    WS_HILOGE("bundleName %{public}s is invalid", workInfo.GetBundleName().c_str());
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
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::StartWork");
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (checkBundle_ && !CheckWorkInfo(workInfo, uid)) {
        WS_HILOGE("check workInfo failed, bundleName inconsistency.");
        return E_CHECK_WORKINFO_FAILED;
    }
    if (!CheckCondition(workInfo)) {
        return E_REPEAT_CYCLE_TIME_ERR;
    }
    time_t baseTime;
    (void)time(&baseTime);
    workInfo.RequestBaseTime(baseTime);
    WS_HILOGD("workInfo %{public}s/%{public}s ID: %{public}d, uid: %{public}d",
        workInfo.GetBundleName().c_str(), workInfo.GetAbilityName().c_str(), workInfo.GetWorkId(), uid);
    shared_ptr<WorkStatus> workStatus = make_shared<WorkStatus>(workInfo, uid);
    int32_t ret = workPolicyManager_->AddWork(workStatus, uid);
    if (ret == ERR_OK) {
        workQueueManager_->AddWork(workStatus);
        if (workInfo.IsPersisted()) {
            std::lock_guard<std::mutex> lock(mutex_);
            workStatus->workInfo_->RefreshUid(uid);
            persistedMap_.emplace(workStatus->workId_, workStatus->workInfo_);
            RefreshPersistedWorks();
        }
        GetHandler()->RemoveEvent(WorkEventHandler::CHECK_CONDITION_MSG);
        GetHandler()->SendEvent(InnerEvent::Get(WorkEventHandler::CHECK_CONDITION_MSG, 0),
            CHECK_CONDITION_DELAY);
    }
    return ret;
}

void WorkSchedulerService::AddWorkInner(WorkInfo& workInfo)
{
    WS_HILOGD("come in");
    if (workInfo.GetUid() > 0) {
        shared_ptr<WorkStatus> workStatus = make_shared<WorkStatus>(workInfo, workInfo.GetUid());
        if (workPolicyManager_->AddWork(workStatus, workInfo.GetUid()) == ERR_OK) {
            workQueueManager_->AddWork(workStatus);
        }
    } else {
        WS_HILOGE("uid is invalid : %{public}d", workInfo.GetUid());
    }
}

int32_t WorkSchedulerService::StopWork(WorkInfo& workInfo)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::StopWork");
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (checkBundle_ && !CheckWorkInfo(workInfo, uid)) {
        WS_HILOGE("check workInfo failed, bundleName inconsistency.");
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
        WS_HILOGE("check workInfo failed, bundleName inconsistency.");
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
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::StopAndClearWorks");
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
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::IsLastWorkTimeout");
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

int32_t WorkSchedulerService::ObtainAllWorks(std::list<std::shared_ptr<WorkInfo>>& workInfos)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::ObtainAllWorks");
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    workInfos = workPolicyManager_->ObtainAllWorks(uid);
    return ERR_OK;
}

int32_t WorkSchedulerService::GetWorkStatus(int32_t &workId, std::shared_ptr<WorkInfo>& workInfo)
{
    HitraceScoped traceScoped(HITRACE_TAG_OHOS, "WorkSchedulerService::GetWorkStatus");
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        workInfo = nullptr;
        return E_SERVICE_NOT_READY;
    }
    workInfo = workPolicyManager_->GetWorkStatus(uid, workId);
    return ERR_OK;
}

int32_t WorkSchedulerService::GetAllRunningWorks(std::list<std::shared_ptr<WorkInfo>>& workInfos)
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    workInfos = workPolicyManager_->GetAllRunningWorks();
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

bool WorkSchedulerService::AllowDump()
{
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetFirstTokenID();
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, "ohos.permission.DUMP");
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        WS_HILOGE("CheckPermission failed");
        return false;
    }
    int32_t ENG_MODE = OHOS::system::GetIntParameter("const.debuggable", 0);
    bool SECURE_MODE = OHOS::system::GetBoolParameter("const.security.developermode.state", false);
    if (ENG_MODE == 1 || SECURE_MODE) {
        return true;
    }

    WS_HILOGE("Not eng mode and developer mode");
    return false;
}

void WorkSchedulerService::DumpProcess(std::vector<std::string> &argsInStr, std::string &result)
{
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
            if (argsInStr[DUMP_OPTION] == "-k") {
                string key = argsInStr[DUMP_PARAM_INDEX];
                string value;
                WorkDatashareHelper::GetInstance().GetStringValue(key, value);
                result.append("key: " + key + ", value: " + value);
                break;
            }
            DumpParamSet(argsInStr[DUMP_OPTION], argsInStr[DUMP_PARAM_INDEX], result);
            break;
        case DUMP_VALUE_INDEX + 1:
            if (argsInStr[DUMP_OPTION] == "-d") {
                EventPublisher eventPublisher;
                eventPublisher.Dump(result, argsInStr[DUMP_PARAM_INDEX], argsInStr[DUMP_VALUE_INDEX]);
            } else if (argsInStr[DUMP_OPTION] == "-t") {
                DumpProcessWorks(argsInStr[DUMP_PARAM_INDEX], argsInStr[DUMP_VALUE_INDEX], result);
            } else if (argsInStr[DUMP_OPTION] == "-x") {
                DumpRunningWorks(argsInStr[DUMP_PARAM_INDEX], argsInStr[DUMP_VALUE_INDEX], result);
            } else {
                result.append("Error params.");
            }
            break;
        default:
            result.append("Error params.");
    }
}

int32_t WorkSchedulerService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    if (!AllowDump()) {
        return ERR_OK;
    }
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
    DumpProcess(argsInStr, result);
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
        .append("    -t (bundleName) (abilityName): trigger the work.\n")
        .append("    -x (uid) (option): pause or resume the work.\n")
        .append("    -memory (number): set the available memory.\n")
        .append("    -watchdog_time (number): set watch dog time, default 120000.\n")
        .append("    -repeat_time_min (number): set min repeat cycle time, default 1200000.\n")
        .append("    -min_interval (number): set min interval time, set 0 means close test mode.\n")
        .append("    -cpu (number): set the usage cpu.\n")
        .append("    -count (number): set the max running task count.\n");
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

bool WorkSchedulerService::IsDebugApp(const std::string &bundleName)
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
    if (bundleMgr->GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_WITH_ABILITIES,
        bundleInfo, currentAccountId)) {
        WS_HILOGD("bundleUid : %{public}d , debug : %{public}d.", bundleInfo.uid, bundleInfo.applicationInfo.debug);
        return bundleInfo.applicationInfo.debug;
    }
    WS_HILOGE("Get bundle info failed.");
    return false;
}

void WorkSchedulerService::DumpProcessWorks(const std::string &bundleName, const std::string &abilityName,
    std::string &result)
{
    if (bundleName.empty() || abilityName.empty()) {
        result.append("param error");
        return;
    }
    workPolicyManager_->DumpCheckIdeWorkToRun(bundleName, abilityName);
}

void WorkSchedulerService::DumpRunningWorks(const std::string &uidStr, const std::string &option, std::string &result)
{
    if (uidStr.empty() || option.empty()) {
        result.append("param error");
        return;
    }

    int32_t uid = std::stoi(uidStr);
    int32_t ret = ERR_OK;
    if (option == "p") {
        ret = workPolicyManager_->PauseRunningWorks(uid);
    } else if (option == "r") {
        ret = workPolicyManager_->ResumePausedWorks(uid);
    } else {
        result.append("param error");
    }

    if (ret != ERR_OK) {
        auto iter = paramErrCodeMsgMap.find(ret);
        if (iter != paramErrCodeMsgMap.end()) {
            result.append("BussinessError:" + iter->second);
        }
    }
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
    if (!std::all_of(value.begin(), value.end(), ::isdigit)) {
        result.append("Error params.");
        return;
    }
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
    } else if (key == "-cpu") {
        workPolicyManager_->SetCpuUsageByDump(std::stoi(value));
        result.append("Set cpu success.");
    } else if (key == "-nap") {
#ifdef DEVICE_STANDBY_ENABLE
        standbyStateObserver_->OnDeviceIdleMode(std::stoi(value), 0);
#endif
    } else if (key == "-count") {
        workPolicyManager_->SetMaxRunningCountByDump(std::stoi(value));
        result.append("Set max running task count success.");
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
    CreateNodeDir(PERSISTED_PATH);
    CreateNodeFile(PERSISTED_FILE_PATH);
    ofstream fout;
    std::string realPath;
    if (!WorkSchedUtils::ConvertFullPath(PERSISTED_FILE_PATH, realPath)) {
        WS_HILOGE("Get real path failed");
        return;
    }
    WS_HILOGD("Refresh path %{private}s", realPath.c_str());
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

bool WorkSchedulerService::CheckStandbyApplyInfo(std::string& bundleName)
{
    WS_HILOGD("%{public}s is checking standby applyInfo", bundleName.c_str());
#ifdef  DEVICE_STANDBY_ENABLE
    std::lock_guard<std::mutex> observerLock(observerMutex_);
    if (!standbyStateObserver_) {
        return true;
    }
    std::vector<DevStandbyMgr::AllowInfo> allowInfoArray;
    DevStandbyMgr::StandbyServiceClient::GetInstance().GetAllowList(DevStandbyMgr::AllowType::WORK_SCHEDULER,
        allowInfoArray, DevStandbyMgr::ReasonCodeEnum::REASON_APP_API);
    WS_HILOGD("allowInfoArray size is %{public}d", static_cast<int32_t>(allowInfoArray.size()));
    for (const auto& item : allowInfoArray) {
        if (item.GetName() == bundleName) {
            return true;
        }
    }
#endif
    return false;
}

void WorkSchedulerService::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId == DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID) {
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
        GroupObserverInit();
#endif
    }
    if (systemAbilityId == DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID) {
        RegisterStandbyStateObserver();
    }
}

void WorkSchedulerService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId == DEVICE_STANDBY_SERVICE_SYSTEM_ABILITY_ID) {
        if (!workQueueManager_) {
            return;
        }
        workQueueManager_->OnConditionChanged(WorkCondition::Type::STANDBY,
            std::make_shared<DetectorValue>(0, 0, false, std::string()));
#ifdef  DEVICE_STANDBY_ENABLE
        std::lock_guard<std::mutex> observerLock(observerMutex_);
        standbyStateObserver_ = nullptr;
#endif
    } else if (systemAbilityId == DEVICE_USAGE_STATISTICS_SYS_ABILITY_ID) {
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
        std::lock_guard<std::mutex> observerLock(observerMutex_);
        groupObserver_ = nullptr;
#endif
    }
}

#ifdef DEVICE_USAGE_STATISTICS_ENABLE
__attribute__((no_sanitize("cfi"))) void WorkSchedulerService::GroupObserverInit()
{
    if (!workQueueManager_) {
        return;
    }
    std::lock_guard<std::mutex> observerLock(observerMutex_);
    if (!groupObserver_) {
        groupObserver_ = new (std::nothrow) WorkBundleGroupChangeCallback(workQueueManager_);
    }
    if (groupObserver_ && g_hasGroupObserver != ERR_OK) {
        g_hasGroupObserver =
            DeviceUsageStats::BundleActiveClient::GetInstance().RegisterAppGroupCallBack(groupObserver_);
    }
}
#endif

void WorkSchedulerService::RegisterStandbyStateObserver()
{
    if (!workQueueManager_) {
        return;
    }
#ifdef  DEVICE_STANDBY_ENABLE
    std::lock_guard<std::mutex> observerLock(observerMutex_);
    if (standbyStateObserver_) {
        WS_HILOGD("standbyStateObserver_ is already exist, do not need repeat process.");
        return;
    }
    standbyStateObserver_ = new (std::nothrow) WorkStandbyStateChangeCallback(workQueueManager_);
    if (!standbyStateObserver_) {
        return;
    }
    standbyStateObserver_->SetSubscriberName(STRATEGY_NAME);
    ErrCode ret = DevStandbyMgr::StandbyServiceClient::GetInstance().SubscribeStandbyCallback(standbyStateObserver_);
    if (ret != ERR_OK) {
        WS_HILOGE("Subscriber standbyStateObserver_ failed.");
        standbyStateObserver_ = nullptr;
    }
#endif
}

bool WorkSchedulerService::CheckProcessName()
{
    Security::AccessToken::AccessTokenID tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo callingTokenInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, callingTokenInfo);
    WS_HILOGD("process name: %{public}s called CheckProcessName.", callingTokenInfo.processName.c_str());
    if (WORK_SCHED_NATIVE_OPERATE_CALLER.find(callingTokenInfo.processName) == WORK_SCHED_NATIVE_OPERATE_CALLER.end()) {
        WS_HILOGE("CheckProcessName illegal access to this interface; process name: %{public}s.",
            callingTokenInfo.processName.c_str());
        return false;
    }
    return true;
}

int32_t WorkSchedulerService::PauseRunningWorks(int32_t uid)
{
    WS_HILOGD("Pause Running Work Scheduler Work, uid:%{public}d", uid);
    if (!CheckProcessName()) {
        return E_INVALID_PROCESS_NAME;
    }

    int32_t ret = workPolicyManager_->PauseRunningWorks(uid);
    return ret;
}

int32_t WorkSchedulerService::ResumePausedWorks(int32_t uid)
{
    WS_HILOGD("Resume Paused Work Scheduler Work, uid:%{public}d", uid);
    if (!CheckProcessName()) {
        return E_INVALID_PROCESS_NAME;
    }

    int32_t ret = workPolicyManager_->ResumePausedWorks(uid);
    return ret;
}

void WorkSchedulerService::TriggerWorkIfConditionReady()
{
    ConditionChecker checker(workQueueManager_);
    checker.CheckAllStatus();
}

void WorkSchedulerService::SetScreenOffTime(uint64_t screenOffTime)
{
    screenOffTime_.store(screenOffTime);
}

uint64_t WorkSchedulerService::GetScreenOffTime()
{
    return screenOffTime_.load();
}

void WorkSchedulerService::SetDeepIdle(bool deepIdle)
{
    deepIdle_.store(deepIdle);
}

bool WorkSchedulerService::IsDeepIdle()
{
    return deepIdle_.load();
}

int32_t WorkSchedulerService::StopDeepIdleWorks()
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return E_SERVICE_NOT_READY;
    }
    std::list<std::shared_ptr<WorkStatus>> works =  workPolicyManager_->GetDeepIdleWorks();
    if (works.size() == 0) {
        WS_HILOGD("stop work by condition, no matched works");
        return ERR_OK;
    }

    for (shared_ptr<WorkStatus> workStatus : works) {
        WS_HILOGI("stop work by condition, bundleName:%{public}s, workId:%{public}s",
            workStatus->bundleName_.c_str(), workStatus->workId_.c_str());
        StopWorkInner(workStatus, workStatus->uid_, false, false);
        workPolicyManager_->RemoveWatchDog(workStatus);
    }
    return ERR_OK;
}

void WorkSchedulerService::LoadSa()
{
    if (!ready_) {
        WS_HILOGE("service is not ready.");
        return;
    }
    if (saMap_.empty()) {
        WS_HILOGD("saMap_ is empty.");
        return;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        WS_HILOGE("get sa manager failed.");
        return;
    }
    for (auto it : saMap_) {
        std::vector vec = {it.first};
        std::string action = "strat";
        if (!it.second) {
            auto res = samgr->LoadSystemAbility(it.first, TIME_OUT);
            if (res == nullptr) {
                WS_HILOGE("load sa: %{public}d failed.", it.first);
                continue;
            }
            WS_HILOGD("load sa: %{public}d success.", it.first);
        }
        int32_t ret = samgr->SendStrategy(DEVICE_IDLE, vec, 0, action);
        if (ret != ERR_OK) {
            WS_HILOGE("sa: %{public}d sendStrategy failed.", it.first);
            continue;
        }
        WS_HILOGD("sa: %{public}d sendStrategy successed.", it.first);
    }
}
} // namespace WorkScheduler
} // namespace OHOS
