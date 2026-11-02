# 方案建议：WorkInfo 增加 isPerformanceSensitive

> 变更编号：20261020311883
> 关联 PR：[openharmony/resourcechedule_work_scheduler#793](https://gitcode.com/openharmony/resourcechedule_work_scheduler/pull/793)
> 编写日期：2026-10-30

## 1. 问题分析

### 1.1 现状

延迟任务调度组件当前对所有任务一视同仁：任务在条件就绪后进入就绪队列，按优先级选取执行，看门狗超时后强制终止。任务运行在哪个 CPU 核心域由系统调度器决定，组件无法干预。

存在的问题：
- 部分业务需要更高性能（高性能 CPU 核心），但无法表达该诉求
- 若所有任务都倾向于高性能核心，会导致高性能核心资源争用与功耗上升
- 缺少对"高性能任务"的频率与全局额度管控

### 1.2 需求

| 需求编号 | 描述 |
|---|---|
| REQ-1 | 应用可标记延迟任务为"性能敏感"，表示期望在高性能 CPU 核心上运行 |
| REQ-2 | 性能敏感任务仅在深度空闲 + 充电场景触发，避免影响日常性能 |
| REQ-3 | 单应用每日最多执行一次性能敏感任务 |
| REQ-4 | 全设备当日累计高性能运行时长有上限（10 分钟） |
| REQ-5 | 多个性能敏感任务避免同时启动，通过离散窗口错峰 |
| REQ-6 | 新字段向后兼容，不影响现有任务行为 |

## 2. 方案选择

### 2.1 方案 A：WorkInfo 新增 isPerformanceSensitive 字段（采纳）

在 `WorkInfo` 中新增 `bool isPerformanceSensitive` 字段，从申请端标记性能敏感任务，服务侧在条件校验、就绪判定、调度策略三个阶段施加约束。

**优势：**
- 字段语义清晰，应用侧设置简单
- 复用现有 `WorkInfo` → `WorkStatus` → `WorkPolicyManager` 调用链，改动集中
- 持久化 JSON 天然兼容（缺失字段默认 false）

**劣势：**
- IPC Parcel 序列化顺序变更，需端到端同版本（持久化 JSON 不受影响）

### 2.2 方案 B：独立 API startPerformanceSensitiveWork

新增独立接口 `startPerformanceSensitiveWork(workInfo)`，与 `startWork` 并行。

**优势：** 接口语义显式
**劣势：**
- 新增公共 API，需走 API 审批流程
- IDL 接口变更，影响 Proxy/Stub 生成
- 与现有 `startWork` 逻辑大量重复，维护成本高

**结论：** 不采纳，公共 API 变更成本过高且违反"渐进式扩展"原则。

### 2.3 方案 C：通过 parameters 透传

应用在 `parameters` 中以自定义 key 传递性能敏感标记，服务侧解析。

**优势：** 零字段变更
**劣势：**
- 无类型保证，依赖字符串约定
- 无法在 IPC 序列化层强制校验
- 持久化 JSON 中难以规范化
- 与 `parameters` 的"自定义参数"语义混淆

**结论：** 不采纳，缺乏类型安全与规范化能力。

## 3. 采纳方案详细设计

### 3.1 字段流转链路

```
应用 (napi/taihe)
  → WorkInfo.isPerformanceSensitive_ (frameworks)
  → Parcel 序列化 (Marshalling/Unmarshalling)
  → WorkSchedulerService.StartWorkInner
  → CheckPerformanceSensitiveCondition() 条件校验
  → WorkStatus (services)
  → IsReady() → IsPerformanceSensitiveReady() 就绪判定
  → WorkPolicyManager.CheckWorkToRun() → RealStartWork/DiscreteScheduled
  → StartHighPerfTimer/StopHighPerfTimer 高性能计时
  → UpdateUidLastTimeMap() 频率控制记录
```

### 3.2 约束策略分层

| 阶段 | 约束 | 实现 |
|---|---|---|
| 申请校验 | `isDeepIdle=true` + `isCharging=true` + 重复周期 `>= 24h` | `CheckPerformanceSensitiveCondition` |
| 就绪判定 | 单应用 24h 频率控制 + 当日全局 10min 限额 | `IsPerformanceSensitiveReady` |
| 调度策略 | 离散窗口错峰 + 固定 2min 看门狗 | `DiscreteScheduled` / `UpdateWatchdogTime` |

### 3.3 高性能计时方案

采用进程级单例计时（`highPerfStartTime_`），而非每任务独立计时：

- **启动**：第一个性能敏感任务启动时记录起始时间，后续任务启动时不覆盖
- **停止**：最后一个性能敏感任务停止时结算累计时长
- **跨天**：与 `lastHighPerfEndTime_` 比较，跨天则重置 `highPerfDuration_`

理由：全局限额是"当日全设备累计"，进程级单例计时天然匹配该语义，无需逐任务累加。

### 3.4 离散调度方案

仅在 6:00-9:00 的 0 分和 30 分窗口触发，随机延迟 1~120 秒：

- 窗口选择：凌晨低负载时段，避免影响用户使用
- 随机延迟：以 `getOppositeTime()+getpid()+gettid()` 为种子，避免多任务同时启动
- 适用范围：系统应用 + 网络条件 + 性能敏感任务

### 3.5 锁方案

将原 `s_uid_last_time_mutex` 与 `dumpAppGroupMutex_` 合并为单一静态 `mutex_`：

- 减少锁数量，简化锁层次
- 所有静态 map 与计数字段统一保护
- 锁获取顺序：`conditionMapMutex_` → `mutex_`，无反向获取，无死锁风险

## 4. 兼容性评估

| 维度 | 兼容性 | 说明 |
|---|---|---|
| 持久化 JSON | 兼容 | `IsHasBoolProp` 容错，旧文件无字段时默认 `false` |
| IPC Parcel | 不兼容 | 字段写入位置变更，需端到端同版本 |
| napi/taihe 接口 | 兼容 | `Optional<bool>`，未设置时默认 `false` |
| 错误码 | 新增 | 不影响已有错误码 |
| 非性能敏感任务 | 无影响 | `isPerformanceSensitive=false` 时所有新增逻辑短路返回 |

## 5. 风险与缓解

| 风险 | 等级 | 缓解措施 |
|---|---|---|
| IPC 跨版本不兼容 | 中 | 升级期间保证端到端同版本；持久化数据兼容 |
| 高性能计时为进程级单例 | 低 | 设计预期：多任务并发时第一个启动/最后停止结算 |
| `StopHighPerfTimer` 上报 type 语义不一致 | 低 | 已知待优化，不影响功能 |
| `workStartTime_` 读取未加 `statusMutex_` | 低 | 已知数据竞争，待后续优化 |

## 6. 验证策略

```bash
hb build --build-target work_scheduler
hb build --build-target test_work_scheduler_all
```

| 验证层 | 覆盖点 |
|---|---|
| 单元测试 | 字段读写、条件校验、就绪判定、高性能计时、离散调度 |
| 接口测试 | napi/taihe/ANI 字段透传 |
| 兼容性测试 | 旧持久化 JSON 文件加载 |
