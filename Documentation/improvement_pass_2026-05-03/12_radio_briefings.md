# Item 12 — WAV radio briefings (replacing macOS TTS)

## What changed
`ACodeRescueGameMode::SpeakRadioBriefing` now prefers a cooked
`USoundBase*` cue over the macOS `say` TTS path. The cue table is
`CityRadioBriefingCues` (`TArray<TSoftObjectPtr<USoundBase>>`).

If `CityRadioBriefingCues[ActiveCampaignCityIndex]` is set and resolves,
that cue plays via `UGameplayStatics::PlaySound2D` and the function returns
without invoking `say`. If unset (or the cue won't load), the legacy
PLATFORM_MAC TTS path runs as a fallback.

## Files touched
- `Source/CodeRescueUnreal/CodeRescueGameMode.h/.cpp`

## Existing infrastructure (unchanged)
- `Scripts/generate_radio_voiceovers.py` already generates per-city WAVs via
  macOS `say`+`afconvert`. It writes to
  `Content/CodeRescueAssets/Audio/RadioSamples/<slug>_radio_briefing.wav`.
- `Content/CodeRescueData/radio_briefings.tsv` has all 342 city briefing
  texts + voice + slug.
- One sample WAV is already in the repo (`new_york_radio_briefing.wav`).

## Why this matters
The packaged Mac build no longer hard-depends on `/usr/bin/say` once
`CityRadioBriefingCues` is populated. This is the difference between
"works on the dev machine" and "works on a clean Mac at PAX."

## How to bake & wire all 342 clips (run on macOS)
```bash
cd /Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix
python3 Scripts/generate_radio_voiceovers.py --limit 0
```
Then in editor:
1. Right-click `Content/CodeRescueAssets/Audio/RadioSamples/` → Import.
2. Multi-select every WAV; UE imports them as `USoundWave` assets.
3. Open `BP_CodeRescueGameMode` → `CityRadioBriefingCues` array → resize to
   342 → assign each row to the matching SoundWave.
   *(If 342 manual assignments is too tedious, write a small editor
   utility script that walks the asset registry by name match.)*

## Known limitations
- The C++ side keys into `CityRadioBriefingCues` by `CityIndex`, but the
  array is empty until populated in editor. Default behavior reverts to
  macOS TTS.
- The cue is `PlaySound2D`, so there's no positional context (intentional —
  it's a radio briefing, not a diegetic speaker).

## Follow-up work
- Add an editor utility script that auto-fills `CityRadioBriefingCues` by
  scanning `Content/CodeRescueAssets/Audio/RadioSamples/*.uasset` and
  matching slugs to mission rows.
- Author a quick pre-roll "radio static" cue prepended to each briefing.
