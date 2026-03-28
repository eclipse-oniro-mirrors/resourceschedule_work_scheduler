/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "workinfoparsefromjson_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <string>

#include "iwork_sched_service.h"
#include "work_scheduler_service.h"
#include "work_sched_common.h"
#include "work_condition.h"
#include "work_info.h"
#include "nlohmann/json.hpp"

void OHOS::RefBase::DecStrongRef(void const* obj) {}

namespace OHOS {
namespace WorkScheduler {

const std::u16string WORK_SCHEDULER_STUB_TOKEN = u"ohos.workscheduler.iworkschedservice";
static std::shared_ptr<WorkSchedulerService> workSchedulerService_;

bool WorkSchedulerService::GetUidByBundleName(const std::string &bundleName, int32_t &uid)
{
    return true;
}

void InitWorkSchedulerService()
{
    workSchedulerService_ = DelayedSingleton<WorkSchedulerService>::GetInstance();
    workSchedulerService_->OnStart();
    workSchedulerService_->InitBgTaskSubscriber();
    if (!workSchedulerService_->ready_) {
        workSchedulerService_->ready_ = true;
    }
    if (workSchedulerService_->checkBundle_) {
        workSchedulerService_->checkBundle_ = false;
    }
}

static int32_t GetInt32FromData(const uint8_t* data, size_t size, size_t& offset)
{
    int32_t val = 0;
    if (offset + sizeof(int32_t) <= size) {
        if (memcpy_s(&val, sizeof(val), data + offset, sizeof(int32_t)) == EOK) {
            offset += sizeof(int32_t);
        }
    }
    return val;
}

static uint32_t GetUint32FromData(const uint8_t* data, size_t size, size_t& offset)
{
    uint32_t val = 0;
    if (offset + sizeof(uint32_t) <= size) {
        if (memcpy_s(&val, sizeof(val), data + offset, sizeof(uint32_t)) == EOK) {
            offset += sizeof(uint32_t);
        }
    }
    return val;
}

static bool GetBoolFromData(const uint8_t* data, size_t size, size_t& offset)
{
    constexpr uint8_t modTwo = 2;
    bool val = false;
    if (offset < size) {
        val = (data[offset] % modTwo) == 1;
        offset += 1;
    }
    return val;
}

static int64_t GetInt64FromData(const uint8_t* data, size_t size, size_t& offset)
{
    int64_t val = 0;
    if (offset + sizeof(int64_t) <= size) {
        if (memcpy_s(&val, sizeof(val), data + offset, sizeof(int64_t)) == EOK) {
            offset += sizeof(int64_t);
        }
    }
    return val;
}

void FuzzParseFromJsonBasic(const uint8_t* data, size_t size)
{
    std::string rawJson(reinterpret_cast<const char*>(data), size);
    nlohmann::json value = nlohmann::json::parse(rawJson, nullptr, false);
    if (value.is_discarded()) {
        return;
    }
    WorkInfo workInfo;
    workInfo.ParseFromJson(value);
}

void FuzzParseFromJsonWithAllConditions(const uint8_t* data, size_t size)
{
    size_t offset = 0;

    nlohmann::json root;
    root["workId"] = GetInt32FromData(data, size, offset);
    root["bundleName"] = "com.example.fuzz";
    root["abilityName"] = "FuzzAbility";
    root["persisted"] = GetBoolFromData(data, size, offset);
    root["preinstalled"] = GetBoolFromData(data, size, offset);
    root["callBySystemApp"] = GetBoolFromData(data, size, offset);
    root["uriKey"] = "fuzzUriKey";
    root["appIndex"] = GetInt32FromData(data, size, offset);
    root["uid"] = GetInt32FromData(data, size, offset);
    root["earliestStartTime"] = GetInt32FromData(data, size, offset);
    root["createTime"] = static_cast<uint64_t>(GetInt64FromData(data, size, offset));

    constexpr int32_t modNetwork = 7;
    constexpr int32_t modCharger = 6;
    constexpr int32_t modBattery = 4;
    constexpr int32_t modStorage = 4;
    
    nlohmann::json conditions;
    conditions["network"] = GetInt32FromData(data, size, offset) % modNetwork;
    conditions["isCharging"] = GetBoolFromData(data, size, offset);
    conditions["chargerType"] = GetInt32FromData(data, size, offset) % modCharger;
    conditions["batteryLevel"] = GetInt32FromData(data, size, offset);
    conditions["batteryStatus"] = GetInt32FromData(data, size, offset) % modBattery;
    conditions["storage"] = GetInt32FromData(data, size, offset) % modStorage;
    conditions["isDeepIdle"] = GetBoolFromData(data, size, offset);
    conditions["deepIdleTime"] = GetInt32FromData(data, size, offset);

    conditions["timer"] = static_cast<int32_t>(GetUint32FromData(data, size, offset));
    conditions["repeat"] = GetBoolFromData(data, size, offset);
    conditions["cycle"] = GetInt32FromData(data, size, offset);
    conditions["baseTime"] = GetInt64FromData(data, size, offset);
    root["conditions"] = conditions;

    WorkInfo workInfo;
    workInfo.ParseFromJson(root);

    std::string jsonStr = workInfo.ParseToJsonStr();
    std::string dumpResult;
    workInfo.Dump(dumpResult);
}

void FuzzParseFromJsonSaPath(const uint8_t* data, size_t size)
{
    size_t offset = 0;
    int32_t workId = GetInt32FromData(data, size, offset);
    int32_t saId = GetInt32FromData(data, size, offset);
    if (saId == -1) {
        saId = 0;
    }
    bool residentSa = GetBoolFromData(data, size, offset);

    nlohmann::json root;
    root["workId"] = workId;
    root["saId"] = saId;
    root["residentSa"] = residentSa;
    root["persisted"] = true;

    WorkInfo workInfo;
    workInfo.RefreshSaId(saId);
    workInfo.ParseFromJson(root);

    workInfo.IsSA();
    workInfo.GetSaId();
    workInfo.IsResidentSa();
    workInfo.GetBriefInfo();

    std::string jsonStr = workInfo.ParseToJsonStr();
    std::string dumpResult;
    workInfo.Dump(dumpResult);
}

void FuzzParseFromJsonRoundTripPart2(WorkInfo& srcInfo)
{
    std::string jsonStr = srcInfo.ParseToJsonStr();

    nlohmann::json parsed = nlohmann::json::parse(jsonStr, nullptr, false);
    if (!parsed.is_discarded()) {
        WorkInfo dstInfo;
        dstInfo.ParseFromJson(parsed);

        dstInfo.GetWorkId();
        dstInfo.GetBundleName();
        dstInfo.GetAbilityName();
        dstInfo.IsPersisted();
        dstInfo.GetNetworkType();
        dstInfo.GetChargerType();
        dstInfo.GetBatteryLevel();
        dstInfo.GetBatteryStatus();
        dstInfo.GetStorageLevel();
        dstInfo.IsRepeat();
        dstInfo.GetTimeInterval();
        dstInfo.GetCycleCount();
        dstInfo.GetBaseTime();
        dstInfo.GetDeepIdle();
        dstInfo.GetConditionMap();
        dstInfo.GetExtras();
        dstInfo.IsCallBySystemApp();
        dstInfo.GetUid();
        dstInfo.GetAppIndex();
        dstInfo.IsPreinstalled();
        dstInfo.GetUriKey();
        dstInfo.GetBriefInfo();
        dstInfo.GetIsInnerApply();
        dstInfo.GetEarliestStartTime();
        dstInfo.GetCreateTime();
        dstInfo.GetDeepIdleTime();
        dstInfo.GetTriggerType();
    }
}

void FuzzParseFromJsonRoundTrip(const uint8_t* data, size_t size)
{
    size_t offset = 0;
    int32_t workId = GetInt32FromData(data, size, offset);
    int32_t batteryLevel = GetInt32FromData(data, size, offset);
    uint32_t timeInterval = GetUint32FromData(data, size, offset);
    int32_t cycle = GetInt32FromData(data, size, offset);
    bool isCharging = GetBoolFromData(data, size, offset);
    bool isDeepIdle = GetBoolFromData(data, size, offset);
    int64_t baseTime = GetInt64FromData(data, size, offset);
    int32_t networkType = GetInt32FromData(data, size, offset) % 7;
    int32_t chargerType = GetInt32FromData(data, size, offset) % 6;

    WorkInfo srcInfo;
    srcInfo.SetWorkId(workId);
    srcInfo.SetElement("com.example.fuzz", "FuzzAbility");
    srcInfo.RequestPersisted(true);
    srcInfo.RequestNetworkType(WorkCondition::Network(networkType));
    srcInfo.RequestChargerType(isCharging, WorkCondition::Charger(chargerType));
    srcInfo.RequestBatteryLevel(batteryLevel);
    constexpr int32_t mockUid = 1000;
    constexpr int32_t mockAppIndex = 1;
    constexpr int32_t mockEarliestStartTime = 100;
    constexpr int32_t mockDeepIdleTime = 300;

    srcInfo.RequestBatteryStatus(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY);
    srcInfo.RequestStorageLevel(WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY);
    srcInfo.RequestRepeatCycle(timeInterval, cycle);
    srcInfo.RequestBaseTime(static_cast<time_t>(baseTime));
    srcInfo.RequestDeepIdle(isDeepIdle);
    
    FuzzParseFromJsonRoundTripPart2(srcInfo);
}

void FuzzParseFromJsonEdgeCasesPart2()
{
    constexpr int32_t mockWorkId2 = 2;
    constexpr int32_t mockWorkId3 = 3;
    constexpr int32_t mockWorkId4 = 4;
    constexpr int32_t mockWorkId5 = 5;
    constexpr int32_t mockTimer1200 = 1200;
    constexpr int32_t mockTimer600 = 600;
    constexpr int32_t mockCycle5 = 5;
    constexpr int32_t mockBaseTime1M = 1000000;
    constexpr int32_t mockWrongCallBySystemApp = 123;

    WorkInfo workInfo7;
    nlohmann::json withEmptyParams;
    withEmptyParams["workId"] = mockWorkId2;
    withEmptyParams["bundleName"] = "com.example";
    withEmptyParams["abilityName"] = "Ability";
    withEmptyParams["parameters"] = nlohmann::json::object();
    withEmptyParams["parametersType"] = nlohmann::json::object();
    workInfo7.ParseFromJson(withEmptyParams);

    WorkInfo workInfo8;
    nlohmann::json timerRepeat;
    timerRepeat["workId"] = mockWorkId3;
    timerRepeat["bundleName"] = "com.example";
    timerRepeat["abilityName"] = "Ability";
    nlohmann::json conds;
    conds["timer"] = mockTimer1200;
    conds["repeat"] = true;
    timerRepeat["conditions"] = conds;
    workInfo8.ParseFromJson(timerRepeat);

    WorkInfo workInfo9;
    nlohmann::json timerCycle;
    timerCycle["workId"] = mockWorkId4;
    timerCycle["bundleName"] = "com.example";
    timerCycle["abilityName"] = "Ability";
    nlohmann::json conds2;
    conds2["timer"] = mockTimer600;
    conds2["repeat"] = false;
    conds2["cycle"] = mockCycle5;
    conds2["baseTime"] = mockBaseTime1M;
    timerCycle["conditions"] = conds2;
    workInfo9.ParseFromJson(timerCycle);

    WorkInfo workInfo10;
    nlohmann::json boolPropWrong;
    boolPropWrong["workId"] = mockWorkId5;
    boolPropWrong["bundleName"] = "com.example";
    boolPropWrong["abilityName"] = "Ability";
    boolPropWrong["persisted"] = "notABool";
    boolPropWrong["callBySystemApp"] = mockWrongCallBySystemApp;
    workInfo10.ParseFromJson(boolPropWrong);
}

void FuzzParseFromJsonEdgeCases(const uint8_t* data, size_t size)
{
    WorkInfo workInfo1;
    nlohmann::json emptyJson;
    workInfo1.ParseFromJson(emptyJson);

    WorkInfo workInfo2;
    nlohmann::json nullJson = nullptr;
    workInfo2.ParseFromJson(nullJson);

    WorkInfo workInfo3;
    nlohmann::json noWorkId;
    noWorkId["bundleName"] = "test";
    workInfo3.ParseFromJson(noWorkId);

    WorkInfo workInfo4;
    nlohmann::json badWorkId;
    badWorkId["workId"] = "notAnInt";
    workInfo4.ParseFromJson(badWorkId);

    WorkInfo workInfo5;
    nlohmann::json noBundleName;
    noBundleName["workId"] = 1;
    workInfo5.ParseFromJson(noBundleName);

    WorkInfo workInfo6;
    nlohmann::json minimal;
    minimal["workId"] = 1;
    minimal["bundleName"] = "com.example";
    minimal["abilityName"] = "Ability";
    workInfo6.ParseFromJson(minimal);

    FuzzParseFromJsonEdgeCasesPart2();
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    InitWorkSchedulerService();

    FuzzParseFromJsonBasic(data, size);
    FuzzParseFromJsonWithAllConditions(data, size);
    FuzzParseFromJsonSaPath(data, size);
    FuzzParseFromJsonRoundTrip(data, size);
    FuzzParseFromJsonEdgeCases(data, size);

    if (workSchedulerService_) {
        workSchedulerService_->OnStop();
    }
    return true;
}
} // namespace WorkScheduler
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    OHOS::WorkScheduler::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
