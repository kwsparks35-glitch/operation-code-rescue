#!/usr/bin/env python3
"""Static verifier for the Maple sinister female-narration pass (2026-06-12).

Locks the contract:
- the Maple voice model development (XTTS v2 clone engine + reference WAVs,
  taken ONLY from the mash-ai voice work) lives in Tools/MapleVoice/,
- the sinister FX chain exists with sane tuning constants,
- the generator targets exactly the five female `say` voices assigned by
  RadioVoiceFor() in CodeRescueCampaign.cpp and writes WAVs using the
  `{slug}_radio_briefing.wav` naming the existing wire pipeline expects,
- the editor-side import/wire script and the double-click launcher exist,
- runtime briefing playback still prefers cooked cues (no C++ regression).

Audio coverage is now a hard failure for every expected female-voiced mission
slug. Extra legacy radio sample assets are tolerated, but the expected 230
Maple mission slugs must have both generated WAVs and imported SoundWave
assets.
"""

from __future__ import annotations

import csv
import re
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
TOOLS = PROJECT_ROOT / "Tools/MapleVoice"
SAMPLES = TOOLS / "reference_samples"
OUT_DIR = PROJECT_ROOT / "Content/CodeRescueAssets/Audio/RadioSamples"
TSV = PROJECT_ROOT / "Content/CodeRescueData/radio_briefings.tsv"

FEMALE_VOICES = {"Samantha", "Victoria", "Kyoko", "Tessa", "Karen"}

errors: list[str] = []
infos: list[str] = []


def check(cond: bool, msg: str) -> None:
    if not cond:
        errors.append(msg)


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


# 1. Voice model development present (engine + references).
clone_src = read(TOOLS / "maple_voice_clone.py")
check("xtts_v2" in clone_src and "MapleVoiceClone" in clone_src,
      "maple_voice_clone.py must wrap the XTTS v2 Maple clone")
check("serious_caut" in clone_src,
      "maple_voice_clone.py must map the serious/cautionary reference style")
wavs = sorted(SAMPLES.glob("*.wav")) if SAMPLES.exists() else []
serious = [w for w in wavs if w.name.lower().startswith("serious_caut")]
check(len(wavs) >= 10, f"expected >=10 Maple reference WAVs, found {len(wavs)}")
check(len(serious) >= 2,
      f"expected >=2 serious-cautionary reference WAVs, found {len(serious)}")

# 2. Sinister FX chain with darkening (not brightening) defaults.
fx_src = read(TOOLS / "sinister_fx.py")
m = re.search(r"PITCH_SEMITONES\s*=\s*(-?\d+(?:\.\d+)?)", fx_src)
check(m is not None and float(m.group(1)) < 0.0,
      "sinister_fx.py PITCH_SEMITONES must be negative (deeper voice)")
m = re.search(r"TIME_STRETCH\s*=\s*(\d+(?:\.\d+)?)", fx_src)
check(m is not None and float(m.group(1)) < 1.0,
      "sinister_fx.py TIME_STRETCH must be < 1.0 (slower delivery)")
m = re.search(r"OUTPUT_SR\s*=\s*(\d+)", fx_src)
check(m is not None and int(m.group(1)) == 22050,
      "sinister_fx.py OUTPUT_SR must stay 22050 to match existing radio cues")
check("apply_sinister_fx" in fx_src, "sinister_fx.py must expose apply_sinister_fx")

# 3. Generator targets exactly the female voices RadioVoiceFor() assigns.
gen_src = read(PROJECT_ROOT / "Scripts/generate_maple_sinister_narrations.py")
campaign = read(SRC / "CodeRescueCampaign.cpp")
assigned = set(re.findall(r'return TEXT\("(\w+)"\);', campaign.split(
    "FString RadioVoiceFor")[1].split("}")[0])) if "FString RadioVoiceFor" in campaign else set()
for v in FEMALE_VOICES:
    check(v in assigned, f"RadioVoiceFor() no longer assigns {v} — update verifier/generator scope")
    check(f'"{v}"' in gen_src, f"generator FEMALE_VOICES must include {v}")
for male in ("Alex", "Daniel", "Diego", "Maged", "Rishi"):
    check(f'"{male}"' not in gen_src.split("FEMALE_VOICES")[1].split("}")[0],
          f"generator must NOT re-voice male narration voice {male}")
check("_radio_briefing.wav" in gen_src,
      "generator must keep the {slug}_radio_briefing.wav naming for wire_radio_cues.py")
check("style=\"serious\"" in gen_src,
      "generator must clone from the serious-cautionary reference style")

