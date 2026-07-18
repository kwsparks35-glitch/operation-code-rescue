# Improvement Pass 2026-05-21 - World Major City and 50-to-1 Outbreak Pass

Author: Codex
Date: 2026-05-21 local session

## Goal

Expand Operation Code Rescue toward a global major-city virtual world and make
the outbreak density read clearly by enforcing at least 50 zombie presences for
every living person or pet presence in the active city.

## Absolute Scope Note

This pass does not create a survey-grade, block-by-block replica of every real
city on Earth. Instead, it extends the campaign catalog and upgrades the
procedural city generator so every loaded major-city stop receives a distinct,
reviewable, playable world layer with regional visual identity and explicit
outbreak population math.

## Completed Work

- Expanded the campaign catalog from the prior 342 U.S. major-city stops to
  465 total major-city stops.
- Added 123 international major-city stops, including Tokyo, Delhi, Shanghai,
  Sao Paulo, Mexico City, Cairo, Mumbai, Beijing, Dhaka, Osaka, Karachi,
  Buenos Aires, Istanbul, London, Paris, Singapore, Sydney, Dubai, Lagos,
  Nairobi, Johannesburg, Toronto, Montreal, Moscow, Kyiv, Prague, and Vienna.
- Added global region classification for:
  - East Asia Megacity Belt,
  - South Asia Monsoon Arc,
  - Southeast Asia Delta Route,
  - European Historic Core,
  - Latin America Metro Spine,
  - African Urban Relay,
  - Middle East Solar Route,
  - Oceania Harbor Ring,
  - Canadian Shield Corridor.
- Added new global art-kit categories:
  - Neon Megacity,
  - Monsoon Megacity,
  - Monsoon Port,
  - Historic Core,
  - Latin Metro,
  - African Urban Relay,
  - Middle East Solar Hub,
  - Oceania Harbor.
- Added `SpawnWorldMajorCitySignatureLayer` to every campaign city.
- The new atlas layer adds:
  - an in-world `WORLD MAJOR CITY ATLAS` identity board,
  - financial core,
  - transit spine,
  - old city,
  - relief market,
  - quarantine edge,
  - regional set dressing such as neon towers, monsoon drainage, historic
    columns, market canopies, water towers, solar shade sails, or ferry/wharf
    forms depending on the city's art kit.
- Added editor tags for the atlas layer:
  - `WorldMajorCityAtlas`,
  - `WorldDevelopment`.

## 50-to-1 Zombie Ratio Implementation

- Added `ZombieToLivingPresenceRatio`, default `50`, with clamp minimum `50`.
- Added `MaxActiveAIZombiesPerCity`, default `120`, to keep combat playable.
- Added `BackgroundHordeClusterSize`, default `10`, so non-AI horde proxies
  can represent the remainder of the required outbreak density.
- Added `EstimateLivingPresenceCountForCity`.
- Added `ComputeTargetZombiePresence`.
- Added `SpawnBackgroundHordePopulation`.
- The default active-city living count is:
  - 1 player,
  - 1 active survivor team lead when not already rescued,
  - 4 friendly NPCs,
  - 3 civilian identity-court characters,
  - 2 safehouse civilians,
  - 3 classroom/debug-lab civilians.
- Default active-city living count before companion changes: `14`.
- Default target zombie presence: `14 * 50 = 700`.
- Default active AI zombies: up to `120`.
- Default remaining background horde representation: `580` zombie presences,
  represented as `58` proxy clusters at `10` zombies per cluster.
- If a survivor is already rescued or a companion is active, the living count is
  recalculated before the zombie target is computed.
- Sandbox mode still suppresses zombie spawning so training mode remains safe.
- Night mode still raises active AI density, but never beyond the configured
  active-AI cap.
- Regular zombie IDs now use the `1000000 + CityIndex * 1000 + Slot` range to
  avoid collision with boss, dog-pack, and horde-event ID ranges.
- Boss, dog-pack, and horde-event IDs were moved to distinct large ranges:
  - boss: `2000000 + CityIndex`,
  - dog-pack: `3000000 + CityIndex * 10 + Slot`,
  - horde events: `4000000 + CityIndex * 100 + Slot`.
- New background horde actors are tagged:
  - `ZombiePopulation50To1`,
  - `BackgroundHordeProxy`,
  - `WorldDevelopment`.
- Each city receives an in-world `50:1 OUTBREAK DENSITY` board showing:
  - living presences,
  - zombie presence target,
  - active AI threats,
  - background horde proxy count.

## Files Updated

- `Source/CodeRescueUnreal/CodeRescueCampaign.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Run_Character_World_Demo.command`
- `progress.md`
- `Documentation/UNREAL_ACCOUNT_SAVE_HANDOFF_2026-05-20.md`

## Demo Review Checklist

Open:

`/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/Run_Character_World_Demo.command`

Review:

1. Open the game and confirm the launcher announces the world-major-city atlas
   and 50-to-1 outbreak layer.
2. In the active city, find the `WORLD MAJOR CITY ATLAS` board near the north
   city edge.
3. Inspect the atlas district masses: financial core, transit spine, old city,
   relief market, and quarantine edge.
4. Review the regional set dressing for the active city's art kit.
5. Find the `50:1 OUTBREAK DENSITY` board.
6. Confirm the board displays living presences, zombie target, active AI count,
   and background proxy count.
7. Inspect city edges for tagged background horde proxy silhouettes.
8. Fight active AI zombies in the city center and verify density feels heavier
   without turning the whole city into an unplayable spawn pile.

## Verification

Commands run from:

`/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix`

```bash
ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)"
"$ENGINE_ROOT/Build/BatchFiles/Mac/Build.sh" CodeRescueUnrealEditor Mac Development -Project="$(pwd)/CodeRescueUnreal.uproject" -WaitMutex
"$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" -run=pythonscript -script="$(pwd)/Scripts/verify_character_world_assets.py" -unattended -NoSound -NullRHI -NoLoadStartupPackages -log
"$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" -game -NullRHI -NoSound -Unattended -NoRadioVoice -ExecCmds="Quit" -log
```

Results:

- Editor build: succeeded.
- Character/world asset verification: succeeded with 0 errors.
- Headless runtime smoke: exited cleanly with code 0.
- Remaining warnings are the same known optional bridge/cvar warnings from
  prior passes.

## Future Production Notes

- The system now spans a larger global city catalog and gives each active city
  richer identity. A true real-world digital twin would require GIS data,
  licensed satellite/photogrammetry assets, and a much larger art pipeline.
- The 50-to-1 ratio is implemented as total zombie presence, split into capped
  active AI plus background horde proxies for performance and playability.
