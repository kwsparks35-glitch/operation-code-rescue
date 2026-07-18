# Standard Direct-Pursuit Zombie Slice

This slice closes the June 25 P0 enemy guidance for ordinary zombies by making the existing direct-chase combat path explicit, readable, and auditable. Standard pressure zombies now opt into a named runtime profile, expose a pursuit state to the HUD, telegraph attacks through the same cooldown that gates damage, and respect protected coding spaces without changing the selected-language save loop.

## Implementation

- Added `bStandardDirectPursuitEnabled`, pursuit tuning fields, `ApplyStandardDirectPursuitProfile`, and `GetStandardPursuitStateSummary` to `ACodeZombieActor`.
- Added runtime tags for `StandardDirectPursuitZombie`, `ZombiePursuitReadableRuntime`, `FairSurvivalPressure`, `NoLearningZonePressure`, `AttackWindupReadable`, attack commit, protected-zone hold, and pursuit state transitions.
- Routed melee timing through `StandardPursuitAttackCooldown` so red glow, threat captions, and actual damage share the same fair windup window.
- Updated `ACodeRescueAIController` to tag direct chase and attack-hold behavior when a profiled zombie is actively moving toward the player.
- Applied the profile to regular city waves, physics ambush zombies, encounter director zombies, language breach patrols, boss horde zombies, and boomer add spawns while leaving special boss/mini-boss identity tags intact.
- Updated the HUD threat compass, tactical readout, and combat alert line to show standard pursuit state and `PURSUIT PRESSURE` when ordinary zombies close distance.

## Player Result

Ordinary zombie pressure now reads as a fair survival system outside coding spaces: the player sees direction, distance, family/role, pursuit state, and attack windup before damage lands. When the player enters a protected learning area, profiled zombies hold pressure outside that space and tag the behavior for review instead of interrupting terminal work or selected-language progression.

## Validation

Added `Content/CodeRescueData/standard_direct_pursuit_zombie_manifest.tsv` and `Scripts/verify_standard_direct_pursuit_zombie_slice_pass.py`, then wired the verifier into local CI and full QA. The creative inclusion plan, human QA checklist, visual regression target list, and progress log now name this slice as the P0 standard zombie implementation path.

Manual QA should leave a protected coding safehouse, let a standard/director/horde zombie acquire the player, confirm the HUD/captions show pursuit and attack windup, take or avoid the telegraphed hit, then step back into the protected learning zone and verify pressure stops without changing the selected-language save.
