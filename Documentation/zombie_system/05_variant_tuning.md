# Item 5 — Per-variant tuning pass

**Status:** DONE — `Scripts/build_zombie_variants_table.py` `PACK_CONFIG`
rebalanced; rerun the script in editor to regenerate `DT_ZombieVariants`.

## Tuning model recap

Each row in `DT_ZombieVariants` carries:

- `HealthMultiplier` (clamped 0.4–1.30 at runtime, see item 18)
- `DamageMultiplier` (clamped 0.5–1.40 at runtime, see item 18)
- `SpeedMultiplier` (uncapped — fast variants should genuinely feel fast)
- `MeshScale` — visual size adjustment
- `ZoneWeights` — per-location spawn weight. The major-city campaign first
  checks an exact city index. If no exact entry exists, it falls back to
  `CityIndex % 3` so the older three-bucket tuning still produces variety
  across the 342-city campaign.

Effective stat = `(city-base) x (campaign difficulty/intensity) x (difficulty-mul) x (clamped variant mul)`.

## Per-pack rationale

| Variant       | H mul | D mul | S mul | Scale | Anchorage | Seattle | Tokyo | Theme |
|---------------|-------|-------|-------|-------|-----------|---------|-------|-------|
| DogZombie     | 0.55  | 0.7   | 1.45  | 0.55  | 0.6       | **1.6** | 0.6   | Fast, light, heavy in Seattle Harbor |
| UrbanZombie4  | 1.0   | 1.0   | 1.0   | 1.0   | **1.8**   | 1.0     | 0.9   | Workhorse humanoid; clinical fits Anchorage Medical |
| BusinessSuit  | 1.1   | 1.05  | 0.95  | 1.0   | 0.4       | **1.6** | **1.4** | Yarrawah M04 — office worker silhouette suits Seattle/Tokyo evac |
| BloatedFemale | 1.6 ⚠ | 1.25 ⚠ | 0.7   | 1.10  | 0.6       | 1.0     | **1.7** | Yarrawah F01 — tank archetype; ramps difficulty in final zone |
| NurseFemale   | 0.95  | 1.0   | 1.05  | 1.0   | **1.4**   | 0.9     | 0.7   | Medical-themed; fits Anchorage |
| BaseMesh      | 0.85  | 0.7   | 0.85  | 1.0   | 0.05      | 0.05    | 0.05  | rivai pack — NO ANIMATIONS, kept as 1% T-pose curiosity |

⚠ The BloatedFemale 1.6 / 1.25 multipliers are pre-clamp design intent;
runtime they're reduced by `InitializeFromVariant` to ≤1.30 / ≤1.40 (item 18).

## Why the BaseMesh weight is so low

The rivai pack ships only a base mesh — no skeleton-driven animations. A
BaseMesh zombie therefore stands in T-pose and slides toward the player.
Setting `ZoneWeights` to 0.05 per bucket (with other variants summing to
~3–5 per bucket) means BaseMesh shows up roughly 1% of spawns, as a rare
"this one's different" moment rather than a T-pose plague.

If you ever want to remove BaseMesh entirely, set all three of its
weights to 0.0 — the picker safely ignores zero-weight options.

## Files touched

- `Scripts/build_zombie_variants_table.py` — `PACK_CONFIG` numbers

## How to verify

1. PIE; open Outliner, search `CodeZombie`, click through several rows.
2. Variant distribution should roughly match the weights above. With active
   city instancing, test by moving through several cities with `T`; each city
   picks by exact city index or its `CityIndex % 3` fallback bucket.
3. Check Health/MoveSpeed/AttackDamage in the Details panel — they
   should equal `(zone base) × (clamped variant mul)`.

## Caveat: save-game persistence

Variant assignments are persisted by `ZombieId` in the SaveGame. Tuning
changes only take effect on a fresh run (call **Restart Fresh** from
the death widget, or delete `Saved/SaveGames/OperationCodeRescue_Profile0.sav`
manually). Existing saves keep their old variant mapping for continuity.
