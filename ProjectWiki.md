# ActionDemo Project Wiki

维护目标：本文件是可查询项目数据库，不是开发日志。新增信息优先更新已有记录；历史只保留为索引，技术细节归入对应数据库条目。

文档状态：2026-05-24 更新。已核对 `ActionDemo.uproject`、`ActionDemo.Build.cs`、`Source/ActionDemo` 核心源码、`Content/ActionDemo` 关键资产引用；伤害/受击/死亡链路与角色插槽驱动命中窗口已通过本机 UE 5.7 编译和编辑器场景验收；武器管理、生成、切换与武器插槽来源 C++ 已编译通过，待编辑器资产验证。

## 查询索引
* 项目边界：`Project Snapshot`
* 类和模块职责：`Module Database`
* 已实现逻辑链：`Logic Chain Database`
* 标签：`GameplayTag Database`
* 数据资产和编辑器配置：`Asset / Editor Database`
* 已确认后续规划：`Planning Database`
* 历史原因：`Decision Ledger`、`Change Ledger`

## Project Snapshot
| 字段 | 当前事实 |
| --- | --- |
| 目标 | 高机动 ARPG 战斗 Demo，手感方向参考《鸣潮》 |
| 引擎 | Unreal Engine 5.7 |
| 本机引擎路径 | `E:\UE_5.7`，编译入口 `E:\UE_5.7\Engine\Build\BatchFiles\Build.bat` |
| 版本控制 | Git 仓库已初始化；`.uasset/.umap` 等二进制资产使用 Git LFS |
| 主模块 | `Source/ActionDemo` Runtime |
| 依赖 | GameplayAbilities、StateTree、GameplayStateTree、MotionWarping、Enhanced Input、AIModule |
| 架构 | C++ 负责核心链路，蓝图负责配置和表现微调 |
| 已有重点 | 轻攻击连段、取消窗口、输入缓冲、锁敌、命中事件、C++ 伤害/受击/死亡首版、敌人近战 AI 首版、武器生成/切换首版 |
| 主要缺口 | 受击硬直、弹刀、极限闪避、空中连段、Motion Warping 实装 |

