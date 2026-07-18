#!/usr/bin/env python3
"""Audit generated Maple radio WAVs and imported SoundWave assets."""

from __future__ import annotations

import argparse
import array
import csv
import json
import math
import wave
from datetime import datetime, timezone
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
RADIO_DIR = PROJECT_ROOT / "Content/CodeRescueAssets/Audio/RadioSamples"
BRIEFINGS = PROJECT_ROOT / "Content/CodeRescueData/radio_briefings.tsv"
OUT_DIR = PROJECT_ROOT / "Saved/AudioAudit"
FEMALE_VOICES = {"Samantha", "Victoria", "Kyoko", "Tessa", "Karen"}


def read_expected_rows() -> list[dict[str, str]]:
    with BRIEFINGS.open(encoding="utf-8", newline="") as fh:
        return [
            row for row in csv.DictReader(fh, delimiter="\t")
            if row.get("voice", "").strip() in FEMALE_VOICES
        ]


def signal_stats(path: Path) -> dict:
    with wave.open(str(path), "rb") as wav:
        channels = wav.getnchannels()
        sample_width = wav.getsampwidth()
        sample_rate = wav.getframerate()
        frame_count = wav.getnframes()
        frames = wav.readframes(frame_count)

    duration = frame_count / sample_rate if sample_rate else 0.0
    peak = 0
    rms = 0.0
    if sample_width == 2 and frames:
        samples = array.array("h")
        samples.frombytes(frames)
        if samples:
            peak = max(abs(value) for value in samples)
            rms = math.sqrt(sum(value * value for value in samples) / len(samples))

    return {
        "channels": channels,
        "sample_width_bytes": sample_width,
        "sample_rate": sample_rate,
        "frame_count": frame_count,
        "duration_seconds": round(duration, 3),
        "peak_abs": int(peak),
        "rms": round(rms, 2),
        "size_bytes": path.stat().st_size,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--min-duration", type=float, default=1.0)
    parser.add_argument("--max-duration", type=float, default=90.0)
    parser.add_argument("--expected-count", type=int, default=230)
    args = parser.parse_args()

    errors: list[str] = []
    warnings: list[str] = []
    entries: list[dict] = []

    if not BRIEFINGS.exists():
        errors.append(f"missing {BRIEFINGS.relative_to(PROJECT_ROOT)}")
        rows: list[dict[str, str]] = []
    else:
        rows = read_expected_rows()

    if len(rows) != args.expected_count:
        errors.append(f"expected {args.expected_count} female Maple rows, found {len(rows)}")

    for row in rows:
        slug = row["slug"]
        wav_path = RADIO_DIR / f"{slug}_radio_briefing.wav"
        asset_path = RADIO_DIR / f"{slug}_radio_briefing.uasset"
        entry = {
            "slug": slug,
            "city": row.get("city", ""),
            "voice": row.get("voice", ""),
            "wav": str(wav_path.relative_to(PROJECT_ROOT)),
            "uasset": str(asset_path.relative_to(PROJECT_ROOT)),
            "wav_exists": wav_path.exists(),
            "uasset_exists": asset_path.exists(),
        }

        if not wav_path.exists():
            errors.append(f"missing generated WAV for {slug}")
            entries.append(entry)
            continue
        if not asset_path.exists():
            errors.append(f"missing imported SoundWave asset for {slug}")

        try:
            stats = signal_stats(wav_path)
        except (OSError, EOFError, wave.Error) as exc:
            errors.append(f"cannot read WAV {slug}: {exc}")
            entries.append(entry)
            continue

        entry.update(stats)
        if stats["channels"] != 1:
            errors.append(f"{slug} must be mono, found {stats['channels']} channels")
        if stats["sample_width_bytes"] != 2:
            errors.append(f"{slug} must be 16-bit PCM, found sample width {stats['sample_width_bytes']}")
        if stats["sample_rate"] != 22050:
            errors.append(f"{slug} must be 22050 Hz, found {stats['sample_rate']}")
        if stats["duration_seconds"] < args.min_duration:
            errors.append(f"{slug} duration {stats['duration_seconds']}s is under {args.min_duration}s")
        if stats["duration_seconds"] > args.max_duration:
            errors.append(f"{slug} duration {stats['duration_seconds']}s is over {args.max_duration}s")
        if stats["peak_abs"] < 500:
            errors.append(f"{slug} peak {stats['peak_abs']} is too quiet or silent")
        if stats["rms"] < 25:
            errors.append(f"{slug} RMS {stats['rms']} is too quiet or silent")
        if stats["peak_abs"] > 32600:
            warnings.append(f"{slug} peak {stats['peak_abs']} is close to clipping")
        entries.append(entry)

    wav_count = len(list(RADIO_DIR.glob("*_radio_briefing.wav"))) if RADIO_DIR.exists() else 0
    asset_count = len(list(RADIO_DIR.glob("*_radio_briefing.uasset"))) if RADIO_DIR.exists() else 0
    durations = [entry["duration_seconds"] for entry in entries if "duration_seconds" in entry]
    report = {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "project_root": str(PROJECT_ROOT),
        "expected_count": args.expected_count,
        "expected_rows": len(rows),
        "expected_wavs_present": sum(1 for entry in entries if entry.get("wav_exists")),
        "expected_uassets_present": sum(1 for entry in entries if entry.get("uasset_exists")),
        "radio_wav_file_count": wav_count,
        "radio_uasset_file_count": asset_count,
        "duration_min_seconds": round(min(durations), 3) if durations else None,
        "duration_max_seconds": round(max(durations), 3) if durations else None,
        "duration_average_seconds": round(sum(durations) / len(durations), 3) if durations else None,
        "error_count": len(errors),
        "warning_count": len(warnings),
        "errors": errors[:100],
        "warnings": warnings[:100],
        "entries": entries,
    }

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    text = json.dumps(report, indent=2, sort_keys=True)
    (OUT_DIR / f"maple_audio_audit_{stamp}.json").write_text(text + "\n", encoding="utf-8")
    latest = OUT_DIR / "maple_audio_audit_latest.json"
    latest.write_text(text + "\n", encoding="utf-8")

    print(
        "[audit_maple_audio_assets] "
        f"wavs={report['expected_wavs_present']}/{args.expected_count} "
        f"uassets={report['expected_uassets_present']}/{args.expected_count} "
        f"duration={report['duration_min_seconds']}..{report['duration_max_seconds']}s"
    )
    if warnings:
        print(f"[audit_maple_audio_assets] WARN: {len(warnings)} warning(s); see {latest}")
    if errors:
        for error in errors[:20]:
            print(f"[audit_maple_audio_assets] FAIL: {error}")
        print(f"[audit_maple_audio_assets] wrote {latest}")
        return 1
    print(f"[audit_maple_audio_assets] PASS: technical Maple audio audit wrote {latest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
