CODEAGENT.md
This file provides guidance to CodeAgent when working with code in this reposity.

# 延迟任务调度组件开发指南

> 文档版本：v2.0
> 更新时间：2026-09-10
> 本文件为 Level0 宪法 + 知识路由，仅提供项目概述与到各知识库文档的路径导航。
> 详细内容请查阅对应层级的 docs 文档。

## 1. 项目概述

**功能定位**：延迟任务（Work Scheduler）调度管理，为时效性要求不高的后台任务提供条件触发、策略过滤、统一调度和超时监控能力。支持后台预取器（Background Loader）和执行频率控制（Frequency Control）。

**子系统归属**：`resourceschedule` → `work_scheduler`

## 2. 开发要求

1. 渐进式披露：按任务场景按需加载文档，禁止全量加载；路由见 §3。改码前须声明任务类别、已加载文档与已发现的约束。
2. 改码红线（Do not / Ask before）：公共 API/IDL、权限、持久化 JSON 兼容、错误码映射、条件编译特性开关、SysCap，变更须升级确认；详见 `docs/knowledge/coding_standards.md` 的"高风险边界"。
3. 验证与 Done：构建 `hb build --build-target work_scheduler`、测试 `hb build --build-target test_work_scheduler_all`（本仓无独立 lint 目标，规范见 `docs/knowledge/coding_standards.md`）；Done = 构建组通过 + 受影响 UT/FUZZ 通过 + 未破坏公共 API/IDL/持久化兼容（或标注待人工核对）；无法本地验证时须明确标注并给人工验证项，不得默认通过。明细见 `docs/knowledge/architecture.md` 的"编译方式"。
4. 文件换行符必须 CRLF，禁止 LF。

## 3. 知识库路由

知识库按文档类型组织，分三层：Level1-Level2 知识（`docs/knowledge/`）、Level3 特性规格（`docs/spec/`）、Level3 功能设计（`docs/design/`）。
需要按照场景加载知识库文件，非必要不加载。

### 3.1 知识库（Level1-Level2）— 跨模块通用知识

| 文档 | 路径 | 内容 | 加载场景 |
|---|---|---|---|
| 业务术语表 | [docs/knowledge/glossary.md](docs/knowledge/glossary.md) | 延迟任务/后台预取器/频率控制/条件监听/策略管理等术语定义 | 遇到不熟悉的业务术语或需区分模块概念时加载 |
| 业务背景 | [docs/knowledge/business_context.md](docs/knowledge/business_context.md) | 组件定位、核心功能、子系统归属、组件信息、与相近子系统关系 | 了解组件定位、核心功能或子系统归属时加载 |
| 编码要求 | [docs/knowledge/coding_standards.md](docs/knowledge/coding_standards.md) | 错误码、IDL 接口规范、编码规范、并发处理、日志规范、条件编译、高风险边界（Do not / Ask before） | 编写或审查代码时加载（错误码、日志、条件编译、IDL 规范、改码红线适用） |
| 架构原则 | [docs/knowledge/architecture.md](docs/knowledge/architecture.md) | 三层架构、SA 概念、目录结构、编译方式、构建特性开关、部署拓扑、公共基础设施 | 编写或审查代码时，需理解三层架构、SA 注册启动、部署拓扑或公共基础设施时加载 |

### 3.2 特性规格（Level3）— 模块规格描述

| 文档 | 路径 | 内容 | 加载场景 |
|---|---|---|---|
| 延迟任务规格 | [docs/spec/work_scheduler.md](docs/spec/work_scheduler.md) | 接口说明、规则与约束（运行限制/频率约束/WorkInfo 约束/条件类型/状态流转）、innerAPI、数据模型、DFX 设计 | 涉及延迟任务规格行为的设计或分析时加载 |

### 3.3 功能设计（Level3）— 模块代码实现设计

| 文档 | 路径 | 内容 | 加载场景 |
|---|---|---|---|
| 公共框架代码设计 | [docs/design/overview.md](docs/design/overview.md) | IPC 调用链、部署拓扑、核心组件交互、插件能力、类继承关系 | 需理解 IPC 调用链、部署拓扑、跨模块交互或公共基础设施时加载 |
| 延迟任务代码设计 | [docs/design/work_scheduler.md](docs/design/work_scheduler.md) | 运行时序、知识关联、演进版本、核心类/方法、条件监听器、策略过滤器、数据模型、关键数据标记、DFX 实现 | 实际设计/实现延迟任务代码时加载 |

### 3.4 路径级路由

改码或排查特定目录时，对应加载的文档：

| 改动路径 | 加载文档 |
|---|---|
| `interfaces/kits/`（对外 API 绑定） | `docs/spec/work_scheduler.md`（接口说明/枚举）+ `docs/design/overview.md`（IPC 调用链） |
| `frameworks/`（客户端代理 / IDL） | `docs/design/overview.md`（IPC 调用链）+ `docs/knowledge/coding_standards.md`（IDL 规范） |
| `services/native/`（核心服务） | `docs/design/work_scheduler.md`（核心类/时序/数据模型）+ `docs/spec/work_scheduler.md`（规格/约束） |
| `services/zidl/`（IPC stub/proxy、回调 IDL） | `docs/knowledge/coding_standards.md`（IDL 规范） |
| `services/plugin/`（插件框架） | `docs/design/overview.md`（插件能力/事件分发） |
| `utils/native/`（错误码/常量/日志/工具） | `docs/knowledge/architecture.md`（公共基础设施索引） |

### 3.5 高频改动路径

| 路径 | 职责 |
|---|---|
| `services/native/src/work_scheduler_service.cpp` | 服务入口、IPC 分发、权限校验 |
| `services/native/src/work_policy_manager.cpp` | 策略过滤、就绪/运行队列、看门狗超时选取、优先级选取 |
| `services/native/src/work_status.cpp` | 任务状态、分组频率计算、优先级 |
| `services/native/src/work_queue.cpp` / `work_queue_manager.cpp` | 条件队列、就绪计算、优先级排序 |
| `frameworks/IWorkSchedService.idl` | 对外 IPC 接口源头（生成 Proxy/Stub） |
| `utils/native/include/work_sched_errors.h` / `work_sched_constants.h` | 错误码 + 映射表、运行限制/超时常量 |

> 完整目录结构与公共基础设施索引详见 `docs/knowledge/architecture.md` 的"目录结构总览""公共基础设施索引"。

---

> 💡 **提示**：本文件为 Level0 宪法，仅提供概述与路由。业务概念查阅 `docs/knowledge/`，规格查阅 `docs/spec/`，代码设计查阅 `docs/design/`。
