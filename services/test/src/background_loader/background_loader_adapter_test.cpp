/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "background_loader/background_loader_adapter.h"
#include "background_loader/background_loader_mgr.h"
#include "res_data.h"
#include "res_type.h"
#include "nlohmann/json.hpp"

using namespace testing::ext;
using namespace OHOS::ResourceSchedule;
namespace OHOS {
namespace WorkScheduler {
namespace {
constexpr int32_t SEED_UID = 1001;
const std::string SEED_BUNDLE = "com.seed.bundle";

void SeedState()
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.isReady_.store(false);
    {
        std::lock_guard<ffrt::mutex> lock(mgr.taskLock_);
        mgr.taskMap_.clear();
    }
    {
        std::lock_guard<ffrt::mutex> lock(mgr.blockListLock_);
        mgr.blockLists_.clear();
    }
    TaskInfo info;
    info.bundleName_ = SEED_BUNDLE;
    info.abilityName_ = "SeedAbility";
    info.appIndex_ = 0;
    info.taskId_ = 1;
    info.uid_ = SEED_UID;
    {
        std::lock_guard<ffrt::mutex> lock(mgr.taskLock_);
        mgr.taskMap_[SEED_UID] = info;
    }
    {
        std::lock_guard<ffrt::mutex> lock(mgr.blockListLock_);
        mgr.blockLists_.insert(SEED_BUNDLE);
    }
}
}

class BackgroundLoaderAdapterTest : public testing::Test {
public:
    void SetUp() override
    {
        SeedState();
    }
    void TearDown() override {}
};

/**
 * @tc.name: DispatchResource_NullData_001
 * @tc.desc: Test DispatchResource returns early when resData is null.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_NullData_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    BackgroundLoaderAdapter::GetInstance().DispatchResource(nullptr);
    EXPECT_EQ(mgr.taskMap_.count(SEED_UID), 1);
    EXPECT_EQ(mgr.blockLists_.count(SEED_BUNDLE), 1);
    EXPECT_FALSE(mgr.isReady_.load());
}

/**
 * @tc.name: DispatchResource_StartBackgroundLoaderTask_001
 * @tc.desc: Test DispatchResource dispatches RES_TYPE_START_BACKGROUND_LOADER_TASK.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_StartBackgroundLoaderTask_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    auto data = std::make_shared<ResData>(ResType::RES_TYPE_START_BACKGROUND_LOADER_TASK, 0, nlohmann::json{});
    BackgroundLoaderAdapter::GetInstance().DispatchResource(data);
    EXPECT_EQ(mgr.taskMap_.count(SEED_UID), 1);
    EXPECT_EQ(mgr.blockLists_.count(SEED_BUNDLE), 1);
    EXPECT_FALSE(mgr.isReady_.load());
}

/**
 * @tc.name: DispatchResource_AppInstallUninstall_001
 * @tc.desc: Test DispatchResource dispatches RES_TYPE_APP_INSTALL_UNINSTALL.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_AppInstallUninstall_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    auto data = std::make_shared<ResData>(ResType::RES_TYPE_APP_INSTALL_UNINSTALL, 0, nlohmann::json{});
    BackgroundLoaderAdapter::GetInstance().DispatchResource(data);
    EXPECT_EQ(mgr.taskMap_.count(SEED_UID), 1);
    EXPECT_EQ(mgr.blockLists_.count(SEED_BUNDLE), 1);
    EXPECT_FALSE(mgr.isReady_.load());
}

/**
 * @tc.name: DispatchResource_AppStateChange_001
 * @tc.desc: Test DispatchResource dispatches RES_TYPE_APP_STATE_CHANGE.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_AppStateChange_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    auto data = std::make_shared<ResData>(ResType::RES_TYPE_APP_STATE_CHANGE, 0, nlohmann::json{});
    BackgroundLoaderAdapter::GetInstance().DispatchResource(data);
    EXPECT_EQ(mgr.taskMap_.count(SEED_UID), 1);
    EXPECT_EQ(mgr.blockLists_.count(SEED_BUNDLE), 1);
    EXPECT_FALSE(mgr.isReady_.load());
}

/**
 * @tc.name: DispatchResource_AudioRenderStateChange_001
 * @tc.desc: Test DispatchResource dispatches RES_TYPE_AUDIO_RENDER_STATE_CHANGE.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_AudioRenderStateChange_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    auto data = std::make_shared<ResData>(ResType::RES_TYPE_AUDIO_RENDER_STATE_CHANGE, 0, nlohmann::json{});
    BackgroundLoaderAdapter::GetInstance().DispatchResource(data);
    EXPECT_EQ(mgr.taskMap_.count(SEED_UID), 1);
    EXPECT_EQ(mgr.blockLists_.count(SEED_BUNDLE), 1);
    EXPECT_FALSE(mgr.isReady_.load());
}

/**
 * @tc.name: DispatchResource_AudioCaptureStatusChanged_001
 * @tc.desc: Test DispatchResource dispatches RES_TYPE_AUDIO_CAPTURE_STATUS_CHANGED.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_AudioCaptureStatusChanged_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    auto data = std::make_shared<ResData>(ResType::RES_TYPE_AUDIO_CAPTURE_STATUS_CHANGED, 0, nlohmann::json{});
    BackgroundLoaderAdapter::GetInstance().DispatchResource(data);
    EXPECT_EQ(mgr.taskMap_.count(SEED_UID), 1);
    EXPECT_EQ(mgr.blockLists_.count(SEED_BUNDLE), 1);
    EXPECT_FALSE(mgr.isReady_.load());
}

/**
 * @tc.name: DispatchResource_LocationStatusChange_001
 * @tc.desc: Test DispatchResource dispatches RES_TYPE_LOCATION_STATUS_CHANGE.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_LocationStatusChange_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    auto data = std::make_shared<ResData>(ResType::RES_TYPE_LOCATION_STATUS_CHANGE, 0, nlohmann::json{});
    BackgroundLoaderAdapter::GetInstance().DispatchResource(data);
    EXPECT_EQ(mgr.taskMap_.count(SEED_UID), 1);
    EXPECT_EQ(mgr.blockLists_.count(SEED_BUNDLE), 1);
    EXPECT_FALSE(mgr.isReady_.load());
}

/**
 * @tc.name: DispatchResource_CameraTorchStateChange_001
 * @tc.desc: Test DispatchResource dispatches RES_TYPE_CAMERA_TORCH_STATE_CHANGE.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_CameraTorchStateChange_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    auto data = std::make_shared<ResData>(ResType::RES_TYPE_CAMERA_TORCH_STATE_CHANGE, 0, nlohmann::json{});
    BackgroundLoaderAdapter::GetInstance().DispatchResource(data);
    EXPECT_EQ(mgr.taskMap_.count(SEED_UID), 1);
    EXPECT_EQ(mgr.blockLists_.count(SEED_BUNDLE), 1);
    EXPECT_FALSE(mgr.isReady_.load());
}

/**
 * @tc.name: DispatchResource_TransientTask_001
 * @tc.desc: Test DispatchResource dispatches RES_TYPE_TRANSIENT_TASK.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_TransientTask_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    auto data = std::make_shared<ResData>(ResType::RES_TYPE_TRANSIENT_TASK, 0, nlohmann::json{});
    BackgroundLoaderAdapter::GetInstance().DispatchResource(data);
    EXPECT_EQ(mgr.taskMap_.count(SEED_UID), 1);
    EXPECT_EQ(mgr.blockLists_.count(SEED_BUNDLE), 1);
    EXPECT_FALSE(mgr.isReady_.load());
}

/**
 * @tc.name: DispatchResource_ContinuousTask_001
 * @tc.desc: Test DispatchResource dispatches RES_TYPE_CONTINUOUS_TASK.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_ContinuousTask_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    auto data = std::make_shared<ResData>(ResType::RES_TYPE_CONTINUOUS_TASK, 0, nlohmann::json{});
    BackgroundLoaderAdapter::GetInstance().DispatchResource(data);
    EXPECT_EQ(mgr.taskMap_.count(SEED_UID), 1);
    EXPECT_EQ(mgr.blockLists_.count(SEED_BUNDLE), 1);
    EXPECT_FALSE(mgr.isReady_.load());
}

/**
 * @tc.name: DispatchResource_ProcessStateChange_001
 * @tc.desc: Test DispatchResource dispatches RES_TYPE_PROCESS_STATE_CHANGE.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_ProcessStateChange_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    auto data = std::make_shared<ResData>(ResType::RES_TYPE_PROCESS_STATE_CHANGE, 0, nlohmann::json{});
    BackgroundLoaderAdapter::GetInstance().DispatchResource(data);
    EXPECT_EQ(mgr.taskMap_.count(SEED_UID), 1);
    EXPECT_EQ(mgr.blockLists_.count(SEED_BUNDLE), 1);
    EXPECT_FALSE(mgr.isReady_.load());
}

/**
 * @tc.name: DispatchResource_Default_001
 * @tc.desc: Test DispatchResource falls through to default for an unknown resType.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_Default_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    auto data = std::make_shared<ResData>(0xFFFF, 0, nlohmann::json{});
    BackgroundLoaderAdapter::GetInstance().DispatchResource(data);
    EXPECT_EQ(mgr.taskMap_.count(SEED_UID), 1);
    EXPECT_EQ(mgr.blockLists_.count(SEED_BUNDLE), 1);
    EXPECT_FALSE(mgr.isReady_.load());
}

/**
 * @tc.name: DispatchResource_TransientTask_Effective_001
 * @tc.desc: Test DispatchResource routes a transient task start to mark hasShortTask_.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_TransientTask_Effective_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.isReady_.store(true);
    {
        std::lock_guard<ffrt::mutex> lock(mgr.taskLock_);
        mgr.taskMap_.clear();
    }
    {
        std::lock_guard<ffrt::mutex> lock(mgr.blockListLock_);
        mgr.blockLists_.clear();
    }

    TaskInfo info;
    info.bundleName_ = "com.test.bundle";
    info.abilityName_ = "TestAbility";
    info.appIndex_ = 0;
    info.taskId_ = 1;
    info.uid_ = 2002;
    info.isBg_ = true;
    {
        std::lock_guard<ffrt::mutex> lock(mgr.taskLock_);
        mgr.taskMap_[2002] = info;
    }

    nlohmann::json payload;
    payload["uid"] = 2002;
    payload["bundleName"] = "com.test.bundle";
    auto data = std::make_shared<ResData>(ResType::RES_TYPE_TRANSIENT_TASK,
        static_cast<int64_t>(ResType::TransientTaskStatus::TRANSIENT_TASK_START), payload);
    BackgroundLoaderAdapter::GetInstance().DispatchResource(data);

    EXPECT_TRUE(mgr.taskMap_.at(2002).hasShortTask_);
    EXPECT_EQ(mgr.taskMap_.at(2002).status_, TaskStatus::NOT_STARTED);
    EXPECT_TRUE(mgr.blockLists_.empty());
}

/**
 * @tc.name: DispatchResource_ContinuousTask_Effective_001
 * @tc.desc: Test DispatchResource routes a continuous task start to mark hasLongTask_.
 * @tc.type: FUNC
 */
HWTEST_F(BackgroundLoaderAdapterTest, DispatchResource_ContinuousTask_Effective_001, TestSize.Level1)
{
    auto& mgr = BackgroundLoaderMgr::GetInstance();
    mgr.isReady_.store(true);
    {
        std::lock_guard<ffrt::mutex> lock(mgr.taskLock_);
        mgr.taskMap_.clear();
    }
    {
        std::lock_guard<ffrt::mutex> lock(mgr.blockListLock_);
        mgr.blockLists_.clear();
    }

    TaskInfo info;
    info.bundleName_ = "com.test.bundle";
    info.abilityName_ = "TestAbility";
    info.appIndex_ = 0;
    info.taskId_ = 1;
    info.uid_ = 2002;
    info.isBg_ = true;
    {
        std::lock_guard<ffrt::mutex> lock(mgr.taskLock_);
        mgr.taskMap_[2002] = info;
    }

    nlohmann::json payload;
    payload["uid"] = 2002;
    payload["bundleName"] = "com.test.bundle";
    auto data = std::make_shared<ResData>(ResType::RES_TYPE_CONTINUOUS_TASK,
        static_cast<int64_t>(ResType::ContinuousTaskStatus::CONTINUOUS_TASK_START), payload);
    BackgroundLoaderAdapter::GetInstance().DispatchResource(data);

    EXPECT_TRUE(mgr.taskMap_.at(2002).hasLongTask_);
    EXPECT_EQ(mgr.taskMap_.at(2002).status_, TaskStatus::NOT_STARTED);
    EXPECT_TRUE(mgr.blockLists_.empty());
}
} // namespace WorkScheduler
} // namespace OHOS
