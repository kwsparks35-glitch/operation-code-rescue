# Rescue Extraction Presentation Slice

Date: 2026-06-30

## Source Guidance

- `CHARACTER_ANIMATION_DEEPDIVE.pdf`: called for stronger rescue readability, authored hero moments, and a future path for Sequencer, Control Rig, and character-animation polish.
- `WORLD_DEVELOPMENT_DEEPDIVE.pdf`: called for authored gameplay beats that help the world feel intentional rather than purely procedural.
- `TOP_50_RECOMMENDATIONS.pdf`: emphasized clearer feedback, player-facing polish, and stronger moment-to-moment presentation.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: emphasized demo-readiness, auditability, and repeatable verification.

## Implementation

Added `ARescueExtractionPresentationActor`, a cook-safe runtime survivor rescue beat that spawns when `ASurvivorActor::Rescue()` succeeds.

The actor builds a procedural extraction rig from engine primitives:

- landing disc
- vertical rescue beam
- two sweep arms
- lift marker
- three orbit beacons
- key/fill rescue lights

The presentation actor is tagged for future review and asset replacement:

- `RescueExtractionPresentation`
- `SequencerReadyFallback`
- `ControlRigReadyFallback`
- `CharacterAnimationDeepDive`
- `WorldDevelopmentDeepDive`
- `Top50Recommendations`
- `ReleaseDossier`

`ASurvivorActor::Rescue()` now resolves the current city mission colors, spawns the extraction beat at the survivor position, and passes the survivor name, city index, accent color, and reduced-motion setting into the actor. The existing rescue persistence remains intact: the survivor is still marked rescued, the run is still saved, companion spawning still works, and the survivor is still hidden/collision-disabled after rescue.

## Accessibility

The extraction beat respects `UCodeRescueGameInstance::bReducedMotion`. In standard motion mode, the sweep arms and beacons orbit quickly enough to read as an emergency extraction cue. In reduced motion mode, the same color, lighting, lift marker, and pulse remain readable while fast orbit movement is dampened.

## Future Asset Handoff

The actor exposes `OptionalSequencerBeatAsset` and a Blueprint event named `OnRescuePresentationStarted`. These are deliberately non-blocking hooks: the C++ fallback works in cooked builds today, while a later authored Level Sequence, Control Rig handoff, camera shot, or animation montage can attach to the same rescue transition without changing progression logic.

## Verification

Added `Scripts/verify_rescue_extraction_presentation_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks:

- the presentation actor exists and exposes runtime configuration fields
- the actor builds the full extraction rig from engine primitives
- tags identify Sequencer, Control Rig, character-animation, world-development, recommendations, and release-dossier coverage
- the tick function animates and self-cleans the presentation
- reduced-motion behavior is represented in runtime animation
- `ASurvivorActor::Rescue()` spawns the beat while preserving save and hide behavior
