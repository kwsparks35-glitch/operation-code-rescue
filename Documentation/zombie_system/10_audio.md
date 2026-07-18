# Item 10 — Audio cues (variant + ambient)

**Status:** DONE (code) — plumbing for per-variant growls, attacks, and
death cues plus player fire / hit-confirm / dry-fire cues. Actual
`USoundBase` assets need to be authored or imported and wired into rows
+ the player Blueprint.

## What landed

### Per-zombie audio

`FZombieVariantRow` now carries:

```cpp
TSoftObjectPtr<USoundBase> GrowlCue;     // ambient
TSoftObjectPtr<USoundBase> AttackCue;    // on-attack
TSoftObjectPtr<USoundBase> DeathCue;     // on-death
```

`ACodeZombieActor` now has a persistent `UAudioComponent* GrowlAudio`
attached to the Body. On `BeginPlay` it calls `ScheduleNextGrowl()`
which uses `FTimerHandle GrowlTimer` to fire at random 6–14 sec
intervals while `bIsDying` is false. Each fire calls
`GrowlAudio->SetSound(GrowlCue); GrowlAudio->Play();` and re-schedules.

`AttackCue` plays in `Tick` when an attack lands.
`DeathCue` plays in `ApplyRescueDamage` when Health hits 0.

All three early-out gracefully when the soft ref is null, so variants
without audio assigned still work.

### Per-player audio

`ACodeRescueCharacter` now exposes:

```cpp
USoundBase* FireCue;          // each shot
USoundBase* HitConfirmCue;    // played at the impact point on a zombie hit
USoundBase* DryFireCue;       // when out of ammo
```

`Fire()` plays them at the appropriate moments.

## Files touched

- `Source/CodeRescueUnreal/CodeRescueTypes.h` — `FZombieVariantRow`
  cue fields + forward decl `class USoundBase;`
- `Source/CodeRescueUnreal/CodeZombieActor.h` —
  `UAudioComponent* GrowlAudio`, cached cue ptrs, `FTimerHandle GrowlTimer`,
  `ScheduleNextGrowl()`
- `Source/CodeRescueUnreal/CodeZombieActor.cpp` — constructor creates
  GrowlAudio; BeginPlay schedules growls; Tick plays attacks; ApplyRescueDamage
  plays death; InitializeFromVariant resolves soft refs
- `Source/CodeRescueUnreal/CodeRescueCharacter.h` — three cue fields
  + `FireRefireDelay`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp` — Fire() plays cues

## How to author the actual audio

Quickest path:

1. Find royalty-free sources (Freesound.org's `cc0` / `cc-by` filters,
   or a paid pack). You need:
   - 4–6 short growl loops (~1.5–3 sec each) — variation is good
   - 2–3 attack swing/impact sounds (~0.5 sec)
   - 2–3 death moans (~1.5 sec)
   - 1 firearm shot, 1 metal impact (hit confirm), 1 dry-fire click
2. Import into UE: drag the .wav files onto Content Browser
   `/Game/CodeRescueAssets/Audio/`. UE creates `USoundWave` assets.
3. For the growls: create a `SoundCue` that randomly picks one of the
   waves with `Random` node, so each growl trigger varies. Save as
   `SC_ZombieGrowl_Generic` (or per-variant if you want pack-specific).
4. In `DT_ZombieVariants`, assign the cues to each variant's
   `GrowlCue`/`AttackCue`/`DeathCue` columns.
5. On the player BP (or `ACodeRescueCharacter` defaults in editor),
   assign `FireCue`/`HitConfirmCue`/`DryFireCue`.

## Acceptance

PIE; walk near a zombie. You should hear:

- Periodic growls from nearby zombies (3D-attenuated by
  `UAudioComponent`'s default settings — distant zombies are quieter).
- A swing sound the moment a zombie commits an attack against you.
- A moan when a zombie dies.
- Each shot fires a gun sound; hits play a metal-impact at the impact
  point; dry-fire clicks when you're out.

If anything is silent, check `Content/CodeRescueAssets/Audio/` for the
soundwave asset and verify the soft ref in the row points at it.
