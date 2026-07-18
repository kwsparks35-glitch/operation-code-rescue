# Houdini Modular City Output Slice

## Goal

Continue the June 25 world-development guidance by making the future Houdini/PCG city-output contract visible in the playable runtime world, without replacing the current stable C++ city fallback.

## Runtime Implementation

- Extended the existing `SpawnUnrealSystemsCharacterWorldLayer` Houdini/PCG review bay.
- Added a `HOUDINI OUTPUT RECIPE` board that shows a deterministic city seed, art kit, district style, facade-cell, rubble, collision, route-spline, and streaming-budget expectations.
- Added visible review modules for:
  - facade kit output
  - safehouse cell output
  - rubble variation output
  - collision proxy output
  - streaming budget cell output
- Added eight deterministic rubble-variation blocks tagged with `PCGRubbleVariationSet`, `PCGCollisionProxyReady`, and `MacLODBudgetReviewGate`.
- Added route-spline knots tagged with `PCGRouteSplineReady`, `PCGWorldPartitionBakeReview`, and `NoAccessBlocker`.
- Moved the existing Chaos review section farther down the review bay so the new generated-output lane remains readable and walkable.

## Data And QA

- Added `Content/CodeRescueData/houdini_modular_city_output_manifest.tsv`.
- Updated:
  - `Content/CodeRescueData/creative_development_inclusion_plan.tsv`
  - `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
  - `Content/CodeRescueData/visual_regression_targets.tsv`
  - `Content/CodeRescueData/performance_city_layer_budget.tsv`
  - `Run_Full_QA_Audit.command`
  - `Run_Local_CI_Readiness.command`
  - `progress.md`

## Validation

Added `Scripts/verify_houdini_modular_city_output_slice_pass.py`.

The verifier checks:

- runtime tags for deterministic seed, modular city output, facade modules, safehouse cells, rubble variants, collision proxies, streaming budget cells, route splines, and world-partition bake review
- nonblocking route spline knots
- the expanded manifest rows
- creative plan, human QA, visual target, performance budget, progress, documentation, and QA command wiring
- continued compatibility with the existing world promotion validation contract

## Boundary

This slice does not generate final Houdini assets, PCG graphs, World Partition maps, Data Layers, HLODs, or Packed Level Actors. It creates a playable, reviewable runtime target so those future exports have clear placement, collision, streaming, fallback, and package-smoke expectations before they replace any current city fallback geometry.
