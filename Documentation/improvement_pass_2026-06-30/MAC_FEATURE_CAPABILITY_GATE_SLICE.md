# Mac Feature Capability Gate Slice

This pass implements the Nanite, SM6, VSM, and Lumen caveats from the June 25 character-animation and world-development deep dives. The key rule is simple: high-end Mac renderer features can remain available for reviewed content, but packaged gameplay must not assume they are safe on every Apple Silicon Mac.

## Implemented Gate

`SpawnCreativeRecommendationsLayer` now adds visible intake controls for `Nanite SM6` and `Fallback LOD`. The active download board states that Nanite/SM6 promotion requires M2-or-newer class hardware, macOS 15+ review, and a non-Nanite fallback.

The corresponding actors are tagged with:

- `MacNaniteSM6ReviewGate`
- `MacNonNaniteFallbackReady`

The packaged runtime already disables VSMs when entering the language scene and the gameplay world with `r.Shadow.Virtual.Enable 0`. This slice documents and verifies that behavior as the current non-Nanite fallback path for procedural city geometry.

## Config And Manifest Coverage

`Config/DefaultEngine.ini` now explains that VSM capability stays available for reviewed Nanite/foliage content, while runtime play disables VSMs on the current procedural/non-Nanite fallback geometry.

Added `Content/CodeRescueData/mac_feature_capability_manifest.tsv` for:

- SM6 renderer capability
- Nanite hero geometry
- Virtual Shadow Maps
- Lumen hardware ray tracing
- PCG and Fab imports

Updated the performance city-layer budget, creative inclusion plan, human QA checklist, and visual regression targets so Mac feature capability review is visible during QA.

## Verification

Added `Scripts/verify_mac_feature_capability_gate_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks renderer comments/settings, runtime VSM disables, in-world tags and labels, no direct Nanite content hard-reference, package settings, manifest/docs/progress coverage, and the original June 25 guidance. Package validation remains compile, package, packaged null smoke, and packaged render smoke.

## Boundaries

This does not author final Nanite hero geometry or an automatic hardware detector. It creates the production gate those assets must pass: Nanite/SM6 content is target-hardware reviewed, and broad Mac play keeps a non-Nanite fallback path.