# 4. Launcher + editor-side import/wire script.
cmd = PROJECT_ROOT / "Generate_Maple_Sinister_Narrations.command"
cmd_src = read(cmd)
check("caffeinate" in cmd_src, "launcher must caffeinate the long render")
check("COQUI_TOS_AGREED" in cmd_src, "launcher must pre-accept the cached XTTS license")
import_src = read(PROJECT_ROOT / "Scripts/import_and_wire_maple_narrations.py")
check("wire_radio_cues" in import_src,
      "import script must reuse the existing wire_radio_cues assignment")
check("CODE_RESCUE_FORCE_MAPLE_IMPORT" in import_src and "replace_existing = True" in import_src,
      "import script must support idempotent reruns plus forced SoundWave replacement")

# 5. Runtime contract unchanged: cooked cues preferred, subtitles first.
gm = read(SRC / "CodeRescueGameMode.cpp")
speak = gm.split("void ACodeRescueGameMode::SpeakRadioBriefing")[1].split("\n}\n")[0] \
    if "void ACodeRescueGameMode::SpeakRadioBriefing" in gm else ""
check("UCodeRescueSubtitlesWidget::Push" in speak,
      "SpeakRadioBriefing must keep the guaranteed subtitle baseline")
check("CityRadioBriefingCues" in speak and "SpawnSound2D" in speak,  # 2026-07-11 refresh: SpawnSound2D enables the stop-previous-voice arbiter
      "SpeakRadioBriefing must keep preferring cooked cues over say-TTS")
check("StaticLoadObject" in speak and "/Game/CodeRescueAssets/Audio/RadioSamples" in speak,
      "SpeakRadioBriefing must include the native slug-based Maple cue fallback")

# 6. QA audit registration.
audit = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
check("verify_maple_sinister_narration_pass.py" in audit,
      "verifier must be registered in Run_Full_QA_Audit.command")

# 7. Documentation must describe coverage as live/resumable, not as a stale
# hard-coded completion number.
readme = read(TOOLS / "README.md")
handoff_doc = read(PROJECT_ROOT / "Documentation/improvement_pass_2026-06-12/44_MAPLE_SINISTER_FEMALE_NARRATION_PASS.md")
status_doc = read(PROJECT_ROOT / "Documentation/MAPLE_NARRATION_STATUS_2026-06-18.md")
check("coverage is counted" in readme.lower() and "live" in readme.lower() and "230" in readme,
      "Maple README must describe live 230-city coverage tracking")
check("coverage is counted live" in handoff_doc.lower() and "fallback" in handoff_doc.lower(),
      "Maple handoff doc must describe live coverage and fallback behavior")
check("coverage is intentionally counted live" in status_doc.lower(),
      "Maple status doc must describe live coverage verification")

# Coverage of generated/imported sinister Maple cues.
if TSV.exists():
    with TSV.open(encoding="utf-8", newline="") as fh:
        rows = [r for r in csv.DictReader(fh, delimiter="\t")
                if r.get("voice", "").strip() in FEMALE_VOICES]
    missing_wavs = [
        r["slug"] for r in rows
        if not (OUT_DIR / f"{r['slug']}_radio_briefing.wav").exists()
    ]
    missing_assets = [
        r["slug"] for r in rows
        if not (OUT_DIR / f"{r['slug']}_radio_briefing.uasset").exists()
    ]
    done = len(rows) - len(missing_wavs)
    infos.append(f"sinister Maple WAV coverage: {done}/{len(rows)} female-voiced missions"
                 + ("" if done else " (run Generate_Maple_Sinister_Narrations.command)"))
    imported = len(rows) - len(missing_assets)
    infos.append(f"sinister Maple SoundWave import coverage: {imported}/{len(rows)} female-voiced missions")
    check(not missing_wavs,
          "missing generated Maple WAVs for expected slugs: " + ", ".join(missing_wavs[:10])
          + (" ..." if len(missing_wavs) > 10 else ""))
    check(not missing_assets,
          "missing imported Maple SoundWave assets for expected slugs: " + ", ".join(missing_assets[:10])
          + (" ..." if len(missing_assets) > 10 else ""))
else:
    errors.append("missing Content/CodeRescueData/radio_briefings.tsv")

name = "verify_maple_sinister_narration_pass"
for line in infos:
    print(f"[{name}] INFO: {line}")
if errors:
    for e in errors:
        print(f"[{name}] FAIL: {e}")
    sys.exit(1)
print(f"[{name}] PASS: Maple sinister female-narration contract intact "
      f"({len(wavs)} refs, {len(serious)} serious)")
