# 延迟任务规格

> 文档版本：v1.0
> 更新时间：2026-09-10

延迟任务的定义、核心概念、功能边界及与后台任务管理组件的关系详见 [docs/knowledge/glossary.md](../knowledge/glossary.md) 和 [docs/knowledge/business_context.md](../knowledge/business_context.md)。

## 接口说明

### 公共 API（JS/NAPI）

| 接口名 | 接口描述 |
|---|---|
| `startWork(work: WorkInfo): void` | 申请延迟任务 |
| `stopWork(work: WorkInfo, needCancel?: boolean): void` | 停止延迟任务；`needCancel` 标识是否移除任务，`true` 表示移除 |
| `getWorkStatus(workId: number, callback: AsyncCallback<WorkInfo>): void` | 获取延迟任务状态（callback 形式） |
| `getWorkStatus(workId: number): Promise<WorkInfo>` | 获取延迟任务状态（Promise 形式） |
| `obtainAllWorks(callback: AsyncCallback<void>): Array<WorkInfo>` | 获取所有延迟任务（callback 形式） |
| `obtainAllWorks(): Promise<Array<WorkInfo>>` | 获取所有延迟任务（Promise 形式） |
| `stopAndClearWorks(): void` | 停止并清除所有延迟任务 |
| `isLastWorkTimeOut(workId: number, callback: AsyncCallback<boolean>): boolean` | 检查上次延迟任务是否超时（callback 形式） |
| `isLastWorkTimeOut(workId: number): Promise<boolean>` | 检查上次延迟任务是否超时（Promise 形式） |

### 回调 API

| 接口名 | 接口描述 |
|---|---|
| `onWorkStart(work: WorkInfo): void` | 任务开始执行回调 |
| `onWorkStop(work: WorkInfo): void` | 任务停止执行回调 |

### WorkInfo 成员

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
| `repeatCycleTime` | number | 否 | 重复周期时间 |
| `repeatCount` | number | 否 | 重复次数 |
| `parameters` | {[key: string]: any} | 否 | 自定义参数 |

### 枚举常量

| 枚举 | 取值 |
|---|---|
| NetworkType | NETWORK_TYPE_ANY(0) / MOBILE(1) / WIFI(2) / BLUETOOTH(3) / WIFI_P2P(4) / ETHERNET(5) |
| ChargingType | CHARGING_PLUGGED_ANY(0) / AC(1) / USB(2) / WIRELESS(3) |
| BatteryStatus | BATTERY_STATUS_LOW(0) / OKAY(1) / LOW_OR_OKAY(2) |
| StorageRequest | STORAGE_LEVEL_LOW(0) / OKAY(1) / LOW_OR_OKAY(2) |

## 规则与约束

### 运行限制

**单次延迟任务的最长运行时间由看门狗定时器决定**

| 场景 | 看门狗超时时间 | 常量（毫秒） |
|---|---|---|
| 默认 | 2 分钟 | `WATCHDOG_TIME`（2 * 60 * 1000） |
| 申请能效资源，且未充电 | 10 分钟 | `MEDIUM_WATCHDOG_TIME`（10 * 60 * 1000） |
| 申请能效资源，且充电中 | 20 分钟 | `LONG_WATCHDOG_TIME`（20 * 60 * 1000） |
| 深度空闲，且充电中 | 20 分钟 | `LONG_WATCHDOG_TIME`（20 * 60 * 1000） |

**运行个数限制**

- 每个应用（UID）最多同时持有 10 个延迟任务（`MAX_WORK_COUNT_PER_UID = 10`）
- 系统最大同时运行任务数 3（`MAX_RUNNING_COUNT = 3`），设备待机模式下放宽为 6（`STANDBY_MAX_RUNNING_COUNT = 2 * MAX_RUNNING_COUNT`）

**任务执行优先级**

当存在多个就绪任务竞争运行槽位时，按优先级值升序选取执行：值越小优先级越高

- 默认优先级：常规任务统一为默认值（`DEFAULT_PRIORITY = 10000`）。
- 高优先级：仅 debuggable 版本且应用属于免控包名集合时生效（`HIGH_PRIORITY = 0`），优先于常规任务调度。
- 公平轮转：被选中执行的任务优先级值自增，下次排序后移；同优先级任务间轮转执行。

