# World, Loot, Weather, and Grounding Pass 9

Date: 2026-07-17

Final application: `PackagedMac/Mac/CodeRescueUnreal.app`

## Requested Scope

The six supplied screenshots documented four connected problems:

1. Living characters were visibly suspended above the floor.
2. zombie and challenge rewards were undifferentiated glowing cubes.
3. open regions and black/green enclosure blocks lacked a clear purpose.
4. the world needed stronger character art, environmental identity, and
   visible weather with physical consequences.

The source evidence is retained in `Screenshots/Floating_Bodies.png`,
`Item_Clarity.png`, `Region_Unclear.png`, `Structure_Clarity.png`,
`Zombie_Enclosures.png`, and `Zombie_Enclosures_1.png`.

## Grounding and Character Presentation

The original placement logic grounded character actor pivots/capsules, but it
did not consistently reconcile the visible skeletal-mesh foot bounds after an
animation actor refreshed its cached pose. This produced actors whose physics
capsules were valid while their rendered bodies appeared suspended.

The shared `AlignCharacterVisualFeetToCapsule` path now:

- traces against the canonical floor while ignoring characters and catch
  floors;
- aligns the capsule/actor and then the actual visible skeletal foot bounds;
- runs immediately and again after `0.45 s` and `1.65 s` settling windows;
- recaches zombie and companion animation bases after correction so procedural
  animation cannot restore the stale height;
- leaves corpse/ragdoll motion to Chaos while the integrated corpse lifecycle
  audit verifies retained bodies, fade, and removal.

Editor and packaged results are identical:

```text
[CharacterGroundingAudit] COMPLETE PASS ... characters=134 ... floating_after=0 visible_feet=129/129
```

Companions also receive the Blender-authored `ResponderPackV6`, adding a radio,
harness, pouches, and a visible role signal without changing their collision.

## Symbol-First Loot Packaging

`Scripts/BlenderArt/build_world_loot_weather_v6.py` deterministically authors
six grounded package families:

| Family | Physical mark | Gameplay kinds |
| --- | --- | --- |
| Ammo | three cartridges | ammo, ammo pouch |
| Medical | cross | medkit, stim |
| Armor | shield | armor plate |
| Tech | scanner arc and bolt | scanner, bypass kit |
| Salvage | gear | scrap |
| Utility | lightning bolt | flare, smoke, battery |

Each symbol is real geometry on both long faces, uses restrained emissive
contrast, and requires no paragraph text. Normal field drops turn at `18 deg/s`
around a stable ground contact instead of bobbing; approach direction therefore
cannot permanently hide the mark. `APickupActor::RefreshPresentation` resolves
the package after callers assign `Kind`, while `SnapToGround` derives clearance
from imported mesh bounds rather than a hard-coded cube assumption.

The same pickup actor remains the reward path for zombie deaths and coding
challenges. The logistics depot adds six usable icon-stock cases so the region
also functions as a resupply destination. Runtime acceptance reports:

```text
[PickupPresentationAudit] COMPLETE PASS pickups=26 authored=26 grounded=26 icon_styles=6/6 paragraph_labels=0
```

## Purposeful World Regions

Three authored first-level destinations now occupy validated open ground near
the east/perimeter routes:

- `FieldLogisticsDepotV6`: covered supply rack, cases, physical supply emblem,
  practical light, six usable package types, and a district reward.
- `WeatherRelayV6`: cabinet, screen, mast, anemometer, sensor posts, practical
  light, and a technology reward.
- `QuarantineCheckpointV6`: open drive-through gate, concrete barriers,
  warning symbol, practical light, and a medical reward.

Placement checks the final assembled city rather than an early empty layout,
rejects overlaps, and requires all three landmarks to remain within arena
bounds. UE 5.7 on Mac reported valid render sections for the imported depot
shell but omitted that shell from the main pass. The Blender mesh remains the
spatial/siting contract; a 21-piece visible UE realization reproduces its
foundation, posts, canopy, rack, shelves, cases, and physical emblem, and the
acceptance gate fails if any module is absent.

