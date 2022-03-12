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

#include <fstream>
#include <iostream>

#include "battery_srv_client.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "net_supplier_info.h"
#include "want.h"
#include "work_sched_common.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
    static const int TYPE_PARAM = 1;
    static const int DETAIL_PARAM = 2;
    static const std::string NETWORK = "network";
    static const std::string EV_NETWORK_TYPE_WIFI = "wifi";
    static const std::string EV_NETWORK_TYPE_DISCONNECT = "disconnect";
    static const std::string CHARGING = "charging";
    static const std::string EV_CHARGING_TYPE_USB = "usb";
    static const std::string EV_CHARGING_TYPE_AC = "ac";
    static const std::string EV_CHARGING_TYPE_WIRELESS = "wireless";
    static const std::string EV_CHARGING_TYPE_NONE = "none";
    static const std::string BATTERY_STATUS = "batteryStatus";
    static const std::string BATTERY_LEVEL = "batteryLevel";
    static const std::string STORAGE = "storage";
    static const std::string EV_STORAGE_LOW = "low";
    static const std::string EV_STORAGE_OKAY = "ok";
    static const std::string HELP = "help";
    static const std::string HELP_MSG =
        "usage: workscheduler dump -E [<options>]\n"
        "options list:\n"
        "  help                   help menu\n"
        "  network wifi           publish COMMON_EVENT_CONNECTIVITY_CHANGE event(wifi)\n"
        "  network disconnect     publish COMMON_EVENT_CONNECTIVITY_CHANGE event(disconnect)\n"
        "  charging usb           publish usb charging event\n"
        "  charging ac            publish ac charging event\n"
        "  charging wireless      publish wireless charging event\n"
        "  charging none          publish unplugged event\n"
        "  storage low            publish COMMON_EVENT_DEVICE_STORAGE_LOW event\n"
        "  storage ok             publish COMMON_EVENT_DEVICE_STORAGE_OKAY event\n"
        "  batteryStatus low      publish COMMON_EVENT_BATTERY_LOW\n"
        "  batteryStatus ok       publish COMMON_EVENT_BATTERY_OKAY\n"
        "  batteryLevel (number)  publish COMMON_EVENT_BATTERY_CHANGED\n";
}

void EventPublisher::PublishEvent(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo)
{
    if (dumpOption[TYPE_PARAM] == NETWORK) {
        PublishNetworkEvent(dumpOption, dumpInfo);
    } else if (dumpOption[TYPE_PARAM] == CHARGING) {
        PublishChargingEvent(dumpOption, dumpInfo);
    } else if (dumpOption[TYPE_PARAM] == STORAGE) {
        PublishStorageEvent(dumpOption, dumpInfo);
    } else if (dumpOption[TYPE_PARAM] == BATTERY_STATUS) {
        PublishbatteryStatusEvent(dumpOption, dumpInfo);
    } else if (dumpOption[TYPE_PARAM] == BATTERY_LEVEL) {
        PublishbatteryLevelEvent(dumpOption, dumpInfo);
    } else if (dumpOption[TYPE_PARAM] == HELP) {
        dumpInfo.push_back(HELP_MSG);
    } else {
        dumpInfo.push_back(std::string("dump -E need right param."));
        dumpInfo.push_back(HELP_MSG);
    }
}

void EventPublisher::PublishNetworkEvent(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo)
{
    EventFwk::Want want;
    if (dumpOption[DETAIL_PARAM] == EV_NETWORK_TYPE_WIFI) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
        want.SetParam("NetType", 1);
        dumpInfo.push_back(std::string("publishing COMMON_EVENT_WIFI_CONN_STATE"));
        EventFwk::CommonEventData data;
        data.SetWant(want);
        data.SetCode(NetManagerStandard::NetConnState::NET_CONN_STATE_CONNECTED);
        bool isSuccess = EventFwk::CommonEventManager::PublishCommonEvent(data);
        dumpInfo.push_back(std::string("publish result: " + std::to_string(isSuccess)));
    } else if (dumpOption[DETAIL_PARAM] == EV_NETWORK_TYPE_DISCONNECT) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CONNECTIVITY_CHANGE);
        dumpInfo.push_back(std::string("publishing COMMON_EVENT_WIFI_CONN_STATE"));
        EventFwk::CommonEventData data;
        data.SetWant(want);
        data.SetCode(NetManagerStandard::NetConnState::NET_CONN_STATE_DISCONNECTED);
        bool isSuccess = EventFwk::CommonEventManager::PublishCommonEvent(data);
        dumpInfo.push_back(std::string("publish result: " + std::to_string(isSuccess)));
    } else {
        dumpInfo.push_back(std::string("dump need right param."));
        return;
    }
}

