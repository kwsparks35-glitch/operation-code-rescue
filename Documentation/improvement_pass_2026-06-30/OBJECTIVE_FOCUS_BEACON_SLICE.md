# Objective Focus Beacon Slice

Date: 2026-06-30

## Source Guidance

- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: called for stronger wayfinding, readable objective staging, and clearer transitions between mission spaces.
- `TOP_50_RECOMMENDATIONS.pdf`: called out objective clarity, onboarding support, UI readability, enemy/readability feedback, and reduced player confusion.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: emphasized package-safe, auditable improvements with clear verification paths.

## Implementation

Added `AObjectiveFocusBeaconActor`, a state-aware in-world objective beacon that follows the active mission phase:

- terminal phase
- survivor rescue phase
- extraction phase

The beacon is built from engine primitives and a `TextRenderComponent`, so it is visible in packaged builds without requiring authored assets first. It contains a base ring, vertical beacon column, rotating direction arrow, pulse core, phase progress nodes, label text, and point light.

## Integration

`SpawnPurposeClarityLayer()` now spawns one objective beacon per city and configures it with the canonical entry, terminal, survivor, and extraction coordinates already used by the HUD and route pads. The actor reads the current language save state from `UCodeRescueGameInstance`:

- unsolved terminal shows the active language terminal objective
- solved terminal shows survivor rescue
- rescued survivor shows extraction while the player is still near that city

The beacon is registered with streamed city cleanup and tagged as `StateAwareObjectiveBeacon` and `ObjectiveClarityRuntimeLayer`.

## Accessibility

The beacon receives the saved reduced motion setting during configuration. Reduced motion keeps the color, light, target label, and silhouette readable while damping spin, bobbing, pulse, and follow movement.

## Authored Asset Handoff

`OnObjectiveBeaconPhaseChanged(int32 ObjectivePhase)` is exposed as a Blueprint event. Later authored Niagara, camera, sound, route spline, or UI animation work can hook into phase changes without rewriting the objective-state logic.

## Verification

Added `Scripts/verify_objective_focus_beacon_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks:

- actor state fields and target coordinates
- terminal, survivor, and extraction phase resolution
- active language label text
- reduced motion animation damping
- nonblocking component setup
- `SpawnPurposeClarityLayer()` integration
- documentation and QA wiring
