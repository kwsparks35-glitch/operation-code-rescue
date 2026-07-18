# Playable Operator Identity Slice

## Summary

This slice continues the June 25 P0 character guidance by making the playable pawn read as a named rescue operator in the actual runtime. The project already had a Manny player body, first-person arms, camera modes, retargeting tags, and a cast-promotion stage; this pass connects those pieces to a selected-language operator identity that is saved, shown in HUD/status text, surfaced on start-screen resume summaries, and mirrored in the world.

## Runtime Work

- Added operator identity fields to `UCodeRescueSaveGame`:
  - `OperatorCallsign`
  - `OperatorRoleTitle`
  - `OperatorProfileNote`
  - `bHasOperatorIdentityState`
- Added matching `UCodeRescueGameInstance` fields plus:
  - `InitializeOperatorIdentityForLanguage`
  - `GetOperatorIdentitySummary`
- Mapped language runs to stable operator profiles:
  - Java: Rhea Calder, Rescue Operator
  - C: Ilan Cross, Systems Engineer
  - Python: Noor Vance, Signal Analyst
  - MATLAB: Mika Stone, Triage Analyst
  - C+: Jules Ardent, Supply Strategist
  - C++: Rhea Calder, Advanced Rescue Operator
- Threaded identity through fresh language runs, older-save fallback initialization, save serialization, load restoration, and start-screen language save summaries.
- Added `ACodeRescueCharacter::GetOperatorIdentitySummary` and runtime tags:
  - `PlayableOperatorIdentityRuntime`
  - `SelectedLanguageOperatorProfile`
  - `PlayerOperator`
- Updated `UCodeRescueHUDWidget` to show `Operator: callsign (role)` in the runtime status line.
- Updated the cast-promotion stage with an `ACTIVE OPERATOR PROFILE` board using the same `GetOperatorIdentitySummary` output.

## Player-Facing Behavior

- The start screen remains a language-selection screen, not a character-selection screen.
- Starting a fresh language run initializes a matching operator profile automatically.
- Resuming a saved language run preserves and displays its operator callsign and role.
- The HUD makes the active operator identity visible during play.
- The world cast stage now confirms that the active runtime pawn is part of the same MetaHuman/Manny/Quinn promotion plan as the documented character roster.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueSaveGame.h`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.h`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.cpp`
- `Source/CodeRescueUnreal/CodeRescueCharacter.h`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Scripts/verify_save_compatibility_pass.py`
- `Content/CodeRescueData/playable_operator_identity_manifest.tsv`
- `Scripts/verify_playable_operator_identity_slice_pass.py`

## Validation Plan

- `python3 -m py_compile Scripts/verify_playable_operator_identity_slice_pass.py`
- `python3 Scripts/verify_playable_operator_identity_slice_pass.py`
- `python3 Scripts/verify_save_compatibility_pass.py`
- `verify_camera_perspectives_and_character_roster.py` through Unreal commandlet.
- Module recompile.
- Mac package.
- Packaged null and render smoke.
- Manual QA: start and resume at least two language profiles, confirm the start-screen resume row, HUD readout, and cast-stage board show the expected saved operator profile while gameplay still begins from the language chooser.
