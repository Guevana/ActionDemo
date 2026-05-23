# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ActionDemo (Project WutheringStyle) — a high-mobility ARPG combat system demo targeting a "Wuthering Waves"-style feel, built in Unreal Engine 5.7 using GAS + StateTree + component-based combat framework.

Key plugins enabled: GameplayAbilities, StateTree, GameplayStateTree, MotionWarping, EnhancedInput, AIModule.

## Reading Order

When context is missing, read in this order:
1. `ProjectContext.md` — long-term architectural boundaries and collaboration rules
2. `ProjectWiki.md` — query-oriented database of modules, logic chains, tags, assets, decisions
3. Relevant source files

These two files are the authoritative project documentation. If you modify key logic, update `ProjectWiki.md`'s corresponding entries. Only update `ProjectContext.md` for long-term stable rule changes.

## Build Commands

UE 5.7 project. The `.uproject` is at repo root. Local UE 5.7 engine path is `E:\UE_5.7`. Standard UE build workflow:

```powershell
# Editor build (Development)
# From repo root, using UE 5.7 engine:
& "E:\UE_5.7\Engine\Build\BatchFiles\Build.bat" ActionDemoEditor Win64 Development "D:\AAA_UEProjects\ActionDemo\ActionDemo.uproject" -WaitMutex

# Or via UBT directly:
& "E:\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" ActionDemoEditor Win64 Development -Project="D:\AAA_UEProjects\ActionDemo\ActionDemo.uproject"
```

Editor can also be launched via Rider/VS by opening `ActionDemo.sln`.

The project uses **Rider** as the primary IDE (the `.idea/` directory is present).

## Architecture Summary

- **Engine**: UE 5.7, C++ + Blueprint
- **Module**: Single Runtime module `ActionDemo` in `Source/ActionDemo/`
- **Key dependencies**: GameplayAbilities, StateTree, GameplayStateTree, MotionWarping, EnhancedInput, AIModule
- **Architecture**: C++ handles core logic; Blueprint is for class inheritance, asset configuration, animation/VFX/camera/UI polish

### Source Directory Layout

| Directory | Responsibility |
|-----------|---------------|
| `Core/Tags/` | Native GameplayTags — single source of truth |
| `Character/Base/` | `ADCharacterBase` — combat entity base class, assembles ASC + components |
| `Character/Player/` | `ADPlayerCharacter` — camera, player StateTree, cancel-window input re-dispatch |
| `Character/Enemy/` | `ADEnemyCharacter` — enemy config, startup ability grants, hit reception |
| `AbilitySystem/` | ASC subclass, ability base classes, attack ability, target-lock ability, attributes, combat data assets |
| `Components/Combat/` | Combat context, hit detection, hit types and receiver interface |
| `Components/Input/` | Input buffering (0.35s default window) |
| `Components/Target/` | Target acquisition, scoring, and lock-on |
| `Tree/StateTree/` | Conditions, evaluators, and tasks for both player and enemy StateTrees |
| `Input/Data/` | Input config data asset — MappingContext → InputAction → InputTag → controller functions |
| `AI/` | Enemy AIController — perception, combat target, enemy StateTree |
| `Animation/Notifies/` | Cancel window and hit detection AnimNotifyStates — timing sources for montages |
| `Game/` | GameMode, PlayerController (EnhancedInput binding, input buffering dispatch, ability grants) |

### Core Design Rules

- **Tag-driven**: action states, cancel windows, inputs, events, and blocking conditions use GameplayTags
- **Atomic abilities**: each independently lifecycle-managed action is a `UGameplayAbility`; only one attack ability active per character at a time
- **Action context serial IDs**: prevent stale callbacks from old abilities/notifies/tasks from corrupting the current action
- **StateTree decides "what to do"; GameplayAbility decides "how to do it"; Components maintain queryable facts**
- **Montage-driven attack lifecycle**: attacks are driven by `UAbilityTask_PlayMontageAndWait`; cancel windows and hit windows come from AnimNotifyStates on the montage

## Coding Conventions

- Comments on key functions and non-obvious logic use **Chinese**
- Unresolved details marked with `//TODO`
- UE object members prefer `TObjectPtr<T>`
- Do not fabricate module/class/function/asset paths that don't exist — verify against source or ask
- Keep logic in the right layer: input stays in controller/input components, combat in ability system/components, presentation in animation/notifies — don't pile everything back into character classes
