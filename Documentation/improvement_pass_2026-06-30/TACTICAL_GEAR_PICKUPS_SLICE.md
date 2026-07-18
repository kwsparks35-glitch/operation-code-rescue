# Tactical Gear Pickups Slice

Date: 2026-06-30

## Source Guidance

This pass implements the tactical gear pickup recommendation from `TOP_50_RECOMMENDATIONS.pdf`, `GAME_PHYSICS_DEEPDIVE.pdf`, and the release dossier's playability-readability guidance. The specific target was to move scanner, flashlight, armor, ammo pouch, medkit, and bypass kit from a manual/visual promise into saved, usable gameplay systems.

## Runtime Implementation

- `EPickupKind` now includes `RadioScanner`, `FlashlightBattery`, `AmmoPouch`, and `BypassKit` alongside the existing ammo, medkit, throwables, scrap, and armor plate resources.
- `APickupActor::Collect` grants those resources through real `ACodeRescueCharacter` methods and immediately saves the selected language run after a successful pickup.
- `ACodeRescueCharacter` now tracks scanner charges, flashlight batteries, bypass kits, ammo pouch capacity, throwables, scrap, armor, and medkits as one field kit.
- `L` toggles the field flashlight by activating a real `FieldFlashlight` point light and spending one flashlight battery when turning it on.
- `Z` spends a radio scanner charge and reports active route guidance through subtitles and the HUD/debug channel.
- Ammo pouch pickups expand reserve capacity and refill the newly added capacity amount.
- `CodeTerminalWidget` adds a `USE BYPASS KIT [Ctrl+B]` action. It consumes one bypass kit, records the selected language track, opens the survivor route, and disables clean-solve rewards for that terminal.

## Save And Resume Contract

The expanded gear is persisted in the selected language save slot through `UCodeRescueSaveGame` fields:

- `PlayerArmorPlates`
- `PlayerFlares`
- `PlayerSmokes`
- `PlayerStims`
- `PlayerScrap`
- `PlayerRadioScannerCharges`
- `PlayerFlashlightBatteries`
- `PlayerBypassKits`
- `PlayerAmmoPouchCapacityBonus`
- `bHasPlayerTacticalGear`

Older saves continue through the existing `RestorePlayerResources` path. Saves with the new gear fields use `RestorePlayerResourcesDetailed` so close/relaunch preserves the full field kit without changing the start-screen language resume flow.

## Placement And UI

The tactical armory, creative gear hub, and city route now spawn scanner, flashlight, ammo pouch, and bypass kit pickups. The HUD top panel, second line, weapon strip, and tactical readout show scanner, flashlight, bypass, ammo pouch, armor, medkit, throwable, and scrap state so players can see that pickups are functional.

## Verification

- `Scripts/verify_tactical_gear_pickups_slice_pass.py`
- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks pickup kinds, collection effects, keybinds, terminal bypass UI, selected language save persistence, HUD readouts, spawn placement, manifests, and this documentation.
