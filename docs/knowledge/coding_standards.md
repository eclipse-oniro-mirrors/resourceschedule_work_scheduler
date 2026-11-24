# 编码要求

> 文档版本：v1.0
> 更新时间：2026-09-10
> 本文档汇总延迟任务调度组件的编码与文档规范。

## 错误处理

| 规范项 | 要求 |
|---|---|
| 错误码 | 使用 `work_sched_errors.h` 定义的错误码，分为服务错误码（`E_PERMISSION_DENIED` 等）和参数错误码（`E_PARAM_NUMBER_ERR` 等）两类 |
| 错误码消息映射 | `saErrCodeMsgMap` 和 `paramErrCodeMsgMap` 维护码到消息的映射，新增错误码须同步更新映射表 |

## IPC 接口规范

新增 IPC 接口时使用 IDL 机制。IDL 接口文件 `frameworks/IWorkSchedService.idl` 定义了全部对外 IPC 方法，由 IDL 工具自动生成 Proxy 与 Stub 代码。`services/zidl/IWorkScheduler.idl` 定义了回调接口（`OnWorkStart` / `OnWorkStop`）。

## 编码规范

函数应尽量简洁，函数体不要超过 50 行，函数最大深度不要超过 4。
对于类似逻辑的代码尽量复用逻辑，不要产生重复代码。当要产生重复代码时，必须进行针对性重构。

## 并发场景处理

- 使用 `ffrt::mutex` 和 `ffrt::recursive_mutex` 保护共享数据，ffrt线程内执行的任务避免使用裸 `std::mutex`
- 业务处理逻辑在 `WorkEventHandler` 的事件线程中执行，保证无并发问题

## 日志规范

使用 `work_sched_hilog.h` 定义的 `WS_HILOG*` 宏（映射 HILOG），支持 F/E/W/I/D 五级日志。使用 `%{public}` 打印公开数据，使用 `%{private}` 打印隐私敏感数据。

## 条件编译规范

特性开关定义于 `workscheduler.gni`，代码中使用 `#ifdef` / `#if defined()` 守卫条件编译代码块。特性关闭时对应源文件不参与编译。

## 高风险边界（Do not / Ask before）

改码前须确认不触碰以下红线；如需变更须升级确认或先核对对应规格：

- **公共 API / IPC 接口**：`frameworks/IWorkSchedService.idl` 与 `services/zidl/IWorkScheduler.idl` 是 IPC 接口源头，Proxy/Stub 由 IDL 工具自动生成，禁止手改生成代码；改 IDL 即变更公共 API，须先核对 `docs/spec/work_scheduler.md` 的"接口说明"与兼容性。`interfaces/kits/` 下 JS/NAPI/CJ/Taihe 绑定为对外 API，签名/语义变更须兼容性评审。
- **权限 / 安全**：涉及 `ohos.permission.SET_WORK_SCHEDULER_PROPERTY` 等权限校验（`services/native/src/work_scheduler_service.cpp`），改权限校验逻辑须安全评审；Dump 诊断入口须保持 ENG 模式或 `ohos.permission.DUMP` 门控。
- **持久化数据兼容**：延迟任务 JSON 落盘由 `ParseFromJson` 解析，新增字段须兼容旧版 JSON，禁止删除已有字段（见 `docs/spec/work_scheduler.md` "兼容性设计"）。
- **错误码**：错误码定义于 `utils/native/include/work_sched_errors.h`，新增错误码须同步更新 `saErrCodeMsgMap` / `paramErrCodeMsgMap` 码到消息映射。
- **条件编译 / 特性开关**：特性开关定义于 `workscheduler.gni`，改动须保持 `#ifdef` / `#if defined()` 守卫与编译参与一致（详见"条件编译规范"）。
- **SysCap**：`SystemCapability.ResourceSchedule.WorkScheduler` 声明不得破坏。

## 文档命名规范

知识库采用按文档类型组织的结构，文档按 `knowledge/`、`spec/`、`design/` 三类目录组织，目录结构参考 `AGENTS.md`。

## 文档更新规范

新增或更新文档时遵循以下步骤：

1. **确定文档类型与目录**：根据内容性质选择 `knowledge/`、`spec/`、`design/` 对应目录。
2. **保持三类分离**：基本概念归入 `knowledge/`（除重大概念变更，轻易不做修改），规则、约束、功能演进方式归入 `spec/`，数据模型、上下文与场景、知识关联、演进与版本、代码与符号（类/方法/变量/IDL + 类图）归入 `design/`。
3. **保留图表语法**：所有 PlantUML 图必须保留 `@startuml`/`@enduml` 原始语法，表格不得裁剪。
4. **语言规范**：所有文档用中文撰写，固定文件名、命令名（如 `hb build`）、代码标识符（如类名 `WorkSchedulerService`）保持英文。
5. **更新索引**：新增/删除文档后，同步更新 `AGENTS.md` 中的知识库索引表，确保文档路径与说明一致。
6. **文档更新场景**：更新代码规格或架构时需要同步修改 `spec/`、`design/` 下文档。文档更新时需要同步更新最新版本更新时间和版本号。
