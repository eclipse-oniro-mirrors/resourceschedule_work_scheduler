/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

import {AsyncCallback} from './basic';

declare namespace workScheduler {
    export enum NetworkType {
        NETWORK_TYPE_ANY = 0,
        NETWORK_TYPE_MOBILE,
        NETWORK_TYPE_WIFI,
        NETWORK_TYPE_BLUETOOTH,
        NETWORK_TYPE_WIFI_P2P,
        NETWORK_TYPE_ETHERNET
    }
    export enum ChargingType {
        CHARGING_PLUGGED_ANY = 0,
        CHARGING_PLUGGED_AC,
        CHARGING_PLUGGED_USB,
        CHARGING_PLUGGED_WIRELESS
    }
    export enum BatteryStatus {
        BATTERY_STATUS_LOW = 0,
        BATTERY_STATUS_OKAY,
        BATTERY_STATUS_LOW_OR_OKAY
    }
    export enum StorageRequest {
        STORAGE_LEVEL_LOW = 0,
        STORAGE_LEVEL_OKAY,
        STORAGE_LEVEL_LOW_OR_OKAY
    }
    export interface WorkInfo {
        readonly workId: number;
        networkType: NetworkType;
        isDeepIdle: boolean;
        idleWaitTime: number;
        isCharging: boolean;
        chargingType: ChargingType;
        batteryLevel: number;
        batteryStatus: BatteryStatus;
        storageRequest: StorageRequest;
        isRepeat: boolean;
        isPersisted: boolean;
        repeatCycleTime: number;
        repeatCount: number;
        bundleName: string;
        abilityName: string;
    }
    function startWork(work: WorkInfo): boolean;
    function stopWork(work: WorkInfo, needCancel?: boolean): boolean;
    function getWorkStatus(workId: number, callback: AsyncCallback<WorkInfo>): void;
    function getWorkStatus(workId: number): Promise<WorkInfo>;
    function obtainAllWorks(callback: AsyncCallback<void>): Array<WorkInfo>;
    function obtainAllWorks(): Promise<Array<WorkInfo>>;
    function stopAndClearWorks(): boolean;
    function isLastWorkTimeOut(workId: number, callback: AsyncCallback<void>): boolean;
    function isLastWorkTimeOut(workId: number): Promise<boolean>;
}
export default workScheduler;