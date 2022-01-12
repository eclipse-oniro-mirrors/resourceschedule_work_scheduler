# 延迟任务调度

-   [简介](#section11660541593)
-   [目录](#section161941989596)
-   [延迟任务调度](#section1312121216216)
    -   [接口说明](#section114564657874)
    -   [使用说明](#section129654513264)
        -   [延迟任务调度使用约束](#section1551164914237)

-   [相关仓](#section1371113476307)

## 简介<a name="section11660541593"></a>

在资源调度子系统中，延迟任务调度部件给应用提供一个可以执行实时性不高的任务的机制。当满足设定条件时，会被放入可执行队列，系统根据设备情况，延迟触发可执行队列内的任务。

## 目录<a name="section161941989596"></a>

```

├── frameworks       # 接口实现
├── interfaces
│   ├── innerkits    # 对内接口目录
│   └── kits         # 对外接口目录
├── sa_profile       # 组件服务配置
├── services         # 组件服务实现
└── utils            # 组件工具实现

```
## 延迟任务调度<a name="section1312121216216"></a>

### 接口说明<a name="section114564657874"></a>

接口名                                                    |     接口描述                            
---------------------------------------------------------|-----------------------------------------
function startWork(work: WorkInfo): boolean; | 延迟调度任务申请 
function stopWork(work: WorkInfo, needCancel?: boolean): boolean;        | 延迟调度任务取消 
function getWorkStatus(workId: number, callback: AsyncCallback<WorkInfo>): void;| 获取延迟调度任务状态（Callback形式） 
function getWorkStatus(workId: number): Promise<WorkInfo>; | 获取延迟调度任务状态（Promise形式） 
function obtainAllWorks(callback: AsyncCallback<void>): Array<WorkInfo>;| 获取所有延迟调度任务（Callback形式） 
function obtainAllWorks(): Promise<Array<WorkInfo>>;| 获取所有延迟调度任务（Promise形式） 
function stopAndClearWorks(): boolean;| 停止并清除任务
function isLastWorkTimeOut(workId: number, callback: AsyncCallback<void>): boolean;| 获取上次任务是否超时（Callback形式）
function isLastWorkTimeOut(workId: number): Promise<boolean>;| 获取上次任务是否超时（Promise形式）

### 使用说明<a name="section129654513264"></a>
应用要执行对实时性要求不高的任务的时候，比如设备空闲时候做一次数据学习等场景，可以使用延迟调度任务，该机制在满足应用设定条件的时候，会根据系统当前状态，如内存、功耗、热等统一决策调度时间。

#### 延迟调度任务使用约束<a name="section1551164914237"></a>

延迟调度任务的使用需要遵从如下约束和规则：

- **超时**：每次最长运行120s。

## 相关仓<a name="section1371113476307"></a>

资源调度子系统

**work\_scheduler**

notification_ces_standard

appexecfwk_standard

powermgr_battery_manager