#!/usr/bin/env python3
"""Generate WAV radio briefings from Content/CodeRescueData/radio_briefings.tsv.

This uses macOS `say` plus `afconvert`, matching the runtime system-radio
voice path in ACodeRescueGameMode. Run with `--limit 0` to generate all 342
city clips, or omit it to generate the default first-city sample.
"""

from __future__ import annotations

import argparse
import csv
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
BRIEFINGS = ROOT / "Content" / "CodeRescueData" / "radio_briefings.tsv"
OUT_DIR = ROOT / "Content" / "CodeRescueAssets" / "Audio" / "RadioSamples"


def generate_clip(row: dict[str, str]) -> Path:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    out_path = OUT_DIR / f"{row['slug']}_radio_briefing.wav"
    with tempfile.NamedTemporaryFile(suffix=".aiff", delete=False) as tmp:
        aiff_path = Path(tmp.name)

    subprocess.run(
        ["/usr/bin/say", "-v", row["voice"], "-o", str(aiff_path), row["briefing"]],
        check=True,
    )
    subprocess.run(
        ["/usr/bin/afconvert", "-f", "WAVE", "-d", "LEI16@22050", str(aiff_path), str(out_path)],
        check=True,
    )
    aiff_path.unlink(missing_ok=True)
    return out_path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--limit",
        type=int,
        default=1,
        help="Number of clips to generate. Use 0 for all rows.",
    )
    args = parser.parse_args()

    rows = list(csv.DictReader(BRIEFINGS.open(encoding="utf-8"), delimiter="\t"))
    selected = rows if args.limit == 0 else rows[: max(args.limit, 0)]
    for row in selected:
        print(generate_clip(row))
    print(f"Generated {len(selected)} clip(s).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