## Module Database
| ID | 主要文件 | 查询事实 | 状态 |
| --- | --- | --- | --- |
| MOD-Core-Tags | `Core/Tags/ADGameplayTags.*` | Native GameplayTags 唯一集中声明处 | 已实现 |
| MOD-Character-Base | `Character/Base/ADCharacterBase.*` | 战斗实体基类；组装 ASC、AttributeSet、Combat、AbilityQueue、Targeting、HitDetection、WeaponManager；`Get/SetEquippedWeapon` 仅委托武器管理组件；监听 Health 归零并进入死亡入口 | 已实现 |
| MOD-Character-Player | `Character/Player/ADPlayerCharacter.*` | 玩家 Pawn；相机、玩家 StateTree、取消窗口预输入重发 | 已实现 |
| MOD-Character-Enemy | `Character/Enemy/ADEnemyCharacter.*` | 敌人 Pawn；敌人配置、启动 Ability 授予、默认受击 Ability 授予、受击入口、死亡广播 | 已实现 |
| MOD-AI-Enemy | `AI/ADEnemyAIController.*` | 感知、CombatTarget、敌人 StateTree、追击和范围判断 | 首版 |
| MOD-Input | `Input/Data/ADInputConfigData.h` | MappingContext、InputAction -> InputTag -> Controller 函数、StartupAbilities | 已实现 |
| MOD-PlayerController | `Game/ADPlayerController.*` | Enhanced Input 绑定、输入缓冲、ASC 激活、StateTree 事件、玩家 Ability 授予 | 已实现 |
| MOD-ASC | `AbilitySystem/ADAbilitySystemComponent.*` | 按 InputTag / AbilityClass 激活 Ability；攻击连段前取消旧攻击 Ability | 已实现 |
| MOD-Ability-Base | `AbilitySystem/Abilities/ADGameplayAbility.*` | InputTag、AbilityTag、重复输入取消、蓝图扩展点 | 已实现 |
| MOD-Ability-Attack | `AbilitySystem/Abilities/ADGameplayAbility_AttackBase.*` | 攻击登记、Montage 生命周期、默认时长兜底、旧 Montage 停止、命中伤害值与伤害 GE 配置 | 已实现 |
| MOD-Ability-ReceiveHit | `AbilitySystem/Abilities/ADGameplayAbility_ReceiveHit.*` | 受击 Ability；由 `Event.Hit.Confirm` 触发，目标侧应用伤害 GE，忽略无敌/死亡目标 | 首版已验收 |
| MOD-Effect-Damage | `AbilitySystem/Effects/ADGameplayEffect_Damage.*` | 默认即时伤害 GE；通过 `SetByCaller(Data.Damage)` 修改 `Health` | 首版已验收 |
| MOD-Ability-TargetLock | `AbilitySystem/Abilities/ADGA_Target_Lock.*` | 锁敌 Ability；目标事实委托给 TargetingComponent | 已实现 |
| MOD-Data-CombatAction | `AbilitySystem/Data/ADCombatActionData.h` | 派生输入、下一段 Ability、空中允许标记 | 骨架 |
| MOD-Equipment-Weapon | `Equipment/ADWeaponBase.*` | 简单武器 Actor 基类；提供 Weapon Mesh 和持有者引用，供武器管理组件绑定角色 owner、供命中检测读取武器插槽 | 首版，C++ 已编译 |
| MOD-Equipment-WeaponManager | `Components/Equipment/ADWeaponManagerComponent.*` | 角色武器事实源；按 `FADWeaponSpawnConfig` 生成武器实例、附着到角色 Mesh、维护武器实例列表和当前装备索引；切换时更新当前装备武器、显隐/碰撞状态、owner 绑定和装备变化广播；提供装备武器、武器 Mesh、按索引/前后切换查询函数 | 首版，C++ 已编译 |
| MOD-Attributes | `AbilitySystem/Attributes/ADAttributeSet.*` | Health、MaxHealth、Stamina、MaxStamina；默认值、Clamp、GE 执行后修正 | 首版已验收 |
| MOD-Combat | `Components/Combat/ADCombatComponent.*` | 当前动作上下文、动作实例编号、取消窗口、命中确认入口、命中伤害上下文补全 | 已实现 |
| MOD-InputQueue | `Components/Input/ADAbilityQueueComponent.*` | InputTag 缓存、0.35 秒默认有效期、消费输入 | 已实现 |
| MOD-Targeting | `Components/Target/ADTargetingComponent.*` | 候选目标搜索、评分、切换、目标变更广播 | 已实现 |
| MOD-HitDetection | `Components/Combat/ADHitDetectionComponent.*` | 插槽驱动命中窗口 Sweep、同窗口去重、标准命中事件构造 | 已验收 |
| MOD-HitTypes | `Components/Combat/ADCombatHitTypes.h`、`ADCombatHitReceiverInterface.h` | 标准命中数据和同步受击接口 | 已实现 |
| MOD-AnimNotifies | `Animation/Notifies/ADAnimNotifyState_CancelWindow.*`、`ADAnimNotifyState_HitDetection.*` | 取消窗口和命中窗口时机源 | 已实现 |
| MOD-StateTree-Player | `Tree/StateTree/Conditions/*`、`Evaluators/ADStateTreeEvaluator_CombatSnapshot.*`、`Tasks/ADStateTreeTask_PlayCombatAbility.*` | 玩家战斗条件、快照、Ability 激活和动作实例跟踪 | 已实现 |
| MOD-StateTree-Enemy | `Evaluators/ADStateTreeEvaluator_EnemyCombatSnapshot.*`、`Tasks/ADStateTreeTask_MoveToEnemyTarget.*` | 敌人目标快照和追击任务 | 首版 |

