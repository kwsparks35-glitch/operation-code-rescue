# Weapon Quick-Slot Armory Slice

## Purpose

This slice completes the P0 weapon quick-slot and visible armory row by making the immediate arsenal understandable and save-backed. The project already had a broad weapon loadout, number-key bindings, cycling controls, and an armory display; this pass makes the 1-0 quick-slot layout visible in the HUD and world, then persists active weapon and per-slot ammo in the selected-language save.

## Implementation

- Added save fields to `UCodeRescueSaveGame`: `ActiveWeapon`, `WeaponMagazines`, `WeaponReserveAmmo`, and `bHasWeaponQuickSlotState`.
- Added matching runtime cache fields to `UCodeRescueGameInstance`.
- `CaptureWorldStateFromLevel` now records the player active weapon, magazine array, and reserve array.
- `SavePersistentRunInternal` writes quick-slot state into the active language slot, and `LoadPersistentRun` restores the cached arrays only when `bHasWeaponQuickSlotState` is present.
- `ApplyWorldStateToLevel` restores weapon quick-slot state after health/resources so saved magazines and reserves are not flattened into a generic ammo pool.
- Added `ACodeRescueCharacter::RestoreWeaponQuickSlotState` and `ACodeRescueCharacter::GetWeaponQuickSlotSummary`.
- Updated `UCodeRescueHUDWidget` so the weapon strip shows a compact full row of `1-0` quick slots with an active marker, slot name, and magazine/reserve ammo.
- Updated `SpawnTacticalArmoryLayer` with a visible `QUICK SLOT BOARD`, key labels for the first ten weapons, and wheel labels for the extended arsenal.

## Player Impact

The player can now read the full weapon quick-slot row at a glance, switch by key, cycle through the extended arsenal, and trust that active weapon plus per-slot ammo state will follow the selected coding-language save. The armory, HUD, and save system now describe the same weapon contract.

## Files Changed

- `Source/CodeRescueUnreal/CodeRescueSaveGame.h`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.h`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.cpp`
- `Source/CodeRescueUnreal/CodeRescueCharacter.h`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Content/CodeRescueData/weapon_quick_slot_armory_manifest.tsv`
- `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Scripts/verify_weapon_quick_slot_armory_slice_pass.py`
- `Scripts/verify_save_compatibility_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`
- `progress.md`

## Verification

- `python3 -m py_compile Scripts/verify_weapon_quick_slot_armory_slice_pass.py`
- `python3 Scripts/verify_weapon_quick_slot_armory_slice_pass.py`
- `python3 Scripts/verify_may27_tactical_arsenal_mcp_runtime.py`
- `python3 Scripts/verify_save_compatibility_pass.py`
- `./Recompile_Module.command < /dev/null`
- `./Package_Mac_App.command < /dev/null`
- `./Smoke_Test_Packaged_App.command null`
- `./Smoke_Test_Packaged_App.command render`

## Human QA Notes

Start a selected-language run, stand at the tactical armory, and confirm the board lists quick slots `1` through `0` plus wheel-only extended weapons. Press each number key, fire or reload at least two weapons, save, relaunch through the start screen, and resume the same language. Confirm the HUD still marks the correct active slot and preserves the magazine/reserve values for those weapons.
