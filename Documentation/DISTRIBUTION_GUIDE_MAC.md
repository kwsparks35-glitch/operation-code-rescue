# Operation Code Rescue - Mac Distribution Guide

## Local Build

Use this for testing on the development Mac:

```bash
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command null
```

Use `./Smoke_Test_Packaged_App.command render` for a visual sign-off pass on
the packaged app.

The app will be at:

```text
PackagedMac/Mac/CodeRescueUnreal.app
```

## Public-Release Safety Gate

As of the June 18, 2026 hardening pass, external compiler/interpreter
validation is disabled by default:

```text
CodeRescue.AllowExternalCodeValidation=0
```

With the default setting, terminal challenges use the in-engine validator and
do not spawn local `javac`, `clang`, `python3`, or MATLAB processes. Enable
external OS toolchains only for trusted local development or classroom lab
machines:

```text
CodeRescue.AllowExternalCodeValidation=1
-AllowExternalCodeValidation
```

The terminal dependency banner reports the disabled state, and MATLAB desktop
launch is blocked while the gate is off.

## Maple Narration Status

The Maple sinister female-narration pipeline is present and the June 18
demo-readiness pass fixed the XTTS/Transformers compatibility issue in the
project shim. The full female-voice batch is now generated, imported, wired,
verified, and cooked into the current Mac package. Coverage is counted live by
`Scripts/generate_release_manifest.py` from generated `*_radio_briefing.wav`
files; the current manifest reports `230/230` female-voiced missions. Female-
voiced cities use generated/imported cues, with a native slug-based runtime
load fallback in `CodeRescueGameMode.cpp` and the radio sample directory listed
in `DefaultGame.ini` as always-cooked content. See
`Documentation/MAPLE_NARRATION_STATUS_2026-06-18.md` for the refresh runbook.

## Demo Readiness Tooling

The June 18 next-20 pass adds:

```bash
python3 Scripts/verify_save_compatibility_pass.py
python3 Scripts/verify_asset_budget_pass.py
python3 Scripts/verify_demo_readiness_pass.py
python3 Scripts/generate_visual_regression_manifest.py --min-count 1
python3 Scripts/generate_release_manifest.py
python3 Scripts/create_support_bundle.py
```

Use `Run_Local_CI_Readiness.command` for the full compile, full QA, package,
null smoke, render smoke, release-manifest, and support-bundle path.

For a focused non-human release preflight after a package already exists, run:

```bash
./Run_NonHuman_Release_Readiness.command
```

This refreshes the Maple audio technical audit, package-integrity/signing
preflight, release manifest, non-human readiness verifier, and support bundle.
The strict distribution check remains separate until final Apple credentials
and a final bundle ID are present:

```bash
python3 Scripts/verify_package_integrity_pass.py --strict-distribution
```

## Current Local Demo Package

Latest refreshed package from the June 18 next-20 demo-readiness implementation,
Maple narration generation/import/wiring, recommendation implementation,
public hardening, runtime-contract QA, GameMode spawning split, June 12/13 Next
100, squad status, survivability, U.S. city identity/signature-silhouette/
district micro-scene, city arena confinement/fall-recovery, full QA, and
packaged smoke-test pass:

```text
/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app
```

