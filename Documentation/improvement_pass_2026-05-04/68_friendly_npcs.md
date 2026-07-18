# #68 — Ambient friendly NPCs (Engineer / Medic / Scientist / Trader)

Adds a brand-new actor class `AFriendlyNPCActor` plus a per-city spawn
helper, so every city has four non-rescue NPCs gathered around the
language-station plaza. Each one offers a small daily perk on Interact.

| Role        | Color tint  | Interact effect                                    |
| ----------- | ----------- | -------------------------------------------------- |
| Engineer    | amber       | +1 Scrap (free)                                    |
| Medic       | red         | +25 Health (free; only if below max)               |
| Scientist   | lab blue    | +1 ResearchPoint (free)                            |
| Trader      | ledger green| -5 Scrap, +1 ResearchPoint (one-time per visit)    |

Each NPC's perk is **once per day cycle**. When the GameMode flips
`bIsNight` (#35) at sunset/sunrise, every `AFriendlyNPCActor` in the
world has its `bPerkUsedThisDay` flag reset, so the player can return
the next day for another grant. NPCs whose perk is unavailable
(Medic at full HP, Trader without 5 scrap) speak a refusal subtitle
and don't burn the cooldown.

## Visual design
Procedural cube body + sphere head + per-role tinted point light, same
silhouette pattern used by `ASurvivorActor`. A `ProfessionalNPCMesh`
slot is exposed so a Blueprint subclass can swap to MetaHuman — same
pattern as the survivor's `ProfessionalSurvivorMesh`.

## Spawn pattern
`SpawnFriendlyNPCsForCity(CityIndex, Origin, CityLabel)` places all
four NPCs at small offsets around the language-station plaza
(`(-2400 to -1900, -1700 to -1300)` local). Names rotate
deterministically off `CityIndex` from a 16-name first-name pool, so
each city has a stable line-up across reloads.

## Player Interact dispatch
`ACodeRescueCharacter::IsInteractableActor` and `Interact()` both gained
a new `Cast<AFriendlyNPCActor>` branch. When the player presses E with
an NPC in their crosshair (or within the assist radius), the NPC's
`Interact()` runs and surfaces its result via subtitle + on-screen
debug message.

## Files added
- `Source/CodeRescueUnreal/FriendlyNPCActor.h` (91 lines)
- `Source/CodeRescueUnreal/FriendlyNPCActor.cpp` (234 lines)

## Files modified
- `Source/CodeRescueUnreal/CodeRescueGameMode.{h,cpp}` —
  `SpawnFriendlyNPCsForCity` declaration, implementation, call site,
  day-flip reset loop.
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp` — interact include
  + dispatch.

## Gameplay rationale
Before this pass the player only ever encountered survivors-to-rescue,
zombies-to-shoot, and stations/terminals. The cities felt depopulated.
Adding four named NPCs per city with small perks turns the
language-station plaza into a recognizable "safe hub" the player can
plan around — an Engineer trickle for steady Scrap, a Medic for
emergency top-ups, a Scientist for slow Research progress, and a Trader
for converting surplus Scrap to skill-tree currency.

The design intentionally avoids combat dependency: NPCs are not
escorts, not killable by zombies, and don't open a quest chain. They're
ambient improvement to the world's *texture*, which lets the next pass
focus on real curriculum content without needing a quest system first.
