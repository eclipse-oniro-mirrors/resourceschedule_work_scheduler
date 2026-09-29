# 业务背景

> 文档版本：v1.0
> 更新时间：2026-09-10
> 本文档介绍延迟任务调度组件的业务定位、核心功能、子系统归属、组件信息及与相近子系统的关系。

## 组件定位

延迟任务调度（Work Scheduler）是 OpenHarmony 资源调度子系统的核心组件，负责在应用退至后台后为时效性要求不高的任务提供延迟调度能力。当应用满足设定的触发条件（包括网络类型、充电类型、存储状态、电池状态、定时状态等）时，将任务添加到执行队列，系统根据内存、功耗、设备温度、用户使用习惯等统一调度拉起应用，执行相应的延迟任务。组件位于 `foundation/resourceschedule/work_scheduler` 目录下。

## 核心功能

| 功能 | 说明 | 服务入口类 |
|---|---|---|
| 延迟任务调度（Work Scheduler） | 条件监听、策略过滤、任务队列管理、进程拉起、超时监控 | `WorkSchedulerService` |
| 后台预取器（Background Loader） | 面向应用的轻量级后台任务加载，通过 IPC 远程对象拉起 Ability | `BackgroundLoaderMgr` |

## 子系统归属

`resourceschedule` → `work_scheduler`

## 组件信息

| 项目 | 值 |
|---|---|
| 组件名 | `work_scheduler` |
| 子系统 | `resourceschedule` |
| 构建系统 | GN + Ninja |
| 构建配置 | `BUILD.gn`（根目录）、`workscheduler.gni`（路径与特性开关定义）、`bundle.json`（组件元数据） |
| 系统服务 ID | 1904（`WORK_SCHEDULE_SERVICE_ID`） |
| 进程名 | `resource_schedule_service` |
| SysCap | `SystemCapability.ResourceSchedule.WorkScheduler` |

## 与相近子系统的关系

延迟任务调度组件作为资源调度子系统的核心组件，在运行时与多个子系统和系统服务协作完成任务调度与执行。

各协作关系说明：

- **资源调度子系统**：延迟任务调度组件归属资源调度子系统，通过 `services/plugin/` 插件框架接入资源调度服务（`resource_schedule_service`）的事件分发体系。`WorkSchedPluginMgr` 继承 `ResourceSchedule::Plugin`，通过 `DispatchResource` 接收系统资源事件（如云配置更新、后台任务状态变化等）。
- **后台任务管理（background_task_mgr）**：延迟任务通过 `SchedulerBgTaskSubscriber` 订阅能效资源申请/释放事件，维护效率资源白名单（`whitelist_`）。申请了 `WORK_SCHEDULER` 能效资源的应用享有更宽松的执行频率约束。该依赖通过条件编译 `RESOURCESCHEDULE_BGTASKMGR_ENABLE` 控制。
- **设备使用信息统计（device_usage_statistics）**：通过 `WorkBundleGroupChangeCallback` 监听应用使用频率分组变化，`GroupListener` 据此调整任务执行频率约束。该依赖通过条件编译 `DEVICE_USAGE_STATISTICS_ENABLE` 控制。
- **设备待机（device_standby）**：通过 `WorkStandbyStateChangeCallback` 监听设备待机状态变化，待机模式下限制后台任务执行，支持白名单和限制名单。该依赖通过条件编译 `DEVICE_STANDBY_ENABLE` 控制。
- **公共事件（common_event_service）**：通过公共事件监听应用安装/卸载/更新、用户切换等事件，`AppDataClearListener` 据此清理对应任务。
- **包管理（bundle_framework）**：查询应用包信息，验证 Ability 声明，判断系统应用身份。
- **元能力（ability_runtime）**：通过 `WorkConnManager` 调用 `AbilityManager` 拉起 `WorkSchedulerExtensionAbility` 执行延迟任务。
- **电源管理（power_manager / battery_manager / thermal_manager）**：省电模式策略读取电源模式，电池状态/电量监听器读取电池信息，热管理策略读取设备温度。各依赖通过条件编译控制。
- **网络管理（netmanager_base）**：网络监听器查询网络连接状态。该依赖通过条件编译 `COMMUNICATION_NETMANAGER_BASE_ENABLE` 控制。
- **定时器服务（time_service）**：定时器监听器通过时间服务实现周期性定时触发。
- **数据共享（data_share）**：通过 `WorkDataShareHelper` 访问 DataShare URI 关联的数据。
