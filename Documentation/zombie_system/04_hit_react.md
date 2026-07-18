# Item 4 — Hit-react flinch

**Status:** DONE (code) — plays automatically once a `HitReactMontage` is
wired into a row.

## What landed

`FZombieVariantRow.HitReactMontage` (TSoftObjectPtr<UAnimMontage>).

Plays in `ACodeZombieActor::ApplyRescueDamage` on any non-fatal hit, after
applying damage and spawning the existing `HitVFX`. Won't play on the
fatal blow (the death montage handles that). Won't play if the row's
HitReactMontage is unset (graceful fallback).

```cpp
if (Health > 0.0f && HitReactMontage && SkeletalBody && SkeletalBody->GetAnimInstance())
{
    SkeletalBody->GetAnimInstance()->Montage_Play(HitReactMontage);
}
```

## Why it matters

Without a flinch, the only feedback the player gets that a shot landed
is the `HitVFX` decal and the on-screen "Direct hit" text. Adding a
mesh-level flinch pose helps players track which zombie they damaged in
a busy scene and accelerates kill-confirm reads.

## Files touched

- `Source/CodeRescueUnreal/CodeRescueTypes.h` — `HitReactMontage` field
- `Source/CodeRescueUnreal/CodeZombieActor.h` — cached pointer
- `Source/CodeRescueUnreal/CodeZombieActor.cpp` —
  `ApplyRescueDamage` body; `InitializeFromVariant` resolution

## Authoring guidance

- Hit reacts should be 0.25–0.5 sec — long enough to read, short enough
  not to interrupt the chase.
- Most marketplace zombie packs ship a `Hit_Front` / `Hit_Back` /
  `Hit_Left` / `Hit_Right` set. For now we just play whatever's assigned;
  picking by impact-direction is a future polish item.
