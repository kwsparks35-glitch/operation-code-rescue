# Distinct Weapon Presentation Slice

This pass implements the P1 weapons request for distinct weapon models and animations in a package-safe way, following the `CHARACTER_ANIMATION_DEEPDIVE` and `TOP_50_RECOMMENDATIONS` guidance for readable first-person combat identity. The arsenal already had 17 mechanically distinct weapon definitions, but the first-person presentation did not clearly show which weapon family was active. This slice adds a local first-person silhouette, tint, stance, recoil, reload, and melee/readability layer for every weapon type while leaving clean hooks for authored meshes and animation montages later.

## Player-Facing Changes

- Added an owner-only `FirstPersonWeaponSilhouette` component attached to the first-person camera.
- Added 17 weapon presentation profiles covering every `EWeaponType`: pistols, shotguns, rifles, grenades, knife, launchers, and utility explosives.
- Each profile has a unique fallback shape, scale, camera-space stance, tint, recoil distance, reload dip, reload roll, and bob amount.
- Fire, melee fallback, and reload actions now stamp presentation cue times so the visible weapon responds immediately to combat input.
- Camera switching hides the weapon outside first-person views so third-person, tactical, top-down, isometric, and side-view modes remain clean.
- Reduced Motion scales the procedural weapon movement down while preserving the visible stance and weapon identity.

## Implementation

The runtime work lives in `ACodeRescueCharacter`.

- The constructor creates `FirstPersonWeaponSilhouette` from `/Engine/BasicShapes/Cube.Cube`, disables collision and overlap, sets owner-only visibility, and tags it as a fallback model/animation hook.
- `GetWeaponPresentationProfile` maps each weapon type to a named profile such as `WeaponProfile_BalancedHandgun`, `WeaponProfile_PumpShotgun`, and `WeaponProfile_RocketLauncher`.
- `SyncActiveWeaponStateFromLoadout` invalidates the presentation profile on weapon swap so the model changes immediately.
- `UpdateFirstPersonWeaponPresentation` applies profile shape, material tint, first-person visibility, movement bob, aim sway, fire recoil, reload dip, reload roll, and reduced-motion scaling.
- `Fire`, no-ammo melee fallback, `Reload`, and `OnReloadComplete` update the presentation cue state.

## Documentation And Audit Trail

- Added `Content/CodeRescueData/distinct_weapon_presentation_manifest.tsv`.
- Updated the creative-development inclusion plan so the P1 distinct weapon row now routes through `verify_distinct_weapon_presentation_slice_pass.py`, the tactical arsenal verifier, the player first-person animation verifier, packaged smoke, and manual visual review.
- Added visual regression, human QA, and accessibility manifest coverage.
- Wired the new verifier into full QA and local CI.

## Verification

Planned verification for this slice:

- `python3 -m py_compile Scripts/verify_distinct_weapon_presentation_slice_pass.py`
- `python3 Scripts/verify_distinct_weapon_presentation_slice_pass.py`
- existing tactical arsenal verifier
- existing player first-person animation verifier
- module recompile
- Mac packaging
- packaged null smoke
- packaged render smoke
- scoped `git diff --check`
- touched-file trailing-whitespace scan

## Human QA Notes

Enter first-person view, cycle all weapon slots, fire each ammo weapon, trigger reloads, and exhaust one weapon to observe the melee fallback. The active weapon should have a distinct stance and motion even before authored weapon assets are imported, and the HUD text should remain the authoritative accessible label.

## Remaining Art Hooks

This is a working procedural fallback, not final weapon art. Future imported weapon packs can replace the cube-derived silhouette component with authored meshes, sockets, animation montages, Niagara muzzle effects, and per-weapon sounds while keeping the same named profiles and verification contract.
