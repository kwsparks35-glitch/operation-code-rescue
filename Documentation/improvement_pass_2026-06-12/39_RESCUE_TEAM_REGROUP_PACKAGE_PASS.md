# Rescue Team Regroup Control and Fresh Package Pass

Date: 2026-06-12

## Package

Fresh Mac package:

```text
/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app
```

Package evidence:

```text
Size: 1.9G
Timestamp: Jun 12 14:57:04 AKDT 2026
```

## Gameplay Updates

- Added a player-facing `Y` regroup command for the rescue team. Pressing `Y`
  calls active, operational companions back into a staggered formation behind
  the player.
- Added companion-side `RegroupNearPlayer` support that stops companion
  movement before teleporting each teammate into formation. This gives the
  player a direct recovery control if the squad falls behind, crowds an
  access point, or needs to gather before a fight.
- Added subtitle and on-screen debug feedback after regrouping. The player now
  receives an exact count of teammates pulled back to position, or a clear
  message if no active rescue-team members are available.
- Updated the rescue-team HUD line so it normally advertises `Y REGROUP` and
  briefly changes to `REGROUPED N` after a successful command.
- Expanded `Scripts/verify_june01_rescue_survivability_pass.py` so the full
  QA audit now locks the regroup command, HUD feedback, companion movement
  stop, and teleport-regroup behavior alongside the existing health,
  survivability, attack-direction, rescue-team, squad-HUD, non-blocking
  companion, access-cleanup, and documentation contracts.

## Validation

Passed:

```bash
python3 Scripts/verify_june01_rescue_survivability_pass.py
./Recompile_Module.command
./Run_Full_QA_Audit.command
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
```

Relevant logs:

```text
Saved/Logs/HeadlessFullQASmoke.log
Saved/Logs/PackagedSmoke_null.log
Saved/Logs/PackagedSmoke_render.log
```

The full QA audit rebuilt the module, passed static verifiers, ran Unreal
commandlets, launched headless runtime smoke, and completed the smoke log scan.
The package command rebuilt, cooked, staged, signed locally, and archived the
Mac app. Both packaged smoke modes mounted cooked Pak/IoStore data, loaded
`/Engine/Maps/Entry`, initialized `CodeRescueGameMode`, emitted the expected
`[CodeRescueUnrealSystems]`, `[CodeRescuePublicDemoQuality]`,
`[CodeRescueSafeLearning]`, `[CodeRescueCreativeImplementation]`, and
`[CodeRescueEntryAccess]` markers, and exited cleanly.

## Regressions and Diagnostics

No new blocking regressions were found in this pass.

Allowed diagnostics remained limited to:

- Immediate-quit navigation dirty-area warning in headless/package smoke.
- Immediate-quit crowd-following RecastNavMesh warning in headless/package
  smoke.
- Unattended macOS CoreAudio sample-rate query warning in packaged render
  smoke.

## Manual Playtest Focus

- Press `Y` while the full rescue team is active and confirm all operational
  teammates regroup behind the player without blocking the player.
- Confirm the HUD normally shows `Y REGROUP`, then briefly shows
  `REGROUPED N` after the command.
- Confirm the subtitle count matches the visible active squad.
- Repeat the regroup check near entry, armory, safehouse, language plaza,
  terminal, survivor, and helipad access points to verify the command helps
  recover from crowding without introducing new access blockers.
- Continue balance playtesting around player survivability, medic support, and
  attack-direction readability in live combat.
