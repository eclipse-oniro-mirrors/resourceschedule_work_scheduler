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
#ifndef FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORKINFO_TEST_H
#define FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORKINFO_TEST_H

#include <gtest/gtest.h>

namespace OHOS {
namespace WorkScheduler {
class WorkInfoTest : public testing::Test {
public:
    /**
     * @brief Set up test case.
     */
    static void SetUpTestCase(void);
    /**
     * @brief Tear down test case.
     */
    static void TearDownTestCase(void);
    /**
     * @brief Set up.
     */
    void SetUp();
    /**
     * @brief Tear down.
     */
    void TearDown();
};
} // namespace WorkScheduler
} // namespace OHOS
#endif // FOUNDATION_RESOURCESCHEDULE_WORKSCHEDULER_WORKINFO_TEST_H