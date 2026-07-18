# Production World, Camera, Character, Vehicle, and Weapon Pass

Date: 2026-07-09

## Request Addressed

The work addressed two connected problems:

1. The normal game presented development geometry and under-refined world art
   instead of a coherent city arrival with readable characters, weapons,
   vehicles, sidewalks, and completed structures.
2. Building walls and large environment meshes could sit between the character
   and third-person camera, making location and interaction unreadable.

The implementation preserves the existing educational gameplay, language-only
save profiles, mountable Jeep, combat systems, objective guidance, and all six
camera modes while changing the production presentation around them.

## Root Causes

### Development layers were enabled in production

The world generator accumulated prototype review galleries, geometric briefing
walls, large objective pads and pillars, cloud plates, background horde proxies,
and text labels. These were useful during development but visually competed with
the playable city and, in several cases, occupied the camera sight line.

### The arrival had no protected visual corridor

The closest main-street buildings and the primitive safehouse shell crowded the
spawn. The road was primarily longitudinal, so there was no open intersection
to establish orientation or reveal sidewalks, vehicles, and structures at once.

### Collision alone could not solve camera occlusion

The spring arm can shorten against collision, but several presentation meshes
were configured with no collision or had collision cleared for player access.
Those meshes remained visually opaque even though the camera probe could pass
through them.

### Packaged UI used two lifecycle authorities

The launch selector UObject was strongly retained by the GameMode, but the
focus-independent input path still consulted a temporary static weak pointer
cleared during packaged Slate reconstruction. A packaged hold test exposed the
split ownership before release.

## Production World Profile

`ACodeRescueGameMode::ShouldSpawnDevelopmentShowcaseLayers()` now makes the
production/development distinction explicit. Normal play uses the curated path;
`-CodeRescueDevelopmentShowcase` restores the review-only layers for developers.

`ApplyProductionPresentationCleanup()` removes world labels and hides only
known review actors, background horde display proxies, weather identity plates,
and bounded arrival blockers. It deliberately retains gameplay structures and
records the result through `[ProductionPresentation]`.

Additional production decisions include:

- geometric objective pads and pillars are omitted; HUD distance, focus
  beacons, and the guidance drone continue to provide route information;
- giant geometric overcast plates are omitted; atmosphere, fog, sun, skylight,
  and post processing retain the weather and mood;
- random systemic skyline blocks are disabled in the production profile while
  authored CityKitV3 buildings remain active;
- primitive briefing floors, walls, halos, and labels are omitted while the
  support characters and their gameplay roles remain;
- the background horde display proxies are hidden without removing the live
  encounter systems;
- the prototype entry clearance marker is omitted in production.

## Streets, Sidewalks, Structures, and Vehicles

The streetscape now uses `RoadIntersectionV3` at the arrival and adds a
perpendicular cross street. Connected sidewalk segments, crosswalk markings,
streetlights, traffic signals, signs, power infrastructure, trees, hydrants,
barriers, refuse clusters, and vehicle cover establish a readable city block.

The CityKitV3 layer leaves the three closest main-street building slots open,
then adds six buildings beyond the cross-street sidewalks. This creates a clear
center view while keeping a continuous architectural edge around the player.
The final packaged ledger is 54/0 for streetscape spawns and 38/0 for CityKitV3
spawns. The first-level V4 storefront now replaces one former V3 facade slot,
rather than overlapping it, while the combined architectural edge remains
continuous.

The primitive safehouse box was replaced in production by an authored
`SidewalkV3` approach and `BusStopV3` glass pavilion. It was also moved farther
from the spawn so the player begins in open street space rather than against a
facade.

Authored delivery van, pickup, police cruiser, sedan, and damaged sedan meshes
are visible as believable traffic and combat cover. The pre-existing mountable
`AJeepActor` remains active in every city, with WASD driving, E dismount,
surface-aware traction, speed, and turning behavior; its physics contract was
re-verified during this pass.

## Camera Containment and Occlusion

All non-first-person modes keep `CameraBoom->bDoCollisionTest` enabled. The
existing camera modes remain:

- first person
- third person, 320 uu
- tactical third person, 650 uu
- top down, 1150 uu inside the street canyon
- isometric, 1250 uu inside the street canyon
- side-view 2.5D, 920 uu