```text
[PurposeDistrictRuntimeAudit] COMPLETE PASS landmarks=3/3 district_types=3 open_space=3/3 bounded=3/3 depot_modules=21/21 depot_icon_stock=6/6
```

The prior large enclosure blocks were replaced by `ThreatGroundRingV6`. Rings
attach to each zombie at capsule-base height, do not collide, do not cast
shadows, and move with their owner:

```text
[ThreatMarkerAudit] COMPLETE PASS markers=124 compact=124 attached=124 enclosure_cubes=0 collision=0
```

## Wind, Rain, and Fog

`ACodeRescueWeatherFieldActor` follows the player and cycles three 120-second
phases. The first city begins with readable rain; later cities rotate their
starting phase.

- Rain: 112 animated Blender streaks, driven rain direction, `0.88` traction
  and braking scale, reduced AI visibility, and rain-weighted fog.
- Wind: 24 animated Blender debris pieces, gusting velocity, elevated
  `cr.WindStrength`, and existing foliage/wind-sway response.
- Fog: denser height fog, restrained opacity, longer visual falloff, and a
  stronger AI sight-distance reduction.

The glTF import pipeline now sets
`used_with_instanced_static_meshes=True` on rain and wind base materials and
persists the packages. This removes the default-material substitution and
per-launch shader recompile warning found by the release log verifier.

```text
[WeatherPhysicsAudit] COMPLETE PASS wind=1 rain=112/112 fog=1 traction=1 ai_visibility=1 phase_cycle=1 authored_assets=1
```

## Blender and Unreal Asset Inventory

Raw sources live in `RawArt/WorldLootWeatherV6/`; cooked source packages live
under `Content/CodeRescueArt/WorldLootWeatherV6/`.

The 13 production assets are:

```text
PickupAmmoV6            PickupMedicalV6       PickupArmorV6
PickupUtilityV6         PickupTechV6          PickupSalvageV6
ThreatGroundRingV6      ResponderPackV6       RainStreakV6
WindDebrisV6            FieldLogisticsDepotV6 WeatherRelayV6
QuarantineCheckpointV6
```

Automation:

- `Scripts/BlenderArt/build_world_loot_weather_v6.py`: deterministic Blender
  generation, grounded origins, dimensions, triangle budgets, and GLB export.
- `Scripts/import_world_loot_weather_v6.py`: idempotent Unreal import, material
  usage repair, package save, and unattended exit option.
- `Scripts/inspect_world_loot_weather_v6_unreal.py`: triangle, LOD, section,
  material, bounds, ground-origin, and instancing verification.
- `Scripts/verify_world_loot_weather_grounding_pass_2026_07_17.py`: 15-part
  static implementation/asset contract.

UE 5.7 occasionally opens CrashReportClient after the editor-only Python
import/inspection script requests shutdown. The accepted import log reaches
all 13 saved objects and both material-usage saves before that shutdown path;
the subsequent independent mesh audit, runtime tests, clean cook, and packaged
tests all pass. This is an editor automation exit quirk, not a game-runtime
crash, and is retained here so future reviewers do not mistake the tool exit
code for an asset-import failure.

The mesh audit passed all 13 assets. The six package meshes have grounded
`min_z=0`, five or six material sections, and visible dual-face symbol geometry.

## Acceptance Matrix

