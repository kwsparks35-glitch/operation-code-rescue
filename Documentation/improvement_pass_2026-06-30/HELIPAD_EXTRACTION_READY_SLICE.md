# Helipad Extraction-Ready Slice

Date: 2026-06-30

## Source Guidance

- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: called for clearer authored rescue spaces, readable extraction destinations, and world-state changes that reflect completed objectives.
- `TOP_50_RECOMMENDATIONS.pdf`: emphasized stronger player feedback after progress milestones and clearer next-step guidance.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: emphasized demo-ready, reviewable behavior that survives save/load and packaging paths.

## Implementation

Extended `AHelipadActor` with an extraction-ready state that turns the helipad into an active rescue destination after survivor rescue. The helipad now owns a dormant runtime visual rig made from engine primitives:

- vertical extraction column
- two rotating sweep bars
- hovering beacon
- rescue-colored point light pulse

`ASurvivorActor::Rescue()` now finds the current city's helipad and calls `SetExtractionReady()` as part of the successful survivor rescue flow. This happens before the existing persistent save call, so the visible extraction state and the saved survivor progression stay aligned.

`ACodeRescueGameMode::SpawnHelipadForCity()` also restores the extraction-ready state when a helipad is spawned for a city whose survivor has already been rescued. This keeps the same start screen and save/load flow intact while allowing players to return later and see which cities already have an active extraction point.

## Tags

Extraction-ready helipads are tagged for audit and future authored-asset replacement:

- `Helipad`
- `ExtractionReadyHelipad`
- `RescueLoopClosure`
- `WorldDevelopmentDeepDive`
- `Top50Recommendations`
- `ReleaseDossier`

## Accessibility

The helipad extraction-ready animation respects the saved reduced motion setting. Standard mode uses rotating sweep bars, beacon bobbing, and a stronger light pulse; reduced motion keeps the color, light, and extraction-ready silhouette visible while damping motion speed and vertical movement.

## Player Impact

The rescue loop now has a clearer close:

- coding work reveals routes and guidance
- reaching the survivor triggers a rescue presentation
- the current city helipad becomes visibly extraction-ready
- saved survivor rescue progress restores that helipad state on later sessions

This makes survivor rescue feel connected to the broader world instead of ending only with the survivor disappearing and a save update.

## Verification

Added `Scripts/verify_helipad_extraction_ready_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks:

- helipad extraction-ready state, tick behavior, and visual rig components
- rescue-time activation from `ASurvivorActor::Rescue()`
- saved progress restoration from `ACodeRescueGameMode::SpawnHelipadForCity()`
- reduced motion damping
- documentation, progress-log, and QA wiring