## Logic Chain Database
| ID | 查询目标 | 当前链路 | 状态 |
| --- | --- | --- | --- |
| LC-Input-Player | 玩家输入如何进入战斗 | `AADPlayerController` 读取 `UADInputConfigData`；注入 MappingContext；绑定 `InputAction -> InputTag -> Ability_` 函数；轻攻击写入 `UADAbilityQueueComponent` 并发送 StateTree 事件；锁敌按 `Input.Target.Lock` 直接激活 ASC；`StartupAbilities` 只从 Controller 的 `InputConfig` 授予 | 已实现 |
| LC-Combo | 轻攻击连段如何跳转 | `Input.Attack.Light` 进入输入缓冲；StateTree 使用 `BufferedInputMatchesTag`、`CombatCurrentAction`、`LastCompletedActionMatches` 判断状态；`UADStateTreeTask_PlayCombatAbility` 消费输入、激活 Ability、记录 `TrackedActionTag/Serial`；旧动作被新实例替换时当前任务成功退出 | 已实现 |
| LC-AttackAbility | 攻击 Ability 生命周期 | `UADGameplayAbility_AttackBase` Commit 后登记动作上下文；优先用 `UAbilityTask_PlayMontageAndWait` 驱动结束；Montage 完成正常结束，中断/取消则取消结束；无 Montage 时用 `DefaultAttackDuration` 兜底；激活新攻击前 ASC 取消旧攻击 Ability | 已实现 |
| LC-ActionContext | 如何防旧回调污染当前动作 | `UADCombatComponent` 维护 `CurrentActionTag/Serial/AbilityClass/StartTime` 与 `LastCompletedActionTag/Serial`；动作开始生成递增 Serial；动作结束必须 `NotifyActionEndedByContext(ActionTag, Serial)` 校验，过期回调忽略 | 已实现 |
| LC-CancelWindow | 取消窗口和预输入 | Montage `AD Cancel Window` 驱动 `Begin/EndCancelWindowFromNotify()`；组件用 `CancelWindowNotifyCount` 处理重叠窗口；同步 Loose Tag `Ability.Cancel.Active`；窗口开启时广播并让玩家角色重发仍有效的缓冲输入 | 已实现 |
| LC-WeaponManagement | 武器如何生成和切换 | `UADWeaponManagerComponent` 在 BeginPlay 可按 `WeaponSpawnConfigs` 生成 `AADWeaponBase` 子类；生成参数包含 `WeaponClass`、`AttachSocketName`、`AttachRelativeTransform`，空插槽名回退到 `DefaultAttachSocketName`；生成武器注册到 `WeaponInstances`，默认隐藏未装备武器并关闭碰撞；`EquipWeaponByIndex/EquipNextWeapon/EquipPreviousWeapon` 只在管理实例列表内切换，切换后刷新当前装备索引、绑定 owner、附着角色 Mesh、广播 `OnEquippedWeaponChanged`；组件 EndPlay 可销毁自己生成的武器，不销毁手动引用的关卡武器 | C++ 已编译，资产待配置/验证 |
| LC-HitDetection | 命中确认如何产生 | Montage `AD Hit Detection` 只给时机、插槽来源、双插槽和盒体截面配置；`UADHitDetectionComponent` 按 `SocketSource` 从 Montage 角色 Mesh 或 `UADWeaponManagerComponent` 当前装备武器的 Weapon Mesh 读取 `StartSocketName/EndSocketName`，生成世界空间盒体 Sweep、去重并构造 `FADCombatHitEventData`；`UADCombatComponent::HandleHitConfirmed()` 补全当前攻击的 `DamageAmount/DamageEffectClass`、广播、发送 GAS GameplayEvent，并调用 Hit Receiver 接口；命中检测本身仍不直接扣血 | C++ 已编译，武器来源待编辑器验证 |
| LC-Damage-ReceiveHit | 命中如何转为扣血和死亡 | 攻击 Ability CDO 提供 `DamageAmount/DamageEffectClass`；`Event.Hit.Confirm` 使用 `EventMagnitude` 携带伤害、`OptionalObject` 携带 GE 类、`Data.Damage` SetByCaller 修改 `Health`；敌人默认授予 `UADGameplayAbility_ReceiveHit`；`UADAttributeSet` Clamp 生命/体力；`AADCharacterBase` 在 Health 归零时加 `State.Dead`、取消 Ability、停止移动并广播死亡 | 首版已验收 |
| LC-TargetLock | 锁敌目标事实在哪里 | `UADGA_Target_Lock` 按 `Input.Target.Lock` 激活；`UADTargetingComponent::CurrentTarget` 是唯一事实；`AcquireBestTarget()` 按半径、视角、距离、方向和可选 LineTrace 评分；重复输入取消 Ability 并清空目标 | 已实现 |
| LC-EnemyAI | 敌人首版近战 AI | `AADEnemyAIController` 持有感知、`CombatTarget` 和 StateTree；同步目标到 Pawn 的 TargetingComponent；`UADStateTreeEvaluator_EnemyCombatSnapshot` 输出目标/距离/攻击范围；`UADStateTreeTask_MoveToEnemyTarget` 追击到攻击范围；`AADEnemyCharacter` 授予启动 Ability 并接收命中 | 首版 |

