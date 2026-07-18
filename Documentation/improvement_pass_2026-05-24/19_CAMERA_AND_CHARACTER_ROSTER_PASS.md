# Operation Code Rescue - Camera and Character Roster Pass

Date: 2026-05-24

Scope: add selectable gameplay perspectives, verify that the player can switch perspectives during active character operation, and incorporate the locally available Unreal character roster into the runtime learning-rescue environment.

## Completed This Session

1. Expanded the player camera system from three distance states to six named gameplay perspectives: First-Person, Third-Person, Tactical Third-Person, Top-Down, Isometric, and Side-View 2.5D.
2. Added direct in-game perspective selection controls: `C` cycles every mode, while `5`, `6`, `7`, `8`, `9`, and `0` select specific perspectives.
3. Added a Blueprint-callable and console-callable `SelectCameraPerspective(int32)` API so gameplay, tests, and future UI can select views without duplicating camera setup logic.
4. Added perspective labels and a getter so HUD/UI/test code can inspect the active camera state.
5. Updated movement vectors so top-down, isometric, and side-view modes keep useful directional controls instead of inheriting disorienting player-facing movement.
6. Suppressed vertical look input in fixed-camera modes so top-down/isometric/side-view play stays stable.
7. Updated the first-minute in-world controls board and the mission help panel with the new camera controls.
8. Incorporated the Quinn mannequin into `ACompanionActor`, including mesh placement, animation assignment, visibility, and collision cleanup.
9. Preserved Manny-backed player/NPC use and Quinn-backed survivor/companion use as the core friendly roster.
10. Extended the zombie variant fallback roster so a partial `DT_ZombieVariants` asset no longer suppresses built-in regular and elite zombie variants.
11. Added built-in elite rows for `EliteSpitter`, `EliteCharger`, and `EliteBoomer`.
12. Extended the character/world asset verifier to require the local Manny, Quinn, simple mannequin, legacy UE4 mannequin, and zombie character assets.
13. Added `Scripts/verify_camera_perspectives_and_character_roster.py`, an active commandlet test that loads required character assets, spawns gameplay character classes, and exercises all camera perspectives repeatedly.
14. Tightened roster verification so spawned characters must expose their expected interaction, rescue, damage, or camera-selection methods and include skeletal visual components.
15. Updated `Run_Character_World_Demo.command` so playtesters see the new camera controls and roster verification scope before launch.

## Character Roster Incorporated

- Player: Manny mannequin mesh and animation.
- Survivor: Quinn mannequin mesh and animation.
- Companion: Quinn mannequin mesh and animation.
- Friendly NPCs: Manny/Quinn mannequin-backed roles.
- Regular enemies: dog zombie, urban zombie, business zombie, bloated zombie, nurse zombie, and base zombie meshes.
- Elite enemies: `EliteSpitter`, `EliteCharger`, and `EliteBoomer` fallback variants.
- Legacy compatibility: UE4 mannequin mesh remains verified for projects or content that still reference it.

The session verified and incorporated the character assets already present inside the project content tree. External Epic/Fab/Marketplace downloads require launcher/account interaction and were not performed from this command-line development session.

## Active Test Coverage

`Scripts/verify_camera_perspectives_and_character_roster.py` verifies:

- Required local character assets are present.
- Runtime character classes load.
- Player, survivor, friendly NPC, companion, regular zombie, and boss zombie actors spawn.
- Spawned characters expose their expected interaction/rescue/damage/camera methods.
- Spawned characters include skeletal visual components for authored character presentation.
- The player perspective selector moves through all six modes across three passes.
- Each perspective reports the expected label, active camera component, and spring-arm length.

`Scripts/verify_character_world_assets.py` verifies:

- The expanded mannequin and zombie asset roster is present.
- City/world dressing assets needed by the generated rescue environment are present.
- `DT_ZombieVariants` can be loaded for runtime variant selection.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueCharacter.h`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Source/CodeRescueUnreal/CompanionActor.h`
- `Source/CodeRescueUnreal/CompanionActor.cpp`
- `Scripts/verify_character_world_assets.py`
- `Scripts/verify_camera_perspectives_and_character_roster.py`
- `Run_Character_World_Demo.command`
- `Documentation/improvement_pass_2026-05-24/19_CAMERA_AND_CHARACTER_ROSTER_PASS.md`
- `progress.md`

## Verification

- `./Recompile_Module.command` succeeded for `CodeRescueUnrealEditor Mac Development`.
- `Scripts/verify_camera_perspectives_and_character_roster.py` passed with `Success - 0 error(s), 0 warning(s)`.
- `Scripts/verify_character_world_assets.py` passed with `Success - 0 error(s), 0 warning(s)`.
- Headless `-game -NullRHI -NoSound -Unattended -NoRadioVoice -ExecCmds="Quit"` boot smoke exited with code 0.
- A multi-command `-ExecCmds` camera-selection smoke booted but did not reliably self-quit through the game command path, so camera selection validation is intentionally covered by the active commandlet verifier above.
