# Project Context: ActionDemo (Project WutheringStyle)

## 0. 使用方式
* 本文件只维护长期稳定的项目约束、架构边界和协作规则。
* 当前源码事实、类职责、逻辑链路、标签、资产配置、待办与历史决策维护在根目录 `ProjectWiki.md`。
* 当上下文缺失时，阅读顺序为：`ProjectContext.md` -> `ProjectWiki.md` -> 相关源码。
* 架构方向、核心战斗规则或 Wiki 维护约定发生变化前，应先确认意图，再同步更新这两个文件。

## 1. 项目愿景
* 目标：制作一套对标《鸣潮》手感方向的高机动 ARPG 战斗系统 Demo。
* 重点体验：高频操作反馈、动画取消、输入缓冲、空中派生、锁敌、近战命中反馈和强表现技能演出。
* 核心工程挑战：在 UE 5.7 中以 GAS + StateTree + 组件化战斗框架实现逻辑与表现解耦。

## 2. 技术基线
* 引擎：Unreal Engine 5.7。
* 开发：C++ + Blueprint，IDE 以 JetBrains Rider + Codex 为主。
* 架构：C++ 负责主要逻辑链路；蓝图优先用于继承 C++ 类、配置资产、动画/特效/镜头/UI 表现微调。
* 核心依赖：
  * Gameplay Ability System：Ability、属性、GameplayEvent、GameplayTag。
  * StateTree / GameplayStateTree：玩家和敌人的宏观战斗状态决策。
  * Enhanced Input：输入映射、输入标签、输入缓冲入口。
  * Motion Warping：近战朝向与位移修正的后续接入点。
  * AIModule：敌人 AIController、感知和移动任务。

## 3. 架构边界
* `Core`：项目级标签、公共类型、日志与全局约定。
* `Character`：角色基类、玩家角色、敌人角色；负责实体装配和生命周期，不堆叠战斗细节。
* `Components`：战斗上下文、输入缓冲、锁敌、命中检测等可复用事实与能力。
* `AbilitySystem`：ASC、AttributeSet、Ability 基类、攻击 Ability、数据资产。
* `Tree/StateTree`：宏观状态、条件、Evaluator、任务；产生 Ability 激活意图，不承接伤害和动画细节。
* `Game/Input/AI`：玩家控制器处理输入分发；敌人 AIController 处理感知、目标事实和 StateTree。
* `Presentation`：动画通知、Montage、特效、镜头、UI 只消费事件和标签，尽量不成为业务规则唯一来源。

## 4. 运行时职责原则
* `AADCharacterBase` 是所有战斗实体基类，只挂载并暴露 ASC、AttributeSet、Combat、InputQueue、Targeting、HitDetection 等核心组件。
* `AADPlayerCharacter` 保持轻量，负责相机、玩家 StateTree 组件和取消窗口预输入重发；输入绑定与分发归 `AADPlayerController`。
* `AADPlayerController` 负责 Enhanced Input 绑定、MappingContext 注入、启动 Ability 授予，并按输入配置分发到输入缓冲、ASC 或 StateTree。
* `AADEnemyCharacter` 保持轻量，负责敌人配置、启动 Ability 授予和受击入口；敌人的感知、目标事实、追击和宏观决策归 `AADEnemyAIController`。
* StateTree 负责“应该做什么”；Gameplay Ability 负责“动作如何执行”；组件负责维护可查询事实；表现层负责“如何看起来正确”。

## 5. 核心战斗规则
* 原子化 Ability：每个可独立拥有生命周期的动作应是一个 `UGameplayAbility`，同类动作通过项目 Ability 基类复用逻辑。
* 标签驱动：动作状态、取消窗口、输入、事件和阻塞条件优先使用 GameplayTag 表达。
* 连段与取消：
  * 连段延续、派生切换和取消窗口优先数据驱动，不写死在角色类中。
  * 激活新攻击 Ability 前，应取消仍活跃的旧攻击 Ability，确保同一角色同一时间只有一个攻击动作持有动作所有权。
  * 攻击动作的开始、结束和 StateTree 跟踪必须使用动作实例编号，避免旧 Ability、旧 Notify 或旧 Task 的迟到回调污染当前动作。
