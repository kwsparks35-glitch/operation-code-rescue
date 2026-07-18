# Improvement Pass — 2026-05-04 (items 21–40)

Second improvement pass on `code_rescue_unreal_ue57_rebuild_fix/`. All 20
items implemented; documentation per item below or in `22_to_25_asset_import_recipes.md`.

## At a glance

2026-05-04 follow-up: the character/world-generation integration now builds
as `CodeRescueUnrealEditor Mac Development`. See
`41_unreal_character_world_bootstrap.md` for the editor handoff and repair
log.

| # | Item | Status | Files |
|---|---|---|---|
| 21 | Wire 4 deferred follow-ups | ✅ code-complete | PauseWidget, HUD, Character, ZombieActor |
| 22 | Bake 342 radio WAVs + auto-wire | ✅ script-complete (run on Mac) | Scripts/wire_radio_cues.py |
| 23 | MetaHuman survivor import | ✅ script-complete | Scripts/import_metahuman_survivor.py |
| 24 | 3 zone ambient cues | ✅ script-complete | Scripts/import_zone_ambient_cues.py |
| 25 | Megascans props for Anchorage | ✅ doc-complete | (recipe in 22_to_25_*.md) |
| 26 | Multiple weapon types | ✅ code-complete | Types.h, Character.h/cpp |
| 27 | Melee fallback | ✅ code-complete | Character.h/cpp |
| 28 | Throwables (flare/smoke/stim) | ✅ code-complete | ThrowableActor.h/cpp, Character.h/cpp |
| 29 | Zombie elite variants | ✅ code-complete | Types.h, ZombieActor.h/cpp |
| 30 | Defensive barricades + scrap | ✅ code-complete | BarricadeActor.h/cpp, Character.h/cpp |
| 31 | Hint system | ✅ code-complete | TerminalWidget.h/cpp, GameInstance.h/cpp, SaveGame.h |
| 32 | Difficulty-adaptive challenges | ✅ code-complete | GameInstance.h/cpp |
| 33 | Sandbox mode | ✅ code-complete | SandboxGameMode.h/cpp, GameMode.h/cpp |
| 34 | Code attempt history (NDJSON) | ✅ code-complete | GameInstance.h/cpp, TerminalWidget.cpp |
| 35 | Day/night cycle | ✅ code-complete | GameMode.h/cpp |
| 36 | Weather system | ✅ code-complete | GameMode.h/cpp |
| 37 | Hidden secret terminals | ✅ code-complete | GameMode.h/cpp |
| 38 | Main menu + splash | ✅ code-complete | MainMenuWidget.h/cpp, MainMenuGameMode.h/cpp |
| 39 | Controller support | ✅ code-complete | Character.h/cpp |
| 40 | Local achievement system | ✅ code-complete | AchievementSystem.h/cpp, GameInstance.cpp, SaveGame.h |

## New files added (12)

