# Item 3 — Death montage on `ApplyRescueDamage`

**Status:** DONE (code) — plays automatically once a `DeathMontage` soft
ref is wired into a row of `DT_ZombieVariants`. With no montage assigned
the actor falls back to a 2-second timer-driven destroy so primitive-cube
zombies don't insta-vanish either.

## What landed

`FZombieVariantRow` now has:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite)
TSoftObjectPtr<UAnimMontage> DeathMontage;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
TSoftObjectPtr<USoundBase> DeathCue;
```

`ACodeZombieActor::ApplyRescueDamage` is rewritten to:

1. Early-out if `bIsDying` so a corpse-in-progress doesn't restart the
   pipeline when a stray bullet still hits it.
2. On Health ≤ 0:
   - Set `bIsDying = true`.
   - Clear the ambient growl timer.
   - Spawn `DeathVFX` if assigned.
   - Play `DeathCue` via `UGameplayStatics::PlaySoundAtLocation`.
   - Persist + save-game IMMEDIATELY (so the zombie stays dead even if
     the player quits during the death animation).
   - Play `DeathMontage` on the skeletal mesh's anim instance and read
     the returned playback length (`Montage_Play` returns the duration).
   - Disable the body's collision so the dying corpse doesn't keep
     blocking the player.
   - Schedule `Destroy()` via `FTimerHandle DeathDestroyTimer` after
     `max(montage length, 0.5)` seconds (or 2.0 sec if no montage).

## Files touched

- `Source/CodeRescueUnreal/CodeRescueTypes.h` — `FZombieVariantRow` fields
- `Source/CodeRescueUnreal/CodeZombieActor.h` —
  `bIsDying`, `DeathDestroyTimer`, cached `DeathMontage` + `DeathCue` ptrs
- `Source/CodeRescueUnreal/CodeZombieActor.cpp` —
  `ApplyRescueDamage` rewrite; `InitializeFromVariant` resolves soft refs

## How to verify

1. Open `/Game/CodeRescueAssets/DT_ZombieVariants` in Content Browser.
2. Pick a row (e.g. UrbanZombie4) and drag a death animation montage
   from `/Game/UrbanZombie4/Animations/` into the **Death Montage** field.
   The Andryuha1981 pack ships several death anims; pick any.
3. Save the data table.
4. PIE, find a zombie of that variant, shoot it. The mesh should play
   the death animation and remain visible for ~2 sec before despawning.

## Authoring guidance

- Death montages should typically be 1.5–2.5 seconds. Anything shorter
  feels abrupt; anything longer leaves dead zombies cluttering the
  scene.
- Pair with `DeathVFX` (Niagara blood spray) for a satisfying hit-confirm
  beat. The variant struct already has `DeathVFX` carried over from the
  pre-roadmap actor design.
