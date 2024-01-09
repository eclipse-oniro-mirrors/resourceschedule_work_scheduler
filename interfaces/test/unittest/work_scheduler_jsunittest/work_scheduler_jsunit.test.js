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

    function formatWorkInfo(workIdParms) {
        var workInfo = {
            workId: workIdParms,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        return workInfo;
    }

    /*
     * @tc.name: WorkSchedulerJsTest002
     * @tc.desc: test work scheduler work id = 0
     * @tc.type: FUNC
     * @tc.require: SR000GGTN7 AR000GH89H AR000GH89I AR000GH899 issueI5QJR8
     */
    it("WorkSchedulerJsTest002", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest002---------------------------');
        let workInfo2 = formatWorkInfo(0);
        try{
            workScheduler.startWork(workInfo2);
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
        let workInfo3 = {
            workId: 3,
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.startWork(workInfo3);
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
        let workInfo4 = {
            workId: 4,
            bundleName: "com.example.myapplication",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY
        }
        try{
            workScheduler.startWork(workInfo4);
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
        let workInfo5 = {
            workId: 5,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName"
        }
        try{
            workScheduler.startWork(workInfo5);
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
        let workInfo6 = formatWorkInfo(6);
        try{
            workScheduler.startWork(workInfo6);
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
        let workInfo7 = formatWorkInfo(7);
        try{
            workScheduler.startWork(workInfo7);
            workScheduler.stopWork(workInfo7, false);
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
        let workInfo8 = formatWorkInfo(8);
        try{
            workScheduler.startWork(workInfo8);
            workScheduler.stopWork(workInfo8, false);
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
        let workInfo9 = formatWorkInfo(9);
        try{
            workScheduler.startWork(workInfo9);
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
            console.info('WORK_SCHEDULER getWorkStatuscallback callback exception');
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
        let workInfo10 = {
            workId: 10,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            batteryLevel: 15
        }
        try{
            workScheduler.startWork(workInfo10);
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
            console.info('WORK_SCHEDULER getWorkStatuscallback promise exception');
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
        let workInfo11 = formatWorkInfo(11);
        try{
            workScheduler.startWork(workInfo11);
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
            console.info('WORK_SCHEDULER obtainAllWorks callback exception');
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
        let workInfo12 = {
            workId: 12,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            batteryLevel: 15
        }
        try{
            workScheduler.startWork(workInfo12);
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
        let workInfo13 = formatWorkInfo(13);
        try{
            workScheduler.startWork(workInfo13);
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
        let workInfo14 = formatWorkInfo(14);
        try{
            workScheduler.stopAndClearWorks();
            workScheduler.startWork(workInfo14);
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
            console.info('WORK_SCHEDULER isLastWorkTimeOut callback exception');
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
        let workInfo15 = formatWorkInfo(15);
        try{
            workScheduler.stopAndClearWorks();
            workScheduler.startWork(workInfo15);
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
            console.info('WORK_SCHEDULER isLastWorkTimeOut promise exception');
            expect(false).assertEqual(true)
        }
        setTimeout(()=>{
            done();
        }, 500);
    })

    function getTime(){
        var time = new Date();
        var Milliseconds = time.getTime();
        return Milliseconds;
    }

    function formatWorkInfoByParms(workIdParms, myKey4Parms) {
        var workInfo = {
            workId: workIdParms,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            parameters: {
                mykey0: 2147483647111,
                mykey1: 10,
                mykey2: "string",
                mykey3: true,
                mykey4: myKey4Parms
            }
        }
        return workInfo;
    }

    /*
     * @tc.name: WorkSchedulerJsTest016
     * @tc.desc: test spent time by startWork.
     * @tc.type: FUNC
     * @tc.require:
     */
    it("WorkSchedulerJsTest016", 0, async function (done) {
        console.info('----------------------WorkSchedulerJsTest016---------------------------');
        let begin = getTime();
        let workInfo16 = formatWorkInfoByParms(16, 2.55);
        try{
            workScheduler.startWork(workInfo16);
        } catch (error) {
            console.info('WORK_SCHEDULER startWork startWork exception');
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
        let begin = getTime();
        let workInfo17 = formatWorkInfo(17);
        for (var i = 0 ; i < 20 ; i++) {
            try{
                workScheduler.startWork(workInfo17);
            } catch (error) {
                console.info('WORK_SCHEDULER startWork takes 20 times startWork exception');
            }
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
        let workInfo18 = formatWorkInfo(18);
        try{
            workScheduler.startWork(workInfo18);
            let begin = getTime();
            workScheduler.stopWork(workInfo18, false);
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
        let workInfo19 = formatWorkInfo(19);
        try{
            workScheduler.startWork(workInfo19);
        } catch (error) {
            console.info('WORK_SCHEDULER stopWork startWork exception');
            expect(false).assertEqual(true)
            done();
        }

        let begin = getTime();
        try{
            workScheduler.stopWork(workInfo19, true);
        } catch (error) {
            console.info('WORK_SCHEDULER stopWork stopWork exception');
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
        let begin = getTime();
        let workInfo20 = formatWorkInfo(20);
        let end = 0;
        let times = 0;
        for (var i = 0 ; i < 20 ; i++) {
            try{
                workScheduler.startWork(workInfo20);
            } catch (error) {
                console.info('WORK_SCHEDULER stopWork takes 20 times startWork exception');
            }
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
        let workInfo21 = formatWorkInfo(21);
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
        try{
            workScheduler.startWork(workInfo21);
        } catch (error) {
            console.info('WORK_SCHEDULER getWorkStatus callback startWork exception');
            expect(false).assertEqual(true)
            done();
        }
        let begin = getTime();
        try{
            workScheduler.getWorkStatus(21, workSchedulerCallback);
        } catch (error) {
            console.info('WORK_SCHEDULER getWorkStatus callback getWorkStatus exception');
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
        let workInfo22 = formatWorkInfo(22);
        try{
            workScheduler.startWork(workInfo22);
        } catch (error) {
            console.info('WORK_SCHEDULER getWorkStatus promise startWork promise exception');
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
            console.info('WORK_SCHEDULER getWorkStatus promise exception');
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
        let workInfo23 = formatWorkInfo(23);
        let begin = getTime();
        try{
            workScheduler.startWork(workInfo23);
        } catch (error) {
            console.info('WORK_SCHEDULER getWorkStatus takes 20 times startWork exception');
            expect(false).assertEqual(true)
            done();
        }
        let end = 0;
        let times = 0;
        for (var i = 0 ; i < 20 ; i++) {
            try{
                workScheduler.getWorkStatus(23, (err, res) => {
                    end = getTime();
                    times = end - begin;
                });
            } catch (error) {
                console.info('WORK_SCHEDULER getWorkStatus takes 20 times exception');
            }
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
        let workInfo24 = formatWorkInfo(24);
        try{
            workScheduler.startWork(workInfo24);
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
        } catch (error) {
            console.info('WORK_SCHEDULER obtainAllWorks callback exception');
        }
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
        let workInfo25 = formatWorkInfo(25);
        try{
            workScheduler.startWork(workInfo25);
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
        } catch (error) {
            console.info('WORK_SCHEDULER obtainAllWorks promise exception');
        }
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
        let workInfo26 = formatWorkInfo(26);
        try{
            workScheduler.stopAndClearWorks();
            workScheduler.startWork(workInfo26);
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
            } catch (error) {
                console.info('WORK_SCHEDULER obtainAllWorks takes 20 times exception');
            }
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
        let workInfo27 = formatWorkInfo(27);
        try{
            workScheduler.startWork(workInfo27);
        } catch (error) {}
        let begin = getTime();
        try{
            workScheduler.stopAndClearWorks();
        } catch (error) {
            console.info('WORK_SCHEDULER stopAndClearWorks exception');
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
        let workInfo28 = formatWorkInfo(28);
        try{
            workScheduler.startWork(workInfo28);
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
            } catch (error) {
                console.info('WORK_SCHEDULER stopWork takes 20 times exception');
            }
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
        let workInfo29 = formatWorkInfo(29);
        try{
            workScheduler.stopAndClearWorks();
            workScheduler.startWork(workInfo29);
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
        } catch (error) {
            console.info('WORK_SCHEDULER isLastWorkTimeOut callback exception');
        }
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
        let begin = getTime();
        let workInfo30 = formatWorkInfo(30);
        try{
            workScheduler.stopAndClearWorks();
            workScheduler.startWork(workInfo30);
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
        } catch (error) {
            console.info('WORK_SCHEDULER isLastWorkTimeOut promise exception');
        }
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
            } catch (error) {
                console.info('WORK_SCHEDULER isLastWorkTimeOut takes 20 times exception');
            }
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
        let workInfo32 = formatWorkInfoByParms(32, 2.56);
        try{
            workScheduler.startWork(workInfo32);
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
        let workInfo33 = {
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
            workScheduler.startWork(workInfo33);
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
        let workInfo34 = {
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
                mykey4: 2.57
            }
        }
        try{
            workScheduler.startWork(workInfo34);
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
        let workInfo35 = {
            workId: 35,
            bundleName: "com.example.myapplication",
            abilityName: "com.mytest.abilityName",
            storageRequest: workScheduler.StorageRequest.STORAGE_LEVEL_OKAY,
            parameters: 1
        }
        try{
            workScheduler.startWork(workInfo35);
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
        let workInfo36 = formatWorkInfoByParms(36, 2.58);
        try{
            workScheduler.startWork(workInfo36);
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
        let workInfo37 = formatWorkInfoByParms(37, 2.59);
        try{
            workScheduler.startWork(workInfo37);
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
        let workInfo38 = {
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
                mykey4: 2.60
            }
        }
        try{
            workScheduler.startWork(workInfo38);
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
        let workInfo39 = formatWorkInfoByParms(39, 2.61);
        try{
            workScheduler.startWork(workInfo39);
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
        let workInfo40 = {
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
                mykey4: 2.62
            }
        }
        try{
            workScheduler.startWork(workInfo40);
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
            console.info('WORK_SCHEDULER getWorkStatus promise exception');
            expect(false).assertEqual(true)
        })

        setTimeout(()=>{
            done();
        }, 500);
    })
})