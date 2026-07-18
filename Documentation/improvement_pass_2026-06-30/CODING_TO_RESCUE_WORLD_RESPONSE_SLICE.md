# Coding-To-Rescue World Response Slice - 2026-06-30

## Purpose

This pass continues creative development by making the core fantasy more visible: solving code now causes the rescue world to respond. The player already needed to solve a terminal before rescuing a survivor, but the environment did not strongly show that the lesson had opened the route. This slice adds an immediate and save-restored world response: route pulses, beacons, a survivor extraction arch, subtitles, and a small route cache.

## Source Guidance

- `WORLD_DEVELOPMENT_DEEPDIVE`: make the coding-as-rescue premise legible through authored spaces, environmental storytelling, route clarity, and cause/effect.
- `TOP_50_RECOMMENDATIONS`: improve objective clarity and first-order gameplay readability before broad asset replacement.
- Prior `CREATIVE_PHYSICS_WORLD_VERTICAL_SLICE`: keep systems playable and reviewable through tagged world actors and static regression checks.

## Implemented Systems

### Immediate Solve Response

- `UCodeTerminalWidget::OnValidateClicked` captures the terminal location before hiding the solved actor.
- On a successful validation, the widget calls `ACodeRescueGameMode::RevealSolvedTerminalRescueRoute`.
- The player receives a subtitle and on-screen confirmation that the rescue route is unlocked.

### World Route Reveal

`ACodeRescueGameMode::RevealSolvedTerminalRescueRoute` now creates a tagged solved-state route layer:

- a solved-code relay beacon at the terminal,
- visible pulse strips from the terminal toward the survivor,
- mid-route beacons,
- a survivor extraction arch,
- a survivor-route-open light,
- a small reward cache with stim, armor, and smoke pickups.

All route response actors receive review tags:

- `CodingToRescueWorldResponse`
- `SolvedTerminalWorldResponse`
- `WorldDevelopmentDeepDive`
- `CodingCauseEffect`
- `TerminalSolvedRouteVisible`
- a per-terminal `SolvedRoute_<TerminalId>` tag

The per-terminal tag makes the response idempotent so repeated solve/save-restore paths do not stack duplicate route actors.

### Save/Reload Reconstruction

- `SpawnTerminal` now calls the route reveal when it sees that a terminal ID is already solved.
- `UCodeRescueGameInstance::ApplyObjectiveStateToLevel` also calls the route reveal before marking solved terminals hidden.
- This keeps the start screen and language-specific save loop intact: the start screen still appears on future launches, but resuming a language save rebuilds solved-route world feedback inside the game.

## Regression Coverage

- Added `Scripts/verify_coding_world_response_slice_pass.py`.
- Wired it into:
  - `Run_Full_QA_Audit.command`
  - `Run_Local_CI_Readiness.command`
- The verifier checks the GameMode API, route tags, duplicate guard, terminal validation hook, save-restore hook, terminal-spawn solved-state hook, documentation, and progress-log entry.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Source/CodeRescueUnreal/CodeTerminalWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.cpp`
- `Scripts/verify_coding_world_response_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Remaining Work

This slice makes solved-code cause/effect visible, but it is still procedural-block implementation. The next world-development slice should replace the route pulses and survivor arch with authored modular assets or PCG-authored set pieces, add stronger audio/VFX feedback, and connect the unlocked route to a small encounter beat that changes based on the selected coding-language track.
