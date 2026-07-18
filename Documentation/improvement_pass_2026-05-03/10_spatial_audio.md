# Item 10 — 3D spatial audio for ambience and zombies

## What changed
1. **Zombie growl spatialization**: `ACodeZombieActor::GrowlAudio` now sets
   `bAllowSpatialization=true` so any USoundCue with attenuation curves
   honors listener distance. The legacy code created the audio component
   but didn't ask for spatialization explicitly.
2. **Per-zone ambient bed**: `ACodeRescueGameMode::ZoneAmbientCues` is a
   `TArray<TSoftObjectPtr<USoundBase>>` (sized 3 by convention — Anchorage,
   Seattle, Tokyo). `SpawnAmbientSoundForCity` spawns an `AAmbientSound` at
   the city center +1500 units up if a cue is set for `CityIndex % 3`.
   Volume is 0.45 to leave headroom for SFX.

## Files touched
- `Source/CodeRescueUnreal/CodeRescueGameMode.h/.cpp` — `ZoneAmbientCues`
  property + `SpawnAmbientSoundForCity` helper.
- `Source/CodeRescueUnreal/CodeZombieActor.cpp` — spatialization flags.

## Design decisions
- Soft-object pointers throughout so no asset references force a load
  before the cue is imported.
- Spatialization flag set once in the constructor; we don't override the
  attenuation asset programmatically because doing so would silence designer
  customization on the cue itself.

## Known limitations
- **No sound assets are actually shipped** — `ZoneAmbientCues` is empty by
  default, so `SpawnAmbientSoundForCity` early-returns. The user must
  import wind/rain/distant-traffic cues in editor and assign them on
  `BP_CodeRescueGameMode`.
- No reverb effects per zone.

## Follow-up work
1. Import 3 ambient WAVs (wind, rain, urban-night) as `USoundWave`.
2. Wrap each in a `USoundCue` with `Attenuation` set to ~5000 unit falloff
   and `bAllowSpatialization=true`.
3. Assign on `BP_CodeRescueGameMode → ZoneAmbientCues[0..2]`.
4. Author `USoundAttenuation` assets per growl variant in `DT_ZombieVariants`
   for individual-zombie audio reach.
