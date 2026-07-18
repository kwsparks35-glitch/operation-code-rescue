# First-Level V5 Access, Target Lock, World, and Physics Pass

Date: 2026-07-09

Status: implemented, compiled, clean-cooked, packaged, and exercised in the
final Mac application.

This record supersedes the V4 pass wherever target-lock behavior, first-level
building placement, sky behavior, hit locality, or zombie physics differ.

## Scope

The authored V5 buildings and placement corrections are restricted to campaign
city index `0`, New York. Target-lock validation, physical weapon locality,
anatomical wounds, and robust zombie death handling are shared gameplay-system
corrections. The ordinary language-selection start screen and its per-language
save histories remain the required entry path; audit launches bypass it only
when `-VisualReviewStart` is supplied explicitly.

## 1. Two-Arm Aim and Auto Target Lock

- The player scans only living, collidable zombies within 4,200 Unreal units.
- Acquisition requires a 24-degree camera cone and a clear physical weapon
  path. A wider 36-degree break cone avoids unstable lock flicker.
- While a target remains valid, controller rotation and character facing blend
  toward its torso. The Manny presentation mesh preserves locomotion while
  both upper arms rotate toward the same point.
- The weapon remains attached to `hand_R`, so the animated arms, muzzle, HUD
  lock marker, and physical shot direction agree.
- A locked shot redirects the real weapon ray only while range, cone, target
  state, and line of sight still pass. It does not apply remote damage after a
  miss.
- The HUD reports `TARGET LOCKED` and identifies the trace as physical.

Primary implementation:
`Source/CodeRescueUnreal/CodeRescueCharacter.h`,
`Source/CodeRescueUnreal/CodeRescueCharacter.cpp`, and
`Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp`.

## 2. Accessible V5 Streetscape

Five Blender-authored assets were added:

| Asset | Runtime role |
| --- | --- |
| `AccessibleMarketV5` | Open market interior, readable entrance, ammo pickup |
| `AccessibleClinicV5` | Open clinic interior, practical lights, medkit pickup |
| `OpenStreetCafeV5` | Open cafe interior, street seating, battery pickup |
| `PointStarFieldV5` | Sparse point-star field with no camera-blocking shell |
| `MoonDetailedV5` | Detailed moon mesh used by the night cycle |

Each gameplay building has a literal 230 cm entrance opening. Interchange's
generated convex hull still enclosed those openings, so runtime placement now
disables collision on the visual mesh and creates explicit hidden floor, wall,
roof, and fixture boxes around the open route. The market uses 13 boxes, the
clinic 12, and the cafe 17. A player-sized capsule sweep passes from outside to
inside at all three doors, and ground traces on both sides remain within the
accepted level tolerance.

Roads, sidewalks, crosswalks, structures, and pickups now share the canonical
first-level ground plane. The former post-spawn transform rewrite is skipped
for New York, preventing the stacked-city effect. One V4 checkpoint that
overlapped the clinic entrance was removed, while the useful V4 field armory
and sandbag cover remain. Decorative skyline facades remain scenery; the three
V5 gameplay interiors and all required arena routes are traversable.

Primary implementation:
`Source/CodeRescueUnreal/CodeRescueGameMode.cpp` and
`Source/CodeRescueUnreal/CodeRescueGameModeSpawning.cpp`.

## 3. Continuous Sky Cycle

- GameMode ticking is enabled in the constructor, before `BeginPlay`, which
  fixes the prior static-sky failure.
- Solar cosine blending drives noon, sunset, midnight, and sunrise continuously.
- Sun intensity/color, moon key light, skylight, fog, stars, and the detailed
  moon respond to the same normalized time.
- Stars appear only at night, and the point-field design cannot enclose or
  obstruct the third-person camera.
- The moon follows the playable area at a stable offset rather than being lost
  behind the finite arena.

The packaged audit captured and checked all four representative phases.

## 4. Local Weapon Influence and Grounded Death

- Firearms resolve through the weapon trace channel, zombie object tracing,
  exact segment-to-capsule geometry, and world occlusion.
- The old post-miss proximity damage path was removed. A deliberate skyward
  miss during the acceptance run left a nearby target at exactly the same
  health.
- Piercing and area effects still require a real impact; area victims also
  require range and line of sight.
- Bullet impacts create localized decals and `WoundCavityV4` geometry at the
  anatomical hit. Bites create side-aware wounds on the player's injured body
  region.
- Valid skeletal rigs create physical bodies lazily on first impact. Invalid or
  unavailable body setups fall back immediately to the procedural pose path,
  avoiding repeated `Invalid Bodies` physical-animation calls.
- Death impulses are direction-based and clamped. A corpse remains visible for
  9.0 seconds, captures its settled pose, fades/sinks for 2.8 seconds, and is
  removed only after that interval.

Primary implementation:
`Source/CodeRescueUnreal/CodeZombieActor.h`,
`Source/CodeRescueUnreal/CodeZombieActor.cpp`, and character weapon handling.

## 5. Reproducible Art Pipeline

- Blender source: `Scripts/BlenderArt/build_first_level_world_v5.py`
- Blender review render: `Scripts/BlenderArt/render_first_level_world_v5_review.py`
- Unreal import: `Scripts/import_first_level_world_v5.py`
- Unreal mesh inspection: `Scripts/inspect_first_level_v5_meshes_unreal.py`
- Collision configuration: `Scripts/configure_first_level_v5_collision_unreal.py`
- Generated exchange assets: `RawArt/FirstLevelV5`
- Cooked Unreal assets: `Content/CodeRescueArt/FirstLevelV5`

Blender 5.1.2 was invoked directly from
`/Applications/Blender.app/Contents/MacOS/Blender`. No Blender MCP server was
exposed to this session, so the local deterministic scripts were used instead.

## Runtime Evidence

- `TestLogs/FirstLevelV5MeshAudit.log`: five meshes load, have render triangles,
  materials, LODs, and sane bounds.
- `TestLogs/FirstLevelWorldAccessAuditV5Final.log`: 33 ground surfaces, 1.00 cm
  Z spread, three enterable buildings, and 3/3 clear level doors.
- `TestLogs/FirstLevelSkyAuditV5Retest.log`: day, sunset, night, sunrise, stars,
  and moon all pass.
- `TestLogs/FirstLevelIntegratedAcceptanceAuditV5CleanPhysics.log`: clean
  editor integration pass without physical-animation body warnings.
- `TestLogs/PackagedFirstLevelIntegratedAcceptanceAuditV5Final.log`: the final
  packaged application passes access, sky, challenges, target lock, combat,
  corpse lifecycle, and armory in one run.

## Visual Evidence

- `Renders/first_level_v5_accessible_structures.png`
- `Renders/first_level_sky_day.png`
- `Renders/first_level_sky_sunset.png`
- `Renders/first_level_sky_night.png`
- `Renders/first_level_sky_sunrise.png`
- `Renders/first_level_combat_wound.png`
- `Renders/first_level_grounded_corpse.png`
- `Renders/first_level_corpse_fade.png`
- `Renders/first_level_armory_cycle_complete.png`
