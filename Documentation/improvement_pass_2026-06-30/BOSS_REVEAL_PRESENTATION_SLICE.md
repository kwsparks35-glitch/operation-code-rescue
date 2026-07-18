# Boss Reveal Presentation Slice

Date: 2026-06-30

## Source Guidance

- `CHARACTER_ANIMATION_DEEPDIVE.pdf`: called for boss reveals to become authored story beats with Sequencer and Control Rig replacement paths.
- `TOP_50_RECOMMENDATIONS.pdf`: specifically called for short Sequencer beats for intro, boss reveal, and extraction.
- `OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf`: emphasized reviewable, package-safe implementation that can be audited and smoke-tested.

## Implementation

Added `ABossRevealPresentationActor`, a cook-safe runtime boss reveal beat that spawns with each undefeated city boss. The actor stays invisible until the player approaches the boss arena, then briefly plays a nonblocking reveal using engine primitives:

- expanding arena ring
- paired threat gates
- rotating sweep bars
- hovering boss crown marker
- three orbit beacons
- warning point light

`ACodeRescueGameMode::SpawnBossForCity()` now creates and registers this reveal layer after the existing boss marker, halo, and guide text. The boss actor, city index, city name, reveal title, mission-tinted warning color, and reduced motion setting are passed into `ConfigureReveal()`.

## Cinematic Handoff

The actor exposes `OptionalSequencerRevealAsset` and `OnBossRevealStarted()`. These are forward-looking hooks for a later authored Sequencer and Control Rig boss reveal while preserving a working C++ fallback today.

## Accessibility

The reveal respects reduced motion. Standard mode uses stronger beacon orbit, sweep rotation, crown bobbing, and light pulsing; reduced motion keeps the warning silhouette, color, and light intensity while damping motion speed and vertical movement.

## Player Impact

Bosses now get a readable reveal beat instead of only distant signage. The player receives a clear moment of escalation when entering the boss zone, and the presentation self-cleans after the reveal so it does not block combat or leave extra collision in the arena.

## Verification

Added `Scripts/verify_boss_reveal_presentation_slice_pass.py` and wired it into:

- `Run_Full_QA_Audit.command`
- `Run_Local_CI_Readiness.command`

The verifier checks:

- boss reveal actor fields, components, and cinematic hooks
- cook-safe primitive construction
- proximity trigger behavior
- reduced-motion damping
- self-cleanup after the reveal duration
- integration in `SpawnBossForCity()`
- documentation and QA wiring