* 动画时机：
  * 攻击 Ability 生命周期优先由 Montage AbilityTask 驱动。
  * 取消窗口由 `AD Cancel Window` 动画通知状态驱动。
  * 命中窗口由 `AD Hit Detection` 动画通知状态驱动。
* 命中链路：
  * 命中检测组件负责 Sweep、同窗口去重和标准命中数据构造。
  * `CombatComponent` 是命中确认统一入口，向表现层广播，并向受击目标发送 GAS GameplayEvent / Hit Receiver 接口事件。
  * 当前命中链路不直接扣血、不播放受击、不施加 GameplayEffect；这些由后续受击 Ability、GameplayEffect 或表现系统消费。
* 未稳定定义的后续系统：弹刀、极限闪避、空中连段、完整伤害计算与属性公式、处决、Boss 阶段和完整运镜演出。
* 伤害、受击、死亡等阶段进度以 `ProjectWiki.md` 为准，不在本文件维护。

## 6. 编码与 AI 协作规范
* 注释：关键函数、关键状态和不直观逻辑使用中文注释；未明确细节使用 `//TODO`在代码中进行注释。
* 指针：UE 对象成员优先使用 `TObjectPtr<T>`。
* 接口真实性：不得虚构不存在的模块、类、函数或资产路径；不确定时先查源码、配置或询问用户。
* 代码落点：优先沿用现有目录、命名、数据资产和组件边界；避免把输入、战斗、表现逻辑重新堆回角色类。
* 修改关键逻辑且实现方向已确定后，必须同步更新 `ProjectWiki.md` 中对应数据库条目；纯探索、临时代码和被放弃的试错分支不进入 Wiki。
* 未完成编辑器验证的实现只能在 Wiki 标为 `C++ 首版`、`待验证`、`资产待配` 等状态；只有实际跑通对应场景后才标为 `已验收`。
* 只有长期稳定规则变化时才更新本文件。

## 7. Wiki 维护约定
* `ProjectWiki.md` 是可随时查询的项目数据库，不是单纯按时间追加的开发日志。
* 每次更新应优先修改已有条目，保持唯一事实来源；不要把同一事实在多个章节重复堆叠。
* 长期信息按查询场景归档：
  * 模块和类职责 -> Module Database。
  * 输入、连段、取消、命中、锁敌、AI 等流程 -> Logic Chain Database。
  * 已确认的后续模块规划、逻辑链草案和阶段推进方案 -> Planning Database。
  * GameplayTag、数据资产、编辑器配置 -> 对应数据库章节(GameplayTag Database,Asset /Editor Database)。
  * 技术决策和原因 -> Decision Ledger。
  * 编译、编辑器验证、已知风险 -> Planning Database 的验收条目或对应模块/逻辑链条目。
  * 短期历史只进 Change Ledger，并附上指向数据库条目的索引。
* 每当出现经过用户确认或实现方向已明确的后续开发规划，例如新模块职责、系统逻辑链、数据流、阶段验收点，应记录到 Wiki 的 `Planning Database` ，并随着开发进度更新状态：
  * 如规划已实现，则将相应规划条目移出 `Planning Database` 。
  * 如规划已修改，应及时同步或移除相应规划条目。
* 写 Wiki 时应记录来源和状态：已实现、C++ 首版、资产待配、待验证、已验收、计划中、废弃。
* 快速迭代期间，先让源码和资产事实收敛；编译通过且设计方向确定后再更新 Wiki，编辑器验证完成后再升级验收状态。
* 当实现变化导致旧信息失效时，更新或标记废弃，而不是在文件末尾继续追加相互矛盾的新日志。
