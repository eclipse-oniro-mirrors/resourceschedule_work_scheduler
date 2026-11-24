# SERVICES 模块

## 功能概述

服务端核心逻辑，实现延迟任务调度与管理。负责接收客户端请求、条件监听、策略过滤、任务执行与持久化存储。

## 子目录分布

- **native/**：核心服务实现
  - `include/`：核心组件头文件
  - `include/conditions/`：条件监听器
  - `include/policy/`：策略过滤器
  - `src/`：实现文件
- **zidl/**：IPC stub/proxy 实现
  - `include/`：stub 头文件

## 核心类说明

### WorkSchedulerService

主服务类，继承 `SystemAbility`，实现 `WorkSchedServiceStub`，是延迟任务调度的入口。

**关键成员变量：**

| 变量名 | 类型 | 作用 |
|--------|------|------|
| workQueueManager_ | shared_ptr<WorkQueueManager> | 条件队列管理器 |
| workPolicyManager_ | shared_ptr<WorkPolicyManager> | 策略与运行队列管理器 |
| handler_ | shared_ptr<WorkEventHandler> | 事件处理器 |
| persistedMap_ | map<string, WorkInfo> | 持久化任务映射 |
| whitelist_ | set<int32_t> | 效率资源白名单 UID |
| exemptionBundles_ | set<string> | 免控包名集合 |

**核心接口：**
- `StartWork/StopWork`：启动/停止延迟任务
- `OnConditionReady`：条件满足回调，触发任务执行
- `WatchdogTimeOut`：超时回调，强制终止任务
- `Dump`：调试信息输出

### WorkQueueManager

条件队列管理器，维护各条件类型的任务队列和监听器映射。

**关键成员变量：**

| 变量名 | 类型 | 作用 |
|--------|------|------|
| queueMap_ | map<Type, WorkQueue> | 条件类型 → 任务队列映射 |
| listenerMap_ | map<Type, IConditionListener> | 条件类型 → 监听器映射 |
| timeCycle_ | uint32_t | 定时器周期 |

**核心方法：**
- `Init()`：初始化监听器，订阅系统事件
- `AddListener()`：添加条件监听器
- `AddWork/RemoveWork`：添加/移除延迟任务
- `OnConditionChanged()`：条件变化回调，计算就绪队列

### WorkPolicyManager

策略与运行队列管理器，协调策略过滤器，管理就绪队列和运行队列。

**关键成员变量：**

| 变量名 | 类型 | 作用 |
|--------|------|------|
| uidQueueMap_ | map<int32_t, WorkQueue> | UID → 任务队列映射 |
| conditionReadyQueue_ | shared_ptr<WorkQueue> | 条件就绪队列 |
| policyFilters_ | list<IPolicyFilter> | 策略过滤器列表 |
| workConnManager_ | shared_ptr<WorkConnManager> | 连接管理器 |
| watchdog_ | shared_ptr<Watchdog> | 超时监控器 |
| watchdogIdMap_ | map<uint32_t, WorkStatus> | Watchdog ID → 任务映射 |
| currentRunningCount | int32_t | 当前运行任务数 |

**核心方法：**
- `OnConditionReady()`：处理条件就绪任务，添加到就绪队列
- `OnPolicyChanged()`：策略变化回调，调整运行上限
- `GetWorkToRun()`：按优先级获取待执行任务
- `RealStartWork()`：真正启动任务，拉起 Ability
- `AddWatchdogForWork()`：为任务添加超时监控

### WorkStatus

延迟任务状态类，封装任务的运行状态和条件匹配逻辑。

**状态枚举（Status）：**

| 状态值 | 说明 |
|--------|------|
| WAIT_CONDITION (0) | 等待条件满足 |
| CONDITION_READY (1) | 条件已满足，待执行 |
| RUNNING (2) | 正在执行 |
| REMOVED (3) | 已移除 |

**关键成员变量：**

| 变量名 | 类型 | 作用 |
|--------|------|------|
| workId_ | string | 任务唯一标识（workId + uid） |
| uid_ | int32_t | 用户 ID |
| currentStatus_ | Status | 当前状态 |
| workStartTime_ | uint64_t | 任务开始时间 |
| duration_ | uint64_t | 执行时长 |
| priority_ | int32_t | 执行优先级 |
| paused_ | bool | 是否暂停 |
| needRetrigger_ | bool | 是否需要重新触发 |
| minInterval_ | int64_t | 最小执行间隔 |
| conditionMap_ | map<Type, Condition> | 条件映射 |
| lastTimeout_ | bool | 上次是否超时 |

**核心方法：**
- `IsReady()`：判断所有条件是否满足
- `OnConditionChanged()`：处理单个条件变化
- `MarkStatus()`：标记任务状态
- `SetMinIntervalByGroup()`：根据应用分组设置最小间隔

### WorkQueue

任务队列类，按优先级排序管理任务列表。

**核心方法：**
- `OnConditionChanged()`：条件变化，返回就绪任务列表
- `Push()`：添加任务到队列
- `GetWorkToRunByPriority()`：按优先级获取任务
- `Remove()`：移除任务
- `GetRunningCount()`：获取运行任务数

### WorkConnManager

连接管理器，负责拉起 Ability 扩展并管理连接。

**核心方法：**
- `StartWork()`：拉起 Ability，建立连接
- `StopWork()`：断开连接，停止 Ability
- `WriteStartWorkEvent()`：写入启动事件日志

### Watchdog

超时监控器，确保任务在执行超时时终止。

**工作机制：**
- 任务开始时创建 Watchdog 定时器
- 超时后触发 `WatchdogTimeOut` 回调
- 强制终止超时任务

## 条件监听器（conditions/）

监听系统状态变化，触发延迟任务调度。

| 监听器 | 文件 | 监听内容 | 触发条件 |
|--------|------|----------|----------|
| NetworkListener | network_listener.h | 网络连接状态 | WiFi/蜂窝/以太网连接/断开 |
| BatteryLevelListener | battery_level_listener.h | 电池电量 | 电量变化超过阈值 |
| BatteryStatusListener | battery_status_listener.h | 电池状态 | 低电量/电量正常 |
| ChargerListener | charger_listener.h | 充电状态 | 充电/放电/充电类型变化 |
| ScreenListener | screen_listener.h | 屏幕状态 | 屏幕亮/灭；车机亮屏等同解锁 |
| StorageListener | storage_listener.h | 存储状态 | 存储空间低/正常 |
| TimerListener | timer_listener.h | 定时器 | 定时周期到期 |
| GroupListener | group_listener.h | 应用分组 | 应用分组变化 |

**IConditionListener 接口：**
- `OnConditionChanged(Type, DetectorValue)`：条件变化回调
- `Start()`：启动监听
- `Stop()`：停止监听

## 策略管理器（policy/）

根据系统状态过滤和控制延迟任务执行。

| 策略器 | 文件 | 控制逻辑 |
|--------|------|----------|
| PowerModePolicy | power_mode_policy.h | 省电模式限制运行数 |
| MemoryPolicy | memory_policy.h | 内存压力限制运行数 |
| ThermalPolicy | thermal_policy.h | 过热状态限制运行数 |
| CpuPolicy | cpu_policy.h | CPU 占用过高限制 |
| AppDataClearListener | app_data_clear_listener.h | 监听应用更新、应用退出、用户切换等场景清理任务 |

**IPolicyFilter 接口：**
- `GetPolicyMaxRunning(WorkSchedSystemPolicy)`：获取策略允许的最大运行数

## 核心功能流程

### 延迟任务调度流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                        延迟任务调度流程                                │
└─────────────────────────────────────────────────────────────────────┘

1. 任务添加
   ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
   │ 客户端调用    │───▶│ WorkScheduler │───▶│ WorkPolicy   │
   │ StartWork    │    │ Service       │    │ Manager      │
   └──────────────┘    │ .StartWork()  │    │ .AddWork()   │
                       └──────────────┘    └──────────────┘
                                               │
                                               ▼
                       ┌──────────────┐    ┌──────────────┐
                       │ WorkQueue    │◀───│ WorkQueue    │
                       │ Manager      │    │ (uidQueueMap)│
                       │ .AddWork()   │    │ 按UID存储    │
                       └──────────────┘    └──────────────┘
                           │
                           ▼
                       ┌──────────────┐
                       │ 创建监听器    │
                       │ 按条件类型    │
                       │ 添加到对应队列│
                       └──────────────┘

2. 条件监听
   ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
   │ 系统事件      │───▶│ Condition    │───▶│ WorkQueue    │
   │ (网络/电池等) │    │ Listener     │    │ Manager      │
   └──────────────┘    │ .OnCondition │    │ .OnCondition │
                       │ Changed()    │    │ Changed()    │
                       └──────────────┘    └──────────────┘
                                               │
                                               ▼
                       ┌──────────────────────────────────┐
                       │ 遍历 queueMap_，计算就绪任务列表    │
                       │ WorkQueue.OnConditionChanged()   │
                       │ → 返回满足条件的任务               │
                       └──────────────────────────────────┘

3. 条件就绪处理
   ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
   │ 就绪任务列表  │───▶│ WorkScheduler │───▶│ WorkPolicy   │
   │              │    │ Service       │    │ Manager      │
   └──────────────┘    │ .OnCondition │    │ .OnCondition │
                       │ Ready()      │    │ Ready()      │
                       └──────────────┘    └──────────────┘
                                               │
                                               ▼
                       ┌──────────────────────────────────┐
                       │ 添加到 conditionReadyQueue_       │
                       │ 标记状态 CONDITION_READY          │
                       │ CheckWorkToRun() 触发执行         │
                       └──────────────────────────────────┘

4. 策略过滤与执行
   ┌──────────────────────────────────┐
   │ WorkPolicyManager.CheckWorkToRun │
   └──────────────────────────────────┘
                    │
                    ▼
   ┌──────────────────────────────────┐
   │ GetMaxRunningCount()             │
   │ 遍历 policyFilters_ 获取策略上限  │
   └──────────────────────────────────┘
                    │
                    ▼
   ┌──────────────────────────────────┐
   │ GetWorkToRun()                   │
   │ 从 conditionReadyQueue_ 按优先级 │
   │ 获取待执行任务                    │
   └──────────────────────────────────┘
                    │
                    ▼
   ┌──────────────────────────────────┐
   │ RealStartWork()                  │
   │ WorkConnManager.StartWork()      │
   │ 拉起 Ability 执行任务            │
   │ AddWatchdogForWork() 添加监控    │
   └──────────────────────────────────┘

5. 超时监控
   ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
   │ Watchdog     │───▶│ 超时触发     │───▶│ WorkPolicy   │
   │ 定时器       │    │              │    │ Manager      │
   └──────────────┘    └──────────────┘    │ .Watchdog    │
                                           │ TimeOut()    │
                                           └──────────────┘
                                               │
                                               ▼
                       ┌──────────────────────────────────┐
                       │ StopWork() 强制终止任务           │
                       │ 断开 Ability 连接                │
                       │ 标记 lastTimeout_ = true         │
                       └──────────────────────────────────┘
```

### 状态流转

```
                    ┌─────────────────┐
                    │ WAIT_CONDITION  │
                    │ 等待条件满足     │
                    └─────────────────┘
                           │
                           │ 条件全部满足
                           │ OnConditionChanged()
                           ▼
                    ┌─────────────────┐
                    │ CONDITION_READY │
                    │ 条件就绪，待执行 │
                    └─────────────────┘
                           │
                           │ CheckWorkToRun()
                           │ RealStartWork()
                           ▼
                    ┌─────────────────┐
                    │    RUNNING      │
                    │ 正在执行         │
                    └─────────────────┘
                           │
           ┌───────────────┼───────────────┐
           │               │               │
           │ 正常完成       │ 超时          │ 用户停止
           │               │               │
           ▼               ▼               ▼
    ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
    │ 重复任务     │ │ 移除/标记    │ │   REMOVED   │
    │ 重新等待     │ │ 超时状态     │ │   已移除    │
    └─────────────┘ └─────────────┘ └─────────────┘
```

## IPC 接口（zidl/）

- `WorkSchedulerStubImp`：IPC stub 实现，处理客户端请求转发
- `WorkSchedulerStubAni`：ArkTS/NAPI 绑定 stub

**IDL 接口（IWorkSchedService.idl）：**
- StartWork、StopWork、StopAndCancelWork
- ObtainAllWorks、GetWorkStatus、GetAllRunningWorks
- IsLastWorkTimeout、PauseRunningWorks、ResumePausedWorks

## 开发规范

1. **线程安全**：使用 `ffrt::mutex` 和 `ffrt::recursive_mutex` 保护共享数据
2. **内存管理**：使用 `std::shared_ptr` 和 `std::weak_ptr` 管理生命周期
3. **事件处理**：通过 `WorkEventHandler` 在事件线程处理异步操作
4. **日志输出**：使用 `WS_HILOG*` 系列宏，敏感数据不使用 `%{public}`
5. **条件编译**：使用 `#ifdef DEVICE_USAGE_STATISTICS_ENABLE` 等宏控制功能开关
6. **状态一致性**：任务状态变更需同步更新队列和 Watchdog 映射