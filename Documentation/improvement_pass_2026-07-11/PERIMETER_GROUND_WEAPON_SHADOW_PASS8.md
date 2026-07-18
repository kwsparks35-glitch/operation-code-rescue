# Pass 8: Full-Perimeter Ground and Held-Weapon Shadow Repair

Date: 2026-07-17

## Scope

This pass addresses the two defects shown in the eight player-provided
screenshots:

1. The complete right/east edge of the city, plus matching perimeter strips,
   dropped far below the interior play surface.
2. Third-person weapon art became building-sized above the player and cast a
   street-wide shadow instead of remaining visible in the character's hand.

The fixes apply to every campaign city because both behaviors came from shared
city-spawn and player-weapon code. The first level received the requested
focused visual and integrated verification, and city 2 was streamed as a
cross-city regression test.

## Root Causes

### Perimeter ground

`SpawnCampaignCity` created a canonical mission floor with local cube scale
`82 x 70`. After the campaign's 2x span scale, that floor ended at world
half-extents `8200 x 7000`, while the arena walls stand at `10800 x 9400`.
This left uncovered strips 2600 uu wide on east/west and 2400 uu wide on
north/south inside the playable walls.

The only support under those strips was the intentional fall-recovery floor,
whose top is `Z=-611`; the canonical floor top is `Z=0`. Existing ground
unification correctly preserved that 6.11 m safety floor, so it could not fix a
missing canonical slab. This exactly matched the hard floor edge and deep
lower district visible in `pass8_before_right_edge_1.png` through `_6.png`.

### Weapon scale and shadow

The visible hero skeleton is imported with a 100x normalization scale. The held
weapon used `SnapToTargetNotIncludingScale`, but immediately replaced the
attachment compensation with a fixed relative scale of `1.35` and a nonzero
bone-relative offset. The weapon therefore inherited the imported hand-bone
scale, grew to architectural dimensions, moved above the player, and then cast
a full dynamic shadow because `SetCastShadow(true)` was enabled.

## Implementation

### Continuous campaign floor

Files:

- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.h`

The mission floor now derives its dimensions from
`ArenaWallHalfXLocal/ArenaWallHalfYLocal` instead of unrelated magic numbers.
Its world half-extents are now `10950 x 9550`, giving 150 uu of support beneath
each wall line while remaining inside the `11200 x 9800` recovery floor.

The canonical slab is created before streets, buildings, and grounded props.
Consequently, all later `GroundZAt` placement along the perimeter now resolves
to the canonical top rather than the lower catch floor. The recovery floor
remains beneath the city as a safety net but is never accepted as playable
perimeter support.

New tags record the contract:

- `FullPerimeterMissionGround`
- `RightEdgeGroundContinuity`
- `FirstLevelIntegratedPerimeterGroundPass`

`AuditCampaignPerimeterGround` now performs 36 direct collision traces against
the canonical slab: nine on each arena edge. It separately requires all nine
east/right probes, verifies a walkable upward normal, checks top-height error,
requires collision, verifies overlap beneath the wall lines, and explicitly
rejects the recovery floor. The result is part of both the first-level world
acceptance gate and the cross-city ground-recovery audit.

### Scale-safe held weapons

File: `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`

The third-person weapon component now:

- uses absolute component scale, independent of imported body/bone scale;
- attaches at zero translation to the right-hand grip origin;
- falls back to the unscaled capsule when a compatible hand bone is absent;
- normalizes each mesh to a real-world target length by weapon family;
- has cosmetic shadow casting disabled in the constructor and on refresh.

Target lengths range from 14 cm grenades and 28 cm handguns through 92-96 cm
rifles/shotguns and a 112 cm rocket launcher. This preserves meaningful weapon
silhouette differences without depending on inconsistent source-mesh units.

`HeldWeaponPresentationAudit` now blocks the combat/integrated pass unless the
weapon asset exists, is attached, uses absolute scale, is within its expected
world size and hand offset, and reports `casts_shadow=0`.

### Review harness and verifier maintenance

The existing `-CodeRescuePerspectiveReview` harness now finishes with three
right-edge frames at south, center, and north positions. It keeps the rifle
equipped so each frame verifies ground continuity and held-weapon presentation
together. `Scripts/verify_runtime_log_contracts.py` was corrected to require
the actual `Backspace recovery guidance` marker; F8 was removed from the input
map in an earlier control-cleanup pass.

## Verification

### Build

Unreal 5.7 editor and game targets compiled successfully. The final cook
reported `Success - 0 error(s), 0 warning(s)`, and BuildCookRun completed with
`ExitCode=0`.

### Focused ground tests

First level:

```text
[CampaignPerimeterGroundAudit] COMPLETE PASS city=0 samples=36/36 east_right=9/9 floor_covers_walls=1 collision=1 max_top_delta=0.00 floor_extent=(10950,9550) wall_half=(10800,9400) catch_floor_accepted=0
```

Streamed city 2:

```text
[CampaignPerimeterGroundAudit] COMPLETE PASS city=1 samples=36/36 east_right=9/9 floor_covers_walls=1 collision=1 max_top_delta=0.00 floor_extent=(10950,9550) wall_half=(10800,9400) catch_floor_accepted=0
[CampaignGroundRecoveryAudit] COMPLETE PASS city=1 recovered_relative_z=92.00 height=1 bounds=1 ground=1 perimeter_ground=1 population=1
```

The south, center, and north right-edge screenshots show a continuous level
surface with no exposed catch floor or impassable drop.

### Focused weapon tests

The rifle was visually reviewed in third-person, tactical, top-down,
isometric, and side perspectives. It remains hand-sized at the right hand, and
the screenshots contain no overhead weapon geometry or oversized weapon
shadow. The runtime audit independently reported:

```text
[HeldWeaponPresentationAudit] COMPLETE PASS asset=PistolV4 target_length_cm=28.0 world_longest_cm=30.7 center_offset_cm=8.7 attached=1 absolute_scale=1 casts_shadow=0
```

The rifle normalization record is:

```text
[HeldWeapon] body weapon 'RifleV4' attached via hand_R target_length_cm=92.0 local_longest=110.90 absolute_scale=0.82958 casts_shadow=0
```

### Single integrated first-level run

The final editor run and the independently executed packaged-binary run both
reported:

```text
[FirstLevelAccessAudit] COMPLETE PASS ... perimeter_ground=1 ...
[FirstLevelCombatAudit] COMPLETE PASS ... animation=1 held_weapon=1
[FirstLevelIntegratedAudit] COMPLETE PASS world=1 access=1 ground=1 population=1 characters_grounded=1 sky=1 day_period=1 challenges=1 alternate_solution=1 guidance=1 progression=1 supplies=1 target_lock=1 combat=1 corpse=1 animation=1 reader=1 armory=1 pause_mouse=1 overlay_passthrough=1 crafting=1
```

Both logs pass `scan_audit_warnings.py`. The packaged integrated log and normal
Metal/CoreAudio launch smoke also pass `verify_runtime_log_contracts.py`.

## Package

- App: `PackagedMac/Mac/CodeRescueUnreal.app`
- Bundle ID: `com.operationcoderescue.CodeRescueUnreal`
- Bundle version: `51494982.0.209`
- Size: 2055.0 MB
- Required pak/ucas/utoc payload: present
- `codesign --verify --deep --strict`: pass
- Local package integrity: ready
- Normal packaged render launch: pass
- Packaged full integrated acceptance: pass
- Developer ID notarization for external Gatekeeper distribution: pending
  external Apple credentials; not a local gameplay or package-integrity defect

## Evidence Index

Logs:

- `TestLogs/EditorIntegrated_PerimeterWeapon_Pass8.log`
- `TestLogs/EditorCampaignGroundRecovery_Perimeter_Pass8.log`
- `TestLogs/PerspectiveReview_PerimeterWeapon_Pass8.log`
- `TestLogs/PackagingBuild_PerimeterWeapon_Pass8.log`
- `TestLogs/PackagedIntegrated_PerimeterWeapon_Pass8.log`
- `TestLogs/PackagedSmoke_PerimeterWeapon_Pass8.log`

Visual evidence:

- `Screenshots/pass8_before_right_edge_1.png` through `_6.png`
- `Screenshots/pass8_before_weapon_shadow_1.png` and `_2.png`
- `Screenshots/pass8_after_right_perimeter_south.png`
- `Screenshots/pass8_after_right_perimeter_center.png`
- `Screenshots/pass8_after_right_perimeter_north.png`
- `Screenshots/pass8_after_rifle_third_person.png`
- `Screenshots/pass8_after_rifle_tactical.png`
- `Screenshots/pass8_after_rifle_side.png`

Release evidence:

- `Release/package_integrity_perimeter_weapon_pass8.json`
