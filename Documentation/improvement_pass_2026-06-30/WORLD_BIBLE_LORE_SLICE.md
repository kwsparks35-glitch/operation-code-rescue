# World Bible Lore Slice

Date: 2026-06-30

## Purpose

This slice continues `WORLD_DEVELOPMENT_DEEPDIVE` section 7.2 and `TOP_50_RECOMMENDATIONS` item 36. The project needs a compact canonical lore contract so 465 generated city chapters can stay coherent as authored props, audio, UI, and Level Instance work continue.

## Implemented

- Added `ACodeRescueGameMode::SpawnWorldBibleLoreLayer` and call it for every campaign city immediately after the environmental storytelling layer.
- Built a nonblocking field lore deck with five cards: premise, pillars, factions/forces, technology rules, and per-city lore data.
- Expressed the canonical premise directly in the world: near-total automation failed, and field engineers restore physical systems through code.
- Named the pillars from the deep-dive guidance: survival-horror dread, coding-as-empowerment, and the 465-city widening rescue effort.
- Named light-touch factions and forces: the surviving engineers' network, abandoned automated systems as environmental antagonist, and infected pressure outside safe beats.
- Formalized `MissionBrief`, `RadioBriefing`, `CharacterStoryPlan`, and `SurvivorName` as per-city lore data through the SURVIVOR CITY CHAPTER card.
- Tagged all actors with `WorldBibleLoreLayer`, `CanonicalLoreContract`, `WorldBibleAndLoreGuidance`, `CodingAsEmpowermentPillar`, `SurvivingEngineersNetwork`, `AutomationAntagonistForce`, `InfectedPressureForce`, `TechnologyRulesReadable`, `PerCityLoreData`, `Top50Recommendation36`, and `WorldDevelopmentDeepDive`.
- Added `Content/CodeRescueData/world_bible_lore_manifest.tsv` and wired this slice into creative-development, performance, visual-review, human-QA, full-QA, local-CI, and character/world design manifests.

## Player-Facing Behavior

Players can inspect a small lore station beside the environmental story deck and understand the rules of the fiction before entering the city: code repairs systems, allies are the engineer network, automation is broken and hostile, infected pressure remains a field threat, and the current city contributes a named survivor chapter. The layer is collisionless, optional, and does not modify selected-language saves, terminal validation, safehouse safety, combat pressure, or extraction behavior.

## Honesty Boundary

This is not a finished narrative bible document, final graffiti pass, quest-writing pass, or authored prop art replacement. It establishes a runtime and data contract for the future written bible and art/audio implementation to preserve.

## Validation

Run:

```zsh
python3 Scripts/verify_world_bible_lore_slice_pass.py
python3 Scripts/verify_environmental_storytelling_slice_pass.py
python3 Scripts/verify_coding_world_response_slice_pass.py
python3 Scripts/verify_survivor_archetype_roster_slice_pass.py
python3 Scripts/verify_runtime_data_layer_migration_slice_pass.py
./Recompile_Module.command < /dev/null
./Package_Mac_App.command < /dev/null
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
```
