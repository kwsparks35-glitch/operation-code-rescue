# Pass 5 — Perspectives, Aim-Down-Sights, Grenade Ballistics, Import-Scale Root Cause
**Date:** 2026-07-16 · **Requested by:** Kenny (3 packaged-app screenshots) · **Status:** shipped

Kenny's report, in his words: (a) "the perspectives have been modified in a less
helpful way" — third-person showed a tiny dark speck / floating weapon;
(b) "a large area that does not align" — the remaining ground disagreement;
(c) enhanced visual aiming — "see down the weapon's barrel/neck/into the
scope", user-controlled magnification 1x/2x/5x/10x/20x/50x, and "aim before
they shoot"; (d) grenades — "identify the launch angle and anticipated
kinematic area of landing" and replace the "grid-lain sphere/hemisphere"
explosion with proper explosion physics.

---

## 1. THE ROOT CAUSE OF EVERY INVISIBLE CHARACTER: import unit scale

The single most important finding of this pass, reached after ~10 diagnostic
cycles (bounds forensics, OwnerNoSee, fixed-skel-bounds, scaled-skeleton
re-exports, Blender action-slot corruption, structural FBX diffs, bare-spawn
A/B tests):

> **Character FBX imported through our automation context lands at 1/100
> scale.** The proven July-4 file re-imported today produced `bounds_r=1.0`
> (invisible, frustum-culled); the same file with
> `import_uniform_scale=100.0` produced `bounds_r=100.6` — byte-identical to
> the original working import.

**Fix** (both production pipelines — `Scripts/import_art_pass_v3_v4.py`,
`Scripts/import_art_pass_v4_hero_world.py`):

```python
o.skeletal_mesh_import_data.set_editor_property("import_uniform_scale", 100.0)
```

After re-import: all 5 CharactersV3 + SurvivorKennyV4 hero validate (meshes,
physics assets ≥6 analytic bodies, all anims) and **render in game** — the
hero (green fatigues, red beard) is visible in every non-FP perspective.
`/Game/DiagReimport` diagnostic assets deleted after confirmation.

**Costume note:** the silver "robot" figures are the V2/V3 suit design; the
green-fatigues soldier is the V4 hero — if a silver body ever shows up where
the hero belongs, presentation wiring (not import) is the suspect.

## 2. The second invisible-character bomb: SKM_Manny "first-person arms"

`FirstPersonArmsMesh` carried a **full SKM_Manny body** hung under the FP
camera (head directly in the lens). It looked harmless for weeks only because
the import-scale bug culled it invisible; the moment imports were fixed, a
chrome skull filled every first-person frame (cycle-12 review). BeginPlay now
**parks the component** (`SetSkeletalMesh(nullptr)`, hidden): first person is
weapon-only until a dedicated arms rig ships.
`verify_player_first_person_animation_slice_pass.py` migrated to pin the new
contract (plus the stale pre-hero body-visibility pin and the July-7 top-down
boom pin 1150→820 in `verify_gameplay_fixes_2026_07_07.py`).

## 3. Perspectives (Kenny item a)

`ApplyCameraPerspective` retune (all six):
FP `0`; over-shoulder `300/(0,55,55)`; far chase `480/(0,75,70)`;
top-down `820/-82°` (inside the street canyon); iso `900/-54°+45°`;
side `780/(0,0,130)/-8°`. Probe always on; fixed cams (3–5) drop pawn
control rotation. Verified by screenshot in all six perspectives — hero
readable, no speck, no roof-riding.

## 4. Aim-down-sights + scope zoom (Kenny item c)

- **Aim before fire**: RMB (`BeginAim`) sights in without firing — full
  crosshair, zoom, and steadied view first; LMB fires only when chosen.
- **Zoom ladder**: `Z` cycles **1x → 2x → 5x → 10x → 20x → 50x**
  (`GScopeZoomLadder`) on scope weapons (Rifle, PrecisionRifle,
  SemiAutoRifle, BoltLauncher, RocketLauncher). FOV = `Base/zoom`
  (clamped ≥1.6°), blended at 9/s; look input scaled by `1/zoom^0.82`;
  HUD shows `[ + ] Nx`.
- **Down-the-barrel view**: per-weapon sight-line heights (pistols 10,
  shotguns 8.7, SMG 9, rifles 12.8, bolt 8.4, rocket 14) drop the viewmodel
  so the camera looks along the barrel/optic at 1x–2x; at ≥5x the weapon
  body hides and the view is *through* the scope (magnified image + reticle).
- **Sharp optics**: DoF f-stop 32 / focal distance 1e6 / motion blur 0 while
  sighted (50x was unreadable mush with cinematic DoF).