void EventPublisher::PublishChargingEvent(const std::vector<std::string> &dumpOption,
    std::vector<std::string> &dumpInfo)
{
    EventFwk::Want want;
    EventFwk::CommonEventData data;
    if (dumpOption[DETAIL_PARAM] == EV_CHARGING_TYPE_AC) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        data.SetWant(want);
        data.SetCode(PowerMgr::BatteryInfo::COMMON_EVENT_CODE_PLUGGED_TYPE);
        data.SetData(std::to_string(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_AC)));
    } else if (dumpOption[DETAIL_PARAM] == EV_CHARGING_TYPE_USB) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        data.SetWant(want);
        data.SetCode(PowerMgr::BatteryInfo::COMMON_EVENT_CODE_PLUGGED_TYPE);
        data.SetData(std::to_string(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB)));
    } else if (dumpOption[DETAIL_PARAM] == EV_CHARGING_TYPE_WIRELESS) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
        data.SetWant(want);
        data.SetCode(PowerMgr::BatteryInfo::COMMON_EVENT_CODE_PLUGGED_TYPE);
        data.SetData(std::to_string(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS)));
    } else if (dumpOption[DETAIL_PARAM] == EV_CHARGING_TYPE_NONE) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
        data.SetWant(want);
        data.SetCode(PowerMgr::BatteryInfo::COMMON_EVENT_CODE_PLUGGED_TYPE);
        data.SetData(std::to_string(static_cast<uint32_t>(PowerMgr::BatteryPluggedType::PLUGGED_TYPE_NONE)));
    } else {
        dumpInfo.push_back(std::string("dump need right param."));
        return;
    }
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    bool ret = EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo);
    dumpInfo.push_back(std::string("publish charging event ret: ") + (ret ? "true" : "false"));
}

void EventPublisher::PublishStorageEvent(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo)
{
    EventFwk::Want want;
    if (dumpOption[DETAIL_PARAM] == EV_STORAGE_LOW) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_LOW);
        dumpInfo.push_back(std::string("publishing COMMON_EVENT_DEVICE_STORAGE_LOW"));
    } else if (dumpOption[DETAIL_PARAM] == EV_STORAGE_OKAY) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_STORAGE_OK);
        dumpInfo.push_back(std::string("publishing COMMON_EVENT_DEVICE_STORAGE_OKAY"));
    } else {
        dumpInfo.push_back(std::string("dump need right param."));
        return;
    }
    EventFwk::CommonEventData data;
    data.SetWant(want);
    bool isSuccess = EventFwk::CommonEventManager::PublishCommonEvent(data);
    dumpInfo.push_back(std::string("publish result: ") + std::to_string(isSuccess));
}

void EventPublisher::PublishbatteryStatusEvent(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo)
{
    EventFwk::Want want;
    if (dumpOption[DETAIL_PARAM] == EV_STORAGE_LOW) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW);
        dumpInfo.push_back(std::string("publishing COMMON_EVENT_BATTERY_LOW"));
        EventFwk::CommonEventData data;
        data.SetWant(want);
        data.SetCode(PowerMgr::BatteryInfo::COMMON_EVENT_CODE_CAPACITY);
        data.SetData("0");
        bool isSuccess = EventFwk::CommonEventManager::PublishCommonEvent(data);
        dumpInfo.push_back(std::string("publish result: ") + std::to_string(isSuccess));
    } else if (dumpOption[DETAIL_PARAM] == EV_STORAGE_OKAY) {
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY);
        dumpInfo.push_back(std::string("publishing COMMON_EVENT_BATTERY_OKAY"));
        EventFwk::CommonEventData data;
        data.SetWant(want);
        data.SetCode(PowerMgr::BatteryInfo::COMMON_EVENT_CODE_CAPACITY);
        data.SetData("100");
        bool isSuccess = EventFwk::CommonEventManager::PublishCommonEvent(data);
        dumpInfo.push_back(std::string("publish result: ") + std::to_string(isSuccess));
    } else {
        dumpInfo.push_back(std::string("dump need right param."));
        return;
    }
}

void EventPublisher::PublishbatteryLevelEvent(const std::vector<std::string> &dumpOption, std::vector<std::string> &dumpInfo)
{
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    dumpInfo.push_back(std::string("publishing COMMON_EVENT_BATTERY_CHANGED"));
    EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(PowerMgr::BatteryInfo::COMMON_EVENT_CODE_CAPACITY);
    data.SetData(dumpOption[DETAIL_PARAM]);
    bool isSuccess = EventFwk::CommonEventManager::PublishCommonEvent(data);
    dumpInfo.push_back(std::string("publish result: ") + std::to_string(isSuccess));
}
}
}