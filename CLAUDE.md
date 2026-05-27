# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ActionDemo (Project WutheringStyle) is a high-mobility ARPG combat system demo built in Unreal Engine 5.7.

This file is only a quick entry point for Claude Code. The authoritative project rules and current facts live in `ProjectContext.md` and `ProjectWiki.md`; avoid duplicating detailed module or logic-chain facts here.

## Reading Order

When context is missing, read in this order:
1. `ProjectContext.md` — long-term architectural boundaries and collaboration rules
2. `ProjectWiki.md` — query-oriented database of modules, logic chains, tags, assets, decisions
3. Relevant source files

These two files are the authoritative project documentation. If they conflict with this file, follow `ProjectContext.md` and `ProjectWiki.md`.

Wiki update threshold:
- Do not record temporary experiments or abandoned branches.
- After key logic compiles and the implementation direction is settled, update the matching `ProjectWiki.md` database entry.
- Mark unfinished editor validation as `C++ 首版`, `待验证`, or `资产待配`; use `已验收` only after the relevant editor scenario has actually run through.
- Only update `ProjectContext.md` for long-term stable rule changes.

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

If `E:\UE_5.7` does not exist on the current machine, detect the local UE 5.7 installation path or ask before inventing a replacement path.

## Architecture and Coding

Use `ProjectContext.md` for stable architecture boundaries and coding rules. Use `ProjectWiki.md` for current module responsibilities, logic chains, tags, asset/editor state, plans, decisions, and validation status.

Before touching code, verify any class, module, function, tag, or asset path against the source tree, config, or Wiki. Do not fabricate Unreal symbols or asset paths.
