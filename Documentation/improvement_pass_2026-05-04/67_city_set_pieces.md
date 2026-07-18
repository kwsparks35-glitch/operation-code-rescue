# #67 — Themed city set-pieces

Every city now spawns one of five themed set-pieces in its southeast
quadrant, picked deterministically by `CityIndex % 5`. The same city
always has the same theme across reloads.

| ThemeIndex | Theme              | Visual / behavior                           |
| ---------- | ------------------ | ------------------------------------------- |
| 0          | Lab Vault          | 4 stacked emissive crates on a pedestal     |
| 1          | Radio Tower        | tall amber spire with a horizontal beam     |
| 2          | Dog-Pack Den       | ring of stones + 3 fast `DogZombie`-variant zombies (skipped in sandbox) |
| 3          | Hospital Triage    | 4 cot blocks + a free Medkit pickup         |
| 4          | Drone Wreckage     | hull, two wings, glowing green core         |

The Dog-Pack Den is the gameplay-flavored one — the three "dog" zombies
get `DogZombie` variant (the PxItiger pack mesh) and a higher MoveSpeed
of 290 so they read as fast attackers rather than regular shamblers.
Their IDs use a separate range (`300000 + CityIndex * 10 + i`) so the
saved-neutralized list tracks them independently.

## Why deterministic-by-index
Players who replay a city should see the same visual landmark so the
city has its own identity, but a fully random pick across runs would
defeat that. `CityIndex % 5` is the simplest stable mapping.

## Files
- `Source/CodeRescueUnreal/CodeRescueGameMode.{h,cpp}` —
  `SpawnSetPieceForCity()` declaration + implementation; called from
  `SpawnCampaignCity` add-ons block.
