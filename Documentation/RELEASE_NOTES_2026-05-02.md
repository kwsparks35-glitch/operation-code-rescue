# Operation Code Rescue - Release Notes

Date: 2026-05-02  
Package: `PackagedMac/Mac/CodeRescueUnreal.app`  
Engine: Unreal Engine 5.7.4  
Configuration: Mac Development

> Scope note added 2026-06-23: these release notes describe the May 2 package.
> Current source has expanded to 465 campaign missions: 342 U.S. stops plus 123
> global extension stops.

## Highlights

- 342-city U.S. major-city campaign remains active.
- Active city streaming keeps only the current city hub live.
- Terminal, survivor, and zombie objective markers now clean up immediately when objectives complete.
- Save/load counters are normalized from saved objective IDs.
- Save/load now also preserves player health, ammo, and medkit resources for saves created with version `0.6.0-player-resource-state`.
- Player movement, interaction, weapon, healing, pickup, city-density, and encounter-balance values are exposed as named gameplay parameters.
- Each campaign city now uses a shared `50.0x` horizontal span scale for city spacing, player start offsets, floors, landmarks, art-kit geometry, skyline placement, navigation bounds, objectives, pickups, and encounters.
- Player traversal, interaction reach, and weapon ranges were retuned so the larger cities remain playable.
- Pickups now support both overlap collection and deliberate interact collection while respecting resource caps.
- Assisted fire now uses an aim cone instead of damaging the nearest hostile anywhere in range.
- City perimeter rails are now visual, non-blocking guide rails so the player can enter each streamed city from the starting approach.
- A visible southwest entry pad/corridor now marks the playable route into each city.
- The objective journal now scrolls across the full campaign list and jumps to the active city.
- Mac packaging now archives the fully staged app with cooked `Contents/UE` data.
- Packaged app startup passed both headless and normal renderer/audio smoke tests.

## Verification

- Editor target build: passed.
- BuildCookRun Mac package: passed.
- Package integrity check: passed.
- NullRHI smoke launch: passed.
- Metal/CoreAudio smoke launch: passed.
- Package script syntax check: passed.
- Gameplay parameter tuning documentation: added.
- Post-tuning package rebuild and render/null smoke re-run: passed.
- 50x city span editor build: passed.
- 50x city span package rebuild and render/null smoke re-run: passed.

## Known Notes

- The current art style is still a procedural prototype with optional professional asset hooks.
- macOS system speech is used for immediate city radio briefings unless `-NoRadioVoice` is passed.
- External distribution still needs final bundle identifier, signing, and notarization decisions.
- Human QA should complete `Documentation/QA_PLAYTEST_CHECKLIST.md` before a public release.
