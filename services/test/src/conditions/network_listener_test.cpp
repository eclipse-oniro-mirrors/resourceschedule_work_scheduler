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

#include <functional>
#include <gtest/gtest.h>

#include "conditions/network_listener.h"
#include "work_scheduler_service.h"
#include "work_queue_manager.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "net_supplier_info.h"

using namespace testing::ext;

namespace OHOS {
namespace WorkScheduler {
const int32_t DEFAULT_VALUE = -1;
const int32_t BEARER_CELLULAR = 0;
const int32_t BEARER_WIFI = 1;
const int32_t BEARER_BLUETOOTH = 2;
const int32_t BEARER_ETHERNET = 3;
const int32_t BEARER_WIFI_AWARE = 5;

class NetworkListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
    static std::shared_ptr<WorkQueueManager> workQueueManager_;
    static std::shared_ptr<NetworkListener> networkListener_;
};

std::shared_ptr<WorkQueueManager> NetworkListenerTest::workQueueManager_ = nullptr;
std::shared_ptr<NetworkListener> NetworkListenerTest::networkListener_ = nullptr;

void NetworkListenerTest::SetUpTestCase()
{
    std::shared_ptr<WorkSchedulerService> workSchedulerService_ = std::make_shared<WorkSchedulerService>();
    workQueueManager_ = std::make_shared<WorkQueueManager>(workSchedulerService_);
    networkListener_ = std::make_shared<NetworkListener>(workQueueManager_);
}
}
}