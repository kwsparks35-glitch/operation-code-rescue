# Language Launch, Grounding, Symbols, and Pickup Availability Pass

Date: 2026-06-18

## Goal

This pass implements the requested launch and gameplay cleanup:

- The player is prompted to choose a coding language at game start.
- Choosing a language immediately starts the campaign.
- Active play stays on that single selected coding language.
- Characters and pickups read as grounded on solid platforms.
- Most world word ribbons collapse into compact symbolic markers.
- All player-usable pickup categories are spawned and collectible.

## Gameplay Changes

1. Main menu language buttons now call `SelectLanguageAndLaunch`, reset the run while preserving the selected language, save it, and immediately open the campaign map.
2. The fallback New Game path is relabeled `START SELECTED LANGUAGE` and continues to start the campaign with the stored selected track.
3. Procedural active-play language stations no longer spawn interactive `ALanguageStationActor` instances. Only the selected launch-language marker is emitted.
4. `ALanguageStationActor::ActivateStation` no longer changes the active run language if a placed station is encountered.
5. Terminal, HUD, progress summary, academy, banner, and mentor surfaces now present selected-language-only runtime information.
6. Generic terminal hints and campaign briefing text no longer direct players toward another in-world language station.
7. `SpawnGuideText` now keeps text for controls and safety-critical prompts, but converts most ambient signage into compact symbols.
8. Large zone and city mission floors were raised from center z -35 to z -6 so the visible slab top meets the gameplay plane.
9. `APickupActor` now snaps down to the nearest `WorldStatic` surface on BeginPlay and uses a larger trigger sphere for easier collection.
10. The tactical armory and city route now spawn ammo, medkit, flare, smoke, stim, scrap, and armor plate pickups.

## Next 20 Recommendations Implemented

The detailed next-20 implementation ledger is:

`Content/CodeRescueData/launch_language_grounding_symbol_pickup_next20_manifest.tsv`

That manifest captures the 20 highest follow-up improvements made in this pass across launch flow, language locking, visual clarity, grounding, pickups, and QA.

## Files Updated

- `Source/CodeRescueUnreal/CodeRescueMainMenuWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueMainMenuWidget.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.cpp`
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeTerminalWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueCampaign.cpp`
- `Source/CodeRescueUnreal/LanguageStationActor.cpp`
- `Source/CodeRescueUnreal/PickupActor.cpp`
- `Source/CodeRescueUnreal/PickupActor.h`
- `Documentation/QA_PLAYTEST_CHECKLIST.md`
- `Run_Full_QA_Audit.command`
- `Scripts/verify_june18_launch_grounding_symbol_pickup_pass.py`

## Verification

Added static verifier:

`python3 Scripts/verify_june18_launch_grounding_symbol_pickup_pass.py`

The verifier is wired into `Run_Full_QA_Audit.command` and checks the language launch contract, active-play language lock, selected-language-only surfaces, guide-text symbolization, raised floor slabs, pickup ground snap, all pickup-kind availability, documentation, and the next-20 manifest.

Validation completed after implementation:

- `python3 Scripts/verify_june18_launch_grounding_symbol_pickup_pass.py`: passed.
- `python3 Scripts/verify_may27_safe_learning_city_controls_pass.py`: passed after updating its stale menu/HUD expectations.
- `python3 Scripts/verify_june01_rescue_survivability_pass.py`: passed after updating stale launch-language wording.
- `python3 Scripts/verify_june18_public_hardening_pass.py`: passed after clarifying Maple fallback wording in `Tools/MapleVoice/README.md`.
- `./Recompile_Module.command`: build succeeded.
- `./Run_Full_QA_Audit.command`: completed successfully; smoke log written to `Saved/Logs/HeadlessFullQASmoke.log`.

## 2026-06-19 Launch Language Choice Cleanup

The original 2026-06-18 pass exposed both `C+` and `C++` in the user-facing
language picker. During the operational review this was cleaned up so the
startup chooser now presents only the active supported tracks:

- Java
- C
- C++
- Python
- MATLAB

The legacy `ECodingLanguage::CPlus` value remains in source for compatibility
with older save/data paths, but there is no longer a `LEARN C+` launch button
or `OnCPlusLanguageClicked` direct-start path. The static verifier now checks
that `C+` is not exposed in the launch chooser while Java, C, C++, Python, and
MATLAB all launch immediately.

## 2026-06-19 Operational Package Follow-up

A follow-up review found that the source implementation was present, but the
previous runnable Mac package was stale. The package visible at
`PackagedMac/Mac/CodeRescueUnreal.app` had been archived before the late
2026-06-18 source updates in the main gameplay files:

- `Source/CodeRescueUnreal/CodeRescueMainMenuWidget.cpp`: `2026-06-18 20:16:02 AKDT`
- `Source/CodeRescueUnreal/PickupActor.cpp`: `2026-06-18 20:16:58 AKDT`
- `Source/CodeRescueUnreal/LanguageStationActor.cpp`: `2026-06-18 20:18:45 AKDT`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`: `2026-06-18 20:30:32 AKDT`

