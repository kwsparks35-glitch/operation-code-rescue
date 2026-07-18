#!/usr/bin/env python3
"""Hash current visual-review screenshots into a regression baseline manifest."""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from datetime import datetime, timezone
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = PROJECT_ROOT / "Saved/VisualRegression"
SCREENSHOT_ROOTS = (
    PROJECT_ROOT / "Saved/Screenshots/VisualReview",
    PROJECT_ROOT / "Saved/Screenshots/MacEditor",
)


def png_size(path: Path) -> tuple[int | None, int | None]:
    try:
        with path.open("rb") as fh:
            header = fh.read(24)
        if header.startswith(b"\x89PNG\r\n\x1a\n") and header[12:16] == b"IHDR":
            return struct.unpack(">II", header[16:24])
    except OSError:
        pass
    return None, None


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def screenshot_entry(path: Path) -> dict:
    width, height = png_size(path)
    stat = path.stat()
    return {
        "path": str(path),
        "relative_path": str(path.relative_to(PROJECT_ROOT)),
        "size_bytes": stat.st_size,
        "mtime_utc": datetime.fromtimestamp(stat.st_mtime, timezone.utc).isoformat(),
        "width": width,
        "height": height,
        "sha256": sha256(path),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--min-count", type=int, default=1)
    args = parser.parse_args()

    screenshots: list[Path] = []
    for root in SCREENSHOT_ROOTS:
        if root.exists():
            screenshots.extend(p for p in root.glob("*.png") if p.is_file())
    screenshots.sort(key=lambda p: p.stat().st_mtime, reverse=True)
    if len(screenshots) < args.min_count:
        print(f"[generate_visual_regression_manifest] FAIL: expected at least {args.min_count} screenshots, found {len(screenshots)}")
        return 1

    manifest = {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "project_root": str(PROJECT_ROOT),
        "screenshot_count": len(screenshots),
        "targets_manifest": "Content/CodeRescueData/visual_regression_targets.tsv",
        "screenshots": [screenshot_entry(p) for p in screenshots],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    stamped = OUT_DIR / f"visual_regression_manifest_{stamp}.json"
    latest = OUT_DIR / "visual_regression_manifest_latest.json"
    text = json.dumps(manifest, indent=2, sort_keys=True)
    stamped.write_text(text + "\n", encoding="utf-8")
    latest.write_text(text + "\n", encoding="utf-8")
    print(f"[generate_visual_regression_manifest] wrote {latest} ({len(screenshots)} screenshots)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