## GameplayTag Database
| Tag | Native symbol | 用途 |
| --- | --- | --- |
| `State.Action.Airborne` | `State_Action_Airborne` | 空中状态 |
| `State.Dead` | `State_Dead` | 死亡状态 |
| `Ability.Cancel.Active` | `Ability_Cancel_Active` | 可取消窗口 |
| `Ability.Attack.Light.01` | `Ability_Attack_Light_01` | 轻攻击一段 |
| `Ability.Attack.Light.02` | `Ability_Attack_Light_02` | 轻攻击二段 |
| `Ability.Attack.Light.03` | `Ability_Attack_Light_03` | 轻攻击三段 |
| `Ability.Target.Lock` | `Ability_Target_Lock` | 锁敌 Ability |
| `Ability.Hit.React` | `Ability_Hit_React` | 受击 Ability |
| `Event.Hit.Confirm` | `Event_Hit_Confirm` | 命中确认事件 |
| `Event.Target.Locked` | `Event_Target_Locked` | 目标锁定事件 |
| `Event.Target.Unlocked` | `Event_Target_Unlocked` | 目标解锁事件 |
| `Status.Invincible` | `Status_Invincible` | 无敌帧 |
| `Data.Damage` | `Data_Damage` | 伤害 GE SetByCaller 数据标签 |
| `Input.Attack.Light` | `Input_Attack_Light` | 轻攻击输入 |
| `Input.Dodge` | `Input_Dodge` | 闪避输入 |
| `Input.Target.Lock` | `Input_Target_Lock` | 锁敌输入 |

维护规则：新增核心标签时同步更新 `ADGameplayTags.h/.cpp` 和本表。