That explained why launching the packaged app could still appear to show old
behavior even though the source and static verifiers contained the requested
changes.

The package was rebuilt on 2026-06-19 with:

`./Package_Mac_App.command`

Fresh package evidence:

- Rebuilt app: `PackagedMac/Mac/CodeRescueUnreal.app`
- Package timestamp: `2026-06-19 15:36:48 AKDT`
- Package size: `2.0G`
- Build result: `BUILD SUCCESSFUL`

Operational verification after the package refresh:

- `./Smoke_Test_Packaged_App.command null`: passed against the rebuilt packaged
  app; log written to `Saved/Logs/PackagedSmoke_null.log` with timestamp
  `2026-06-19 16:20:47 AKDT`.
- `./Smoke_Test_Packaged_App.command render`: passed against the rebuilt
  packaged app's rendered startup path; log written to
  `Saved/Logs/PackagedSmoke_render.log` with timestamp
  `2026-06-19 16:24:20 AKDT`.
- `python3 Scripts/verify_june18_launch_grounding_symbol_pickup_pass.py`:
  passed.
- `git diff --check`: passed.
- `./Run_Full_QA_Audit.command`: completed successfully after the package
  refresh; smoke log written to `Saved/Logs/HeadlessFullQASmoke.log` with
  timestamp `2026-06-19 16:23:17 AKDT`.

Current operational conclusion: the requested source changes are implemented,
the stale package gap has been closed, and the refreshed runnable Mac app now
passes the packaged smoke test plus the full QA audit.

## 2026-06-19 Black First-Frame Visual Follow-up

A second follow-up review used the user's attached launch screenshot as direct
evidence. The image at
`/var/folders/p4/rg7xv9gj12d60fnq392wq0mm0000gp/T/codex-clipboard-2b737a71-3527-4067-8e11-c7c90a5f621e.png`
was completely black. The new verifier confirms that failure mode:

`python3 Scripts/verify_png_not_black.py /var/folders/p4/rg7xv9gj12d60fnq392wq0mm0000gp/T/codex-clipboard-2b737a71-3527-4067-8e11-c7c90a5f621e.png`

Result: `FAIL: image appears blank/black (mean_luma=0.00, max_luma=0.00, visible_ratio=0.0000)`.

Root cause found during this pass:

- Packaged startup loaded `/Engine/Maps/Entry` directly into
  `ACodeRescueGameMode`, but the previously requested language chooser was
  only a widget/menu path and did not have a visible world/camera scene behind
  it.
- The main menu's campaign and sandbox defaults still pointed at non-existent
  `Main` and `Sandbox` maps, so language selection could try to open missing
  content instead of relaunching the generated Entry gameplay environment.
