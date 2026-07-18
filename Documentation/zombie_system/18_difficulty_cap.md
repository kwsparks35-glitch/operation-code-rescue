# Item 18 — Difficulty rebalance + variant multiplier cap

**Status:** DONE — variant Health and Damage multipliers are clamped at
runtime so they can't compound with EGameDifficulty into 200-HP
zombies.

## The problem

Stat math at spawn:

```
ZombieHealth = (zone-base) × DifficultyHealthMul × VariantHealthMul
```

Pre-roadmap values:

| Difficulty | DifficultyHealthMul | DifficultyDamageMul |
|---|---|---|
| Easy | 0.65 | 0.6 |
| Normal | 1.0 | 1.0 |
| Hard | 1.6 | 1.75 |

A BloatedFemale variant (HealthMul 1.6 in the data table) on Hard would
deliver `1.6 × 1.6 = 2.56× base health`. Anchorage zone-base is 55, so
that's 140 HP per zombie — a magazine to drop.

## The fix

`ACodeZombieActor::InitializeFromVariant` now clamps the variant
multipliers BEFORE applying them:

```cpp
const float ClampedHealthMul = FMath::Clamp(Row.HealthMultiplier, 0.4f, 1.30f);
const float ClampedDamageMul = FMath::Clamp(Row.DamageMultiplier, 0.5f, 1.40f);
Health       *= ClampedHealthMul;
AttackDamage *= ClampedDamageMul;
MoveSpeed    *= Row.SpeedMultiplier;  // intentionally NOT clamped
```

Speed is uncapped. Fast variants like DogZombie (1.45× speed) should
*genuinely* feel fast, even on Hard. Health and damage are the stats
that turn fights tedious.

## Worst case after the cap

Hard + BloatedFemale, Anchorage:

```
55 × 1.6 (difficulty) × 1.30 (clamped) = 114 HP
```

114 HP at 50 dmg per shot = 3 shots — feels like a "tank zombie",
not a sponge.

Damage on Hard + BloatedFemale:

```
8 × 1.75 × 1.40 = 19.6 damage per hit
```

About 5 hits to kill the player from full Health (100). Tense, fair.

## Why the design intent in the data table is preserved at 1.6

Designers can still SEE the row `HealthMultiplier = 1.6` and read it as
"this variant should feel ~60% beefier than baseline." The clamp is a
runtime-only safeguard against the multiplicative blowup. If a future
balance pass lowers the difficulty multipliers, the variant can use up
to its full design intent.

## Files touched

- `Source/CodeRescueUnreal/CodeZombieActor.cpp` — `InitializeFromVariant`
  clamp logic.

## Acceptance

The earlier 2026-04-29 PIE confirmed the math works correctly:
`Health 46.75 = 55 × 0.85 (BaseMesh's HealthMultiplier)`. After this
clamp, a `Row.HealthMultiplier = 1.6` zombie on Normal difficulty
yields `Health = 55 × 1.0 × min(1.6, 1.30) = 71.5` — exactly 1.30×
not the data table's 1.60×.

## Tuning knobs

If you want softer or harder caps, edit the literals:

```cpp
const float ClampedHealthMul = FMath::Clamp(Row.HealthMultiplier, 0.4f, 1.30f);
const float ClampedDamageMul = FMath::Clamp(Row.DamageMultiplier, 0.5f, 1.40f);
```

The lower bound (0.4 / 0.5) prevents a misconfigured row from giving
zombies near-zero stats.