### 执行频率约束

延迟任务的最小执行间隔由应用分组、充电状态、是否邮件应用、动态频率设置共同决定。

**默认场景（未充电、非邮件应用）**

| 应用分组 | 最小执行间隔 |
|---|---|
| 活跃分组（active group） | 2 小时 |
| 日常使用分组（daily used group） | 4 小时 |
| 固定分组（fixed group） | 24 小时 |
| 罕见使用分组（rare used group） | 48 小时 |
| 受限分组（restricted group） | 禁止执行 |
| 未使用分组（unused group） | 禁止执行 |

**充电场景（充电中，邮件与非邮件应用共用）**

| 应用分组 | 最小执行间隔 |
|---|---|
| 活跃 / 日常 / 固定 / 罕见分组 | 2 小时 |
| 受限分组 | 48 小时 |

> 邮件应用在充电中且为活跃分组时，进一步缩短为 20 分钟。

**邮件应用场景（未充电）**

应用为邮件应用且未充电时，采用专用间隔：

| 应用分组 | 最小执行间隔 |
|---|---|
| 活跃分组 | 30 分钟 |
| 日常使用分组 | 4 小时 |
| 固定分组 | 12 小时 |
| 罕见使用分组 | 24 小时 |
| 受限分组 | 48 小时 |

**动态执行频率（接口设置）**

通过 innerAPI `SetExecFrequency` / `ResetExecFrequency` 可对指定 UID 的任务自定义最小执行间隔，覆盖分组基准：

- `SetExecFrequency(FrequencyInfo)`：设置目标应用某任务的最小执行间隔。间隔不得小于 2 小时（`SET_INTERVAL_LOWER`）。
- `ResetExecFrequency(uid)`：清除调用方对该 UID 设置的动态频率，恢复分组基准。

### WorkInfo 约束

1. `workId`、`bundleName` 和 `abilityName` 为必填项
2. 至少设置一个条件（网络、充电、电池、存储、定时器等）
3. `repeatCycleTime` 至少 20 分钟（`minTimeCycle_ = 20 * 60 * 1000`），设置时须在 `isRepeat` 和 `repeatCount` 中选择其一
4. `parameters` 支持 number、string 和 bool 类型
5. 重复添加相同 `workId` 的任务返回 `E_ADD_REPEAT_WORK_ERR`

### 持久化约束

- `persisted = true` 的任务信息持久化存储到 JSON 文件，设备重启后自动恢复
- 非 `persisted` 的任务在服务重启后丢失
- 预安装任务（`preinstalled = true`）从配置文件加载

### 条件类型

`WorkCondition::Type` 枚举（定义于 `frameworks/include/work_condition.h`）：

| 条件类型 | 枚举值 | 说明 |
|---|---|---|
| `NETWORK` | 0 | 网络类型条件 |
| `CHARGER` | 1 | 充电类型条件 |
| `BATTERY_STATUS` | 2 | 电池状态条件 |
| `BATTERY_LEVEL` | 3 | 电池电量条件 |
| `STORAGE` | 4 | 存储状态条件 |
| `TIMER` | 5 | 定时器条件 |
| `GROUP` | 6 | 应用分组条件 |
| `DEEP_IDLE` | 7 | 深度空闲条件 |
| `STANDBY` | 8 | 设备待机条件 |
| `UNKNOWN` | 9 | 未知类型 |

### 任务状态流转

`WorkStatus::Status` 枚举（定义于 `services/native/include/work_status.h`）：

| 状态值 | 枚举名 | 说明 |
|---|---|---|
| 0 | `WAIT_CONDITION` | 等待条件满足 |
| 1 | `CONDITION_READY` | 条件已满足，待执行 |
| 2 | `RUNNING` | 正在执行 |
| 3 | `REMOVED` | 已移除 |

状态流转：`WAIT_CONDITION` →（条件全部满足）→ `CONDITION_READY` →（策略过滤通过）→ `RUNNING` →（正常完成/超时/用户停止）→ `REMOVED` 或重新回到 `WAIT_CONDITION`（重复任务）