- Unreal's screenshot capture path could capture the empty Entry world behind
  UMG, so a menu that was technically present could still produce a black
  automated visual result.

Implementation completed in response:

- Added a transient launch-language session gate to
  `UCodeRescueGameInstance` so first startup always stops at language
  selection, while post-selection travel and automation can continue into
  active play.
- Added `SpawnLaunchLanguageSelectionScene` in `ACodeRescueGameMode`; it
  creates a visible solid platform, backdrop, lights, camera, prompt, and
  language-choice markers before the UMG language chooser is added.
- Changed the menu's campaign and sandbox map defaults to `Entry`, matching the
  generated gameplay map that actually exists in the package.
- Set the session gate when New Game, Continue, Sandbox, or direct language
  selection launches active play.
- Added `-CodeRescueBypassLaunchLanguageMenu` to smoke/full-QA runtime commands
  so automated active-play tests still exercise gameplay after the new launch
  prompt became mandatory.
- Added `Run_Launch_Menu_Visual_Check.command` to capture the packaged
  non-bypass launch menu, require the launch-language log marker, scan warnings,
  and reject black PNG output.
- Added `Scripts/verify_png_not_black.py` and expanded
  `Scripts/verify_june18_launch_grounding_symbol_pickup_pass.py` so the launch
  scene, launch camera, Entry map defaults, bypass flag, and static mesh
  mobility ordering are now covered by static checks.
- Removed the legacy user-facing `C+` direct launch button from the startup
  chooser to avoid a confusing duplicate between C and C++.
- Adjusted the launch camera setup with a short view-target refresh so the
  packaged first frame stays on the readable launch scene during automated
  visual capture.

Fresh package and visual evidence after the black-screen fix:

- Rebuilt app: `PackagedMac/Mac/CodeRescueUnreal.app`
- Package timestamp: `2026-06-19 19:38:24 AKDT`
- Launch visual log: `Saved/Logs/LaunchMenuVisual.log`
- Launch visual capture:
  `Saved/Screenshots/LaunchMenu/launch_menu_20260619_193834.png`
- Visual verifier result:
  `PASS: mean_luma=97.53, max_luma=245.36, visible_ratio=0.6686`

Operational note: Unreal's `HighResShot` path can capture the rendered launch
world without including the UMG overlay, so the automated launch-menu check now
requires both a nonblack PNG and the `[CodeRescueLaunchLanguageMenu]` log marker.
Together they prove the packaged app entered the mandatory language-menu code
path and produced a visible first frame instead of the black screenshot reported
by the user.

Final verification completed after this follow-up:

- `./Recompile_Module.command`: passed.
- `./Package_Mac_App.command`: passed.
- `./Run_Launch_Menu_Visual_Check.command`: passed.
- `./Smoke_Test_Packaged_App.command null`: passed; log
  `Saved/Logs/PackagedSmoke_null.log`, timestamp
  `2026-06-19 19:38:48 AKDT`.
- `./Smoke_Test_Packaged_App.command render`: passed; log
  `Saved/Logs/PackagedSmoke_render.log`, timestamp
  `2026-06-19 19:38:53 AKDT`.
- `python3 Scripts/verify_june18_launch_grounding_symbol_pickup_pass.py`:
  passed.
- `python3 Scripts/verify_png_not_black.py Saved/Screenshots/LaunchMenu/launch_menu_20260619_193834.png`:
  passed.
- `git diff --check`: passed.
- `./Run_Full_QA_Audit.command`: passed; smoke log
  `Saved/Logs/HeadlessFullQASmoke.log` with final runtime scan at
  `2026-06-19 19:41:36 AKDT`.

Current operational conclusion: the user's black screenshot represented a real
first-frame visual gap. That gap has now been fixed in source, included in a
fresh package, covered by a reusable visual check, and verified by both
packaged launch-menu capture and the full QA audit.
