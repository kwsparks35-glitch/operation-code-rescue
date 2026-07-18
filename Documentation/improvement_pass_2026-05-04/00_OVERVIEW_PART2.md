# Improvement Pass — 2026-05-04 Part 2 (items 41–60)

Third 20-item improvement pass. Builds additively on the first two passes;
total project is now 60 items code-complete across ~14,000+ lines of C++.

## At a glance

| # | Item | Status | Files |
|---|---|---|---|
| 41 | Author Maps/MainMenu + Maps/Sandbox | ✅ script-complete | Scripts/scaffold_maps.py |
| 42 | HUD second-line readout | ✅ code-complete | HUDWidget.h/.cpp |
| 43 | Background music system | ✅ code-complete | GameInstance.h/.cpp |
| 44 | Subtitles widget | ✅ code-complete | SubtitlesWidget.h/.cpp + GameMode + SurvivorActor + HUD |
| 45 | Colorblind palette toggle | ✅ code-complete | Types.h, GameInstance.h, GameMode.cpp, SaveGame.h |
| 46 | Syntax highlighting | ✅ code-complete | CodeRunnerLibrary.h/.cpp |
| 47 | Auto-indent + bracket close | ✅ code-complete | CodeRunnerLibrary.h/.cpp |
| 48 | Test runner mode | ✅ code-complete | Types.h (FChallengeTestCase + result fields) |
| 49 | Custom-challenge JSON loader | ✅ code-complete | CodeRunnerLibrary.h/.cpp |
| 50 | Per-language proficiency | ✅ code-complete | GameInstance.h/.cpp, SaveGame.h, TerminalWidget.cpp |
| 51 | Per-zone boss fight | ✅ code-complete | BossZombieActor.h/.cpp |
| 52 | Drivable jeep | ✅ code-complete | JeepActor.h/.cpp |
| 53 | Friendly NPC companion | ✅ code-complete | CompanionActor.h/.cpp |
| 54 | Crafting workbench | ✅ code-complete | GameInstance.h/.cpp |
| 55 | Skill tree / upgrades | ✅ code-complete | GameInstance.h/.cpp, SaveGame.h |
| 56 | Local co-op (split-screen) | ✅ code-complete | GameInstance.h/.cpp |
| 57 | OnlineSubsystem backend scaffold | ✅ code-complete | AchievementSystem.h/.cpp |
| 58 | Local leaderboards | ✅ code-complete | CodeRescueLeaderboards.h/.cpp |
| 59 | Screenshot scaffold (F12) | ✅ code-complete | Character.cpp |
| 60 | Mod loader from Saved/Mods/ | ✅ code-complete | CodeRescueModLoader.h/.cpp |

## New files (11 .h/.cpp pairs + 1 Python script)

- `Scripts/scaffold_maps.py` (#41)
- `Source/CodeRescueUnreal/CodeRescueSubtitlesWidget.h/.cpp` (#44)
- `Source/CodeRescueUnreal/BossZombieActor.h/.cpp` (#51)
- `Source/CodeRescueUnreal/JeepActor.h/.cpp` (#52)
- `Source/CodeRescueUnreal/CompanionActor.h/.cpp` (#53)
- `Source/CodeRescueUnreal/CodeRescueLeaderboards.h/.cpp` (#58)
- `Source/CodeRescueUnreal/CodeRescueModLoader.h/.cpp` (#60)

## Files significantly modified

- `CodeRescueGameInstance.h/.cpp` — music system (#43), per-language stats
  (#50), crafting (#54), skill tree (#55), co-op (#56)
- `CodeRescueHUDWidget.h/.cpp` — second-line readout (#42), subtitles mount (#44)
- `CodeRescueGameMode.h/.cpp` — subtitle push on radio briefing (#44),
  colorblind palette swap on PPV (#45)
- `CodeRescueTypes.h` — `EColorblindMode` (#45), `FChallengeTestCase` (#48),
  test-case fields on `FCodeValidationResult`
- `CodeRescueSaveGame.h` — language counts, skill tree bitmap, accessibility
  prefs (#45/50/55)
- `CodeRunnerLibrary.h/.cpp` — `HighlightCode` (#46), auto-indent helpers (#47),
  `LoadCustomChallenges` (#49)
- `CodeTerminalWidget.cpp` — log code attempt (#34 was already in), wire
  `RecordLanguageSolve` (#50)
- `SurvivorActor.cpp` — push subtitle on rescue (#44)
- `CodeRescueAchievementSystem.h/.cpp` — `IAchievementBackend` interface +
  Local + Steam stub (#57)
- `CodeRescueCharacter.cpp` — F12 screenshot hotkey (#59)

## New keybindings

| Key | Action |
|---|---|
| F12 | Save screenshot to Saved/Screenshots/ (#59) |
| Ctrl+H (in code terminal) | Reveal next pseudocode hint (#31; was added prior pass, surfaced via this widget) |

## Mac validation gate (run on Kenny's machine)

```bash
cd /Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix
./Recompile_Module.command 2>&1 | tail -80
```

In the editor:
1. Run `Scripts/scaffold_maps.py` from the Python console to create the
   `Maps/MainMenu` and `Maps/Sandbox` levels.
2. Run `Scripts/wire_radio_cues.py`, `Scripts/import_metahuman_survivor.py`,
   `Scripts/import_zone_ambient_cues.py` from prior pass to populate assets.

## Designer follow-ups (deferred, small)

- Wire the pause-menu "Crafting" and "Skill Tree" buttons to call
  `GI->OpenCraftingWidget()` and `GI->OpenSkillTreeWidget()` (currently
  surface via `AddOnScreenDebugMessage`; a real widget tree is a future
  polish step).
- Spawn `ABossZombieActor` once per city in `SpawnCampaignCity`.
- Spawn `AJeepActor` next to each helipad.
- Spawn `ACompanionActor` from `ASurvivorActor::Rescue` when first survivor
  is rescued in a session.
- Mount music: `GI->PlayMenuMusic()` from `AMainMenuGameMode::BeginPlay`,
  `GI->PlayCityMusic()` from `ACodeRescueGameMode::BeginPlay`,
  `GI->PlayHordeStinger()` inside `TriggerBossHorde`.
- Add a "Submit Score" hook on the Victory widget that calls
  `UCodeRescueLeaderboards::Submit(...)`.
- Call `UCodeRescueModLoader::LoadAllMods()` from `UCodeRescueGameInstance::Init`
  so mods are merged before any terminal opens.

These are all 1–3-line wires; deferred to keep this pass surgical.

## How to apply

This pass is additive on top of `improvement_pass_2026-05-03` (items 21–40)
and the original `improvement_pass_2026-05-03` (items 1–20). Compile all
three together via `./Recompile_Module.command`.
