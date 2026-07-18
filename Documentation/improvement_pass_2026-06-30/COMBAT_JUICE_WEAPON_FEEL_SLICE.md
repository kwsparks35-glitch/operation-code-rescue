# Combat Juice Weapon Feel Slice

This pass implements a package-safe combat-feel bridge from `TOP_50_RECOMMENDATIONS` recommendation 27, combat juice, recommendation 38, weapon feel, and the `CHARACTER_ANIMATION_DEEPDIVE` guidance for readable fire/reload combat timing. The game already had magazines, weapon profiles, fire/reload presentation, surface impacts, and damage feedback; this slice ties those events together with small runtime pulses that make shots, hit confirms, reloads, and incoming damage feel intentional without requiring new art assets.

## Player-Facing Changes

- Ammo fire, dry fire, and melee fallback now trigger reduced-motion-aware camera kick.
- Confirmed zombie, barricade, area, melee, and aim-assist hits now trigger a short hit-stop-style first-person weapon hold.
- Headshots receive a stronger `CombatJuiceHeadshotCrunch` cue while still using the existing HUD and audio hit confirmation.
- Reload start and reload completion now emit readable camera settle pulses without changing magazine or reserve ammo behavior.
- Incoming zombie damage now adds a compact direction-aware camera nudge that complements the damage overlay, mitigation text, knockback, and emergency medkit logic.
- All motion scales through the existing `bReducedMotion` setting, so Reduced Motion behavior remains centralized and save-backed.

## Implementation

The runtime work lives in `ACodeRescueCharacter`.

- Added `bEnableCombatJuice` and tuning fields for fire kick, yaw kick, hit-stop-style duration, hit-confirm kick, damage kick, and reload settle kick.
- Added combat-juice state for last fire, hit confirm, reload stage, damage cue, hit-stop duration, hit-stop scale, and headshot state.
- Added `GetCombatJuiceMotionScale`, `TriggerCombatJuiceFireCue`, `TriggerCombatJuiceHitConfirm`, `TriggerCombatJuiceReloadStageCue`, `TriggerCombatJuiceDamageCue`, and `UpdateCombatJuice`.
- Wired fire kick into ammo fire, dry fire, and no-ammo melee fallback.
- Wired hit-confirm feedback into direct zombie damage, barricade hits, area weapons, melee hits, and aim-assist damage.
- Wired reload stage feedback into reload start, empty-reserve dry reload, and reload completion.
- Wired damage feedback into `ApplyDamage` after the damage overlay receives source direction.
- Extended `UpdateFirstPersonWeaponPresentation` so hit-confirm state creates a brief weapon offset/rotation cue alongside the existing recoil and reload motion.

## Documentation And Audit Trail

- Added `Content/CodeRescueData/combat_juice_weapon_feel_manifest.tsv`.
- Added creative-development, visual-regression, human-QA, and accessibility entries for combat juice and weapon feel.
- Wired `Scripts/verify_combat_juice_weapon_feel_slice_pass.py` into local CI and full QA.
- Logged this slice in `progress.md`.

## Verification

Planned verification for this slice:

- `python3 -m py_compile Scripts/verify_combat_juice_weapon_feel_slice_pass.py`
- `python3 Scripts/verify_combat_juice_weapon_feel_slice_pass.py`
- `python3 Scripts/verify_distinct_weapon_presentation_slice_pass.py`
- `python3 Scripts/verify_player_first_person_animation_slice_pass.py`
- `python3 Scripts/verify_damage_feedback_accessibility_slice_pass.py`
- module recompile
- Mac packaging
- packaged null smoke
- packaged render smoke
- scoped `git diff --check`
- touched-file trailing-whitespace scan

## Human QA Notes

Enter first-person, fire multiple weapon families, land body and head hits, damage barricades, trigger an area weapon, exhaust one weapon into melee fallback, reload a partial magazine, and take a zombie hit from each side. The player should feel compact weight from the camera and weapon silhouette while the HUD, subtitles, damage overlay, weapon labels, and reduced-motion setting remain the accessible source of truth.

## Remaining Art Hooks

This is a procedural fallback. Future authored weapon montages, Niagara impact variants, decals, sound variations, force-feedback assets, and animation notifies can call the same trigger functions instead of rewriting the combat-feel contract.
