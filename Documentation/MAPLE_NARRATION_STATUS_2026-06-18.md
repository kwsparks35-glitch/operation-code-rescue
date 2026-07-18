# Maple Narration Status - 2026-06-18

## Current State

The Maple sinister female-narration work now has a working generator,
reference set, FX chain, import script, runtime cue fallback, always-cook audio
directory, and live coverage verifier. The June 18 demo-readiness pass patched
the project shim for the current mash-ai XTTS/Transformers environment,
generated the 230-city female-voice Maple batch, imported the cues into Unreal
SoundWave assets, and verified complete coverage for that Maple subset. The
full playable campaign currently contains 465 missions.

Final local evidence from the June 18 readiness run:

- Generated Maple WAV coverage: `230/230` female-voiced missions in the Maple
  subset.
- Imported Unreal radio cue assets: `231` `*_radio_briefing.uasset` files under
  `Content/CodeRescueAssets/Audio/RadioSamples/`.
- The extra imported cue comes from an older/non-slug New York briefing asset;
  verifier coverage remains based on the expected 230 Maple mission slugs.
- `Import_And_Wire_Maple_Narrations.command` completed successfully after the
  Blueprint path was corrected to
  `/Game/CodeRescueAssets/Blueprints/BP_CodeRescueGameMode`.
- `CodeRescueGameMode.cpp` loads `/Game/CodeRescueAssets/Audio/RadioSamples/<slug>_radio_briefing`
  at runtime when a cue is not explicitly assigned, so the native game mode can
  play imported Maple cues.
- `Config/DefaultGame.ini` always cooks
  `/Game/CodeRescueAssets/Audio/RadioSamples` into packaged builds.
- `Run_Local_CI_Readiness.command` completed full QA, package, packaged null
  smoke, packaged render smoke, release manifest, visual regression manifest,
  and support-bundle creation with Maple coverage recorded at `230/230`.

Coverage is intentionally counted live instead of hard-coded here:

```bash
python3 Scripts/verify_maple_sinister_narration_pass.py
python3 Scripts/generate_release_manifest.py
```

Generated/imported female-voiced cities play Maple cues when cooked radio voice
is enabled. `SpeakRadioBriefing` now checks cooked cues before macOS Samantha
TTS, so `bPreferCookedRadioBriefingCues` and `-UseCookedRadioVoice` actually
select the imported cues. If a future cue is removed or not imported, the
existing Samantha/system-voice fallback path remains available. Male-voiced and
not-yet-generated cities remain intentionally unchanged.

## Files Involved

- `Tools/MapleVoice/maple_voice_clone.py`
- `Tools/MapleVoice/sinister_fx.py`
- `Tools/MapleVoice/reference_samples/`
- `Scripts/generate_maple_sinister_narrations.py`
- `Scripts/import_and_wire_maple_narrations.py`
- `Scripts/verify_maple_sinister_narration_pass.py`
- `Generate_Maple_Sinister_Narrations.command`
- `Import_And_Wire_Maple_Narrations.command`
- `Scripts/wire_radio_cues.py`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Config/DefaultGame.ini`

## Refresh Runbook

1. Run the generator:

   ```bash
   ./Generate_Maple_Sinister_Narrations.command
   ```

2. Open the Unreal editor and run:

   ```python
   exec(open(r"<project>/Scripts/import_and_wire_maple_narrations.py").read())
   ```

   Or run the automated commandlet wrapper. The wrapper is idempotent and skips
   already-imported SoundWave assets unless `CODE_RESCUE_FORCE_MAPLE_IMPORT=1`
   is set:

   ```bash
   ./Import_And_Wire_Maple_Narrations.command
   ```

3. Verify coverage:

   ```bash
   python3 Scripts/verify_maple_sinister_narration_pass.py
   ./Run_Full_QA_Audit.command
   ```

4. Package and smoke test after the cues are imported.

   ```bash
   ./Run_Local_CI_Readiness.command
   ```

## Latest Package Evidence

The latest local package that includes the cooked Maple radio sample directory
is:

```text
/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app
```

The current release manifest reports package size `2.03 GB`, package timestamp
`2026-06-19T01:36:37Z` (`Jun 18 17:36 AKDT 2026`), and Maple coverage
`230/230`.

## Release Caveat

XTTS v2 uses the Coqui Public Model License. Keep the current Maple cue path as
prototype/educational until licensing is reviewed for the intended release
model.
