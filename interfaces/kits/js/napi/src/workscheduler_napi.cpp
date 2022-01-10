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

#include "workscheduler_napi.h"
#include "work_condition.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {

EXTERN_C_START

static const uint8_t ARG_FIRST = 1;

napi_ref networkTypeConstructor_ = nullptr;
napi_ref chargingTypeConstructor_ = nullptr;
napi_ref batteryStatusConstructor_ = nullptr;
napi_ref storageRequestConstructor_ = nullptr;

napi_value Init(napi_env env, napi_value exports)
{
    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);

    // Init constant value.
    InitNetworkType(env, exports);
    InitChargingType(env, exports);
    InitBatteryStatus(env, exports);
    InitStorageRequest(env, exports);

    return exports;
}

napi_value InitNetworkType(napi_env env, napi_value exports)
{
    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    napi_value network_type_any;
    napi_value network_type_mobile;
    napi_value network_type_wifi;
    napi_value network_type_bluetooth;
    napi_value network_type_wifi_p2p;
    napi_value network_type_ethernet;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Network::NETWORK_TYPE_ANY), &network_type_any);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Network::NETWORK_TYPE_MOBILE), &network_type_mobile);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Network::NETWORK_TYPE_WIFI), &network_type_wifi);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Network::NETWORK_TYPE_BLUETOOTH), &network_type_bluetooth);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Network::NETWORK_TYPE_WIFI_P2P), &network_type_wifi_p2p);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Network::NETWORK_TYPE_ETHERNET), &network_type_ethernet);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_TYPE_ANY", network_type_any),
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_TYPE_MOBILE", network_type_mobile),
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_TYPE_WIFI", network_type_wifi),
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_TYPE_BLUETOOTH", network_type_bluetooth),
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_TYPE_WIFI_P2P", network_type_wifi_p2p),
        DECLARE_NAPI_STATIC_PROPERTY("NETWORK_TYPE_ETHERNET", network_type_ethernet),
    };

    napi_value result = nullptr;
    napi_define_class(env, "NetworkType", NAPI_AUTO_LENGTH, EnumNetworkTypeConstructor, nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &networkTypeConstructor_);
    napi_define_class(env, "NetworkType", NAPI_AUTO_LENGTH, EnumNetworkTypeConstructor, nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &networkTypeConstructor_);
    napi_set_named_property(env, exports, "NetworkType", result);
    return exports;

    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: return", __func__);
}

napi_value EnumNetworkTypeConstructor(napi_env env, napi_callback_info info)
{
    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    size_t argc = 0;
    napi_value args[ARG_FIRST] = { 0 };
    napi_value jsthis = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);

    WS_HILOGD(MODULE_JS_NAPI, "EnumNetworkTypeConstructor %{public}d", status);
    if (status != napi_ok) {
        return nullptr;
    }
    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: return", __func__);
    return jsthis;
}

napi_value InitChargingType(napi_env env, napi_value exports)
{
    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    napi_value charging_plugged_any;
    napi_value charging_plugged_ac;
    napi_value charging_plugged_usb;
    napi_value charging_plugged_wireless;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Charger::CHARGER_PLUGGED_ANY), &charging_plugged_any);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Charger::CHARGER_PLUGGED_AC), &charging_plugged_ac);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Charger::CHARGER_PLUGGED_USB), &charging_plugged_usb);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Charger::CHARGER_PLUGGED_WIRELESS), &charging_plugged_wireless);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("CHARGING_PLUGGED_ANY", charging_plugged_any),
        DECLARE_NAPI_STATIC_PROPERTY("CHARGING_PLUGGED_AC", charging_plugged_ac),
        DECLARE_NAPI_STATIC_PROPERTY("CHARGING_PLUGGED_USB", charging_plugged_usb),
        DECLARE_NAPI_STATIC_PROPERTY("CHARGING_PLUGGED_WIRELESS", charging_plugged_wireless),
    };

    napi_value result = nullptr;
    napi_define_class(env, "ChargingType", NAPI_AUTO_LENGTH, EnumChargingTypeConstructor, nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &chargingTypeConstructor_);
    napi_set_named_property(env, exports, "ChargingType", result);
    return exports;
}