## innerAPI 功能

### IPC 接口（IWorkSchedService.idl）

IDL 定义文件：`frameworks/IWorkSchedService.idl`

| 方法 | 说明 |
|---|---|
| `StartWork` | 启动延迟任务 |
| `StartWorkForInner` | 内部接口启动延迟任务 |
| `StopWork` | 停止延迟任务 |
| `StopWorkForInner` | 内部接口停止延迟任务，可强制取消执行中的任务 |
| `StopAndCancelWork` | 停止并取消延迟任务 |
| `StopAndClearWorks` | 停止并清除所有延迟任务 |
| `IsLastWorkTimeout` | 检查上次任务是否超时 |
| `ObtainAllWorks` | 获取所有延迟任务 |
| `ObtainWorksByUidAndWorkIdForInner` | 内部接口按 UID 和 workId 查询 |
| `GetWorkStatus` | 获取延迟任务状态 |
| `GetAllRunningWorks` | 获取运行中任务 |
| `PauseRunningWorks` | 暂停运行中任务 |
| `ResumePausedWorks` | 恢复暂停任务 |
| `SetWorkSchedulerConfig` | 设置配置（云配置下发） |
| `StopWorkForSA` | 停止 SA 关联的延迟任务 |
| `RegisterTask` | 注册后台预取任务 |
| `UnregisterTask` | 取消注册后台预取任务 |
| `FinishTask` | 完成后台预取任务 |
| `GetTaskInfo` | 查询后台预取任务信息 |
| `SetExecFrequency` | 设置执行频率 |
| `ResetExecFrequency` | 重置执行频率 |

### 回调接口（IWorkScheduler.idl）

IDL 定义文件：`services/zidl/IWorkScheduler.idl`

| 方法 | 说明 |
|---|---|
| `OnWorkStart` | 任务开始回调 |
| `OnWorkStop` | 任务停止回调 |

## 数据模型

以下数据结构体的具体字段定义详见代码设计文档 `docs/design/work_scheduler.md` 的"数据模型"章节。

- **WorkInfo**（`frameworks/include/work_info.h`）：延迟任务信息封装，继承 `Parcelable`，含 workId、包名、Ability 名、条件映射、持久化标记、额外参数等。
- **BackgroundLoaderTaskInfo**（`frameworks/include/background_loader_task_info.h`）：后台预取任务信息，含 taskId 和 abilityName。
- **FrequencyInfo**（`frameworks/include/frequency_info.h`）：执行频率信息，含 uid、workId、interval。
- **WorkStatus**（`services/native/include/work_status.h`）：延迟任务运行状态（内部），含任务状态、开始时间、优先级、条件映射等。
- **TaskInfo**（`services/native/include/background_loader_mgr.h`）：后台预取任务内部信息，含包名、Ability 名、任务状态、超时计数等。

## DFX 设计

### 可靠性设计

错误码体系统一定义于 `utils/native/include/work_sched_errors.h`，分为服务错误码（`E_PERMISSION_DENIED` 等）和参数错误码（`E_PARAM_NUMBER_ERR` 等）两类，含码到消息映射表。

持久化任务通过 `WorkSchedulerService::persistedMap_` 维护内存映射，通过 JSON 文件落盘，设备重启后从文件恢复。预安装任务从配置文件加载。`WorkSchedDataManager` 负责设备休眠/深度空闲状态和设备待机白名单/限制名单的内存管理。

超时处理采用 `Watchdog` 定时器机制：任务开始时创建定时器，超时时间根据能效资源申请状态、充电状态、深度空闲条件动态选取（2/10/20 分钟，详见"运行限制"），超时后触发 `WatchdogTimeOut` 回调，强制终止任务并标记 `lastTimeout_ = true`。

服务就绪状态通过 `std::atomic<bool> ready_` 门控，未就绪时拒绝请求。服务初始化时检查关键系统服务（应用管理、Bundle 管理、公共事件等）是否就绪，通过 `OnAddSystemAbility` / `OnRemoveSystemAbility` 监听系统服务状态变化。

