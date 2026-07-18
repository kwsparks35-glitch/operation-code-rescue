# Interior mission spaces slice

This pass implements the P1 world-development request for enterable mission spaces across safehouses, hospitals, schools, stores, and transit hubs. It follows the June 25 world-development and release-readiness guidance by replacing facade-only intent with cook-safe, reviewable rooms that are playable in the current Mac package.

## Runtime implementation

- Added `SpawnInteriorMissionSpacesForCity` and call it from `SpawnCampaignCity` immediately after the enterable civic safehouse layer.
- Every streamed city now receives five open-front room archetypes:
  - Hospital Triage Clinic
  - School Study Shelter
  - Corner Store Cache
  - Transit Operations Hub
  - Civic Records Annex
- Each room has floor/wall/header geometry, readable door gaps, mission boards, text-first room labels, and a point light using the room's accent color.
- Each room also carries functional pickups:
  - Hospital: medkit
  - School: radio scanner charge
  - Store: ammo pouch
  - Transit: flashlight battery
  - Civic records: bypass kit
- Mission boards connect the room to `Mission.CurriculumFocus`, `Mission.SurvivorName`, `Mission.LandmarkName`, and the terminal -> survivor -> helipad route.

## Tags and review hooks

All spawned room actors receive:

- `InteriorMissionSpace`
- `EnterableMissionInterior`
- `InteriorMissionSpaceReady`
- `HumanScaleBuildingProportion`
- `WorldDevelopmentDeepDive`
- `Top50Recommendations`

Each room also receives a type tag such as `InteriorMission_HospitalTriage` or `InteriorMission_TransitOps`. Pickups add `InteriorMissionSupplyPickup` so QA can verify that the rooms are gameplay spaces, not only scenic dressing.

## Data and QA updates

- Added `Content/CodeRescueData/interior_mission_spaces_manifest.tsv`.
- Updated the creative inclusion plan, curriculum feedback manifest, first-ten-minutes onboarding, visual-regression targets, human QA checklist, and accessibility manifest.
- Added `Scripts/verify_interior_mission_spaces_slice_pass.py`.
- Wired the verifier into `Run_Full_QA_Audit.command` and `Run_Local_CI_Readiness.command`.

## Validation

Required validation for this slice:

- `python3 -m py_compile Scripts/verify_interior_mission_spaces_slice_pass.py`
- `python3 Scripts/verify_interior_mission_spaces_slice_pass.py`
- `./Recompile_Module.command < /dev/null`
- `./Package_Mac_App.command < /dev/null`
- `./Smoke_Test_Packaged_App.command null`
- `./Smoke_Test_Packaged_App.command render`
- Runtime log contract scan on packaged smoke logs
- `git diff --check`
- Touched-file trailing-whitespace scan