## Asset / Editor Database
| 配置 | 当前约定 | 状态 |
| --- | --- | --- |
| 玩家输入 | `UADInputConfigData` 配置 MappingContexts、AbilityInputBindings、StartupAbilities | 资产侧配置 |
| 输入函数 | `FunctionNameToCall` 指向 `AADPlayerController` 的 `Ability_` 前缀函数 | 已实现 |
| 验证入口 | `DevelopMap_1` 引用 `BP_Gamemode_dev`；`BP_Gamemode_dev` 指向 `BP_PlayerCharacter` 和 `BP_PlayerController`，用于资产侧输入/Ability 验证 | 资产侧配置 |
| 轻攻击 Ability | 实际资产为 `GA_Player_LightAttack_1`、`GA_Player_LightAttack_2`、`GA_Player_LightAttack_3`；`DA_PlayerConfig` 和 `ST_PlayerCombat` 已引用 | 资产存在，编辑器流程待验证 |
| 攻击参数 | `AttackMontage`、`DefaultAttackDuration`、`bAutoPlayMontage`、`bAutoEndAbility`、`CancelMontageBlendOutTime`、`ActionData`、`DamageAmount`、`DamageEffectClass` | 已实现 |
| 受击 Ability | `UADGameplayAbility_ReceiveHit` 默认监听 `Event.Hit.Confirm`；敌人 `DefaultHitReactionAbilityClass` 默认指向该 C++ Ability，也可在蓝图子类替换 | 首版已验收 |
| 默认伤害 GE | `UADGameplayEffect_Damage` 为 instant GE；要求调用方设置 `Data.Damage`，当前受击 Ability 传负值扣血 | 首版已验收 |
| 武器生成/切换 | 角色 `WeaponManagerComponent` 配置 `WeaponSpawnConfigs`；每项指定 `AADWeaponBase` 蓝图类、附着 Socket 和相对变换；`InitialWeaponIndex` 指定默认装备；`DefaultAttachSocketName` 用作空 Socket 回退；未装备武器默认隐藏且关闭碰撞；切换函数由蓝图、输入或 Ability 调用 | C++ 已编译，资产待配置/验证 |
| 取消窗口 | Montage 添加 `AD Cancel Window` | 资产侧配置 |
| 命中窗口 | Montage 添加 `AD Hit Detection`，配置 `SocketSource`、`StartSocketName`、`EndSocketName`、`SocketAxisPadding`、`CrossSectionHalfExtent`、TraceChannel、HitEventTag、Debug；`SocketSource=CharacterMesh` 时从 Montage 角色 Mesh 读取插槽，`SocketSource=EquippedWeapon` 时从角色 `WeaponManagerComponent` 当前装备武器的 Weapon Mesh 读取插槽；推荐插槽名 `Hit_Start` / `Hit_End`，实际名称以每个 Notify 配置为准；插槽缺失、武器未设置或两插槽距离过短时本窗口跳过 | 角色来源已验收，武器来源待编辑器验证 |
| 敌人配置 | `UADEnemyConfigData` 配置 StartupAbilities、AttackRange、LoseTargetDistance、SightRadius、LoseSightRadius、PeripheralVisionAngleDegrees | 首版 |
| 连段数据 | `UADCombatActionData` 可描述派生输入和下一段 Ability | 尚未成为主数据源 |
| 编译验收 | `ActionDemoEditor Win64 Development` 使用 `E:\UE_5.7\Engine\Build\BatchFiles\Build.bat` 编译；伤害/受击/死亡链路与角色插槽驱动命中窗口已完成编辑器场景验收；武器管理、生成、切换与武器插槽来源已通过 UHT/C++ 编译；当前完整 DLL 链接被已打开的 `UnrealEditor.exe` / Live Coding 锁定影响，需释放编辑器后再做完整链接和资产侧验证；UBT 提示 VS 2022 编译器非首选版本但未阻断 | 2026-05-24 C++ 已通过，完整链接待释放编辑器 |