napi_value EnumChargingTypeConstructor(napi_env env, napi_callback_info info)
{
    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    size_t argc = 0;
    napi_value args[ARG_FIRST] = { 0 };
    napi_value jsthis = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);

    WS_HILOGD(MODULE_JS_NAPI, "EnumChargingTypeConstructor %{public}d", status);
    if (status != napi_ok) {
        return nullptr;
    }
    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: return", __func__);
    return jsthis;
}

napi_value InitBatteryStatus(napi_env env, napi_value exports)
{
    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    napi_value battery_status_low;
    napi_value battery_status_okay;
    napi_value battery_status_low_or_okay;
    int32_t refCount = 1;

    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW), &battery_status_low);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::BatteryStatus::BATTERY_STATUS_OKAY), &battery_status_okay);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::BatteryStatus::BATTERY_STATUS_LOW_OR_OKAY), &battery_status_low_or_okay);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("BATTERY_STATUS_LOW", battery_status_low),
        DECLARE_NAPI_STATIC_PROPERTY("BATTERY_STATUS_OKAY", battery_status_okay),
        DECLARE_NAPI_STATIC_PROPERTY("BATTERY_STATUS_LOW_OR_OKAY", battery_status_low_or_okay),
    };

    napi_value result = nullptr;
    napi_define_class(env, "BatteryStatus", NAPI_AUTO_LENGTH, EnumBatteryStatusConstructor, nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &batteryStatusConstructor_);
    napi_set_named_property(env, exports, "BatteryStatus", result);
    return exports;
}

napi_value EnumBatteryStatusConstructor(napi_env env, napi_callback_info info)
{
    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    size_t argc = 0;
    napi_value args[ARG_FIRST] = { 0 };
    napi_value jsthis = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);

    WS_HILOGD(MODULE_JS_NAPI, "EnumBatteryStatusConstructor %{public}d", status);
    if (status != napi_ok) {
        return nullptr;
    }
    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: return", __func__);
    return jsthis;
}

napi_value InitStorageRequest(napi_env env, napi_value exports)
{
    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    napi_value storage_level_low;
    napi_value storage_level_okay;
    napi_value storage_level_low_or_okay;
    int32_t refCount = 1;
    
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Storage::STORAGE_LEVEL_LOW), &storage_level_low);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Storage::STORAGE_LEVEL_OKAY), &storage_level_okay);
    napi_create_uint32(env, static_cast<uint32_t>(WorkCondition::Storage::STORAGE_LEVEL_LOW_OR_OKAY), &storage_level_low_or_okay);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("STORAGE_LEVEL_LOW", storage_level_low),
        DECLARE_NAPI_STATIC_PROPERTY("STORAGE_LEVEL_OKAY", storage_level_okay),
        DECLARE_NAPI_STATIC_PROPERTY("STORAGE_LEVEL_LOW_OR_OKAY", storage_level_low_or_okay),
    };

    napi_value result = nullptr;
    napi_define_class(env, "StorageRequest", NAPI_AUTO_LENGTH, EnumStorageRequestConstructor, nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    napi_create_reference(env, result, refCount, &storageRequestConstructor_);
    napi_set_named_property(env, exports, "StorageRequest", result);
    return exports;
}

napi_value EnumStorageRequestConstructor(napi_env env, napi_callback_info info)
{
    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: enter", __func__);
    size_t argc = 0;
    napi_value args[ARG_FIRST] = { 0 };
    napi_value jsthis = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);

    WS_HILOGD(MODULE_JS_NAPI, "EnumStorageRequestConstructor %{public}d", status);
    if (status != napi_ok) {
        return nullptr;
    }
    WS_HILOGD(MODULE_JS_NAPI, "%{public}s: return", __func__);
    return jsthis;
}

/*
 * Module register function
 */
__attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
EXTERN_C_END

}
}