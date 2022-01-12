# Work Scheduler

-   [Introduction](#section11660541593)
-   [Directory Structure](#section161941989596)
-   [Work Scheduler](#section1312121216216)
    -   [Available APIs](#section114564657874)
    -   [Usage Guidelines](#section129654513264)
        -   [Restrictions on Using Work Scheduler](#section1551164914237)

-   [Repositories Involved](#section1371113476307)

## Introduction<a name="section11660541593"></a>

When an application needs to perform tasks that do not require high real-time performance, such as data learning when the device is idle, the work scheduler can be used. When the application setting conditions are met, the mechanism will uniformly decide the scheduling time according to the current state of the system, such as memory, power consumption, heat, etc.

## Directory Structure<a name="section161941989596"></a>

```

├── frameworks       # Frameworks
├── interfaces
│   ├── innerkits    # Internal APIs
│   └── kits         # External APIs
├── sa_profile       # SA profile
├── services         # Services
└── utils            # Utilities

```
## Work Scheduler<a name="section1312121216216"></a>

### Available APIs<a name="section114564657874"></a>

API                                                      |     Description                         
---------------------------------------------------------|-----------------------------------------
function startWork(work: WorkInfo): boolean; | work scheduler application
function stopWork(work: WorkInfo, needCancel?: boolean): boolean;        | work scheduler cancel 
function getWorkStatus(workId: number, callback: AsyncCallback<WorkInfo>): void;| get the status of work（Callback form） 
function getWorkStatus(workId: number): Promise<WorkInfo>; | get the status of work（Promise form） 
function obtainAllWorks(callback: AsyncCallback<void>): Array<WorkInfo>;| get all works（Callback form） 
function obtainAllWorks(): Promise<Array<WorkInfo>>;| get all works（Promise form） 
function stopAndClearWorks(): boolean;| stop and clear work
function isLastWorkTimeOut(workId: number, callback: AsyncCallback<void>): boolean;| Get whether the last task has timed out（Callback form）
function isLastWorkTimeOut(workId: number): Promise<boolean>;| Get whether the last task has timed out（Promise form）

### Usage Guidelines<a name="section129654513264"></a>

When an application needs to perform tasks with low real-time performance, work scheduler  can be used. When the conditions set by the application are met, the mechanism will make a unified decision and scheduling time based on the current state of the system, such as memory, power consumption, and heat.

#### Restrictions on Using Work Scheduler<a name="section1551164914237"></a>

Adhere to the following constraints and rules when using work scheduler:

- **Timeout**：The longest running time is 120s each time.

## Repositories Involved<a name="section1371113476307"></a>

Resource Schedule subsystem

**work\_scheduler**

notification_ces_standard

appexecfwk_standard

powermgr_battery_manager