- **ONE POSE WRITER**: `UpdateFirstPersonWeaponPresentation` owns viewmodel
  pose/visibility every tick (hip ↔ ADS by `ADSBlend`, bob/sway collapse
  while sighted). `UpdateADSPresentation` owns FOV/look-scale only — it
  previously also wrote poses and the later tick function stomped them.
- **Real-scale viewmodel**: legacy `Profile.Scale`/`BaseLocation` (tuned for
  the old procedural silhouettes) shrank the V4 rifle to a 2-pixel sliver —
  the updater now uses the real-scale hip poses `(58,24,-22)`/`(46,20,-18)`
  and scale 1.0, matching `RefreshFirstPersonWeapon`.

## 5. Grenade ballistics + explosion presentation (Kenny item d)

**Launch/flight** (`ThrowableActor::ConfigureGrenadePayload`):
- Cycle-11 sky-detonation root cause: the grenade spawned overlapping the
  thrower's capsule; `OnThrowableImpact` fired a ~2000 uu/s velocity-change
  impulse off the contact normal and the grenade detonated 30 m up.
  Fixes: spawn 55 uu forward (clears the capsule),
  `IgnoreActorWhenMoving(thrower)`, gravity explicitly on, linear damping
  0.01 (prediction integrates drag-free), lure self-bounce impulse gated off
  for grenade payloads, lure pulse timer + lure registration cleared
  (a live grenade is ordnance, not a zombie lure), palm-size 0.13 scale +
  olive MID + faint fuse glow.
- Verified: `[GrenadeDetonation] ... Z=10.9` **on the ground**, landing where
  the preview predicted (crosswalk by the truck).

**Aim preview** (`UpdateGrenadeArcPreview`, while aiming a grenade):
- 22 pooled arc dots (16 cm — 5.5 cm was sub-pixel) along
  `PredictProjectilePath` of the *exact* launch velocity;
- landing marker = small impact pad + **20 rim pads outlining the blast
  circle** (the filled disc at frag radius painted 14 m of street solid
  orange); markers skip within 240 uu of the camera.

**Explosion** (`PlayExplosionPresentation` — replaces the debug wire dome,
also used by the rocket area path): impact-burst VFX (3 offset bursts for
frag), movable point-light flash per type (flash-white 26k/0.65 s,
incendiary-orange 7k/2.4 s + re-ignition waves at 0.7/1.4 s, frag-amber
9.5k/0.55 s), scorch disc on the ground (12–16 s), pressure-wave camera kick
within 2.2R. `DrawDebugSphere` removed from `ApplyAreaWeaponEffect`.

## 6. Ground (Kenny item b)

Platform snapping + texture-agnostic toning shipped earlier in the pass
(elevation seam verified gone in cycle-3). Cycle-11 review showed the toned
plazas reading near-black against sunlit speckled road — tone lifted
`0.055 → (0.085, 0.090, 0.096)` so the two grounds meet without a luminance
seam. Remaining brightness split in top-down frames is the building shadow.

## 7. Review harness (how this was all verified)

`-CodeRescuePerspectiveReview` (26 stages, 0.9 s cadence): 6 perspective
shots → ADS 1x/5x/10x/50x → grenade arc → fire → explosion + aftermath, then
exits. **Screenshot staging rule (cycle-11 lesson): `RequestScreenshot`
captures at END of frame — a stage must never mutate state after requesting a
shot, or the file shows the next state.** Even stages arrange, odd stages
photograph. Diagnostic bare-asset spawns removed once the import fix was
proven (they photobombed every FP frame).

Verification chain this pass: perspective review (13 screenshots, cycles
12–14) → watchdog **VERDICT: PASS** (pins migrated, 0 REAL) → single
integrated run `-FirstLevelIntegratedAcceptanceAudit` **COMPLETE PASS — all
21 tokens** → repackage + packaged verify.

## Files touched

- `Source/CodeRescueUnreal/CodeRescueCharacter.{h,cpp}` — ADS/zoom, single
  pose writer, FP-arms park, camera retune, grenade preview + explosion
  presentation, review harness
- `Source/CodeRescueUnreal/ThrowableActor.{h,cpp}` — grenade payload physics
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp` — ground snap + tone
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp` — ADS reticle + zoom label
- `Scripts/import_art_pass_v3_v4.py`, `Scripts/import_art_pass_v4_hero_world.py`
  — **import_uniform_scale=100.0**
- `Scripts/BlenderArt/build_weapons_v5.py` — RocketLauncherV5 (+ optic)
- `Scripts/run_v5_perspective_review.sh`, `Scripts/cleanup_diag_assets.py`
- `Scripts/verify_player_first_person_animation_slice_pass.py`,
  `Scripts/verify_gameplay_fixes_2026_07_07.py` — pin migrations
