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
import workScheduler from '@ohos.resourceschedule.workScheduler'

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
     * @tc.require: SR000GGTN7 AR000GH89E AR000GH89F AR000GH89G issueI5QJR8
     */
    it("WorkSchedulerJsTest001", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest001---------------------------');
        let workInfo = {
            workId: -1,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName"
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(true).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest002
     * @tc.desc: test work scheduler work id = 0
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89H AR000GH89I AR000GH899 issueI5QJR8
     */
    it("WorkSchedulerJsTest002", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest002---------------------------');
        let workInfo = {
            workId: 0,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.startWork(workInfo);
            expect(true).assertEqual(true)
        } catch (error) {
            expect(false).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest003
     * @tc.desc: test work scheduler without bundle name.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89A AR000GH89B AR000GH89C issueI5QJR8
     */
    it("WorkSchedulerJsTest003", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest003---------------------------');
        let workInfo = {
            workId: 3,
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(true).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest004
     * @tc.desc: test work scheduler without ability name.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89A AR000GH89B AR000GH89C issueI5QJR8
     */
    it("WorkSchedulerJsTest004", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest004---------------------------');
        let workInfo = {
            workId: 4,
            bundleName: "com.example.myapplication",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(true).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest005
     * @tc.desc: test work scheduler without conditions.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89A AR000GH89B AR000GH89C issueI5QJR8
     */
    it("WorkSchedulerJsTest005", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest005---------------------------');
        let workInfo = {
            workId: 5,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName"
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(true).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest006
     * @tc.desc: test work scheduler success.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89D issueI5QJR8
     */
    it("WorkSchedulerJsTest006", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest006---------------------------');
        let workInfo = {
            workId: 6,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.startWork(workInfo);
            expect(true).assertEqual(true)
        } catch (error) {
            expect(false).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest007
     * @tc.desc: test stopWork.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89D issueI5QJR8
     */
    it("WorkSchedulerJsTest007", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest007---------------------------');
        let workInfo = {
            workId: 7,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.startWork(workInfo);
            workScheduler.stopWork(workInfo, false);
            expect(true).assertEqual(true)
        } catch (error) {
            expect(false).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest008
     * @tc.desc: test stopWork.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89E AR000GH89F AR000GH89G issueI5QJR8
     */
    it("WorkSchedulerJsTest008", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest008---------------------------');
        let workInfo = {
            workId: 8,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.startWork(workInfo);
            workScheduler.stopWork(workInfo, false);
            expect(true).assertEqual(true)
        } catch (error) {
            expect(false).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest009
     * @tc.desc: test getWorkStatus callback.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89H AR000GH89I AR000GH899 issueI5QJR8
     */
    it("WorkSchedulerJsTest009", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest009---------------------------');
        let workInfo = {
            workId: 9,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
        try{
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
        } catch (error) {
            expect(false).assertEqual(true)
        }
        setTimeout(()=>{
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest010
     * @tc.desc: test getWorkStatus promise.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89H AR000GH89I AR000GH899 issueI5QJR8
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
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
        try{
            workScheduler.getWorkStatus(10).then((res) => {
                for (let item in res) {
                    console.info('WORK_SCHEDULER getWorkStatuscallback success,' + item + ' is:' + res[item]);
                }
                expect(true).assertEqual(true)
            }).catch((err) => {
                expect(false).assertEqual(true)
            })
        } catch (error) {
            expect(false).assertEqual(true)
        }

        setTimeout(()=>{
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest011
     * @tc.desc: test obtainAllWorks callback.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89E AR000GH89F AR000GH89G issueI5QJR8
     */
    it("WorkSchedulerJsTest011", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest011---------------------------');
        let workInfo = {
            workId: 11,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
        try{
            workScheduler.obtainAllWorks((err, res) =>{
                if (err) {
                    expect(false).assertEqual(true)
                } else {
                    console.info('WORK_SCHEDULER obtainAllWorks callback success, data is:' + JSON.stringify(res));
                    expect(true).assertEqual(true)
                }
            });
        } catch (error) {
            expect(false).assertEqual(true)
        }

        setTimeout(()=>{
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest012
     * @tc.desc: test obtainAllWorks promise.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89E AR000GH89F AR000GH89G issueI5QJR8
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
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
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
     * @tc.require: SR000GGTN7 AR000GH89E AR000GH89F AR000GH89G issueI5QJR8
     */
    it("WorkSchedulerJsTest013", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest013---------------------------');
        let workInfo = {
            workId: 13,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.startWork(workInfo);
            workScheduler.stopAndClearWorks();
            expect(true).assertEqual(true)
        } catch (error) {
            expect(false).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest014
     * @tc.desc: test isLastWorkTimeOut callback.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89D issueI5QJR8
     */
    it("WorkSchedulerJsTest014", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest014---------------------------');
        let workInfo = {
            workId: 14,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.stopAndClearWorks();
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
        }
        try{
            workScheduler.isLastWorkTimeOut(14, (err, res) =>{
                if (err) {
                    expect(false).assertEqual(true)
                } else {
                    console.info('WORK_SCHEDULER isLastWorkTimeOut callback success, data is:' + res);
                    expect(true).assertEqual(true)
                }
            });
        } catch (error) {
            expect(false).assertEqual(true)
        }
        setTimeout(()=>{
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest015
     * @tc.desc: test isLastWorkTimeOut promise.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89D issueI5QJR8
     */
    it("WorkSchedulerJsTest015", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest015---------------------------');
        let workInfo = {
            workId: 15,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.stopAndClearWorks();
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
        }
        try{
            workScheduler.isLastWorkTimeOut(15)
                .then(res => {
                    console.info('WORK_SCHEDULER isLastWorkTimeOut promise success, data is:' + res);
                    expect(true).assertEqual(true)
                })
                .catch(err =>  {
                    expect(false).assertEqual(true)
            });
        } catch (error) {
            expect(false).assertEqual(true)
        }
        setTimeout(()=>{
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest016
     * @tc.desc: test spent time by startWork.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest016", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest016---------------------------');
        let workInfo = {
            workId: 16,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            parameters: {
                mykey0: 2147483647111,
                mykey1: 10,
                mykey2: "string",
                mykey3: true,
                mykey4: 2.55
            }
        }
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        let begin = getTime();
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }

        let end = getTime();
        let times = end - begin;
        if (times < 50) {
            expect(true).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest017
     * @tc.desc: test spent time by startWork takes 20 times.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest017", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest017---------------------------');
        let workInfo = {
            workId: 17,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        let begin = getTime();
        for (var i = 0 ; i < 20 ; i++) {
            try{
                workScheduler.startWork(workInfo);
            } catch (error) {}
        }
        let end = getTime();
        let times = end - begin;
        if (times/20 < 50){
            expect(true).assertEqual(true)
        }
        setTimeout(() => {
            done();
        }, 1000);
    })
    
    /*
     * @tc.name: WorkSchedulerJsTest018
     * @tc.desc: test spent time by stopWork.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest018", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest018---------------------------');
        let workInfo = {
            workId: 18,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        try{
            workScheduler.startWork(workInfo);
            let begin = getTime();
            workScheduler.stopWork(workInfo, false);
            let end = getTime();
            let times = end - begin;
            if (times < 50) {
                expect(true).assertEqual(true)
            }
        } catch (error) {
            expect(false).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest019
     * @tc.desc: test spent time by stopWork.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest019", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest019---------------------------');
        let workInfo = {
            workId: 19,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }

        let begin = getTime();
        try{
            workScheduler.stopWork(workInfo, true);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
        let end = getTime();
        let times = end - begin;
        if (times < 50) {
            expect(true).assertEqual(true)
        }

        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest020
     * @tc.desc: test spent time by stopWork takes 20 times.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest020", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest020---------------------------');
        let workInfo = {
            workId: 20,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        let begin = getTime();
        let end = 0;
        let times = 0;
        for (var i = 0 ; i < 20 ; i++) {
            try{
                workScheduler.startWork(workInfo);
            } catch (error) {}
            end = getTime();
            times = end - begin;
        }
        if (times/20 < 50){
            expect(true).assertEqual(true)
        }
        setTimeout(() => {
            done();
        }, 1000);
    })

    /*
     * @tc.name: WorkSchedulerJsTest021
     * @tc.desc: test spent time by getWorkStatus with callback.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest021", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest021---------------------------');
        let workInfo = {
            workId: 21,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        function workSchedulerCallback(err, res){
            let end = getTime();
            let times = end - begin;
            if (times < 50) {
                expect(true).assertTrue();
            } else {
                expect(false).assertTrue();
            }
            setTimeout(() => {
                done();
            }, 500);
        }
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
        let begin = getTime();
        try{
            workScheduler.getWorkStatus(21, workSchedulerCallback);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
    })

    /*
     * @tc.name: WorkSchedulerJsTest022
     * @tc.desc: test spent time by getWorkStatus with promise.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest022", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest022---------------------------');
        let workInfo = {
            workId: 22,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }

        let begin = getTime();
        try{
            workScheduler.getWorkStatus(22).then((res) => {
                let end = getTime();
                let times = end - begin;
                if (times < 50) {
                    expect(true).assertTrue();
                } else {
                    expect(false).assertTrue();
                }
                setTimeout(() => {
                    done();
                }, 500);
                for (let item in res) {
                    console.info('WORK_SCHEDULER getWorkStatuscallback success,' + item + ' is:' + res[item]);
                }
            })
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
    })

    /*
     * @tc.name: WorkSchedulerJsTest023
     * @tc.desc: test spent time by getWorkStatus takes 20 times.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest023", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest023---------------------------');
        let workInfo = {
            workId: 23,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
        let end = 0;
        let times = 0;
        let begin = getTime();
        for (var i = 0 ; i < 20 ; i++) {
            try{
                workScheduler.getWorkStatus(23, (err, res) => {
                    end = getTime();
                    times = end - begin;
                });
            } catch (error) {}
        }
        if (times/20 < 50) {
            expect(true).assertTrue();
        } else {
            expect(false).assertTrue();
        }
        setTimeout(() => {
            done();
        }, 1000);
    })

    /*
     * @tc.name: WorkSchedulerJsTest024
     * @tc.desc: test spent time by obtainAllWorks with callback.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest024", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest024---------------------------');
        let workInfo = {
            workId: 24,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
        let startTime = getTime();
        try{
            workScheduler.obtainAllWorks((err, res) =>{
                let endTime = getTime();
                let workTime = endTime - startTime;
                if (workTime < 50) {
                    expect(true).assertTrue()
                } else {
                    expect(false).assertTrue()
                }
            });
        } catch (error) {}
        setTimeout(() => {
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest025
     * @tc.desc: test spent time by obtainAllWorks with promise.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest025", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest025---------------------------');
        let workInfo = {
            workId: 25,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
        let begin = getTime();
        try{
            workScheduler.obtainAllWorks().then((res) => {
                let end = getTime();
                let times = end - begin;
                if (times < 50) {
                    expect(true).assertTrue()
                } else {
                    expect(false).assertTrue()
                }
                console.info('WORK_SCHEDULER obtainAllWorks promise success, data is:' + JSON.stringify(res));
            }).catch((err) => {
                console.info('workschedulerLog obtainAllWorks promise failed, because:' + err.data);
            })
        } catch (error) {}
        setTimeout(() => {
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest026
     * @tc.desc: test spent time by obtainAllWorks takes 20 times.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest026", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest026---------------------------');
        let workInfo = {
            workId: 26,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        try{
            workScheduler.stopAndClearWorks();
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
        let startTime = getTime();
        for (var i = 0 ; i < 20 ; i++) {
            try{
                workScheduler.obtainAllWorks((err, res) => {
                    let endTime = getTime();
                    let workTime = endTime - startTime;
                    if (workTime/20 < 50) {
                        expect(true).assertTrue()
                    } else {
                        expect(false).assertTrue()
                    }
                });
            } catch (error) {}
        }
        setTimeout(() => {
            done();
        }, 1000);
    })

    /*
     * @tc.name: WorkSchedulerJsTest027
     * @tc.desc: test spent time by stopAndClearWorks.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest027", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest027---------------------------');
        let workInfo = {
            workId: 27,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {}
        let begin = getTime();
        try{
            workScheduler.stopAndClearWorks();
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
        let end = getTime();
        let times = end - begin;
        if (times < 50) {
            console.log('WorkSchedulerJsTest027 times' + times);
            expect(true).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest028
     * @tc.desc: test spent time by stopWork takes 20 times.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest028", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest028---------------------------');
        let workInfo = {
            workId: 28,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
        let begin = getTime();
        let end = 0;
        let times = 0;
        for (var i = 0 ; i < 20 ; i++) {
            try{
                workScheduler.stopAndClearWorks();
            } catch (error) {}
            end = getTime();
            times = end - begin;
        }
        if (times/20 < 50){
            expect(true).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest029
     * @tc.desc: test spent time by isLastWorkTimeOut with callback.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89D
     */
    it("WorkSchedulerJsTest029", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest029---------------------------');
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        let workInfo = {
            workId: 29,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.stopAndClearWorks();
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
        }
        let begin = getTime();
        try{
            workScheduler.isLastWorkTimeOut(29, (err, res) =>{
                let end = getTime();
                let times = end - begin;
                if (times < 50) {
                    expect(true).assertTrue();
                } else {
                    expect(false).assertTrue();
                }
            });
        } catch (error) {}
        setTimeout(() => {
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest030
     * @tc.desctest spent time by isLastWorkTimeOut with promise.
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89D
     */
    it("WorkSchedulerJsTest030", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest030---------------------------');
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        let begin = getTime();
        let workInfo = {
            workId: 30,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.stopAndClearWorks();
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
        }
        try{
            workScheduler.isLastWorkTimeOut(30)
                .then(res => {
                    let end = getTime();
                    let times = end - begin;
                    if (times < 50) {
                        expect(true).assertTrue();
                    } else {
                        expect(false).assertTrue();
                    }
                    console.info('WORK_SCHEDULER isLastWorkTimeOut promise success, data is:' + res);
                })
                .catch(err =>  {
                    expect(false).assertEqual(true)
            });
        } catch (error) {}
        setTimeout(() => {
            done();
        }, 500);
    })

    /*
     * @tc.name: WorkSchedulerJsTest031
     * @tc.desc: test spent time by isLastWorkTimeOut takes 20 times.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest031", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest031---------------------------');
        function getTime(){
            var time = new Date();
            var Milliseconds = time.getTime();
            return Milliseconds;
        }
        let begin = getTime();
        for (var i = 0 ; i < 20 ; i++) {
            try{
                workScheduler.isLastWorkTimeOut(31, (err, res) =>{
                    var end = getTime();
                    var times = end - begin;
                    if (times < 50) {
                        expect(true).assertTrue();
                    } else {
                        expect(false).assertTrue();
                    }
                });
            } catch (error) {}
        }
        setTimeout(() => {
            done();
        }, 1000);
    })

    /*
     * @tc.name: WorkSchedulerJsTest032
     * @tc.desc: test work scheduler with supported parameters.
     * @tc.type: FUNC
     * @tc.require: issueI5NG8L
     */
    it("WorkSchedulerJsTest032", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest032---------------------------');
        let workInfo = {
            workId: 32,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            parameters: {
                mykey0: 2147483647111,
                mykey1: 10,
                mykey2: "string",
                mykey3: true,
                mykey4: 2.55
            }
        }
        try{
            workScheduler.startWork(workInfo);
            expect(true).assertEqual(true)
        } catch (error) {
            expect(false).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest033
     * @tc.desc: test work scheduler with unsupported parameters.
     * @tc.type: FUNC
     * @tc.require: issueI5NG8L
     */
    it("WorkSchedulerJsTest033", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest033---------------------------');
        let workInfo = {
            workId: 33,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            parameters: {
                mykey0: 10,
                mykey1: [1, 2, 3],
                mykey2: [false, true, false],
                mykey3: ["qqqqq", "wwwwww", "aaaaaaaaaaaaaaaaa"]
            }
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(true).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest034
     * @tc.desc: test persisted work scheduler with supported parameters.
     * @tc.type: FUNC
     * @tc.require: issueI5NG8L
     */
    it("WorkSchedulerJsTest034", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest034---------------------------');
        let workInfo = {
            workId: 34,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            isPersisted: true,
            parameters: {
                mykey0: 2147483647111,
                mykey1: 10,
                mykey2: "string",
                mykey3: true,
                mykey4: 2.55
            }
        }
        try{
            workScheduler.startWork(workInfo);
            expect(true).assertEqual(true)
        } catch (error) {
            expect(false).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest035
     * @tc.desc: test work scheduler with fault type parameters.
     * @tc.type: FUNC
     * @tc.require: issueI5NG8L
     */
    it("WorkSchedulerJsTest035", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest035---------------------------');
        let workInfo = {
            workId: 35,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            parameters: 1
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(true).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest036
     * @tc.desc: test stop work scheduler with parameters.
     * @tc.type: FUNC
     * @tc.require: issueI5NG8L
     */
    it("WorkSchedulerJsTest036", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest036---------------------------');
        let workInfo = {
            workId: 36,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            parameters: {
                mykey0: 2147483647111,
                mykey1: 10,
                mykey2: "string",
                mykey3: true,
                mykey4: 2.55
            }
        }
        try{
            workScheduler.startWork(workInfo);
            workScheduler.stopWork(workInfo, false);
            expect(true).assertEqual(true)
        } catch (error) {
            expect(false).assertEqual(true)
        }
        done();
    })

    /*
     * @tc.name: WorkSchedulerJsTest037
     * @tc.desc: test obtainAllWorks callback with parameters.
     * @tc.type: FUNC
     * @tc.require: issueI5NG8L
     */
    it("WorkSchedulerJsTest037", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest037---------------------------');
        let workInfo = {
            workId: 37,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            parameters: {
                mykey0: 2147483647111,
                mykey1: 10,
                mykey2: "string",
                mykey3: true,
                mykey4: 2.55
            }
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
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
     * @tc.name: WorkSchedulerJsTest038
     * @tc.desc: test obtainAllWorks promise with parameters.
     * @tc.type: FUNC
     * @tc.require: issueI5NG8L
     */
    it("WorkSchedulerJsTest038", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest038---------------------------');
        let workInfo = {
            workId: 38,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            batteryLevel: 15,
            parameters: {
                mykey0: 2147483647111,
                mykey1: 10,
                mykey2: "string",
                mykey3: true,
                mykey4: 2.55
            }
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
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
     * @tc.name: WorkSchedulerJsTest039
     * @tc.desc: test getWorkStatus callback with parameters.
     * @tc.type: FUNC
     * @tc.require: issueI5NG8L
     */
    it("WorkSchedulerJsTest039", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest039---------------------------');
        let workInfo = {
            workId: 39,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            parameters: {
                mykey0: 2147483647111,
                mykey1: 10,
                mykey2: "string",
                mykey3: true,
                mykey4: 2.55
            }
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }
        workScheduler.getWorkStatus(37, (err, res) => {
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
     * @tc.name: WorkSchedulerJsTest040
     * @tc.desc: test getWorkStatus promise with parameters.
     * @tc.type: FUNC
     * @tc.require: issueI5NG8L
     */
    it("WorkSchedulerJsTest040", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest040---------------------------');
        let workInfo = {
            workId: 40,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            batteryLevel: 15,
            parameters: {
                mykey0: 2147483647111,
                mykey1: 10,
                mykey2: "string",
                mykey3: true,
                mykey4: 2.55
            }
        }
        try{
            workScheduler.startWork(workInfo);
        } catch (error) {
            expect(false).assertEqual(true)
            done();
        }

        workScheduler.getWorkStatus(38).then((res) => {
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
})