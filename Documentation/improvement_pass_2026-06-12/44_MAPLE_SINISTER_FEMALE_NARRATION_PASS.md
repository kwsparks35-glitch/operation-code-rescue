# 44 — Maple Sinister Female-Narration Pipeline Pass (2026-06-12)

Status as of the June 18, 2026 demo-readiness pass: pipeline/tooling is in
place, the XTTS/Transformers compatibility issue is patched in the project
shim, and generated WAV coverage is counted live by the verifier and release
manifest. The 230-city female-voice Maple subset has been generated, imported,
wired, verified, and cooked into the current package. The full playable campaign
currently has 465 missions. Female-voiced missions play Maple cues after each
generated WAV is imported/wired; any future missing cue keeps the existing
fallback path.

## Goal

Replace the game's current female narrations — the macOS `say` radio voices
Samantha, Victoria, Kyoko, Tessa, and Karen, covering 230 of the 465 campaign
missions — with the **Maple** voice clone from the mash-ai voice model
development, delivered in a darker, more sinister register that fits the
survival-horror tone. Male-voiced missions (Alex, Daniel, Diego, Maged,
Rishi — 235 missions) are intentionally unchanged.

Per explicit project direction, **only the voice model development** was
taken from `/Users/labcomputer/Projects/mash-ai`: the XTTS v2 clone engine
(`voice/voice_clone.py`) and the Maple reference recordings
(`voice/reference_samples/*.wav`). No other mash-ai content (BCI, dome
systems, ITV/blockchain, vision, agents, Edge-TTS engines) is referenced by
or included in the game.

## What shipped

- `Tools/MapleVoice/maple_voice_clone.py` — adapted Maple XTTS v2 clone
  engine (path-independent, multi-reference cloning, style-keyed reference
  selection; the PyTorch 2.6 `weights_only` and torchaudio/soundfile patches
  carried over).
- `Tools/MapleVoice/reference_samples/` — 24 Maple reference WAVs (36 MB),
  including the 3 Serious/Cautionary takes used as the dark delivery
  reference. (Handles the `Serious_Cautinoary` filename misspelling.)
- `Tools/MapleVoice/sinister_fx.py` — original post-FX chain: pitch
  -2.0 semitones, 0.94x time stretch, dark synthetic-IR convolution reverb,
  90 Hz high-pass + 7.5 kHz high-cut tone shaping, -2.5 dBFS normalize with
  soft-clip guard, output 22050 Hz 16-bit mono (matches the project's
  existing LEI16@22050 radio cue convention). All knobs are top-of-file
  constants.
- `Scripts/generate_maple_sinister_narrations.py` — batch driver. Filters
  `radio_briefings.tsv` to the five female voices, synthesizes each briefing
  with the serious-style Maple clone, applies the sinister chain, writes
  `Content/CodeRescueAssets/Audio/RadioSamples/{slug}_radio_briefing.wav`.
  Resumable (skips existing), `--limit N` previews, `--force` re-renders,
  appends `maple_sinister_manifest.tsv`, prints per-clip ETA.
- `Generate_Maple_Sinister_Narrations.command` — double-click launcher.
  Prefers the mash-ai `.venv` interpreter (known-working XTTS install that
  already powers Maple); otherwise creates `Tools/MapleVoice/.venv` and
  installs `TTS==0.22.0` with the same transformers pin mash-ai uses. Wraps
  the render in `caffeinate -i` and sets `COQUI_TOS_AGREED=1` so the
  unattended run never stalls.
- `Scripts/import_and_wire_maple_narrations.py` — UE editor Python:
  idempotently imports missing RadioSamples WAVs by default, optionally
  overwrites existing SoundWaves with `CODE_RESCUE_FORCE_MAPLE_IMPORT=1`, then
  re-runs the existing `wire_radio_cues.py` slug matcher into
  `ACodeRescueGameMode::CityRadioBriefingCues`.
- `Import_And_Wire_Maple_Narrations.command` — commandlet wrapper for the
  import/wire step.
- `Scripts/verify_maple_sinister_narration_pass.py` — static verifier
  (registered in `Run_Full_QA_Audit.command`): voice-model presence,
  darkening-direction FX constants, female-voice scope exactly matching
  `RadioVoiceFor()`, slug naming compatibility, runtime cue-preference
  contract, and hard 230/230 WAV coverage.

## Design decisions

- **Runtime fallback stays layered.** `SpeakRadioBriefing` keeps subtitles as
  the guaranteed baseline, prefers assigned cooked cues, then attempts a native
  slug-based load from `/Game/CodeRescueAssets/Audio/RadioSamples`, then falls
  back to `say`. The native cue-load fallback was added because the packaged
  app uses the native game mode path, and the radio sample directory is now
  always cooked through `DefaultGame.ini`.
- **Same slugs, same folder, same wire script.** The single pre-existing
  cooked cue (`new_york_radio_briefing`, Victoria) is regenerated and
  re-imported in place.
- **Sinister = reference + DSP.** The clone borrows menace from the
  Serious/Cautionary reference takes; the FX chain adds depth (pitch),
  deliberation (tempo), and dead-air dread (dark reverb) while keeping
  briefings intelligible for the learning loop.

## Validation

- `python3 Scripts/verify_maple_sinister_narration_pass.py` reports and
  enforces live generated-WAV coverage against the 230 female-voiced missions.
- Latest completed result: `230/230`.
- `Run_Local_CI_Readiness.command` completed successfully after import/wire,
  package, packaged null smoke, packaged render smoke, release manifest, visual
  regression manifest, and support-bundle creation.
- `py_compile` / AST checks pass on all new Python; launcher shell-syntax
  checked.
- Full audio render + editor import + in-game listen require the Mac-side
  run below (operator step).

## Operator runbook

1. Double-click `Generate_Maple_Sinister_Narrations.command` only when cues need
   to be regenerated. The existing current output already covers all 230
   female-voiced missions.
2. Too subtle / too demonic? Edit constants atop `Tools/MapleVoice/
   sinister_fx.py`, then `FORCE=1 ./Generate_Maple_Sinister_Narrations.command`.
3. Open the editor → Window → Developer → Python Console →
   `exec(open(r"<project>/Scripts/import_and_wire_maple_narrations.py").read())`.
4. Re-run `Run_Local_CI_Readiness.command` to compile, run full QA, package,
   smoke the packaged app, refresh manifests, and create a support bundle.

## Future review items

- Extend the sinister Maple treatment to survivor `RescueVoCue` /
  `IdleBarkCue` lines once those cues are authored.
- Optional in-engine radio band-pass + crackle layer on the briefing channel.
- XTTS v2 weights are CPML-licensed (non-commercial) — revisit before any
  commercial distribution.
