# 任务说明：WorkInfo 增加 isPerformanceSensitive

> 变更编号：20261020311883
> 关联 PR：[openharmony/resourcechedule_work_scheduler#793](https://gitcode.com/openharmony/resourcechedule_work_scheduler/pull/793)
> 关联 Issue：[新需求] workInfo 增加 isPerformanceSensitive
> 编写日期：2026-10-30

## 1. 任务背景

延迟任务（Work Scheduler）为时效性要求不高的后台任务提供条件触发与统一调度。现有任务运行在看门狗超时限制内（默认 2~20 分钟），不区分运行所用的 CPU 核心域。

部分业务需要在高性能 CPU 核心上运行延迟任务以获得更高性能，但高性能核心资源稀缺，若无节制调度会导致功耗与性能抖动。需要新增"性能敏感"标记，对此类任务施加严格的条件约束、频率控制与全局限额。

## 2. 任务目标

| 目标 | 说明 |
|---|---|
| 能力扩展 | `WorkInfo` 新增 `isPerformanceSensitive` 布尔字段，贯穿 IPC 序列化、持久化 JSON、napi/taihe/ANI 接口 |
| 条件约束 | `isPerformanceSensitive=true` 时强制要求 `isDeepIdle=true` 且 `isCharging=true`；重复任务 `repeatCycleTime >= 24h` |
| 运行频率控制 | 单应用 24 小时内仅允许执行一次性能敏感任务 |
| 全局限额 | 当日全设备性能敏感任务累计高性能运行时长上限 `MAX_HIGH_PERF_DURATION`（10 分钟） |
| 离散调度 | 系统应用 + 网络条件的性能敏感任务在 6:00-9:00 的 0/30 分窗口随机延迟 1~120s 启动，避免并发核心争用 |
| DFX 可观测 | 上报 `RES_TYPE_DELAY_TASK_PERF` 事件，支持 hidumper 设置高性能累计时长 |

## 3. 任务范围

### 3.1 涉及模块

| 层级 | 模块 | 改动概述 |
|---|---|---|
| 规格文档 | `docs/spec/`、`docs/design/` | 新增字段定义与约束说明 |
| 接口绑定 | `interfaces/kits/js/napi/`、`interfaces/kits/ets/taihe/` | napi/taihe/ANI 读写新字段 |
| 客户端框架 | `frameworks/include/work_info.h`、`frameworks/src/work_info.cpp` | 字段定义、序列化、JSON 持久化 |
| 核心服务 | `services/native/` | 条件校验、就绪判定、高性能计时、离散调度、dump |
| 条件监听 | `services/native/src/conditions/screen_listener.cpp` | 深度空闲屏幕关闭时长可配 |
| 公共常量 | `utils/native/include/` | 新增限额常量与错误码 |
| 测试 | `frameworks/test/`、`services/test/` | 单元测试覆盖 |

### 3.2 不涉及

- 后台预取器（BackgroundLoader）模块不改动
- 频率控制（FrequencyControl）独立模块不改动
- 插件框架不改动

## 4. 验收标准

| 编号 | 验收项 | 验证方式 |
|---|---|---|
| AC-1 | `WorkInfo` 新增 `isPerformanceSensitive` 字段，默认 `false` | 编译通过 + `workinfo_test.cpp` 用例 |
| AC-2 | `isPerformanceSensitive=true` 时未设置 `isDeepIdle`/`isCharging` 返回 `E_PERFORMANCE_SENSITIVE_CONDITION_ERR` | `workschedulerservice_test.cpp` 用例 |
| AC-3 | 重复任务 `repeatCycleTime < 24h` 时返回 `E_PERFORMANCE_SENSITIVE_CONDITION_ERR` | `workschedulerservice_test.cpp` 用例 |
| AC-4 | 单应用 24h 内第二次执行性能敏感任务不就绪 | `work_status_test.cpp` 用例 |
| AC-5 | 当日累计高性能运行时长超 10 分钟时不就绪 | `work_status_test.cpp` 用例 |
| AC-6 | 高性能计时启动/停止/累加/跨天重置逻辑正确 | `work_status_test.cpp` 用例 |
| AC-7 | 离散调度窗口（6:00-9:00, 0/30 分）正确识别 | 代码审查 + 逻辑验证 |
| AC-8 | napi/taihe/ANI 接口读写 `isPerformanceSensitive` 正确 | 接口层代码审查 |
| AC-9 | 持久化 JSON 向后兼容（旧文件无字段时默认 `false`） | `IsHasBoolProp` 容错检查 |
| AC-10 | 构建 `hb build --build-target work_scheduler` 通过 | CI 构建组 |
| AC-11 | 测试 `hb build --build-target test_work_scheduler_all` 通过 | CI 测试组 |

## 5. 非目标

- 不改变非性能敏感任务的现有调度行为
- 不引入新的公共 API 方法（仅扩展 `WorkInfo` 字段）
- 不修改 IPC IDL 接口定义文件（`IWorkSchedService.idl`）

## 6. 依赖与约束

| 依赖 | 说明 |
|---|---|
| `ResourceSchedule::ResType::RES_TYPE_DELAY_TASK_PERF` | DFX 事件类型，需资源调度服务侧支持 |
| `OHOS::system::GetIntParameter` | 用于 debuggable 判定 |
| `DeviceUsageStats::BundleActiveClient` | 用于应用分组查询（已有依赖） |

| 约束 | 说明 |
|---|---|
| 文件换行符 | CRLF（项目宪法要求） |
| IPC 兼容性 | Parcel 序列化顺序变更，需端到端同版本升级 |
| 持久化兼容 | JSON 须向后兼容旧持久化文件 |