This build includes the June 18 next-20 demo-readiness layer: six difficulty
presets, persistent accessibility controls, release/support bundle tooling,
visual regression manifesting, save compatibility and asset-budget verifiers,
local CI readiness automation, first-ten-minutes onboarding/signoff manifests,
enemy readability/squad personality/curriculum feedback contracts, and the
complete Maple female-voice radio narration batch. It also includes the June 18
public-release hardening layer: material parent resolution that prevents nested
dynamic-material warning chains, runtime log contract verification for full QA
and packaged smoke tests, Maple narration status documentation,
external compiler/interpreter validation disabled by default, and a first
GameMode spawning-helper split. It also includes the May 28 creative
implementation layer, the June 1
survivability and team-support pass, the June 11 non-blocking squad/HUD polish,
the June 12 rescue-team regroup command, the June 12 Next 100 kickoff slice,
the squad command/status continuation, and the U.S. city landscape/architecture
identity and district micro-scene pass. It also includes the June 13 locked
city arena pass: a fall-recovery catch floor, four blocking perimeter walls,
corner rescue beacons, visible boundary lighting/skyline dressing,
Backspace/F8 recovery guidance, automatic below-floor/out-of-bounds recovery,
and a manual `Backspace` or `F8` recovery command. The player has a larger
health HUD
readout, directional attack alerts that report where the player was hit,
higher base health, stronger armor mitigation, a longer hit-mercy window,
per-hit lethal damage protection,
expanded medkit/armor capacity, emergency auto-medkit support, and
critical-health attack callouts. The five-person rescue support squad includes
medic, engineer, rifle-support, scout, and heavy-rescue roles. Squad members
ignore pawn/camera collision while preserving world collision, report
active/medic/support status on the HUD, can be regrouped behind the player with
`Y`, can cycle Tight/Standard/Wide formation spacing with `U`, can be called
for medic help with `N`, and can toggle hold/follow with `O`; the HUD advertises
the squad commands, briefly confirms regroup counts, and shows formation plus
auto-medkit readiness. The first city also receives a final access-point
clearance pass after all set dressing spawns so the entry, armory, safehouse,
language plaza, terminal, survivor area, and helipad remain passable. Weapon
quick slots, mouse-wheel cycling, bracket-key cycling, and gamepad shoulder
cycling remain enabled for the full arsenal.

The build also retains active asset-download intake bays, playable cast
promotion slots, MetaHuman/Control Rig/IK/DCC tags, protected curriculum
concept rooms, functional tactical gear pickups, armor plates, city
district-kit targets, AI director nodes, async physics props, visible
stress-test rigs, protected no-zombie coding safehouses, selected-language
terminal flow, Java/C/Python/MATLAB/C+/C++ curriculum tracks, C+/C++ clang++
validation, survivor-intel rewards after terminal solves, city street-grid
identity, U.S. city-specific landscape/architecture/sky/road/sidewalk/home/
vehicle/clothing cues, non-blocking signature silhouettes, combat-district
zombie separation, death-screen replay/save-and-quit options, `F1`-`F6` direct
camera selection, `C`/`V` camera cycling, `1`-`0` weapon quick slots, direct
enemy pursuit/facing, open interior route entry with a locked outer city arena,
compact survival-horror building proportions, immediate tactical arsenal,
visible armory staging, per-weapon reserve ammo, runtime code-validation
timeout protection, public-demo Fab/detail layer, non-blocking city district
micro-scenes for waterfront/transit/historic/civic/warehouse/venue/campus/
mountain/desert/suburban cues, and Unreal systems character/world hooks.

Package evidence:

```text
Size: 2.03 GB in release manifest (`du -sh`: 2.0G)
Timestamp: Jun 18 17:36 AKDT 2026
```

Included source changes in this package:

- The June 18 next-20 demo-readiness pass: six difficulty presets, persistent
  accessibility controls, Maple generation/import/wiring, native Maple cue load
  fallback, release and support manifests, save compatibility checks, visual
  regression manifesting, asset-budget verification, and local CI readiness
  automation.
- The June 18 recommendation implementation pass: shared material utility,
  runtime log contract verifier, `.gitattributes` LFS guidance, Maple narration
  status correction, default-off external validation gate, and GameMode
  spawning-helper split.
- The June 13 city arena confinement and fall-recovery pass: blocking catch
  floor, four blocking perimeter walls, corner rescue beacons, non-blocking
  boundary aesthetics, protected access cleanup, automatic arena recovery, and
  manual `Backspace`/`F8` recovery.
- The June 12 squad command/status continuation pass: compact squad health
  pips, `N` medic call, and `O` hold/follow behavior.
- The June 12 U.S. city landscape and architecture identity pass: streamed
  city-specific landscape, architecture, sky, road, sidewalk, home, vehicle,
  clothing, signature-silhouette, and district micro-scene cues for all 342
  U.S. campaign cities.

Validation run for the current package:

```bash
./Run_Full_QA_Audit.command
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
./Run_Local_CI_Readiness.command
```