`ACodeRescueCharacter::UpdateCameraOcclusion()` runs every 0.08 seconds in
third-person modes. It restores actors hidden by the previous camera sample,
builds a line from the player's upper body to the active third-person camera,
tests visible architectural-scale static-mesh bounds, and temporarily hides
only meshes whose bounds intersect that line. The pass handles no-collision
architecture and corridor-cleared meshes while respecting
`CameraOcclusionExempt`, `GameplayArenaConfinement`, and permanently curated
`ProductionPresentationHidden` actors.

Spawn framing probes four cardinal directions over `ECC_Camera` and faces the
player toward the direction with the greatest 800 uu rear-camera clearance.
The layout, spring-arm probe, bounds occlusion, and spawn framing therefore
cover different failure modes rather than relying on one brittle workaround.

## Character and Weapon Presentation

Normal play now defaults to complete production rigs:

- player: Manny mesh with Manny animation blueprint;
- survivor: Quinn mesh with Quinn animation blueprint;
- friendly support roles: Manny/Quinn production meshes and animation
  blueprints.

The prior prototype character choices remain accessible only with
`-CodeRescueUsePrototypeCharacters`. Support-team offsets were widened so five
companions read as a group instead of overlapping one another.

The third-person held weapon remains attached to `hand_R`, and the first-person
and third-person visibility contracts remain intact. Five distinct WeaponsV3
assets are available: pistol, SMG, rifle, shotgun, and crossbow.

## Blender Art Refinement

`Scripts/BlenderArt/build_world_art_v3.py` now applies production bevels and
adds small-scale cues that survive gameplay distance:

- road patches, manholes, repair seams, and cracks;
- sidewalk drains and tactile curb cues;
- tapered vehicle cabins and external glass;
- vehicle pillars, panel seams, door handles, rocker panels, and wheel hubs;
- more readable material separation on street and vehicle parts.

`Scripts/BlenderArt/build_weapons_v3.py` now adds production bevels and weapon
identity details, including the pistol ejection port and grip ribs, shotgun
hardware, and rifle optic. All 19 CityKitV3 GLBs and all five WeaponsV3 GLBs
were regenerated, then reimported in place so runtime references did not
change. `Scripts/import_production_art_v3.py` makes that reimport reproducible.

`Scripts/BlenderArt/render_production_art_review.py` generates the two review
sheets in `Renders/` without requiring the Unreal editor.

## Launch Selector Reliability

The launch selector is stored as a transient UPROPERTY on
`ACodeRescueGameMode`. `GetLaunchLanguageMenu()` is now the authoritative input
resolver, with the widget's static weak reference retained only as a fallback.
`NativeConstruct()` re-registers the fallback after Slate reconstruction.

The selector uses Game-and-UI input, a visible cursor, and explicit movement and
look locks. The language gate runs before camera or weapon polling, and a
missing selector never silently commits a language. Existing per-language New
and Resume paths, save summaries, and future start-screen presentation remain
unchanged.

## Code and Pipeline Map

- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`: production world profile,
  streets, cross street, buildings, cleanup, safehouse, route, sky, capture
- `Source/CodeRescueUnreal/CodeRescueGameMode.h`: production helpers and launch
  selector ownership
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp/.h`: production player rig,
  launch-gate resolver, camera occlusion, input ordering
- `Source/CodeRescueUnreal/SurvivorActor.cpp`: production survivor rig
- `Source/CodeRescueUnreal/FriendlyNPCActor.cpp`: production support rigs
- `Source/CodeRescueUnreal/CodeRescueMainMenuWidget.cpp`: Slate lifecycle
  fallback registration
- `Scripts/BlenderArt/build_world_art_v3.py`: reproducible CityKitV3 source
- `Scripts/BlenderArt/build_weapons_v3.py`: reproducible WeaponsV3 source
- `Scripts/import_production_art_v3.py`: in-place Unreal reimport
- `Scripts/verify_production_presentation_camera_pass_2026_07_09.py`: focused
  source, asset, render, and launch contract verifier

## Review Images

- `Renders/packaged_production_arrival.png`: final packaged 1280x720 scene
- `Renders/production_city_assets.png`: city asset review sheet
- `Renders/production_weapon_assets.png`: weapon asset review sheet
