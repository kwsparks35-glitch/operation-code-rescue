# World Promotion Validation Slice

## Purpose

This slice continues the `WORLD_DEVELOPMENT_DEEPDIVE` guidance by turning the authored + PCG hybrid city roadmap into active validation infrastructure.

The project still keeps its current C++ generated city as the playable fallback. That is intentional: the June 25 roadmap says to migrate additively, replacing block-spawned city surfaces only as authored kits, PCG outputs, Packed Level Actors, HLODs, Data Layers, and imported environment packs prove they are safe.

## Implemented

- Added `UCodeRescueWorldPromotionValidator`, a native `UEditorValidatorBase` subclass in the editor module.
- The validator reviews world/city `UStaticMesh` candidates in current and future world import locations, including `ModernBridges`, `Parallax_Night_Building_Material`, `/Game/World`, and `/Game/CodeRescueAssets/World`.
- Runtime-promoted city modules are stricter than review candidates: they must have renderable LOD data, at least one material slot, simple collision, and must not rely on complex-as-simple collision for walkable/blocking gameplay surfaces.
- PCG, World Partition, Data Layer, HLOD, Level Instance, and Packed Level Actor assets get a manual gate message requiring streaming-budget, fallback, safe-beat, and Mac package evidence before promotion.
- Added `Scripts/verify_world_promotion_validation_unreal.py`, which runs in `UnrealEditor-Cmd`, confirms the validator binding, samples world mesh candidates, records strict runtime promotion counts, verifies current source/manifest world gates, and writes `Saved/DataValidation/code_rescue_world_promotion_validation.json`.
- Added `Content/CodeRescueData/world_promotion_validation_contract.tsv` covering authored static meshes, the current C++ fallback city, PCG/World Partition staging, PLA/HLOD kits, Data Layer migration, human-scale collision/accessibility, and Apple Silicon streaming budget review.
- Replaced the old `FutureWorldRuntimeValidator` row in `editor_data_validation_contract.tsv` with `UCodeRescueWorldPromotionValidator`.
- Wired the static and Unreal verifiers into `Run_Full_QA_Audit.command`; wired the static verifier into `Run_Local_CI_Readiness.command`.

## Gameplay Boundary

The validator does not remove the current playable city. `SpawnAuthoredPropsForCity`, `SpawnStaticMeshProp`, `SpawnBlock`, `RegisterStreamedActor`, `ClearStreamedCampaignActors`, and the PCG/Houdini review cells remain the active fallback and staging path.

That means imported world assets can be reviewed without breaking the first-ten-minutes route, safehouse terminal, survivor path, helipad extraction, combat loop, or packaged smoke tests.

## Validation

Validation passed:

- Python syntax check for the new static and Unreal scripts
- `Scripts/verify_world_promotion_validation_contract_pass.py`
- Adjacent editor / Mac asset / Mac feature / character / physics validation passes
- `./Recompile_Module.command < /dev/null`
- UnrealEditor-Cmd world promotion validation smoke through `Scripts/verify_world_promotion_validation_unreal.py`
- `./Package_Mac_App.command < /dev/null`
- `./Smoke_Test_Packaged_App.command null`
- `./Smoke_Test_Packaged_App.command render`

The Unreal smoke report at `Saved/DataValidation/code_rescue_world_promotion_validation.json` confirmed `/Script/CodeRescueUnrealEditor.CodeRescueWorldPromotionValidator`, 22 current world candidate static meshes, 0 strict runtime-promoted static meshes, 32 map assets, 64 HLOD-like assets, 1 Data Layer-like asset, and 0 validation errors. The sampled `ModernBridges` assets reported three LODs and authored simple collision; the sampled parallax building assets reported material slots and review-candidate status. Packaged null and render smoke both passed with only the already-allowed immediate-exit navigation/crowd warnings plus the known render-mode CoreAudio sample-rate warning.

## Honesty Boundary

This slice does not author the final World Partition maps, PCG graphs, Packed Level Actor kit hierarchy, or HLOD proxies. It establishes the build-time and commandlet-time promotion gate those assets must satisfy before they replace the current generated city surfaces.
