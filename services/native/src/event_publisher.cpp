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

#include "event_publisher.h"

#include "battery_srv_client.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "net_supplier_info.h"
#include "want.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
    static const std::string NETWORK = "network";
    static const std::string EV_NETWORK_TYPE_WIFI = "wifi";
    static const std::string EV_NETWORK_TYPE_DISCONNECT = "disconnect";
    static const std::string CHARGING = "charging";
    static const std::string EV_CHARGING_TYPE_USB = "usb";
    static const std::string EV_CHARGING_TYPE_AC = "ac";
    static const std::string EV_CHARGING_TYPE_WIRELESS = "wireless";
    static const std::string EV_CHARGING_TYPE_NONE = "none";
    static const std::string BATTERY_STATUS = "batteryStatus";
    static const std::string STORAGE = "storage";
    static const std::string EV_STORAGE_LOW = "low";
    static const std::string EV_STORAGE_OKAY = "ok";
}

void EventPublisher::Dump(std::string &result, std::string &eventType, std::string &eventValue)
{
    if (eventType == "event" && eventValue == "info") {
        result.append("event info:\n")
            .append("    network wifi: publish COMMON_EVENT_CONNECTIVITY_CHANGE event(wifi).\n")
            .append("    network disconnect: publish COMMON_EVENT_CONNECTIVITY_CHANGE event(disconnect).\n")
            .append("    charging usb: publish usb charging event\n")
            .append("    charging ac: publish ac charging event\n")
            .append("    charging wireless: publish wireless charging event\n")
            .append("    charging none: publish unplugged event\n")
            .append("    storage low: publish COMMON_EVENT_DEVICE_STORAGE_LOW event\n")
            .append("    storage ok: publish COMMON_EVENT_DEVICE_STORAGE_OKAY event\n")
            .append("    batteryStatus low: publish COMMON_EVENT_BATTERY_LOW\n")
            .append("    batteryStatus ok: publish COMMON_EVENT_BATTERY_OKAY\n");
    } else {
        PublishEvent(result, eventType, eventValue);
    }
}

void EventPublisher::PublishEvent(std::string &result, std::string &eventType, std::string &eventValue)
{
    if (eventType == NETWORK) {
        PublishNetworkEvent(result, eventValue);
    } else if (eventType == CHARGING) {
        PublishChargingEvent(result, eventValue);
    } else if (eventType == STORAGE) {
        PublishStorageEvent(result, eventValue);
    } else if (eventType == BATTERY_STATUS) {
        PublishBatteryStatusEvent(result, eventValue);
    } else {
        result.append(std::string("dump -d need right params."));
    }
}

void EventPublisher::PublishNetworkEvent(std::string &result, std::string &eventValue)
{
    EventFwk::Want want;
    if (eventValue == EV_NETWORK_TYPE_WIFI) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
        want.SetParam("NetType", 1);
        result.append("publishing COMMON_EVENT_WIFI_CONN_STATE\n");
        EventFwk::CommonEventData data;
        data.SetWant(want);
        data.SetCode(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED);
        bool isSuccess = EventFwk::CommonEventManager::PublishCommonEvent(data);
        result.append("publish result: " + std::to_string(isSuccess));
    } else if (eventValue == EV_NETWORK_TYPE_DISCONNECT) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
        result.append("publishing COMMON_EVENT_WIFI_CONN_STATE\n");
        EventFwk::CommonEventData data;
        data.SetWant(want);
        data.SetCode(NetManagerStandard::NetConnState::NET_CONN_STATE_DISCONNECTED);
        bool isSuccess = EventFwk::CommonEventManager::PublishCommonEvent(data);
        result.append("publish result: " + std::to_string(isSuccess));
    } else {
        result.append("dump need right param.");
        return;
    }
}

void EventPublisher::PublishChargingEvent(std::string &result, std::string &eventValue)
{
    EventFwk::Want want;
    EventFwk::CommonEventData data;
    if (eventValue == EV_CHARGING_TYPE_AC) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        data.SetWant(want);
        data.SetCode(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_AC));
    } else if (eventValue == EV_CHARGING_TYPE_USB) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        data.SetWant(want);
        data.SetCode(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB));
    } else if (eventValue == EV_CHARGING_TYPE_WIRELESS) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        data.SetWant(want);
        data.SetCode(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS));
    } else if (eventValue == EV_CHARGING_TYPE_NONE) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
        data.SetWant(want);
        data.SetCode(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_NONE));
    } else {
        result.append("dump need right param.");
        return;
    }
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    if (EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo)) {
        result.append("publish charging event ret: true");
    } else {
        result.append("publish charging event ret: false");
    }
}

void EventPublisher::PublishStorageEvent(std::string &result, std::string &eventValue)
{
    EventFwk::Want want;
    if (eventValue == EV_STORAGE_LOW) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_LOW);
        result.append("publishing COMMON_EVENT_DEVICE_STORAGE_LOW\n");
    } else if (eventValue == EV_STORAGE_OKAY) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_OK);
        result.append("publishing COMMON_EVENT_DEVICE_STORAGE_OKAY\n");
    } else {
        result.append("dump need right param.");
        return;
    }
    EventFwk::CommonEventData data;
    data.SetWant(want);
    bool isSuccess = EventFwk::CommonEventManager::PublishCommonEvent(data);
    result.append("publish result: " + std::to_string(isSuccess));
}

void EventPublisher::PublishBatteryStatusEvent(std::string &result, std::string &eventValue)
{
    EventFwk::Want want;
    if (eventValue == EV_STORAGE_LOW) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW);
        result.append("publishing COMMON_EVENT_BATTERY_LOW\n");
        EventFwk::CommonEventData data;
        data.SetWant(want);
        const int32_t lowCapacity = 0;
        data.SetCode(lowCapacity);
        bool isSuccess = EventFwk::CommonEventManager::PublishCommonEvent(data);
        result.append("publish result: " + std::to_string(isSuccess));
    } else if (eventValue == EV_STORAGE_OKAY) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY);
        result.append("publishing COMMON_EVENT_BATTERY_OKAY\n");
        EventFwk::CommonEventData data;
        data.SetWant(want);
        const int32_t fullCapacity = 100;
        data.SetCode(fullCapacity);
        bool isSuccess = EventFwk::CommonEventManager::PublishCommonEvent(data);
        result.append("publish result: " + std::to_string(isSuccess));
    } else {
        result.append("dump need right param.");
        return;
    }
}
} // namespace WorkScheduler
} // namespace OHOS