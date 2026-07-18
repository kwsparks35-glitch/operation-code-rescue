#!/usr/bin/env python3
"""Generate sinister Maple-voice radio briefings for all FEMALE-voiced cities.

Replaces the macOS `say` female narrations (Samantha / Victoria / Kyoko /
Tessa / Karen — 230 of the 465 missions in radio_briefings.tsv) with the
Maple voice clone from the mash-ai voice model development, rendered darker
via Tools/MapleVoice/sinister_fx.py. Male-voiced missions are untouched.

Output WAVs land in Content/CodeRescueAssets/Audio/RadioSamples/ using the
exact `{slug}_radio_briefing.wav` naming the existing import + wire pipeline
(Scripts/wire_radio_cues.py) already understands, so no new wiring scheme is
introduced. At runtime ACodeRescueGameMode::SpeakRadioBriefing prefers these
cooked cues over the `say` fallback automatically.

Run on the Mac via Generate_Maple_Sinister_Narrations.command (double-click).
Direct usage:
    python3 Scripts/generate_maple_sinister_narrations.py --limit 3   # preview
    python3 Scripts/generate_maple_sinister_narrations.py --limit 0   # all
    python3 Scripts/generate_maple_sinister_narrations.py --force     # re-render

The run is resumable: existing outputs are skipped unless --force is given.
"""

from __future__ import annotations

import argparse
import csv
import sys
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT / "Tools" / "MapleVoice"
BRIEFINGS = ROOT / "Content" / "CodeRescueData" / "radio_briefings.tsv"
OUT_DIR = ROOT / "Content" / "CodeRescueAssets" / "Audio" / "RadioSamples"
MANIFEST = OUT_DIR / "maple_sinister_manifest.tsv"

# The five female macOS `say` voices assigned by RadioVoiceFor() in
# CodeRescueCampaign.cpp. Only these missions are re-voiced with Maple.
FEMALE_VOICES = {"Samantha", "Victoria", "Kyoko", "Tessa", "Karen"}

sys.path.insert(0, str(TOOLS))


def load_female_rows() -> list[dict[str, str]]:
    with BRIEFINGS.open(encoding="utf-8", newline="") as fh:
        rows = list(csv.DictReader(fh, delimiter="\t"))
    return [r for r in rows if r.get("voice", "").strip() in FEMALE_VOICES]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--limit", type=int, default=0,
                        help="Clips to generate this run (0 = all remaining).")
    parser.add_argument("--force", action="store_true",
                        help="Re-render even if the output WAV already exists.")
    args = parser.parse_args()

    from maple_voice_clone import MapleVoiceClone
    from sinister_fx import apply_sinister_fx, PITCH_SEMITONES, TIME_STRETCH, REVERB_WET

    rows = load_female_rows()
    todo = []
    for row in rows:
        out_path = OUT_DIR / f"{row['slug']}_radio_briefing.wav"
        if args.force or not out_path.exists():
            todo.append((row, out_path))
    if args.limit > 0:
        todo = todo[: args.limit]

    print("=" * 64)
    print(" Maple Sinister Narration Generator")
    print("=" * 64)
    print(f" Female-voiced missions : {len(rows)}")
    print(f" To generate this run   : {len(todo)} (resume-skip handles the rest)")
    print(f" FX: pitch {PITCH_SEMITONES} st, stretch {TIME_STRETCH}, reverb {REVERB_WET}")
    if not todo:
        print(" Nothing to do — all sinister narrations already rendered.")
        return 0

    vc = MapleVoiceClone()
    status = vc.check_status()
    print(f" Maple refs: {status['wav_count']} WAVs "
          f"({status['serious_ref_count']} serious-cautionary)")
    if not status["ready"]:
        print(" ERROR: no reference samples in Tools/MapleVoice/reference_samples/")
        return 1
    if not vc.load():
        print(" ERROR: XTTS v2 failed to load. See log above.")
        return 1

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    new_manifest_lines = []
    t_start = time.perf_counter()
    ok = fail = 0
    for i, (row, out_path) in enumerate(todo, 1):
        t0 = time.perf_counter()
        with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as tmp:
            raw_path = Path(tmp.name)
        try:
            r = vc.speak(row["briefing"], raw_path, style="serious")
            if "error" in r:
                raise RuntimeError(r["error"])
            fx = apply_sinister_fx(raw_path, out_path)
            if "error" in fx:
                raise RuntimeError(fx["error"])
            ok += 1
            dt = time.perf_counter() - t0
            done_s = time.perf_counter() - t_start
            eta_min = (done_s / i) * (len(todo) - i) / 60.0
            print(f" [{i:3d}/{len(todo)}] {row['slug']:<34} "
                  f"{fx['seconds']:6.1f}s audio  {dt:5.1f}s gen  ETA {eta_min:5.1f} min")
            new_manifest_lines.append(
                f"{row['slug']}\t{row['voice']}\tmaple_sinister\t{fx['seconds']}\n")
        except Exception as e:  # noqa: BLE001 — keep batch alive
            fail += 1
            print(f" [{i:3d}/{len(todo)}] {row['slug']:<34} FAILED: {e}")
        finally:
            raw_path.unlink(missing_ok=True)

    if new_manifest_lines:
        write_header = not MANIFEST.exists()
        with MANIFEST.open("a", encoding="utf-8") as fh:
            if write_header:
                fh.write("slug\treplaced_voice\tnew_voice\tseconds\n")
            fh.writelines(new_manifest_lines)

    total_done = sum(
        1 for r in rows if (OUT_DIR / f"{r['slug']}_radio_briefing.wav").exists())
    print("-" * 64)
    print(f" This run: {ok} generated, {fail} failed.")
    print(f" Coverage: {total_done}/{len(rows)} female-voiced missions have "
          f"sinister Maple WAVs.")
    print(" Next: open the UE editor Python console and run "
          "Scripts/import_and_wire_maple_narrations.py")
    return 0 if fail == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