The local CI readiness command completed successfully for the current package,
including static verifiers, release manifest generation, visual regression
manifest generation, module recompile, full QA, Mac packaging, packaged null
smoke, packaged render smoke, runtime log-contract scans, and support-bundle
creation. The release manifest reports Maple narration coverage at `230/230`
and package size `2.03 GB`. Runtime log contracts also passed for full QA and
both packaged smoke modes. Current logs confirmed the
`[CodeRescueArenaConfinement]`, `[CodeRescueUSCityIdentity]`,
`[CodeRescueSafeLearning]`,
`[CodeRescueUnrealSystems]`, `[CodeRescuePublicDemoQuality]`,
`[CodeRescueCreativeImplementation]`, and `[CodeRescueEntryAccess]` runtime
markers. The packaged smoke logs confirmed the rebuilt app spawns New York's
locked city arena with catch floor, four collision walls, corner beacons,
skyline edge, and Backspace/F8 recovery guidance. They also confirmed New
York's
`signature='harbor statue silhouette and dense island skyline'` field,
`districts='waterfront or beach approach | transit stop and rail/bus corridor | historic core and stoop row'`,
and the post-set-dressing access cleanup. The smoke scanner allowed only the
known unattended macOS CoreAudio sample-rate query warning in render mode plus
the known immediate-quit navigation dirty-area diagnostic and crowd-following
RecastNavMesh diagnostic. The checked logs did not contain `LogMaterial`,
`not a valid parent for MaterialInstanceDynamic`, or `MID_MID_` material-chain
warnings.

Latest generated evidence files:

```text
Saved/Release/release_manifest_latest.json
Saved/Release/package_integrity_latest.json
Saved/AudioAudit/maple_audio_audit_latest.json
Saved/VisualRegression/visual_regression_manifest_latest.json
Saved/SupportBundles/
```

Regressions found and remedied during this pass:

- No new blocking regressions were found in the latest full-QA/package pass.
- The previous dynamic-material parent warning source was fixed and is now
  covered by runtime log-contract checks.
- The full QA curriculum validator now opts into external validators only for
  trusted local QA so the public default remains safe.
- Earlier fixed regressions remain covered: `verify_next100_implementation.py`
  required generated language-track text to include the `cross-training`
  wording, and access cleanup had to freeze simulated Chaos cover props before
  removing blocker collision.

Use a human playtest for final visual sign-off before sharing the app outside
the development Mac.

## Sharing With Another Mac

For a lightweight share:

```bash
ditto -c -k --keepParent PackagedMac/Mac/CodeRescueUnreal.app CodeRescueUnreal_Mac_Development.zip
```

The receiving Mac may warn about an unidentified developer unless the app is signed and notarized.

## Bundle Identity

The current bundle identifier is:

```text
com.operationcoderescue.CodeRescueUnreal
```

This value is declared in `Config/DefaultEngine.ini` for both Unreal's Mac
package generation path and modern Xcode project generation. If ownership later
moves to a different Apple developer organization, update the bundle ID there
and rerun `Package_Mac_App.command`.

## Signing And Notarization

For local testing, the package can be signed ad-hoc to run locally. For external
distribution, use an Apple Developer certificate and notarization workflow. Keep
this separate from gameplay development so packaging failures are easier to
diagnose. As of the 2026-06-23 recommendation-closure pass, the remaining
external-distribution blocker is Developer ID signing/notarization credentials,
not placeholder bundle metadata.

## Package Integrity Checks

A valid packaged app must include:

```text
Contents/MacOS/CodeRescueUnreal
Contents/UE/CodeRescueUnreal/Content/Paks/CodeRescueUnreal-Mac.pak
Contents/UE/CodeRescueUnreal/Content/Paks/CodeRescueUnreal-Mac.ucas
Contents/UE/CodeRescueUnreal/Content/Paks/CodeRescueUnreal-Mac.utoc
Contents/UE/CodeRescueUnreal/Content/Paks/global.ucas
Contents/UE/CodeRescueUnreal/Content/Paks/global.utoc
```

`Package_Mac_App.command` now checks for `Contents/UE` and fails if the archived app is missing cooked staged data.
