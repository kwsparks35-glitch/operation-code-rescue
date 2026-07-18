# Regional city kit identity slice

This pass implements the P1 world-development request for major city regional kits as a playable, reviewable layer in the current runtime-generated world. The June 25 world-development guidance asks the project to keep `FCodeRescueCityMission` as the source of truth while replacing block fallbacks with modular kit, district, landmark, and PCG/level-instance style outputs over time; this slice adds an inspectable bridge layer that proves those data points are visible in the packaged game.

## Runtime implementation

- Added `SpawnRegionalCityKitIdentityLayer` and call it after the existing city landmark, art kit, urban identity, and U.S. city-specific identity layers.
- The layer reuses `BuildUSCityVisualProfile` and `BuildUSCityRealizationParams` so regional cues inherit the active terrain token, signature cue, district cue, and mission colors.
- It spawns three path-adjacent anchors:
  - Regional Kit Entry Gate
  - Landmark Vista Kit
  - Objective District Kit
- Each anchor includes a floor plate, readable kit sign, modular swatches for trim/facade/prop/destruction dressing, and a non-shadow-casting point light.
- It also spawns a regional motif stand-in for harbor, desert solar, mountain, industrial, civic, transit, or metro kit families.

## Tags and review hooks

Regional kit actors receive:

- `RegionalCityKitIdentity`
- `MajorCityRegionalKit`
- `RegionalKitReady`
- `LandmarkWayfindingKit`
- `DistrictLevelInstanceStandIn`
- `KitBibleRuntimeCue`
- `NoAccessBlocker`
- `WorldDevelopmentDeepDive`
- `Top50Recommendations`

Signal lights additionally receive `RegionalKitSignalLight`. Runtime smoke logs now include `[CodeRescueRegionalCityKits]` with city label, art kit, region, district style, landmark, signature profile cue, and terrain token.

## Data and QA updates

- Added `Content/CodeRescueData/regional_city_kit_identity_manifest.tsv`.
- Updated the creative inclusion plan, visual-regression targets, human QA checklist, and accessibility manifest.
- Added `Scripts/verify_regional_city_kit_identity_slice_pass.py`.
- Wired the verifier into `Run_Full_QA_Audit.command` and `Run_Local_CI_Readiness.command`.

## Validation

Required validation for this slice:

- `python3 -m py_compile Scripts/verify_regional_city_kit_identity_slice_pass.py`
- `python3 Scripts/verify_regional_city_kit_identity_slice_pass.py`
- `./Recompile_Module.command < /dev/null`
- `./Package_Mac_App.command < /dev/null`
- `./Smoke_Test_Packaged_App.command null`
- `./Smoke_Test_Packaged_App.command render`
- Runtime log contract scan on packaged smoke logs
- Runtime log confirmation of `[CodeRescueRegionalCityKits]`
- `git diff --check`
- Touched-file trailing-whitespace scan
