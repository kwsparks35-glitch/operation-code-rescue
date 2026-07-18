# Environmental Storytelling Slice

Date: 2026-06-30

## Purpose

This slice continues `WORLD_DEVELOPMENT_DEEPDIVE` sections 6.4 and 7 plus `TOP_50_RECOMMENDATIONS` item 35. The goal is to make the premise that coding rescues people readable inside the playable city, not only in documents, case files, or radio text.

## Implemented

- Added `ACodeRescueGameMode::SpawnEnvironmentalStorytellingLayer` and call it for every campaign city immediately after collectible case files.
- Built a nonblocking environmental story deck with five readable beats: automation failure, safehouse engineer network, survivor stake, code cause/effect, and city chapter navigation.
- Fed the layer from existing mission data: `MissionBrief`, `TerminalTitle`, `RadioBriefing`, `SurvivorName`, `CharacterStoryPlan`, `LandmarkName`, `RegionName`, `DistrictStyle`, and `NovelGameplayDetail`.
- Tagged all story actors with `EnvironmentalStorytellingLayer`, `CodingRescuesPeoplePremise`, `WorldBiblePillar`, `AutomationFailureScene`, `TechnologyRuleReadable`, `NonBlockingWorldStoryCue`, `Top50Recommendation35`, and `WorldDevelopmentDeepDive`.
- Applied runtime Data Layer stand-in tags such as `RuntimeDataLayer_Mode_Storytelling`, `RuntimeDataLayer_Mode_CodingSafehouse`, `RuntimeDataLayer_Mode_RescueTraversal`, and `RuntimeDataLayer_State_Prerecovery` so the story layer remains compatible with the World Partition/Data Layer migration contract.
- Added `Content/CodeRescueData/environmental_storytelling_manifest.tsv` as the traceability manifest and wired the slice into creative-development, performance, visual-review, human-QA, full-QA, and local-CI surfaces.

## Player-Facing Behavior

Players can walk past a compact story station that explains why the terminal matters: automation failed, the safehouse engineer network is repairing systems, a named survivor is at stake, solved code changes the route, and each city has its own chapter. The layer is optional, collisionless, and does not alter selected-language saves, terminal validation, safehouse protection, combat pressure, or extraction progression.

## Honesty Boundary

This is not the final authored prop, graffiti, and world-bible art pass. It establishes a runtime contract and visible placeholder composition for later art, audio, PCG, and Level Instance work to replace with richer assets while preserving the same narrative beats.

## Validation

Run:

```zsh
python3 Scripts/verify_environmental_storytelling_slice_pass.py
python3 Scripts/verify_case_file_collectibles_slice_pass.py
python3 Scripts/verify_coding_world_response_slice_pass.py
python3 Scripts/verify_city_ambient_zone_audio_slice_pass.py
python3 Scripts/verify_runtime_data_layer_migration_slice_pass.py
./Recompile_Module.command < /dev/null
./Package_Mac_App.command < /dev/null
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
```
