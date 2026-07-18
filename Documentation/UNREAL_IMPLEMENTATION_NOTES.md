# Unreal Implementation Notes

## Current implementation

This project is a C++-driven Unreal prototype that creates its world procedurally when the engine entry map starts. That avoids requiring a prebuilt binary `.umap` file from this environment.

## Gameplay systems

- `ACodeRescueCharacter`: first-person player movement, shooting, interaction, health, ammo, medkits.
- `ACodeZombieActor`: procedural infected enemy that chases the player and attacks in range.
- `ASurvivorActor`: rescue target; pressing E while aiming at a survivor rescues that city only after its required coding terminal is solved.
- `ALanguageStationActor`: language selector for Java, C, Python, and MATLAB.
- `ACodingTerminalActor`: in-world coding terminal containing a challenge.
- `UCodeTerminalWidget`: runtime-created UMG coding interface.
- `UCodeRunnerLibrary`: local validation runners for Java, C, Python, and MATLAB.
- `FCodeRescueCampaign`: centralized 465-mission catalog, city unlock rules, lesson metadata, region tags, landmarks, radio briefings, and difficulty tiers.
- `ACodeRescueGameMode`: procedural world generation, active-city instancing, city hubs, terminals, survivors, zombies, fog, lights, and campaign labels.

## Locations

The prototype now spawns a gated 465-stop campaign. Ranks 1-342 are the U.S.
major-city segment; ranks 343-465 are the global city extension. Each city hub
has:

- one unique terminal ID and mission brief
- one coding lesson selected from the rotating lesson families
- one gated survivor team tied to that terminal
- language stations, supplies, skyline markers, a procedural landmark, art-kit geometry, local enemies, visible city signage, and macOS system-spoken radio briefing text

Only the active city hub is live at once. `EnsureCampaignCityLoaded()` streams
the requested city in procedurally and destroys the previous city actor set,
while the full 465-mission catalog remains available to the HUD, journal, and
progression gates.

For this revision, the first 342 missions remain the U.S. Census Vintage 2024
major-city list. The campaign then continues through 123 global city missions,
for 465 total playable stops. The catalog is centralized in
`Source/CodeRescueUnreal/CodeRescueCampaign.cpp`.

## Code execution strategy

The app writes the player's code to `Saved/CodeSandbox` and then runs
language-specific validators when trusted local QA opts in with
`-AllowExternalCodeValidation`.

- Java: `javac` and `java`
- C/C++: `clang` / `clang++` with harness macro undef guards and a per-run success sentinel
- Python: `python3` with user code imported by a separate harness file and a per-run success sentinel
- MATLAB: installed MATLAB through `MATLAB_BIN`, `MATLABROOT`, `/Applications/MATLAB_R*.app/bin/matlab`, or PATH

The validators use city-seeded hidden cases based on `Challenge.Id`, so repeated lesson families keep the same concept while changing hidden inputs per city.

## MATLAB strategy

MATLAB function files must match the function name. The prototype writes:

- `total_power.m` for sum/generator challenges
- `should_unlock.m` for boolean lock challenges

Then MATLAB is invoked in batch mode to run visible and hidden tests.

## Radio strategy

The current runtime always pushes subtitle text, then uses this audio priority:

- If `bPreferCookedRadioBriefingCues` or `-UseCookedRadioVoice` is set, play a
  cooked `CityRadioBriefingCues` entry or slug-loaded Maple cue first.
- Otherwise, on macOS, `ACodeRescueGameMode::SpeakRadioBriefing()` falls back to
  `/usr/bin/say` unless `-NoRadioVoice` is present.
- `Content/CodeRescueData/radio_briefings.tsv` stores generated city lines for
  the 465-mission campaign.
- The current Maple batch covers 230 selected female-voiced mission slugs, not
  the full 465-mission campaign.

## Expansion roadmap

Recommended next development steps:

1. Replace procedural primitive geometry with Nanite-ready environment assets.
2. Add Niagara particle effects for muzzle flash, sparks, fire, smoke, and infection clouds.
3. Add MetaHuman or rigged survivor characters.
4. Replace block zombies with animated skeletal meshes and Behavior Tree AI.
5. Add a persistent mission/save system.
6. Add a full curriculum database for Java, C, Python, and MATLAB.
7. Add spaced-repetition side quests.
8. Import the generated WAV briefings as Unreal `SoundWave` assets for fully cooked packaged builds.
9. Replace procedural art-kit block geometry with authored static meshes as assets become available.

## Pro upgrade notes

This project revision adds implementation scaffolding for the previously listed next upgrade:

- `UCodeRescueAssetManifest` for professional asset references
- skeletal mesh slots on zombies and survivors
- Niagara VFX hooks on the player, zombies, and survivors
- `UCodeRescueSaveGame` and GameInstance save/load helpers
- JSON-driven curriculum database support
- asset import folder hierarchy under `Content/CodeRescueAssets/`

The project intentionally does not bundle paid third-party assets. Use Fab, Marketplace, MetaHumans, Quixel/Megascans, or personally created assets inside Unreal, then bind them to the exposed Blueprint properties.
