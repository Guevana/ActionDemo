# ActionDemo

基于 Unreal Engine 5.7 的高机动 ARPG 战斗系统 Demo，重点验证高频操作反馈、动画取消、输入缓冲、锁敌与近战命中反馈等核心战斗体验。

## 项目目标

- 在 UE 5.7 中以 GAS + StateTree + 组件化战斗框架实现逻辑与表现解耦
- 体验重点：连段、取消窗口、输入缓冲、空中派生、锁敌、命中反馈、强表现技能演出
- 工程目标：保持原子化 Ability、标签驱动、数据驱动连段，避免把战斗逻辑回流到角色类

## 技术基线

- 引擎：Unreal Engine 5.7
- 语言：C++ 主导核心逻辑链路，蓝图负责资产配置、动画/特效/镜头/UI 表现微调
- 主 IDE：JetBrains Rider
- 版本控制：Git + Git LFS（`.uasset` / `.umap` 走 LFS）

### 关键依赖模块

| 模块 | 用途 |
| --- | --- |
| GameplayAbilities | Ability、AttributeSet、GameplayEvent、GameplayTag |
| StateTree / GameplayStateTreeModule | 玩家与敌人的宏观战斗状态决策 |
| EnhancedInput | 输入映射、输入标签、输入缓冲入口 |
| MotionWarping | 攻击锁定时的位移与朝向修正 |
| AIModule | 敌人 AIController、感知、移动任务 |

## 目录结构

```
ActionDemo.uproject          UE 项目入口
Source/ActionDemo/           Runtime 主模块（C++）
  Core/                      项目级标签、公共类型、日志
  Character/                 角色基类、玩家角色、敌人角色
  Components/                战斗、输入缓冲、锁敌、命中检测、武器管理等组件
  AbilitySystem/             ASC、AttributeSet、Ability 基类、攻击/受击 Ability、GE、数据资产
  Animation/Notifies/        AD Cancel Window、AD Hit Detection 等动画通知状态
  Equipment/                 武器 Actor 基类
  Tree/StateTree/            玩家与敌人的 Conditions / Evaluators / Tasks
  Game/                      PlayerController 输入分发与 Ability 授予
  AI/                        敌人 AIController（感知 / 目标 / StateTree）
  Input/                     InputConfigData
Content/                     蓝图、Montage、动画、数据资产等
ProjectContext.md            长期稳定的架构边界与协作规则（权威）
ProjectWiki.md               模块、逻辑链、标签、资产、规划、决策的查询型数据库（权威）
CLAUDE.md                    Claude Code 入口提示
```

## 架构边界

- `AADCharacterBase` 是所有战斗实体基类，仅挂载并暴露 ASC、AttributeSet、Combat、AbilityQueue、Targeting、HitDetection、WeaponManager、MotionWarping 等组件
- `AADPlayerCharacter` 保持轻量，负责相机、玩家 StateTree 与取消窗口预输入重发
- `AADPlayerController` 负责 Enhanced Input 绑定、MappingContext 注入、启动 Ability 授予、Free / LockOn 移动模式切换
- `AADEnemyCharacter` 负责敌人配置、启动 Ability 授予与受击入口；感知、追击、目标事实、宏观决策归 `AADEnemyAIController`
- StateTree 决定“应该做什么”，Gameplay Ability 决定“动作如何执行”，组件维护可查询事实，表现层只消费事件和标签

## 已实现的核心战斗链路

来自 `ProjectWiki.md` 的当前事实，详见 Wiki 内的 Logic Chain Database：

- 输入缓冲与轻攻击连段（`LC-Input-Player`、`LC-Combo`）
- 攻击 Ability 生命周期与取消窗口（`LC-AttackAbility`、`LC-CancelWindow`、`LC-ActionContext`）
- 命中检测、命中确认与 GAS GameplayEvent 投递（`LC-HitDetection`）
- 命中到伤害、受击硬直、死亡清理闭环（`LC-Damage-ReceiveHit`、`LC-HitReact`）
- 锁敌目标事实、锁定朝向、锁定镜头、锁定移动模式（`LC-TargetLock`、`LC-LockOnFacing`、`LC-LockOnCamera`、`LC-LockOnMovement`）
- 攻击锁定 Motion Warping 写入（Combo1 已验收，`LC-LockOnWarpTarget`）
- 武器生成、附着、切换（`LC-WeaponManagement`，C++ 已编译，资产配置进行中）
- 敌人首版近战 AI（`LC-EnemyAI`，首版）

未稳定的后续系统：弹刀、极限闪避、空中连段、完整伤害公式、处决、Boss 阶段、完整运镜演出（详见 `ProjectWiki.md` Planning Database）。

## 构建

UE 5.7 项目，`.uproject` 位于仓库根目录。本机默认引擎路径 `E:\UE_5.7`；若该路径不存在，请先探测本机 UE 5.7 安装路径，不要硬编码替换路径。

通过 UBT 构建 Editor（Development）：

```powershell
& "E:\UE_5.7\Engine\Build\BatchFiles\Build.bat" ActionDemoEditor Win64 Development "D:\AAA_UEProjects\ActionDemo\ActionDemo.uproject" -WaitMutex
```

或直接调用 UnrealBuildTool：

```powershell
& "E:\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" ActionDemoEditor Win64 Development -Project="D:\AAA_UEProjects\ActionDemo\ActionDemo.uproject"
```

也可以打开 `ActionDemo.sln`，从 Rider 或 Visual Studio 启动 Editor。

如编辑器占用 RiderLink 导致编译冲突，可临时追加 `-NoHotReloadFromIDE -DisablePlugin=RiderLink` 验证项目模块。

## 编辑器验证入口

- 验证地图：`DevelopMap_1`
- GameMode：`BP_Gamemode_dev` -> `BP_PlayerCharacter` + `BP_PlayerController`
- 玩家输入与 Ability：`UADInputConfigData`、`DA_PlayerConfig`、`ST_PlayerCombat`
- 命中窗口与取消窗口：在 Montage 中通过 `AD Hit Detection`、`AD Cancel Window` Notify State 配置

## 编码与协作约定

- 关键函数、关键状态和不直观逻辑使用中文注释；未明确细节使用 `//TODO`
- UE 对象成员优先使用 `TObjectPtr<T>`
- 不得虚构不存在的模块、类、函数、标签或资产路径；先查源码、配置或 Wiki，再写代码
- 优先沿用既有目录、命名、数据资产和组件边界，避免把输入、战斗、表现逻辑重新堆回角色类
- 修改关键逻辑且方向已确定后，必须同步更新 `ProjectWiki.md` 中对应条目；纯探索代码不入 Wiki
- 未完成编辑器验证的实现在 Wiki 标 `C++ 首版`、`待验证`、`资产待配`；只有跑通对应场景后才升为 `已验收`

## 文档入口

- `ProjectContext.md`：长期稳定的架构边界、运行时职责与核心战斗规则（权威）
- `ProjectWiki.md`：模块、逻辑链、GameplayTag、资产、规划、决策的查询型数据库（权威）
- `CLAUDE.md`：Claude Code 在本仓库工作的入口提示

当 README 与上述两份文档冲突时，以 `ProjectContext.md` 与 `ProjectWiki.md` 为准。
