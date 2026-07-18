# Mac Asset Import Budget Gate Slice

This pass turns the June 25 performance guidance for imported art into a visible and automated promotion gate. Character meshes, city modules, textures, shaders, VFX, Nanite/HLOD content, and physics props now have named Mac review surfaces before they are treated as runtime-ready game content.

## Implemented Gate

`SpawnCreativeRecommendationsLayer` now extends the active download intake board with:

- `LOD audit` tagged `MacLODBudgetReviewGate`
- `Texture cap` tagged `MacTextureMemoryReviewGate`
- `Shader trim` tagged `MacShaderComplexityReviewGate`
- physics/destruction manifest rows tagged `MacPhysicsBudgetReviewGate`

The board text also states that LOD, texture, and shader Mac budget gates are required before runtime promotion. The runtime breadcrumb now includes `Mac LOD/texture/shader asset budget gates`, so packaged smoke logs prove the compiled game contains this review surface.

## Manifest Coverage

Added `Content/CodeRescueData/mac_asset_import_budget_gate.tsv` for:

- crowd zombie skeletal meshes, requiring LOD0-to-LOD3 chains, lower-LOD bone influence caps, animation URO, and spawn-budget review
- hero and survivor skeletal meshes, requiring skeleton, animation blueprint, close-range LOD policy, physics-asset review, and no shipping fallback primitive
- static city modules and interiors, preferring trim sheets, Packed Level Actors, instancing, and master-material parameters
- Nanite hero detail and HLOD proxies, requiring MacNaniteSM6ReviewGate, MacNonNaniteFallbackReady, and target Mac frame review
- textures and material instances, blocking unreviewed 4K texture sets and requiring compression/master-material review
- shader, VFX, fog, and translucent effects, requiring shader complexity review, reduced-motion fallback, and packaged render smoke
- physics and destruction props, requiring simple collision preference, piece-count limits, sleep/disable behavior, and Chaos stress review

The performance city-layer budget now includes `ImportedAssetBudgetGate`, and the creative inclusion plan, visual regression targets, and human QA checklist all surface the new budget gate.

## Source Guidance Implemented

This slice maps directly to the June 25 documents:

- `CHARACTER_ANIMATION_DEEPDIVE` calls for the Skeletal Mesh Reduction Tool, LOD chains, Max Triangle Count, Max Bones Influence, URO, and Data Validation.
- `WORLD_DEVELOPMENT_DEEPDIVE` calls for trim sheets, master materials, Nanite hero detail, HLOD and instancing, streaming/memory budgets, and Data Validation.
- `GAME_PHYSICS_DEEPDIVE` calls for Apple Silicon physics budgets, low destruction piece counts, Sleep/Disable Fields, and Data Validation.
- `TOP_50_RECOMMENDATIONS_2026-06-25` calls for Apple Silicon performance budgets using HLODs, instancing, and LODs.

## Verification

Added `Scripts/verify_mac_asset_import_budget_gate_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the in-world labels/tags, the new manifest, adjacent Mac hair and feature manifests, performance/creative/QA/visual tables, runtime log contract, this documentation, the June 25 source guidance, and the progress log. Package validation remains compile, package, packaged null smoke, and packaged render smoke because the runtime breadcrumb is emitted only by the built game.

## Boundaries

This does not generate final LOD chains, texture compression settings, HLOD proxies, or Unreal `UEditorValidatorBase` classes for every future imported asset. It establishes the review gate those assets must pass so imported content cannot quietly bypass Mac LOD, texture-memory, shader-complexity, Nanite fallback, and physics-budget review.
