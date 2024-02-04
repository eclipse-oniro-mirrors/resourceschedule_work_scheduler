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

#include "work_datashare_helper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "work_sched_hilog.h"

namespace OHOS {
namespace WorkScheduler {
namespace {
const std::string SETTING_COLUMN_KEYWORD = "KEYWORD";
const std::string SETTING_COLUMN_VALUE = "VALUE";
const std::string SETTING_URI_PROXY = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
constexpr const char *SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
}

WorkDatashareHelper::~WorkDatashareHelper() { }

WorkDatashareHelper& WorkDatashareHelper::GetInstance()
{
    static WorkDatashareHelper workDatashareHelper;
    return workDatashareHelper;
}

__attribute__((no_sanitize("cfi"))) bool WorkDatashareHelper::GetStringValue(const std::string& key, std::string& value)
{
    auto helper = CreateDataShareHelper();
    if (helper == nullptr) {
        return false;
    }
    std::vector<std::string> columns = {SETTING_COLUMN_VALUE};
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri(SETTING_URI_PROXY + "&key=" + key);
    auto resultSet = helper->Query(uri, predicates, columns);
    ReleaseDataShareHelper(helper);
    if (resultSet == nullptr) {
        WS_HILOGE("helper->Query return nullptr");
        resultSet->Close();
        return false;
    }
    int32_t count;
    resultSet->GetRowCount(count);
    if (count == 0) {
        WS_HILOGW("not found value, key=%{public}s, count=%{public}d", key.c_str(), count);
        resultSet->Close();
        return false;
    }
    const int32_t INDEX = 0;
    resultSet->GoToRow(INDEX);
    int32_t ret = resultSet->GetString(INDEX, value);
    if (ret != DataShare::E_OK) {
        WS_HILOGW("resultSet->GetString return not ok, ret=%{public}d", ret);
        resultSet->Close();
        return false;
    }
    resultSet->Close();
    return true;
}

std::shared_ptr<DataShare::DataShareHelper> WorkDatashareHelper::CreateDataShareHelper()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        WS_HILOGE("GetSystemAbilityManager return nullptr");
        return nullptr;
    }
    auto remoteObj = samgr->GetSystemAbility(WORK_SCHEDULE_SERVICE_ID);
    if (remoteObj == nullptr) {
        WS_HILOGE("GetSystemAbility return nullptr");
        return nullptr;
    }
  
    auto helper = DataShare::DataShareHelper::Creator(remoteObj, SETTING_URI_PROXY, SETTINGS_DATA_EXT_URI);
    if (helper == nullptr) {
        WS_HILOGW("helper is nullptr, uri=%{public}s", SETTING_URI_PROXY.c_str());
        return nullptr;
    }
    return helper;
}

bool WorkDatashareHelper::ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper>& helper)
{
    if (!helper->Release()) {
        WS_HILOGW("release helper fail");
        return false;
    }
    return true;
}

} // namespace WorkScheduler
} // namespace OHOS