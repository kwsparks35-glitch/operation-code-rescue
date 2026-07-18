# Production World and First-Level V5 Pass - Overview

Date: 2026-07-09

Status: implemented, compiled, packaged, and runtime-verified on macOS.

## Outcome

This pass converts the default New York arrival from a layered development
showcase into a curated production streetscape. The player now enters a clear,
connected intersection with sidewalks, crosswalks, authored buildings,
street furniture, visible vehicles, a bus shelter, production character rigs,
and a visible held weapon. Prototype walls, objective towers, background horde
proxies, giant geometric cloud plates, review labels, and other presentation
layers are excluded from normal play and remain available only through the
explicit `-CodeRescueDevelopmentShowcase` review flag.

The camera fix has two complementary parts: an always-on spring-arm collision
probe and a bounds-based third-person occlusion pass that temporarily hides
architecture intersecting the pawn-to-camera sight line, including meshes whose
collision is intentionally disabled. The arrival layout also leaves the center
building row open, adds a perpendicular cross street, and selects the spawn
facing with the most camera clearance.

The completed first-level work adds visible two-arm aiming with a persistent,
line-of-sight auto target lock, stamina-aware jump, a clickable live-3D `P`
pause armory, physical hit locality, localized bullet and bite wounds, grounded
death reactions, and readable corpse persistence/fading. The V5 world layer
adds three genuinely enterable Blender-authored structures, explicit
door-preserving collision, functional interior pickups, a continuous
day/sunset/night/sunrise cycle, point stars, and a detailed moon. New York now
uses one canonical ground plane so the roads, sidewalks, buildings, and arena
routes no longer stack vertically.

## Primary Deliverables

- Final app: `PackagedMac/Mac/CodeRescueUnreal.app`
- Package state: local integrity PASS, deep code-sign verification PASS
- Bundle version: `51494982.0.196`
- Package size: approximately 2.0 GB (`2051.4 MB` measured by the verifier)
- Final packaged capture: `Renders/packaged_production_arrival.png`
- Final armory capture: `Renders/packaged_first_level_armory_cycle_complete.png`
- City asset review: `Renders/production_city_assets.png`
- Weapon asset review: `Renders/production_weapon_assets.png`
- Detailed implementation record: `PRODUCTION_WORLD_CAMERA_ART_PASS.md`
- First-level implementation record: `FIRST_LEVEL_COMBAT_INTERACTION_PASS.md`
- First-level test record: `FIRST_LEVEL_TEST_AND_PACKAGE.md`
- V5 world/target-lock record: `FIRST_LEVEL_V5_ACCESS_TARGET_LOCK_WORLD_PASS.md`
- Exact challenge solutions: `FIRST_LEVEL_CHALLENGE_SOLUTIONS_V5.md`
- Final integrated/package record: `FIRST_LEVEL_V5_FINAL_TEST_AND_PACKAGE.md`
- Test and packaging record: `VERIFICATION_AND_PACKAGE.md`

## Runtime Proof

The final packaged run reported:

- `[Streetscape] 01 New York, NY: 54 spawned, 0 failed`
- `[CityBlockV3] 01 New York, NY: 36 spawned, 0 failed`
- `[FirstLevelCombatArtV4] ... spawned 2 authored structures/cover`
- `[FirstLevelAccessV5] ... buildings=3 open_doorways=3 functional_pickups=3`
- `[ProductionWorld] ... curated=1 development_showcases=0`
- `[ProductionPresentation] ... review actors=99 arrival blockers=0`
- production Manny player, Quinn survivor, mannequin support-team rigs
- `PistolV3` attached to `hand_R`
- `[FirstLevelAim] runtime pose copy configured for SKM_Manny`
- `[PauseArmoryAudit] COMPLETE PASS previews=17 final_index=16 equipped=1`
- `[FirstLevelAccessAudit] COMPLETE PASS ... clear_doors=3/3 level_doors=3/3`
- `[FirstLevelChallengeAudit] COMPLETE PASS languages=6/6`
- `[FirstLevelSkyAudit] COMPLETE PASS day=1 sunset=1 night=1 sunrise=1 stars=1 moon=1`
- `[FirstLevelCombatAudit] COMPLETE PASS jump=1 bite=1 miss_locality=1 target_lock=1 trace_hits=2 corpse=1 fade=1 removed=1`
- `[FirstLevelIntegratedAudit] COMPLETE PASS world=1 sky=1 challenges=1 target_lock=1 combat=1 corpse=1 armory=1`

The final package passed null-render and Metal-render launch smoke tests, the
17-entry armory cycle, all six first-level language submissions, four sky
phases, three doorway capsule sweeps, target lock, deliberate miss locality,
and the complete wound/death/corpse lifecycle in one run. The audit logs contain
no missing cooked package, fatal, assertion, ensure, unhandled exception, or
error entries.

## Launch Screen Regression

The ordinary packaged launch path was held on the language selector for more
than 15 seconds. All six languages initialized, the chooser remained the active
gate, no language was selected automatically, the gameplay world did not spawn,
and no launch-gate recovery warning occurred. The selector is now owned by the
GameMode across packaged Slate rebuilds while retaining mouse, keyboard, and
focus-independent input paths.

## Distribution Boundary

The archive is ad-hoc signed and ready for local Mac play. Apple Developer ID
signing, notarization, and Gatekeeper approval are credentialed external-release
steps and were not represented as complete by the local integrity verifier.
