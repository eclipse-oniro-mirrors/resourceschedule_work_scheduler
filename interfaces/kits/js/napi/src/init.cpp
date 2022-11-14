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

#include "init.h"

#include <cstddef>

#include "get_work_status.h"
#include "is_last_work_time_out.h"
#include "start_work.h"
#include "stop_and_clear_works.h"
#include "stop_work.h"
#include "obtain_all_works.h"
#include "work_condition.h"
#include "work_sched_hilog.h"
#include "common.h"

namespace OHOS {
namespace WorkScheduler {
EXTERN_C_START

static const uint8_t ARG_FIRST = 1;

napi_ref g_networkTypeConstructor = nullptr;
napi_ref g_chargingTypeConstructor = nullptr;
napi_ref g_batteryStatusConstructor = nullptr;
napi_ref g_storageRequestConstructor = nullptr;

napi_value InitApi(napi_env env, napi_value exports)
{
    WS_HILOGD("Work scheduler napi init.");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("startWork", StartWork),
        DECLARE_NAPI_FUNCTION("stopWork", StopWork),
        DECLARE_NAPI_FUNCTION("getWorkStatus", GetWorkStatus),
        DECLARE_NAPI_FUNCTION("obtainAllWorks", ObtainAllWorks),
        DECLARE_NAPI_FUNCTION("stopAndClearWorks", StopAndClearWorks),
        DECLARE_NAPI_FUNCTION("isLastWorkTimeOut", IsLastWorkTimeOut),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    // Init constant value.
    InitNetworkType(env, exports);
    InitChargingType(env, exports);
    InitBatteryStatus(env, exports);
    InitStorageRequest(env, exports);

    return exports;
}

napi_value InitNetworkType(napi_env env, napi_value exports)
{
    napi_value networkTypeAny;
    napi_value networkTypeMobile;
    napi_value networkTypeWifi;
    napi_value networkTypeBluetooth;
    napi_value networkTypeWifiP2p;
    napi_value networkTypeEthernet;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Network::NETWORK_TYPE_ANY),
        &networkTypeAny);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Network::NETWORK_TYPE_MOBILE),
        &networkTypeMobile);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Network::NETWORK_TYPE_WIFI),
        &networkTypeWifi);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Network::NETWORK_TYPE_BLUETOOTH),
        &networkTypeBluetooth);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Network::NETWORK_TYPE_WIFI_P2P),
        &networkTypeWifiP2p);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Network::NETWORK_TYPE_ETHERNET),
        &networkTypeEthernet);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_TYPE_ANY", networkTypeAny),
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_TYPE_MOBILE", networkTypeMobile),
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_TYPE_WIFI", networkTypeWifi),
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_TYPE_BLUETOOTH", networkTypeBluetooth),
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_TYPE_WIFI_P2P", networkTypeWifiP2p),
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_TYPE_ETHERNET", networkTypeEthernet),
    };

    napi_value result = nullptr;
    napi_define_class(env, "NetworkType", NAPI_AUTO_LENGTH, EnumNetworkTypeConstructor,
        nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &g_networkTypeConstructor);
    napi_set_named_property(env, exports, "NetworkType", result);
    return exports;
}

napi_value EnumNetworkTypeConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value args[ARG_FIRST] = {nullptr};
    napi_value res = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &res, &data);
    if (status != napi_ok) {
        return nullptr;
    }

    return res;
}

napi_value InitChargingType(napi_env env, napi_value exports)
{
    napi_value chargingPluggedAny;
    napi_value chargingPluggedAc;
    napi_value chargingPluggedUsb;
    napi_value chargingPluggedWireless;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Charger::CHARGING_PLUGGED_ANY),
        &chargingPluggedAny);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Charger::CHARGING_PLUGGED_AC),
        &chargingPluggedAc);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Charger::CHARGING_PLUGGED_USB),
        &chargingPluggedUsb);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Charger::CHARGING_PLUGGED_WIRELESS),
        &chargingPluggedWireless);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("CHARGING_PLUGGED_ANY", chargingPluggedAny),
        DECLARE_NAPI_STATIC_PROPERTY("CHARGING_PLUGGED_AC", chargingPluggedAc),
        DECLARE_NAPI_STATIC_PROPERTY("CHARGING_PLUGGED_USB", chargingPluggedUsb),
        DECLARE_NAPI_STATIC_PROPERTY("CHARGING_PLUGGED_WIRELESS", chargingPluggedWireless),
    };

    napi_value result = nullptr;
    napi_define_class(env, "ChargingType", NAPI_AUTO_LENGTH, EnumChargingTypeConstructor,
        nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &g_chargingTypeConstructor);
    napi_set_named_property(env, exports, "ChargingType", result);
    return exports;
}

napi_value EnumChargingTypeConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value args[ARG_FIRST] = {nullptr};
    napi_value res = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &res, &data);
    if (status != napi_ok) {
        return nullptr;
    }

    return res;
}

napi_value InitBatteryStatus(napi_env env, napi_value exports)
{
    napi_value batteryStatusLow;
    napi_value batteryStatusOkay;
    napi_value batteryStatusLowOrOkay;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW),
        &batteryStatusLow);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::BatteryStatus::BATTERY_STATUS_OKAY),
        &batteryStatusOkay);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY),
        &batteryStatusLowOrOkay);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("BATTERY_STATUS_LOW", batteryStatusLow),
        DECLARE_NAPI_STATIC_PROPERTY("BATTERY_STATUS_OKAY", batteryStatusOkay),
        DECLARE_NAPI_STATIC_PROPERTY("BATTERY_STATUS_LOW_OR_OKAY", batteryStatusLowOrOkay),
    };

    napi_value result = nullptr;
    napi_define_class(env, "BatteryStatus", NAPI_AUTO_LENGTH, EnumBatteryStatusConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &g_batteryStatusConstructor);
    napi_set_named_property(env, exports, "BatteryStatus", result);
    return exports;
}

napi_value EnumBatteryStatusConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value args[ARG_FIRST] = {nullptr};
    napi_value res = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &res, &data);
    if (status != napi_ok) {
        return nullptr;
    }
    return res;
}

napi_value InitStorageRequest(napi_env env, napi_value exports)
{
    napi_value storageLeveLow;
    napi_value storageLevelOkay;
    napi_value storageLevelLowOrOkay;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Storage::STORAGE_LEVEL_LOW),
        &storageLeveLow);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Storage::STORAGE_LEVEL_OKAY),
        &storageLevelOkay);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY),
        &storageLevelLowOrOkay);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("STORAGE_LEVEL_LOW", storageLeveLow),
        DECLARE_NAPI_STATIC_PROPERTY("STORAGE_LEVEL_OKAY", storageLevelOkay),
        DECLARE_NAPI_STATIC_PROPERTY("STORAGE_LEVEL_LOW_OR_OKAY", storageLevelLowOrOkay),
    };

    napi_value result = nullptr;
    napi_define_class(env, "StorageRequest", NAPI_AUTO_LENGTH, EnumStorageRequestConstructor,
        nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &g_storageRequestConstructor);
    napi_set_named_property(env, exports, "StorageRequest", result);
    return exports;
}

napi_value EnumStorageRequestConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value args[ARG_FIRST] = {nullptr};
    napi_value res = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &res, &data);
    if (status != napi_ok) {
        return nullptr;
    }

    return res;
}

/*
 * Module register function
 */
__attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_apiModule);
}
EXTERN_C_END
} // namespace WorkScheduler
} // namespace OHOS