## Planning Database
| ID | 规划主题 | 已确认方向 | 关联记录 |
| --- | --- | --- | --- |
| PLAN-Combat-01 | 真实命中到受击链路 | 首版已验收：攻击 Ability 携带 `DamageEffectClass/DamageAmount`，`UADCombatComponent::HandleHitConfirmed` 经 GameplayEvent 投递，目标侧 `UADGameplayAbility_ReceiveHit` 通过 GameplayEffect 修改 `Health`，Health 归零进入死亡入口。待补：受击硬直/反馈、死亡表现清理 |  `LC-HitDetection`、`LC-Damage-ReceiveHit`、`MOD-Combat`、`MOD-Attributes` |
| PLAN-Combo-01 | 数据驱动连段 | 推进 `UADCombatActionData` 成为连段派生主数据源；先扩展 `FADCombatActionTransition` 增加可选条件（取消窗口/最近完成动作匹配）；攻击 Ability 在结束/取消窗口阶段查表得出候选 `NextAbilityClass`，StateTree 仅判定输入与全局阻塞 |  `LC-Combo`、`MOD-Data-CombatAction` |
| PLAN-Movement-01 | 锁敌与 Motion Warping | 攻击 Ability 在 Commit 阶段基于 `UADTargetingComponent::CurrentTarget` 写入 Motion Warping Target；Montage 上挂 Warp Notify 完成朝向/距离修正；未锁定时回退为根运动 |  `LC-TargetLock`、`MOD-Targeting` |
| PLAN-Attribute-01 | 属性集与 GE 流水扩展 | 基于现有 `UADAttributeSet` 扩展攻击力、防御、韧性（HitStun 抗性）、能量等属性；建立伤害计算 `UGameplayEffectExecutionCalculation`；将 `Event.Hit.Confirm` 携带的 `SourceActionTag` 映射到伤害系数表 |  `MOD-Attributes`、`PLAN-Combat-01` |
| PLAN-Defense-01 | 弹刀与极限闪避 | 新增 `Input.Parry`、`Ability.Parry`、`State.Parry.Window`、`State.Dodge.Perfect` 标签；闪避 Ability 在前若干帧加 `Status.Invincible`；弹刀 Ability 在窗口期内捕获 `Event.Hit.Confirm` 并触发反制；与 `LC-HitDetection` 协作，命中前若目标处于弹刀窗则改走反制分支 |  `LC-HitDetection`、`MOD-Combat` |
| PLAN-Air-01 | 空中状态与空中连段 | 利用现有 `State.Action.Airborne` 与 `UADCombatActionData::bAllowInAir`；玩家落地/起跳维护标签；StateTree 增加空中分支；空中攻击 Ability 通过自定义 RootMotion/Launch 锁定高度，落地或目标距离断裂时结束 |  `MOD-Data-CombatAction`、`MOD-Character-Player` |
| PLAN-Verify-01 | 阶段验收与编辑器验证 | 每个 PLAN 落地时执行：① 使用 `E:\UE_5.7\Engine\Build\BatchFiles\Build.bat` 执行 `ActionDemoEditor Win64 Development` 编译；若编辑器占用 RiderLink，可临时追加 `-NoHotReloadFromIDE -DisablePlugin=RiderLink` 验证项目模块；② 编辑器场景跑通对应 Logic Chain（命中扣血/弹刀反制/空中三段等）；③ Wiki 把对应规划状态同步到模块/Logic Chain 条目；④ 在 `Decision Ledger` 留决策行 |  `Planning Database`、`Decision Ledger` |

## Decision Ledger
| 日期 | 决策 | 关联记录 |
| --- | --- | --- |
| 2026-04-24 | 取消窗口由 Ability 时间参数改为 Montage `AD Cancel Window` | `LC-CancelWindow` |
| 2026-04-24 | 输入缓冲增加写入时间和有效期，取消窗口开启时重发预输入 | `LC-Input-Player`、`LC-CancelWindow` |
| 2026-04-26 | 连段切换前主动取消旧攻击 Ability | `LC-AttackAbility` |
| 2026-04-26 | 攻击结束通知改为标签/实例保护 | `LC-ActionContext` |
| 2026-04-27 | 攻击 Ability 生命周期统一由 Montage AbilityTask 驱动 | `LC-AttackAbility` |
| 2026-04-28 | Combat 动作实例编号和 StateTree 任务跟踪收口 | `LC-ActionContext`、`LC-Combo` |
| 2026-05-18 | Wiki 改为查询型数据库，ProjectContext 只保留稳定约束 | 全文 |
| 2026-05-19 | 已确认后续开发规划必须进入 `Planning Database` 或对应数据库条目，并随开发进度更新 | `Planning Database` |
| 2026-05-19 | Planning Database 扩展为带阶段方案的下一步规划集合：受击链路、数据驱动连段、Motion Warping、属性扩展、弹反/极限闪避、空中连段、阶段验收 | `Planning Database` |
| 2026-05-19 | 移除冗余规划章节，后续规划统一维护在 `Planning Database` | `Planning Database` |
| 2026-05-19 | 本机 UE 5.7 引擎路径确认为 `E:\UE_5.7`，后续构建命令固定使用该路径 | `Project Snapshot`、`PLAN-Verify-01` |
| 2026-05-23 | 项目版本控制采用 Git + Git LFS；UE 二进制资产走 LFS | `Project Snapshot` |
| 2026-05-23 | 真实命中到受击链路先落 C++ 最小闭环：攻击侧提供伤害和 GE 类，目标侧受击 Ability 消费 GameplayEvent 并应用 GE，死亡入口归 CharacterBase | `PLAN-Combat-01`、`LC-Damage-ReceiveHit` |
| 2026-05-23 | 命中检测盒体改为 Montage Mesh 双插槽驱动，缺失插槽时跳过窗口，不回退旧本地偏移 | `LC-HitDetection`、`MOD-HitDetection` |
| 2026-05-24 | 插槽驱动命中窗口已通过编辑器场景验收 | `LC-HitDetection`、`MOD-HitDetection`、`PLAN-Verify-01` |
| 2026-05-24 | 命中窗口插槽来源改为可配置：角色 Mesh 或当前装备武器 Mesh；武器只作为插槽/表现载体，命中事实仍归角色 HitDetection/Combat 组件 | `LC-HitDetection`、`MOD-HitDetection`、`MOD-Equipment-Weapon` |
| 2026-05-24 | 武器运行时事实收口到 `UADWeaponManagerComponent`；角色基类只负责装配和兼容转发，不再直接承接武器管理职责 | `MOD-Character-Base`、`MOD-Equipment-WeaponManager`、`LC-HitDetection` |
| 2026-05-24 | 武器生成和切换职责继续收口到 `UADWeaponManagerComponent`：生成配置、实例列表、当前装备索引、前后切换、显隐/碰撞和生命周期清理都归组件负责 | `MOD-Equipment-WeaponManager`、`LC-WeaponManagement` |

