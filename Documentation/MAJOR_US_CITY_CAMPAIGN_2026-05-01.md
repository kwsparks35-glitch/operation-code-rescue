# Major U.S. City Campaign Update

## Scope

This pass replaces the old small-zone progression model with a city-by-city campaign. Each city is treated as an independent mission location with its own terminal, coding lesson, survivor objective, unlock state, and HUD/journal row.

For this implementation, "major city" is defined as every incorporated/CDP place at or above 100,000 residents in the U.S. Census Vintage 2024 city ranking. That yields 342 campaign cities. The source list is the Census annual city/town population estimate table:

- Census page: https://www.census.gov/data/tables/time-series/demo/popest/2020s-total-cities-and-towns.html
- Direct Vintage 2024 ranking workbook: https://www2.census.gov/programs-surveys/popest/tables/2020-2024/cities/totals/SUB-IP-EST2024-ANNRNK.xlsx

The campaign is intentionally centralized in `Source/CodeRescueUnreal/CodeRescueCampaign.cpp`, so changing the major-city threshold later is a data/catalog change plus performance tuning, not a gameplay rewrite.

## Gameplay Behavior

- The player starts in New York, the first campaign city.
- Every city owns a unique terminal ID, mission title, lesson brief, and survivor team.
- A city is graduated only when its terminal is solved and its survivor team is rescued.
- Future-city terminals and survivors refuse interaction until all earlier cities are graduated.
- The `T` objective teleport now moves the player to the first incomplete city and will not skip past the active lesson gate.
- The `J` journal lists the full campaign in order, marking completed, active, unlocked, and locked cities.
- Victory now requires all campaign terminals and all campaign survivor teams, rather than the old fixed prototype counts.
- The world now uses active-city instancing: the full 342-city campaign exists in data, but only the current city hub is spawned as live actors.
- Reloading or teleporting to another city applies saved terminal, survivor, and zombie state to the newly streamed city without moving the player unexpectedly.

## Lesson Rotation

The campaign rotates six lesson families to keep the player moving through different programming ideas:

- Sum: function arguments, arithmetic, and return values.
- Lock: boolean conditions and gate logic.
- Reverse: string traversal and transformations.
- Palindrome: string normalization and conditional checks.
- FizzBuzz: loops, divisibility, and ordered branching.
- Even filter: arrays/lists and selection logic.

Each city also has a recommended language assigned across Java, C, Python, and MATLAB, while the existing language stations still let the player choose their practice language.

## City Identity

Each city now carries generated gameplay identity metadata:

- `RegionName`: broad U.S. route grouping used for journal context and visual tinting.
- `DistrictStyle`: a city/lesson-specific district label, such as a relay district, triage depot, or power substation.
- `LandmarkName`: a unique procedural landmark name derived from the city and landmark archetype.
- `CurriculumFocus`: city-specific concept, strategy, and recommended-language guidance.
- `RadioBriefing`: in-world mission flavor shown at the active city.
- `DifficultyTier` and `EncounterIntensity`: rank-based combat scaling.
- `SkylineSeed`: deterministic procedural layout seed so city silhouettes stay stable across sessions.
- `ArtKitName`: a profile that spawns city-type geometry such as port cranes, solar fields, mountain ridges, rail lines, or command columns.
- `HintText`, `VisibleTestBrief`, and `HiddenTestBrief`: per-city curriculum guidance shown in terminal mission text.
- `RadioVoiceName`: macOS system voice used by the runtime briefing speaker.

## Active-City Instancing

The original 342-city pass spawned every hub at startup. That proved the catalog but was too expensive for richer gameplay. The current implementation keeps a global safety floor plus the active city's live actors only.

- `ACodeRescueGameMode::EnsureCampaignCityLoaded()` destroys the previous streamed city actor set and spawns the requested city.
- `StreamedCampaignActors` tracks city-local floors, rails, labels, landmarks, stations, terminals, survivors, pickups, nav bounds, and zombies.
- `UCodeRescueGameInstance::ApplyObjectiveStateToLevel()` reapplies solved, rescued, and neutralized state after a city streams in, without teleporting the player.
- `T` now loads the first incomplete city before moving the player to that city's start point.

This is a procedural equivalent to level streaming/World Partition for the current C++-generated prototype.

## Radio and Lesson Variation

- `Content/CodeRescueData/radio_briefings.tsv` contains all 342 generated radio lines.
- `Scripts/generate_radio_voiceovers.py` can turn those rows into WAV files using macOS `say` and `afconvert`.
- `Content/CodeRescueAssets/Audio/RadioSamples/new_york_radio_briefing.wav` is a sample generated clip.
- The runtime also speaks briefings directly through `/usr/bin/say` on macOS when each city streams in.
- Compiler-backed validators use city-seeded hidden tests so repeated lesson families vary by terminal ID.

## Enemy and Difficulty Improvements

- Zombie count increases by city difficulty tier, capped for readability.
- Zombie health, speed, activation range, and encounter pressure scale from `DifficultyTier` and `EncounterIntensity`.
- The zombie variant table is reconnected to the campaign spawner. Variant rows are selected by exact city index when present, otherwise by a three-bucket city theme fallback.
- Variant assignments are still saved by `ZombieId`, so reloads keep the same zombie lineup.

## Source Changes

- `Source/CodeRescueUnreal/CodeRescueCampaign.h`
- `Source/CodeRescueUnreal/CodeRescueCampaign.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueObjectiveJournalWidget.h`
- `Source/CodeRescueUnreal/CodeRescueObjectiveJournalWidget.cpp`
- `Source/CodeRescueUnreal/CodingTerminalActor.h`
- `Source/CodeRescueUnreal/SurvivorActor.h`
- `Source/CodeRescueUnreal/CodeZombieActor.h`
- `Source/CodeRescueUnreal/CodeZombieActor.cpp`

## Remaining Polish

- Replace procedural landmarks with hand-built or licensed city art over time.
- Add real voice/audio files for radio briefings. The text briefing system is ready.
- Add authored lesson variants if you want every repeated lesson family to have different hidden tests and hints, not just different city context.
- Run a full PIE playthrough across several city transitions after packaging settings are finalized.
