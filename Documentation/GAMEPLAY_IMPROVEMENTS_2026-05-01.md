# Gameplay Improvements Completed

## Summary

This follow-up completes the recommended gameplay improvements from the major-city campaign review. The campaign still covers the 342 Census-ranked 100,000+ resident places, but the runtime no longer keeps 342 full hubs active at once.

## Active-City Instancing

Implemented in:

- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.h`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.cpp`

Behavior:

- `ACodeRescueGameMode::EnsureCampaignCityLoaded()` is the streaming entry point.
- `StreamedCampaignActors` tracks every actor spawned for the active city.
- Moving to a new city destroys the old city-local actors and spawns the requested city.
- `T` loads the first incomplete city before teleporting the player there.
- `ApplyObjectiveStateToLevel()` reapplies solved terminals, rescued survivors, and neutralized zombies after streaming without restoring the saved player transform.

This gives the project a World Partition-like gameplay loop while preserving the procedural C++ world generation approach.

## City Identity and Interest

Implemented in:

- `Source/CodeRescueUnreal/CodeRescueCampaign.h`
- `Source/CodeRescueUnreal/CodeRescueCampaign.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Source/CodeRescueUnreal/CodeRescueObjectiveJournalWidget.cpp`

Every city now has:

- a region label
- a district style
- a unique landmark name
- an art-kit profile
- a radio briefing
- a curriculum focus
- a hint, visible-test brief, and hidden-test brief
- a recommended first-pass language
- a difficulty tier
- encounter intensity
- a deterministic skyline seed

The active city uses this metadata for signage, terminal briefing text, journal rows, procedural skyline variation, a unique landmark structure, and art-kit geometry.

## Art-Kit Profiles

Implemented in:

- `Source/CodeRescueUnreal/CodeRescueCampaign.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`

Every city receives one of these authored-style procedural profiles:

- Coastal Port
- Desert Solar Grid
- Mountain Relay
- Great Lakes Industrial
- River Lockworks
- Capital Command
- Rail Yard
- Metro Core

These profiles spawn different supporting geometry: harbor strips and cranes, solar panels, mountain ridges, foundry stacks, river locks, command columns, rail lines, or metro kiosks. They are intentionally asset-ready: the geometry is procedural today, but each profile is a clear hook for replacing blocks with static meshes later.

## Curriculum Metadata

Each city mission now explains:

- the coding concept being practiced
- the recommended strategy for graduation
- the suggested first-pass language

The core lesson families still rotate through sum, boolean lock, reverse, palindrome, FizzBuzz, and even-filter challenges. The per-city metadata makes repeated lesson families feel like a campaign progression instead of clones.

The compiler-backed validators now also use city-seeded hidden cases. A repeated lesson family keeps the same teaching shape, but the hidden input values, strings, FizzBuzz lengths, and even-filter arrays vary by city terminal ID.

## Radio Briefings

Implemented in:

- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Content/CodeRescueData/radio_briefings.tsv`
- `Scripts/generate_radio_voiceovers.py`
- `Content/CodeRescueAssets/Audio/RadioSamples/new_york_radio_briefing.wav`

Behavior:

- On macOS, `ACodeRescueGameMode::SpeakRadioBriefing()` uses `/usr/bin/say` to speak the active city's radio briefing when that city streams in.
- The 342-row TSV stores the generated city briefing text, voice, art kit, slug, city, and state.
- `Scripts/generate_radio_voiceovers.py` converts TSV rows to WAV files with `say` and `afconvert`.
- A New York WAV sample is included as a concrete import/reference clip.

Generation examples:

```bash
Scripts/generate_radio_voiceovers.py --limit 1
Scripts/generate_radio_voiceovers.py --limit 0
```

## Enemy Variety and Difficulty

Implemented in:

- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Source/CodeRescueUnreal/CodeZombieActor.h`
- `Source/CodeRescueUnreal/CodeZombieActor.cpp`

Behavior:

- City rank drives difficulty tier.
- Difficulty tier and encounter intensity scale zombie count, health, speed, and activation range.
- The zombie variant data table is connected to the campaign spawner again.
- Variant selection checks exact city index first, then falls back to a three-bucket `CityIndex % 3` theme so the existing data table remains useful.
- Variant assignments are persisted by `ZombieId`, preserving save/load continuity.

## Persistence Fixes

Implemented in:

- `Source/CodeRescueUnreal/CodeRescueGameInstance.cpp`
- `Source/CodeRescueUnreal/SurvivorActor.cpp`

Behavior:

- Streamed-in city actors receive saved objective state without teleporting the player.
- Survivor rescue is idempotent against `RescuedSurvivorNames`, so a streamed/resaved survivor cannot inflate the survivor counter.
- Solved terminals skip their helper markers when streamed back in.
- Neutralized zombies are skipped or destroyed when their city streams in.

## Verification

The Unreal editor target builds successfully:

```text
CodeRescueUnrealEditor Mac Development
Result: Succeeded
```

## Remaining Polish

These are no longer blockers for the requested gameplay objectives, but they are still good future quality upgrades:

- Playtest several city transitions in PIE and packaged builds to tune pacing.
- Replace procedural art-kit block geometry with imported static meshes or Nanite city packs.
- Import generated WAV briefings as Unreal `SoundWave` assets if you want all voice clips cooked into packaged builds instead of using macOS system speech.