| Gate | Result | Evidence |
| --- | --- | --- |
| Blender generation | PASS, 13 assets | generator console result |
| Static implementation/asset contract | PASS, 15/15 | `TestLogs/WorldLootWeatherGroundingStaticAudit_2026_07_17.log` |
| UE editor compile | PASS | UBT `CodeRescueUnrealEditor Mac Development` |
| Unreal mesh inspection | PASS, 13/13 | `TestLogs/WorldLootWeatherV6MeshAuditInstancedAcceptance_2026_07_17.log` |
| Focused world/access run | PASS | `TestLogs/WorldLootWeatherGroundingFocusedAcceptance_2026_07_17.log` |
| Editor visual review | PASS, 5/5 | `TestLogs/WorldLootWeatherVisualReviewAcceptance_2026_07_17.log` |
| Editor integrated run | PASS, all 29 tokens | `TestLogs/FirstLevelIntegratedWorldLootWeatherCleanAcceptance_2026_07_17.log` |
| Build/cook/stage/archive | PASS, ExitCode 0 | `TestLogs/PackageBuildCookRun_2026_07_17.log` |
| Packaged integrated run | PASS, all 29 tokens | `TestLogs/PackagedFirstLevelIntegratedWorldLootWeatherAcceptance_2026_07_17.log` |
| Packaged visual review | PASS, 5/5 | `TestLogs/PackagedWorldLootWeatherVisualReviewAcceptance_2026_07_17.log` |
| Normal packaged launch | PASS, six-language gate before world | `TestLogs/PackagedNormalLaunchLanguageGateAcceptance_2026_07_17.log` |
| Runtime warning contract | PASS | `TestLogs/PackagedRuntimeContractVerification_2026_07_17.log`, `TestLogs/PackagedWarningScan_2026_07_17.log` |
| Local package/signature | PASS | `Release/package_integrity_world_loot_weather_pass9.json` |

The final editor and packaged runs both report:

```text
[FirstLevelIntegratedAudit] COMPLETE PASS world=1 access=1 ground=1 population=1 characters_grounded=1 visible_feet=1 sky=1 day_period=1 weather=1 challenges=1 alternate_solution=1 guidance=1 progression=1 supplies=1 loot_symbols=1 districts=1 threat_markers=1 target_lock=1 combat=1 corpse=1 animation=1 reader=1 armory=1 pause_mouse=1 overlay_passthrough=1 crafting=1
```

The full run validates the requested work together with all ten first-level
coding stations across Java, C, C+, C++, Python, and MATLAB, plus saving,
combat, target lock, corpse behavior, pause mouse controls, crafting, and the
code-accepted overlay. The warning scanner accepts only the repository's
documented Development diagnostics; there are no failed contracts, material
warnings, errors, assertions, ensures, or fatal errors.

## Visual Evidence

`Screenshots/packaged_world_loot_weather_contact_sheet.png` is the final cooked
five-view contact sheet. Individual 1280x720 packaged captures are:

- `v6_symbol_loot_rain.png`
- `v6_logistics_depot_rain.png`
- `v6_weather_relay_fog.png`
- `v6_quarantine_checkpoint_wind.png`
- `v6_grounded_horde_rain.png`

They were generated by the archived executable, written through the app
sandbox into its normal Saved directory, copied into this record, and visually
inspected after the run.

## Package Handoff

- Bundle version: `51494982.0.210`
- Bundle ID: `com.operationcoderescue.CodeRescueUnreal`
- Minimum macOS: `14.00`
- Architecture: arm64
- Size: `2059.6 MB`
- Required executable, pak, ucas, utoc, and global containers: present
- `codesign --verify --deep --strict`: valid on disk and satisfies its
  designated requirement
- Distribution status: local development archive ready; Developer ID signing
  and Apple notarization remain an external credential step

## Reproduction Commands

```text
/Applications/Blender.app/Contents/MacOS/Blender --background --python Scripts/BlenderArt/build_world_loot_weather_v6.py
python3 Scripts/verify_world_loot_weather_grounding_pass_2026_07_17.py
python3 Scripts/verify_first_level_integrated_v5_pass_2026_07_09.py
./Package_Mac_App.command
python3 Scripts/verify_runtime_log_contracts.py Saved/Logs/PackagedFirstLevelIntegratedWorldLootWeatherAcceptance_2026_07_17.log
python3 Scripts/scan_audit_warnings.py Saved/Logs/PackagedFirstLevelIntegratedWorldLootWeatherAcceptance_2026_07_17.log
python3 Scripts/verify_launch_language_start_screen_save_pass.py
python3 Scripts/verify_package_integrity_pass.py --expected-bundle-id com.operationcoderescue.CodeRescueUnreal
```

Normal player launch must omit all audit flags. It continues to present the
six-language selector and per-language save/resume choices before active play.