## Change Ledger
| 日期 | 摘要 | 详情 |
| --- | --- | --- |
| 2026-05-18 | 文档审计与整理 | `ProjectContext.md`、本 Wiki |
| 2026-05-19 | 增加后续规划数据库和维护规则 | `Planning Database`、`ProjectContext.md` |
| 2026-05-19 | 扩充 Planning Database：受击链路、数据驱动连段、Motion Warping、属性、弹反/闪避、空中连段、阶段验收 | `Planning Database`、`Decision Ledger` |
| 2026-05-19 | 移除冗余规划章节，清理旧 TODO/RISK 引用 | `Planning Database` |
| 2026-05-19 | 同步进度核对偏差：记录本机 UE 5.7 路径、真实轻攻击资产名、DevelopMap 验证入口和编译验收结果 | `Project Snapshot`、`Asset / Editor Database`、`PLAN-Verify-01` |
| 2026-05-23 | 初始化 Git 仓库并新增 `.gitignore`、`.gitattributes`；启用本仓库 Git LFS hooks | `Project Snapshot` |
| 2026-05-23 | 落地 C++ 伤害/受击/死亡首版 | `MOD-Ability-ReceiveHit`、`MOD-Effect-Damage`、`LC-Damage-ReceiveHit` |
| 2026-05-24 | 伤害/受击/死亡链路编辑器验收通过 | `LC-Damage-ReceiveHit`、`PLAN-Combat-01`、`PLAN-Verify-01` |
| 2026-05-23 | 命中检测改为双插槽驱动盒体 | `MOD-HitDetection`、`LC-HitDetection` |
| 2026-05-24 | 插槽驱动命中窗口编辑器验收通过 | `MOD-HitDetection`、`LC-HitDetection`、`PLAN-Verify-01` |
| 2026-05-24 | 命中检测支持从当前装备武器 Mesh 读取插槽 | `MOD-HitDetection`、`MOD-Equipment-Weapon`、`LC-HitDetection` |
| 2026-05-24 | 新增武器管理组件，集中装备武器查询、切换、owner 绑定和装备变化广播 | `MOD-Equipment-WeaponManager`、`MOD-Character-Base`、`LC-HitDetection` |
| 2026-05-24 | 武器管理组件新增按配置生成武器、附着角色 Mesh、按索引/前后切换和生成实例清理 | `MOD-Equipment-WeaponManager`、`LC-WeaponManagement` |
| 2026-04-28 | 动作实例跟踪与 Combat 动作上下文 | `LC-Combo`、`LC-ActionContext` |
| 2026-04-27 | 攻击 Ability 生命周期统一化 | `LC-AttackAbility` |
| 2026-04-26 | 连段旧 Ability 主动取消和旧回调保护 | `LC-AttackAbility`、`LC-ActionContext` |
| 2026-04-24 | 预输入、取消窗口通知状态和 StateTree 触发链路 | `LC-Input-Player`、`LC-CancelWindow` |
