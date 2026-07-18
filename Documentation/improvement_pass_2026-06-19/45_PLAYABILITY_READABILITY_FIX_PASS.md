# Playability Readability Fix Pass

Date: 2026-06-19 AKDT

## Goal

This pass fixes the user-reported operational issues observed after the
launch-language update:

- Launch-language world text appeared backwards from the selectable side.
- Active play could make the player character look prone or airborne while
  walking.
- Narration used generated cooked cues that were not intelligible enough for
  play.
- Health, navigation, weapons, ammo, and item cycling were not obvious enough
  in the HUD.
- The generated architecture read as ambiguous clutter rather than functional
  spaces.

## Implementation

1. Launch-language choices now use a dedicated screen-space
   `bLaunchLanguageOnly` menu mode, so the first actionable controls are the
   five language buttons and selecting any one of them immediately starts play.
2. `SpawnLaunchLanguageSelectionScene` no longer uses 3D readable text for the
   chooser. The launch world is now a nonverbal colored backdrop, avoiding the
   mirrored/back-side text failure mode entirely.
3. Player movement posture is stabilized by keeping
   `bUseControllerRotationPitch = false`; camera pitch still works, but the
   character capsule no longer pitches into a prone/flying posture.
4. Fresh active play now starts in third-person (`CameraPerspective = 1`) so the
   player can immediately see that the avatar is upright.
5. Radio narration now defaults to clear macOS `Samantha` speech at rate `165`
   and keeps subtitles visible for 12 seconds.
6. Generated cooked radio cues remain available, but only through
   `bPreferCookedRadioBriefingCues` or `-UseCookedRadioVoice` after human audio
   QA confirms intelligibility.
7. HUD creation now uses the owning player controller and a reliable viewport
   z-order.
8. The HUD now includes a persistent `NAVIGATION` panel with objective phase,
   distance, direction, and the `T` route-skip reminder.
9. The HUD now includes a persistent `WEAPON SLOT` strip with active weapon,
   magazine/reserve ammo, weapon cycling controls, active throwable count, and
   medkit count.
10. The health label and health bar remain always visible with numeric health,
   percentage, and color states.
11. A new `SpawnPurposeClarityLayer` creates purpose-coded pads, pylons, and
    labels for Entry, Armory, Protected Coding Safehouse, Survivor, Extraction,
    and Optional Boss Risk.
12. The architecture clarity pass tags new markers as
    `PurposeCodedArchitecture` and logs `[CodeRescueArchitectureClarity]`.
13. Route-adjacent decorative static actors are made nonblocking and tagged
    `CriticalPathNonBlockingArchitecture`, while arena lock walls remain
    untouched.

## Verification

Added static verifier:

`python3 Scripts/verify_june19_playability_readability_fix_pass.py`

The verifier checks:

- The launch screen uses the dedicated language-only UI and no longer places
  readable chooser text in the 3D scene.
- Player capsule pitch remains disabled and new play starts in third-person.
- Clear narration is the default, with cooked/generated narration opt-in.
- HUD navigation and weapon/item strips are present.
- Purpose-coded architecture and nonblocking critical-path cleanup are present.
- This report and full-QA registration exist.

This verifier is now wired into `Run_Full_QA_Audit.command`.

## Latest Packaged-App Evidence

Final validation for this pass was completed on 2026-06-19 AKDT against the
rebuilt packaged app:

`PackagedMac/Mac/CodeRescueUnreal.app`

Evidence recorded:

- `./Recompile_Module.command` completed successfully after the source changes.
- `./Run_Full_QA_Audit.command` completed successfully; full QA runtime log:
  `Saved/Logs/HeadlessFullQASmoke.log`, timestamp
  `2026-06-19 20:11:10 AKDT`.
- `./Package_Mac_App.command` rebuilt the packaged app; package timestamp:
  `2026-06-19 20:13:19 AKDT`.
- `./Run_Launch_Menu_Visual_Check.command` completed successfully; capture:
  `Saved/Screenshots/LaunchMenu/launch_menu_20260619_201326.png`, log:
  `Saved/Logs/LaunchMenuVisual.log`, timestamp
  `2026-06-19 20:13:35 AKDT`.
- The launch capture passed `Scripts/verify_png_not_black.py` with mean
  luminance `111.92`, max luminance `228.00`, and visible ratio `0.7388`.
- The launch log contains
  `[CodeRescueLaunchLanguageMenu] Launch-only language widget ready: Java, C,
  C++, Python, MATLAB.`
- `./Smoke_Test_Packaged_App.command null` completed successfully; log:
  `Saved/Logs/PackagedSmoke_null.log`, timestamp
  `2026-06-19 20:13:46 AKDT`.
- `./Smoke_Test_Packaged_App.command render` completed successfully; log:
  `Saved/Logs/PackagedSmoke_render.log`, timestamp
  `2026-06-19 20:13:53 AKDT`.
- Final source formatting check passed with `git diff --check`.

Note: Unreal's high-resolution screenshot path can omit UMG overlay layers. The
visual check therefore records both the nonblack launch-world capture and the
runtime log marker proving the screen-space launch-language widget was created.

## Manual Review Boundary

Automated checks can confirm source wiring, HUD surface creation, and runtime
log contracts. Final confidence for text readability, character posture feel,
narration intelligibility, and architecture comprehension still requires a
human playthrough of the packaged app.