后台预取器通过 `BackgroundLoaderMgr` 管理任务生命周期，支持超时计数（默认 3 次）和黑名单机制，超时次数超限的应用加入黑名单禁止注册任务。

### 安全性设计

- `StartWork` / `StopWork` 等公共 API 通过 `CheckCallingToken` 校验调用方 Token 类型
- `StartWorkForInner` / `StopWorkForInner` 等内部接口通过 `CheckProcessName` / `CheckCallingServiceName` 校验调用方进程名
- `PauseRunningWorks` / `ResumePausedWorks` 仅允许指定系统进程调用
- `SetWorkSchedulerConfig` / `SetExecFrequency` / `ResetExecFrequency` 需校验调用方权限
- `Dump` 诊断入口需 ENG 模式（`WorkSchedUtils::IsUserMode()` 返回 false）或 `ohos.permission.DUMP` 权限
- 通过 `WorkSchedUtils::IsSystemApp()` 判断系统应用身份，部分接口仅系统应用可调用

### 可扩展设计

- **条件监听器扩展**：`IConditionListener` 接口可扩展新的条件类型，在 `WorkQueueManager::Init()` 中注册
- **策略过滤器扩展**：`IPolicyFilter` 接口可扩展新的策略，在 `WorkPolicyManager::Init()` 中通过 `AddPolicyFilter` 注册
- **插件机制**：`WorkSchedPluginMgr` 继承 `ResourceSchedule::Plugin`，通过资源调度子系统 `PluginMgr` 接收系统资源事件

### 可配置设计

- **系统属性**：通过 `system::GetParameter` 读取系统属性参数
- **配置文件**：从 `etc/` 目录加载配置文件（豁免应用列表、预安装任务、最小重复周期等）
- **云配置下发**：通过 `SetWorkSchedulerConfig` IPC 接口接收云配置，更新最小重复时间、豁免应用列表、预安装任务等
- **Dump 调试**：通过 `hidumper` 调用，支持设置内存/CPU/热级别/最大运行数等调试参数

### 可维护设计

- **日志**：统一使用 `WS_HILOG*` 宏（`work_sched_hilog.h`），含隐私格式化标注
- **HiSysEvent 事件埋点**（定义于 `hisysevent.yaml`）：

| 上报场景 | 事件名 | 上报字段 |
|---|---|---|
| 添加延迟任务 | `WORK_ADD` | UID、PID、NAME、WORKID、TRIGGER、TYPE、INTERVAL |
| 移除延迟任务 | `WORK_REMOVE` | UID、PID、NAME、WORKID |
| 任务开始执行 | `WORK_START` | UID、PID、NAME、WORKID、TRIGGER、TYPE、INTERVAL、DELAY_REASON |
| 任务停止 | `WORK_STOP` | UID、PID、NAME、WORKID、REASON、DURATION |
| 深度空闲状态变更 | `DEEP_IDLE_STATE` | STATE |
| 设备待机状态变更 | `DEVICE_STANDBY_STATE` | STATE |
| 策略限制 | `SYSTEM_POLICY_LIMIT` | POLICY |
| 运行时异常 | `WORK_SCHEDULER_RUNTIME_EXCEPTION` | MODULE_NAME、FUNC_NAME、EXCEPTION_INFO |

- **Dump 诊断**：经 `hidumper` 调用，支持 `DumpUsage`（运行态信息）、`DumpCommonUsage`（常用信息）、`DumpUsageForUserMode`（用户模式信息）、`DumpProcessForEngMode`（ENG 模式调试命令，支持触发任务、设置内存/CPU/热级别等）
- **HiTrace**：关键入口标注链路追踪

### 兼容性设计

- **CJ FFI 接口**：提供 `CJ_StartWork` / `CJ_StartWorkV2` 等 V2 版本接口，兼容旧版和新版调用方式
- **Taihe IDL 接口**：提供同步和异步两种调用方式（如 `GetWorkStatusSync` 和异步 `GetWorkStatus`）
- **持久化 JSON 兼容**：新增字段时 `ParseFromJson` 须兼容旧版 JSON 数据，禁止删除已有字段
- **SysCap 声明**：`SystemCapability.ResourceSchedule.WorkScheduler`
