# Mac Rendering AA Readiness Slice

This pass implements the Apple Silicon anti-aliasing guidance from the June 25 `CHARACTER_ANIMATION_DEEPDIVE`. That document calls out Temporal Super Resolution as a higher-cost default on Apple Silicon and specifically flags ghosting risk on fast-moving zombies in dark survival-horror scenes.

## Implemented Renderer Default

`Config/DefaultEngine.ini` now documents and enforces the packaged Mac baseline:

- `r.AntiAliasingMethod=2`
- `r.TemporalAA.Upsampling=False`

In Unreal's renderer CVar values, `2` is TAA and `4` is TSR. The project still keeps software Lumen-compatible defaults, disables hardware ray tracing, and leaves runtime GameMode visibility fallbacks intact, but TSR is no longer the default anti-aliasing method for the Mac package.

## Why This Matters

Operation Code Rescue's camera spends a lot of time moving through dark city streets while zombies, survivors, rescue drones, and route beacons cross the player's view. A lower-risk TAA baseline keeps the package closer to the Mac/Metal guidance in the June 25 documents and reduces the chance that zombie motion smears or ghost trails make combat unreadable.

This change is especially important before larger MetaHuman, zombie-family, Nanite, and imported city-kit work lands. Those assets will raise the visual cost of the scene; the renderer baseline should be conservative before content complexity increases.

## Manifest And QA Integration

The performance row in `Content/CodeRescueData/performance_city_layer_budget.tsv` now includes `RendererProfile`, which records that the Apple Silicon baseline uses TAA with temporal upsampling disabled.

The creative-development inclusion plan now points the P2 performance item at `Scripts/verify_mac_rendering_aa_readiness_slice_pass.py` plus package smoke. The human QA checklist and visual regression targets also name the Mac TAA renderer profile so review passes check the setting intentionally instead of treating it as hidden project configuration.

## Verification

Added `Scripts/verify_mac_rendering_aa_readiness_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks the renderer settings, the performance budget row, the creative plan, the human QA checklist, the visual regression target, this documentation, the original June 25 guidance, progress logging, and both QA entrypoints.

The package-level validation path remains packaged render smoke plus packaged null smoke, because the point of this slice is the runtime Mac baseline rather than only editor project settings.

## Boundaries

This is not the final full performance pass for every imported mesh, shader, texture, VFX, or city-kit asset. Remaining work still includes runtime frame captures on target Mac hardware, LOD chain enforcement, texture memory review, Nanite/SM6 gating, groom-to-card validation, and future Data Validation rules for imported content. This slice closes the concrete anti-aliasing default called out in the June 25 character-animation guidance and makes it difficult to regress silently.
