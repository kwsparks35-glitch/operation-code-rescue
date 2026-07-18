# Item 11 — Player weapon system

**Status:** DONE — fire-rate gating, muzzle-flash VFX, hit-impact VFX,
fire/hit/dry-fire audio cues, ammo decrement, and (existing) line-trace
hit detection are all wired. PIE-tested with the existing rebuild.

## What landed

`ACodeRescueCharacter::Fire()` now:

1. Gates by `TimeSinceLastFire < FireRefireDelay` (default 0.15 sec).
   Prevents the polled-key Fire from emptying the magazine in one frame.
2. On out-of-ammo: plays `DryFireCue`, shows red on-screen warning.
3. On valid shot:
   - Decrements ammo, resets `TimeSinceLastFire`.
   - Spawns `MuzzleFlashVFX` attached to the camera muzzle.
   - Plays `FireCue` at camera location.
   - Forward line-traces 7000 units.
   - On `ACodeZombieActor` hit: applies 50 dmg, spawns `BulletImpactVFX`
     at the hit point, plays `HitConfirmCue` at the hit point.
   - On non-zombie hit: still spawns `BulletImpactVFX` so the player
     gets feedback that the trace landed somewhere.
4. (Existing fallback) If no direct trace hit, picks the nearest zombie
   within 4500 units and applies 40 dmg — kept as a "first-time
   shooter assistance" feature; can be tuned out later.

## New properties on `ACodeRescueCharacter`

```cpp
USoundBase* FireCue;
USoundBase* HitConfirmCue;
USoundBase* DryFireCue;
float FireRefireDelay = 0.15f;
float TimeSinceLastFire = 99.0f;
```

`Tick()` increments `TimeSinceLastFire += DeltaSeconds` so the gate
opens.

## Files touched

- `Source/CodeRescueUnreal/CodeRescueCharacter.h` — new audio properties,
  fire-rate, time tracker
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp` —
  `Fire()` rewrite with VFX/audio; `Tick()` increments timer; new
  `#include "NiagaraFunctionLibrary.h"` and `Sound/SoundBase.h`

## How to verify

1. PIE.
2. LMB / Space / F (any of the polled fire keys) — you should:
   - Hear a fire sound (if FireCue assigned in BP defaults).
   - See a muzzle flash particle (if MuzzleFlashVFX assigned).
   - See a small impact decal at whatever you hit.
3. Hit a zombie — extra impact at the body, hit-confirm sound.
4. Spam the key — only one shot fires per ~0.15 sec.
5. Empty your ammo (start with 150) — dry-fire click.

## Tuning knobs

All exposed on the BP defaults panel under **Code Rescue | Combat** /
**Code Rescue | Audio** / **Code Rescue | VFX**:

| Property | What it does | Default |
|---|---|---|
| `Ammo` | Starting ammo | 150 |
| `FireRefireDelay` | Min seconds between shots | 0.15 |
| `MuzzleFlashVFX` | Niagara at camera muzzle | unset |
| `BulletImpactVFX` | Niagara at hit point | unset |
| `FireCue` | Shot sound | unset |
| `HitConfirmCue` | Played on zombie hit | unset |
| `DryFireCue` | Played on out-of-ammo press | unset |

Consider lowering `FireRefireDelay` to 0.08 for a "rifle" feel, or
raising to 0.6 for a "bolt-action" feel.

## Open follow-ups

- The auto-aim assist (40 dmg to nearest zombie when direct trace
  misses) is a holdover. Remove or keep behind a `bAimAssistEnabled`
  flag once playtesting shows whether players need it.
- No reload mechanic — just a single 150-round pool. If we want a mag
  system, add `MagSize`, `ReserveAmmo`, `ReloadDuration`,
  `ReloadCue` and a polled-key `R` for reload.
