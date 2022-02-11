/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
import workScheduler from '@ohos.workScheduler'

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe("WorkSchedulerJsTest", function () {
    beforeAll(function() {
        /*
         * @tc.setup: setup invoked before all testcases
         */
         console.info('beforeAll caled')
    })

    afterAll(function() {
        /*
         * @tc.teardown: teardown invoked after all testcases
         */
         console.info('afterAll caled')
    })

    beforeEach(function() {
        /*
         * @tc.setup: setup invoked before each testcases
         */
         console.info('beforeEach caled')
    })

    afterEach(function() {
        /*
         * @tc.teardown: teardown invoked after each testcases
         */
         console.info('afterEach caled')
    })

    /*
     * @tc.name: WorkSchedulerJsTest001
     * @tc.desc: test work scheduler work id < 0
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89E AR000GH89F AR000GH89G
     */
    it("WorkSchedulerJsTest001", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest001---------------------------');
        let workInfo = {
            workId: -1,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName"
        }
        var res = workScheduler.startWork(workInfo);
        expect(res).assertEqual(false)
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest002
     * @tc.desc: test work scheduler work id = 0
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89H AR000GH89I AR000GH899
     */
    it("WorkSchedulerJsTest002", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest002---------------------------');
        let workInfo = {
            workId: 0,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        var res = workScheduler.startWork(workInfo);
        expect(res).assertEqual(true)
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest003
     * @tc.desc: test work scheduler without bundle name.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89A AR000GH89B AR000GH89C
     */
    it("WorkSchedulerJsTest003", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest003---------------------------');
        let workInfo = {
            workId: 3,
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        var res = workScheduler.startWork(workInfo);
        expect(res).assertEqual(false)
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest004
     * @tc.desc: test work scheduler without ability name.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89A AR000GH89B AR000GH89C
     */
    it("WorkSchedulerJsTest004", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest004---------------------------');
        let workInfo = {
            workId: 4,
            bundleName: "com.example.myapplication",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        var res = workScheduler.startWork(workInfo);
        expect(res).assertEqual(false)
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest005
     * @tc.desc: test work scheduler without conditions.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89A AR000GH89B AR000GH89C
     */
    it("WorkSchedulerJsTest005", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest005---------------------------');
        let workInfo = {
            workId: 5,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName"
        }
        var res = workScheduler.startWork(workInfo);
        expect(res).assertEqual(false)
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest006
     * @tc.desc: test work scheduler success.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89D
     */
    it("WorkSchedulerJsTest006", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest006---------------------------');
        let workInfo = {
            workId: 6,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        var res = workScheduler.startWork(workInfo);
        expect(res).assertEqual(true)
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest007
     * @tc.desc: test stopWork.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89D
     */
    it("WorkSchedulerJsTest007", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest007---------------------------');
        let workInfo = {
            workId: 7,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        var res = workScheduler.startWork(workInfo);
        if (res == true) {
            var stopRes = workScheduler.stopWork(workInfo, false);
            expect(stopRes).assertEqual(true)
        } else {
            expect(false).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest008
     * @tc.desc: test stopWork.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89E AR000GH89F AR000GH89G
     */
    it("WorkSchedulerJsTest008", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest008---------------------------');
        let workInfo = {
            workId: 8,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        var res = workScheduler.startWork(workInfo);
        if (res == true) {
            var stopRes = workScheduler.stopWork(workInfo, false);
            expect(stopRes).assertEqual(true)
        } else {
            expect(false).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest009
     * @tc.desc: test getWorkStatus callback.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89H AR000GH89I AR000GH899
     */
    it("WorkSchedulerJsTest009", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest009---------------------------');
        let workInfo = {
            workId: 9,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        var res = workScheduler.startWork(workInfo);
        if (res == false) {
            expect(false).assertEqual(true)
            done();
        }
        workScheduler.getWorkStatus(9, (err, res) => {
            if (err) {
                expect(false).assertEqual(true)
            } else {
                for (let item in res) {
                    console.info('WORK_SCHEDULER getWorkStatuscallback success,' + item + ' is:' + res[item]);
                }
                expect(true).assertEqual(true)
            }
        });
        setTimeout(()=>{
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest010
     * @tc.desc: test getWorkStatus promise.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89H AR000GH89I AR000GH899
     */
    it("WorkSchedulerJsTest010", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest010---------------------------');
        let workInfo = {
            workId: 10,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            batteryLevel: 15
        }
        var res = workScheduler.startWork(workInfo);
        if (res == false) {
            expect(false).assertEqual(true)
            done();
        }

        workScheduler.getWorkStatus(10).then((res) => {
            for (let item in res) {
                console.info('WORK_SCHEDULER getWorkStatuscallback success,' + item + ' is:' + res[item]);
            }
            expect(true).assertEqual(true)
        }).catch((err) => {
            expect(false).assertEqual(true)
        })

        setTimeout(()=>{
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest011
     * @tc.desc: test obtainAllWorks callback.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89E AR000GH89F AR000GH89G
     */
    it("WorkSchedulerJsTest011", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest011---------------------------');
        let workInfo = {
            workId: 11,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        var res = workScheduler.startWork(workInfo);
        if (res == false) {
            expect(false).assertEqual(true)
            done();
        }

        workScheduler.obtainAllWorks((err, res) =>{
            if (err) {
                expect(false).assertEqual(true)
            } else {
                console.info('WORK_SCHEDULER obtainAllWorks callback success, data is:' + JSON.stringify(res));
                expect(true).assertEqual(true)
            }
         });

        setTimeout(()=>{
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest012
     * @tc.desc: test obtainAllWorks promise.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89E AR000GH89F AR000GH89G
     */
    it("WorkSchedulerJsTest012", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest012---------------------------');
        let workInfo = {
            workId: 12,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            batteryLevel: 15
        }
        var res = workScheduler.startWork(workInfo);
        if (res == false) {
            expect(false).assertEqual(true)
            done();
        }

        workScheduler.obtainAllWorks().then((res) => {
            console.info('WORK_SCHEDULER obtainAllWorks promise success, data is:' + JSON.stringify(res));
            expect(true).assertEqual(true)
        }).catch((err) => {
            console.info('workschedulerLog obtainAllWorks promise failed, because:' + err.data);
            expect(false).assertEqual(true)
        })

        setTimeout(()=>{
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest013
     * @tc.desc: test stopAndClearWorks.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89E AR000GH89F AR000GH89G
     */
    it("WorkSchedulerJsTest013", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest013---------------------------');
        let workInfo = {
            workId: 13,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        var res = workScheduler.startWork(workInfo);
        if (res == true) {
            var stopRes = workScheduler.stopAndClearWorks();
            expect(stopRes).assertEqual(true)
        } else {
            expect(false).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest014
     * @tc.desc: test isLastWorkTimeOut callback.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89D
     */
    it("WorkSchedulerJsTest014", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest014---------------------------');
        workScheduler.isLastWorkTimeOut(14, (err, res) =>{
            if (err) {
                expect(false).assertEqual(true)
            } else {
                console.info('WORK_SCHEDULER isLastWorkTimeOut callback success, data is:' + res);
                expect(true).assertEqual(true)
            }
        });
        setTimeout(()=>{
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest015
     * @tc.desc: test isLastWorkTimeOut promise.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89D
     */
    it("WorkSchedulerJsTest015", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest015---------------------------');
        workScheduler.isLastWorkTimeOut(15)
            .then(res => {
                console.info('WORK_SCHEDULER isLastWorkTimeOut promise success, data is:' + res);
                expect(true).assertEqual(true)
            })
            .catch(err =>  {
                expect(false).assertEqual(true)
        });
        setTimeout(()=>{
            done();
        }, 500);
    })
})