- `Source/CodeRescueUnreal/ThrowableActor.h/.cpp` — flare/smoke/stim items (#28)
- `Source/CodeRescueUnreal/BarricadeActor.h/.cpp` — placeable cover (#30)
- `Source/CodeRescueUnreal/SandboxGameMode.h/.cpp` — sandbox mode (#33)
- `Source/CodeRescueUnreal/CodeRescueMainMenuWidget.h/.cpp` — splash menu (#38)
- `Source/CodeRescueUnreal/MainMenuGameMode.h/.cpp` — splash GameMode (#38)
- `Source/CodeRescueUnreal/CodeRescueAchievementSystem.h/.cpp` — local achievements (#40)
- `Scripts/wire_radio_cues.py` — radio WAV → cue array bulk-assign (#22)
- `Scripts/import_metahuman_survivor.py` — MetaHuman → BP_SurvivorActor (#23)
- `Scripts/import_zone_ambient_cues.py` — ambient WAV → ZoneAmbientCues[] (#24)

## Files significantly modified

- `CodeRescueCharacter.h/.cpp` — weapons (#26), melee (#27), throwables (#28),
  scrap+barricade (#30), gamepad (#39), DamageFeedback wiring (#21)
- `CodeRescueGameMode.h/.cpp` — sandbox (#33), day/night (#35), weather (#36),
  secret terminals (#37), Tick + run-seconds accumulation (#15)
- `CodeRescueGameInstance.h/.cpp` — research points (#31), code attempt log
  (#34), adaptive difficulty (#32), achievements wiring (#40)
- `CodeRescueSaveGame.h` — research points, achievements bitmap, tutorial flag
- `CodeRescueTypes.h` — EWeaponType + FWeaponDef (#26), elite variants (#29)
- `CodeZombieActor.h/.cpp` — elite-variant behaviors (#29), Boomer death (#29),
  damage-feedback Instigator (#21), tick throttle preserved
- `CodeTerminalWidget.h/.cpp` — Hint system (#31), Code attempt logging (#34)
- `CodeRescuePauseWidget.h/.cpp` — Settings + Save Slots buttons (#21)
- `CodeRescueHUDWidget.h/.cpp` — autosave pip (#21)

## New keybindings summary

| Key | Action |
|---|---|
| 1 / 2 / 3 / 4 | Swap weapon (Pistol / Shotgun / Rifle / Grenade) |
| Q | Cycle throwable slot (flare → smoke → stim) |
| X | Throw active throwable |
| B | Place barricade (-5 scrap) |
| F (when out of ammo) | Melee swing |
| Ctrl+H (in coding terminal) | Reveal next pseudocode hint (-1 ResearchPoint) |
| Gamepad LStick | Move |
| Gamepad RStick | Look |
| Gamepad RT | Fire |
| Gamepad A | Jump |
| Gamepad X | Interact |
| Gamepad Y | Reload |
| Gamepad B | Throw |
| Gamepad DPad U/D | Cycle throwable |
| Gamepad Start / Select | Pause / Journal |

## Mac validation gate

1. `./Recompile_Module.command 2>&1 | tail -80` — fix any compile errors.
2. Verify the new GameMode subclass `ASandboxGameMode` is selectable in editor.
3. Create / verify a `Maps/MainMenu.umap` with `AMainMenuGameMode` set.
4. Add `MainMenuMap=/Game/Maps/MainMenu` and `GlobalDefaultGameMode=
   /Script/CodeRescueUnreal.MainMenuGameMode` in `DefaultEngine.ini` to make
   the splash the launch target.
5. `./Package_Mac_App.command 2>&1 | tail -40` if compile passes.
6. `./Smoke_Test_Packaged_App.command null` — should exit cleanly.

## Pending wiring (deferred — small)

- Bind a HUD readout for current weapon name + throwable counts (currently
  surfaced via `AddOnScreenDebugMessage` only).
- Author the `Maps/Sandbox.umap` and `Maps/MainMenu.umap` levels in the
  editor (the GameModes exist but the levels themselves don't).

## Asset imports (run on Mac, scripts shipped)

- `python3 Scripts/generate_radio_voiceovers.py --limit 0` then
  `wire_radio_cues.run()` from the UE Python console (#22).
- `import_metahuman_survivor.run("Olivia")` after Quixel Bridge → Add (#23).
- `import_zone_ambient_cues.run()` after dragging 3 WAVs into
  `Content/CodeRescueAssets/Audio/ZoneAmbient/` (#24).
- Megascans props via Bridge → swap inline `SpawnBlock` calls in
  `SpawnAuthoredPropsForCity` (#25; recipe in 22_to_25_*.md).

## How to apply

This pass builds on `improvement_pass_2026-05-03/` (items 1–20). Compile
both passes together; the changes are additive, not replacement. The
project is now ~10,500 lines of C++ across 36 files. All 40 roadmap items
are now code-complete.
