# #62 — Spawn Boss Zombie + Jeep per city

`ABossZombieActor` (#51) and `AJeepActor` (#52) existed as actor classes
but were never spawned by `SpawnCampaignCity`. Two new helpers added on
`ACodeRescueGameMode`:

- `SpawnBossForCity(CityIndex, Origin, CityLabel, Mission)` — places a
  boss in the deep northeast quadrant of each non-sandbox city.
- `SpawnJeepForCity(CityIndex, Origin, CityLabel)` — parks a drivable
  jeep next to the city's helipad.

Both are called from the bottom of `SpawnCampaignCity` in the new
"improvement_pass_2026-05-04_part3 add-ons" block.

## Boss tuning

- ID range: `200000 + CityIndex` so it never collides with the regular
  zombie ID space (and the kill is persisted independently).
- HP / damage scales with `Mission.DifficultyTier` plus the global
  `GetZombieHealthMultiplier()` / `GetZombieDamageMultiplier()` from the
  difficulty system.
- Variant: tries `EliteCharger` (uses the Yarrawah BusinessSuit
  silhouette via `DT_ZombieVariants`); falls back to procedural cube if
  the row isn't authored.
- Visual marker block + emissive halo + signage stay above the boss so
  the player can identify it at distance.

## Jeep placement

Parked at the helipad's `(2400, 2400)` corner offset by `(420, -380, 80)`
on a small clearance block, rotated 90° so it points down the runway.
Driven via the existing `AJeepActor::Mount` flow when the player presses
E next to it.

## Files
- `Source/CodeRescueUnreal/CodeRescueGameMode.{h,cpp}`
- `#include "BossZombieActor.h"` and `#include "JeepActor.h"` added to
  the gamemode cpp.
