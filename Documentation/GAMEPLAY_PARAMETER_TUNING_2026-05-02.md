# Gameplay Parameter Tuning Pass - 2026-05-02

## Goal

Improve the game across its major runtime parameters without changing the campaign structure or replacing the existing procedural systems. This pass makes more values designer-tunable, improves resource fairness, aligns pickup behavior with HUD prompts, and persists the player's live resource state.

## Player Parameters

`ACodeRescueCharacter` now exposes these runtime tuning groups to C++ defaults, Blueprint subclasses, and editor details panels.

### Stats

- `MaxAmmo`: caps carried ammo when supply clamping is enabled. Default: `300`.
- `MaxMedkits`: caps carried medkit charges when supply clamping is enabled. Default: `12`.
- `bClampSuppliesToMaximum`: prevents pickups from pushing carried supplies past max values. Default: `true`.

### Movement

- `WalkSpeed`: player ground speed. Default: `9000`.
- `BrakingDeceleration`: walking stop response. Default: `18000`.
- `DirectKeyboardTurnRate`: keyboard/controller yaw rate for polled direct input. Default: `135`.
- `DirectKeyboardLookRate`: keyboard pitch rate for polled direct input. Default: `75`.

### Interaction

- `InteractionTraceDistance`: forward trace reach for direct interaction. Default: `3000`.
- `InteractionAssistRadius`: fallback nearest-interactable radius. Default: `9000`.

### Combat

- `FireRefireDelay`: shot cadence gate. Default: `0.15`.
- `WeaponRange`: weapon trace range. Default: `30000`.
- `DirectHitDamage`: damage on a direct trace hit. Default: `50`.
- `AssistedHitDamage`: damage from assisted aiming. Default: `35`.
- `AssistedHitRadius`: max assisted-hit acquisition distance. Default: `12000`.
- `AssistedHitMaxAngleDegrees`: aim cone for assisted hits. Default: `18`.
- `bEnableAssistedHit`: enables or disables assisted-hit behavior. Default: `true`.

### Healing

- `MedkitHealAmount`: health restored per medkit. Default: `35`.

## Pickup Improvements

`APickupActor` now has a public `Collect(ACodeRescueCharacter*)` function. Pickups can be collected in two ways:

- Walking through the pickup trigger.
- Looking at the pickup and pressing the interaction key.

Pickup collection now goes through the character's `AddAmmo` and `AddMedkits` functions, which respect max supply caps. If the player is already full, the pickup remains in the world and shows a clear message instead of disappearing without benefit.

The HUD prompt was also updated from "grab supplies" to "pick up supplies", and its trace reach now uses the same interaction distance as gameplay.

## Save-State Improvements

Save files now persist live player resources:

- `PlayerHealth`
- `PlayerAmmo`
- `PlayerMedkits`
- `bHasPlayerResources`

`bHasPlayerResources` protects older saves. Saves created before this pass will not overwrite the character's default health/ammo/medkits with newly-added default fields.

The save version string is now:

```text
0.6.0-player-resource-state
```

## Campaign And Encounter Parameters

`ACodeRescueGameMode` now exposes more campaign and encounter values as named parameters.

### Campaign Span

- `FCodeRescueCampaign::GetCitySpanScale()`: shared horizontal city span scale. Default: `50.0`.
- `FCodeRescueCampaign::ScaleCityOffset(...)`: scales city-local X/Y offsets while preserving vertical placement.
- `FCodeRescueCampaign::ScaleCityExtent(...)`: scales city-local X/Y block extents while preserving vertical size.

The 50x scale is applied to city origins, player start offsets, city floors, perimeter guide rails, entry pads, navigation bounds, guide text placement, skyline placement, landmarks, art-kit geometry, language stations, terminals, survivors, pickups, and hostile spawn locations. City origin spacing scales with the same value so the 342-city grid stays separated and nearest-city guidance remains stable.

### Campaign Visual Density

- `CityBuildingBaseCount`: base skyline blocks per city. Default: `8`.
- `CityBuildingPerDifficultyTier`: additional skyline blocks per city difficulty tier. Default: `3`.
- `CityBuildingHeightTierBonus`: building height increase per tier. Default: `0.55`.

### Pickup Distribution

- `AmmoPickupBaseAmount`: base ammo per city pickup. Default: `35`.
- `AmmoPickupCityCycleBonus`: extra ammo per cycle step. Default: `5`.
- `AmmoPickupCityCycleLength`: cycle length for ammo bonus. Default: `4`.
- `MedkitPickupCityInterval`: how often cities spawn medkit pickups. Default: `2`.
- `MedkitPickupAmount`: medkit charges per pickup. Default: `1`.

### Encounter Balance

- `ZombieBaseCount`: base hostile count. Default: `2`.
- `ZombieCountTierDivisor`: difficulty-tier divisor for additional hostiles. Default: `2`.
- `ZombieMinCount`: minimum spawned hostiles. Default: `2`.
- `ZombieMaxCount`: maximum spawned hostiles. Default: `4`.
- `ZombieBaseHealth`: base hostile health. Default: `42`.
- `ZombieHealthPerDifficultyTier`: health added per difficulty tier. Default: `8`.
- `ZombieHealthCityCycleBonus`: city-cycle health variation. Default: `3`.
- `ZombieHealthCityCycleLength`: city-cycle length for health/speed variation. Default: `5`.
- `ZombieBaseAttackDamage`: base attack damage. Default: `8`.
- `ZombieAttackDamagePerDifficultyTier`: damage added per difficulty tier. Default: `0.35`.
- `ZombieBaseMoveSpeed`: base movement speed. Default: `78`.
- `ZombieMoveSpeedPerDifficultyTier`: movement speed added per tier. Default: `7`.
- `ZombieMoveSpeedCityCycleBonus`: city-cycle speed variation. Default: `6`.
- `ZombieAttackRange`: melee attack range. Default: `130`.
- `ZombieBaseActivationRange`: base player detection range. Default: `5600`.
- `ZombieActivationRangePerDifficultyTier`: detection range added per tier. Default: `550`.
- `MinEncounterIntensityScale`: lower clamp for mission encounter intensity. Default: `0.9`.
- `MaxEncounterIntensityScale`: upper clamp for mission encounter intensity. Default: `1.65`.

## Balance Notes

- Ammo pickups were increased from the previous `25 + cycle * 5` pattern to `35 + cycle * 5`. This improves early-city comfort while max ammo prevents runaway hoarding.
- Medkits now appear every 2 cities by default instead of every 3, but the medkit cap keeps long-run stockpiles controlled.
- Assisted fire now uses an aim cone instead of damaging the nearest hostile anywhere in range. This keeps the accessibility help but requires the player to aim near the target.
- Direct hits still do more damage than assisted hits, so accurate aiming remains rewarded.
- Movement, interaction, and weapon ranges were raised after the 50x city span change so the expanded layout remains playable rather than becoming a slow commute.

## QA Focus

Manual playtest should confirm:

- Pickups can be collected by overlap and by pressing interact while looking at them.
- Full ammo or medkit inventory does not consume pickups.
- Saving and loading preserves health, ammo, medkits, position, objective index, and campaign progress.
- Assisted fire helps when aiming near a hostile but does not hit targets behind or far outside the crosshair.
- Early-city supplies feel generous enough for onboarding.
- Hard difficulty remains survivable but meaningfully more dangerous.
- The 50x city span is visible in city boundaries, landmarks, skyline placement, and art-kit geometry.
- Expanded city traversal remains controllable at the updated movement speed.
