# 规格变更：WorkInfo 增加 isPerformanceSensitive

> 变更编号：20261020311883
> 关联 PR：[openharmony/resourcechedule_work_scheduler#793](https://gitcode.com/openharmony/resourcechedule_work_scheduler/pull/793)
> 编写日期：2026-10-30

## 1. WorkInfo 成员变更

### 1.1 新增成员

| 成员 | 类型 | 必填 | 默认值 | 描述 |
|---|---|---|---|---|
| `isPerformanceSensitive` | bool | 否 | `false` | 延迟任务在高性能 CPU 核心上运行；设为 `true` 时须同时满足 `isDeepIdle` 为 `true` 和 `isCharging` 为 `true`，该任务一天只能运行一次 |

### 1.2 完整 WorkInfo 成员表（变更后）

| 成员 | 类型 | 必填 | 描述 |
|---|---|---|---|
| `workId` | number | 是 | 延迟任务 ID |
| `bundleName` | string | 是 | 包名 |
| `abilityName` | string | 是 | Ability 名 |
| `networkType` | NetworkType | 否 | 网络类型条件 |
| `isCharging` | bool | 否 | 是否充电 |
| `chargerType` | ChargingType | 否 | 充电类型 |
| `batteryLevel` | number | 否 | 电池电量 |
| `batteryStatus` | BatteryStatus | 否 | 电池状态 |
| `storageRequest` | StorageRequest | 否 | 存储状态 |
| `isRepeat` | bool | 否 | 是否重复任务 |
| `isPerformanceSensitive` | bool | 否 | 延迟任务在高性能 CPU 核心上运行，设为 `true` 时须同时满足 `isDeepIdle` 为 `true` 和 `isCharging` 为 `true`，该任务一天只能运行一次 |
| `repeatCycleTime` | number | 否 | 重复周期时间 |
| `repeatCount` | number | 否 | 重复次数 |
| `parameters` | {[key: string]: any} | 否 | 自定义参数 |

## 2. 规则与约束

### 2.1 新增约束

| 编号 | 约束 |
|---|---|
| PS-1 | `isPerformanceSensitive` 设为 `true` 时，必须同时设置 `isDeepIdle` 为 `true` 和 `isCharging` 为 `true`，否则返回 `E_PERFORMANCE_SENSITIVE_CONDITION_ERR` |
| PS-2 | `isPerformanceSensitive` 设为 `true` 且为重复任务时，`repeatCycleTime` 必须 >= 24 小时（`ONE_DAY_MS`），否则返回 `E_PERFORMANCE_SENSITIVE_CONDITION_ERR` |
| PS-3 | 单应用 24 小时内仅允许执行一次性能敏感任务；不足 24 小时时任务不就绪，设置 `needRetrigger_` 在剩余时间后重新触发 |
| PS-4 | 当日全设备性能敏感任务累计高性能运行时长上限 `MAX_HIGH_PERF_DURATION`（10 分钟）；超限后所有性能敏感任务不就绪，次日重置 |
| PS-5 | 性能敏感任务固定使用 `WATCHDOG_TIME`（2 分钟）看门狗超时 |
| PS-6 | 系统应用 + 网络条件 + 性能敏感任务在 6:00-9:00 的 0/30 分窗口触发离散调度，随机延迟 1~120 秒启动 |
| PS-7 | `isPerformanceSensitive` 未设置时默认 `false`，不影响现有任务行为 |

### 2.2 napi 参数约束

| 约束 | 错误码 |
|---|---|
| `isPerformanceSensitive` 参数类型非 boolean | `E_IS_PERFORMANCE_SENSITIVE_ERR` |
| `isPerformanceSensitive=true` 但未设置 `isDeepIdle`/`isCharging` | `E_PERFORMANCE_SENSITIVE_CONDITION_ERR` |

### 2.3 已有约束（保持不变）

1. `repeatCycleTime` 至少 20 分钟（`minTimeCycle_ = 20 * 60 * 1000`），设置时须在 `isRepeat` 和 `repeatCount` 中选择其一
2. `parameters` 支持 number、string 和 bool 类型
3. 重复添加相同 `workId` 的任务返回 `E_ADD_REPEAT_WORK_ERR`

## 3. 状态流转

性能敏感任务的状态流转在现有流转基础上增加就绪判定约束：

```
WAIT_CONDITION
  │ (条件变化, OnConditionChanged)
  ├─ IsPerformanceSensitiveReady() == false → 保持 WAIT_CONDITION (设置 needRetrigger_)
  ├─ IsPerformanceSensitiveReady() == true  → CONDITION_READY
  ▼
CONDITION_READY
  │ (CheckWorkToRun, GetWorkToRun)
  ├─ runningCount >= allowRunningCount → 保持 CONDITION_READY (延迟)
  ├─ 离散调度窗口 → DiscreteScheduled (随机延迟后) → RUNNING
  ▼
RUNNING
  │ (StartWork 成功)
  ├─ StartHighPerfTimer()
  │ (看门狗超时 / StopWork / Pause)
  ├─ StopHighPerfTimer() → WAIT_CONDITION (重复) / REMOVED (非重复)
  ▼
WAIT_CONDITION / REMOVED
```

## 4. 错误码

### 4.1 新增错误码

| 错误码 | 名称 | 描述 |
|---|---|---|
| `E_PERFORMANCE_SENSITIVE_CONDITION_ERR` | 性能敏感条件错误 | StartWork failed. When isPerformanceSensitive is true, isDeepIdle and isCharging must be set to true. If it is a repeat work, the repeatCycleTime must be greater than or equal to 24 hours. |
| `E_IS_PERFORMANCE_SENSITIVE_ERR` | 参数类型错误 | The type of isPerformanceSensitive must be boolean. |

### 4.2 错误码映射表更新

`utils/native/include/work_sched_errors.h` 中 `errCodeMsgMap` 新增：

```
{E_PERFORMANCE_SENSITIVE_CONDITION_ERR,
    "StartWork failed. When isPerformanceSensitive is true, isDeepIdle and isCharging must be set to true. "
    "If it is a repeat work, the repeatCycleTime must be greater than or equal to 24 hours."}
```

`paramErrCodeMsgMap` 新增：

```
{E_IS_PERFORMANCE_SENSITIVE_ERR, "The type of isPerformanceSensitive must be boolean."}
```

## 5. 常量

`utils/native/include/work_sched_constants.h` 新增：

| 常量 | 类型 | 值 | 说明 |
|---|---|---|---|
| `MAX_HIGH_PERF_DURATION` | `uint64_t` | `10 * 60 * 1000` | 当日全设备高性能累计运行时长上限（10 分钟） |
| `ONE_DAY_MS` | `uint32_t` | `24 * 60 * 60 * 1000` | 单应用性能敏感任务最小间隔（24 小时） |
| `DELAY_TASK_PERF_START` | `int32_t` | `1` | DFX 事件值：性能敏感任务开始 |
| `DELAY_TASK_PERF_END` | `int32_t` | `0` | DFX 事件值：性能敏感任务结束 |

## 6. IPC 序列化规格

### 6.1 WorkInfo Parcel 序列化顺序（变更后）

| 序号 | 字段 | 类型 | 写入方法 |
|---|---|---|---|
| 1 | workId | int32_t | WriteInt32 |
| 2 | bundleName | string | WriteString |
| 3 | abilityName | string | WriteString |
| 4 | earliestStartTime | int32_t | WriteInt32 |
| 5 | persisted | bool | WriteBool |
| 6 | triggerType | int32_t | WriteInt32 |
| 7 | uid | int32_t | WriteInt32 |
| 8 | conditionMap.size | uint32_t | WriteUint32 |
| 9 | conditionMap entries | (type, condition) | 逐条写入 |
| 10 | **isPerformanceSensitive** | **bool** | **WriteBool** ← 新增 |
| 11 | hasExtras | bool | WriteBool |
| 12 | extras (可选) | WantParams | Marshalling |

### 6.2 兼容性

| 维度 | 兼容性 | 说明 |
|---|---|---|
| IPC Parcel | 不兼容 | 序号 10 为新增字段，旧版本 Parcel 不含此字段 |
| 持久化 JSON | 兼容 | `IsHasBoolProp` 容错检查，缺失时默认 `false` |
| napi/taihe 接口 | 兼容 | `Optional<bool>`，未设置时默认 `false` |

## 7. 接口规格

### 7.1 napi 接口

**读取（GetBaseWorkInfo）：**
```
isPerformanceSensitive = GetBoolProperty(env, objValue, "isPerformanceSensitive", E_IS_PERFORMANCE_SENSITIVE_ERR)
workInfo.RequestPerformanceSensitive(isPerformanceSensitive)
// 未设置时默认 false
```

**写入（GetNapiWorkInfo）：**
```
napi_get_boolean(env, workInfo->IsPerformanceSensitive(), &napiIsPerformanceSensitive)
napi_set_named_property(env, napiWork, "isPerformanceSensitive", napiIsPerformanceSensitive)
```

### 7.2 taihe IDL

`WorkInfoInner` 结构新增：
```
isPerformanceSensitive: boolean | undefined;
```

`WorkInfo` 结构新增：
```
isPerformanceSensitive: Optional<bool>;
```

### 7.3 ANI 扩展回调

`AniWorkSchedulerExtension::OnWorkStart` 新增 `SetPerformanceSensitiveInfo(env, workInfoObject, isPerformanceSensitive)` 透传字段。

### 7.4 WorkInfo C++ 接口

| 方法 | 签名 | 说明 |
|---|---|---|
| `RequestPerformanceSensitive` | `void RequestPerformanceSensitive(bool isPerformanceSensitive)` | 设置性能敏感标记 |
| `IsPerformanceSensitive` | `bool IsPerformanceSensitive() const` | 查询性能敏感标记 |

## 8. DFX 规格

### 8.1 事件上报

| 事件类型 | 触发点 | type 值 | payload |
|---|---|---|---|
| `RES_TYPE_DELAY_TASK_PERF` | `StartHighPerfTimer` | `1` (START) | `{"uid": "<uid>"}` |
| `RES_TYPE_DELAY_TASK_PERF` | `StopHighPerfTimer` | `1` (START) | `{"uid": "<uid>"}` |

> 注：`StopHighPerfTimer` 当前上报 type 为 `DELAY_TASK_PERF_START`，与语义"结束"不一致，待修正为 `DELAY_TASK_PERF_END`。

### 8.2 hidumper

| dump 命令 | 功能 |
|---|---|
| `DumpSetHighPerfDuration` | 设置当日高性能累计运行时长，用于测试调试 |
