# Zombie Motion Readability Slice

This slice continues the June 25 `CHARACTER_ANIMATION_DEEPDIVE` and enemy-readability guidance by making standard zombie combat states visible even before every final authored zombie montage, Control Rig, or retargeted animation set exists. The game already had attack glow, captions, hit impulse, death physics, and optional imported montage playback; this pass adds an additive visual pose layer that makes chase, windup, lunge, protected-zone hold, and hit reaction readable in the live build.

## Implementation

- Added `bEnableMotionReadability`, `MotionReadabilitySwayScale`, `HitReactionPoseDuration`, and `AttackLungePoseDuration` tuning fields to `ACodeZombieActor`.
- Added cached base-pose storage for the skeletal mesh, primitive fallback body/head, and infection glow so additive motion can reset cleanly.
- Added `UpdateMotionReadability()` to drive non-collision component offsets for locomotion sway, chase lean, attack windup, attack lunge, hit recoil, and protected learning-zone hold.
- Added `TriggerAttackMotionCue()` for player attacks and destructible-cover strikes, plus `TriggerHitReactionMotionCue()` for nonfatal player damage.
- Added `ZombieMotionReadabilityRuntime` and component-level additive-pose tags so screenshots, QA notes, and future animation promotion validation can find this runtime layer.
- Kept authored assets first: imported AnimBPs, hit-react montages, attack montages, death montages, ragdolls, and primitive corpse physics still run when available; the additive layer supplies readable fallback motion around them.

## Player Result

Ordinary pursuit enemies now show body motion that matches what the HUD and captions are saying. A chasing zombie sways and leans forward, an attacking zombie pulls back before a lunge, a damaged zombie visibly recoils, and a zombie outside a protected coding zone reads as held pressure rather than an active attacker.

## Validation

Added `Content/CodeRescueData/zombie_motion_readability_manifest.tsv` and `Scripts/verify_zombie_motion_readability_slice_pass.py`, then wired the verifier into local CI and full QA. Updated the creative inclusion plan, enemy readability manifest, human QA checklist, visual regression targets, and progress log so this runtime animation-readability layer remains auditable.

Manual QA should fight a regular pursuit zombie, an encounter-director zombie, and a horde zombie outside the coding safehouse. Confirm chase sway, red windup glow plus body pullback, lunge on attack commit, recoil on nonfatal hits, and a non-attacking hold pose when stepping back into the protected learning zone.
