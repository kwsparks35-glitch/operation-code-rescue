#!/usr/bin/env python3
"""Generate the no-human next-20 improvement evidence bundle.

This script intentionally accepts missing optional artifacts. A fresh checkout
may not yet have visual screenshots, packaged-app evidence, or an in-game
runtime control export. The resulting JSON records those gaps explicitly so
future reviewers can distinguish completed automation from human/credential
boundaries.
"""

from __future__ import annotations

import csv
import json
import subprocess
from collections import Counter
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


PROJECT_ROOT = Path(__file__).resolve().parents[1]
DATA = PROJECT_ROOT / "Content/CodeRescueData/nohuman_next20_recommendations.tsv"
OUT_DIR = PROJECT_ROOT / "Saved/Release"


ARTIFACTS = {
    "control_profile_default": "Saved/Config/ControlProfiles/default_controls_profile.json",
    "control_profile_runtime": "Saved/Config/ControlProfiles/runtime_controls_profile.json",
    "city_layer_static_profile": "Saved/Profiling/city_layer_static_profile_latest.json",
    "visual_regression_manifest": "Saved/VisualRegression/visual_regression_manifest_latest.json",
    "package_integrity": "Saved/Release/package_integrity_latest.json",
    "maple_audio_audit": "Saved/AudioAudit/maple_audio_audit_latest.json",
}


def read_json_if_exists(relative: str) -> dict[str, Any] | None:
    path = PROJECT_ROOT / relative
    if not path.exists():
        return None
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:  # pragma: no cover - evidence should record parser failure.
        return {"parse_error": str(exc), "path": relative}


def git_dirty_summary() -> dict[str, Any]:
    try:
        proc = subprocess.run(
            ["git", "status", "--short"],
            cwd=PROJECT_ROOT,
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
    except OSError as exc:
        return {"available": False, "error": str(exc)}

    lines = [line for line in proc.stdout.splitlines() if line.strip()]
    prefixes = Counter(line[:2] for line in lines)
    return {
        "available": proc.returncode == 0,
        "line_count": len(lines),
        "prefix_counts": dict(sorted(prefixes.items())),
        "stderr": proc.stderr.strip(),
    }


def main() -> int:
    if not DATA.exists():
        print(f"[generate_nohuman_next20_evidence] FAIL: missing {DATA.relative_to(PROJECT_ROOT)}")
        return 1

    with DATA.open(encoding="utf-8", newline="") as fh:
        rows = list(csv.DictReader(fh, delimiter="\t"))

    status_counts = Counter(row.get("status", "unknown") for row in rows)
    boundary_counts = Counter(row.get("boundary", "unknown") for row in rows)
    artifacts = {
        name: {
            "path": relative,
            "exists": (PROJECT_ROOT / relative).exists(),
            "summary": read_json_if_exists(relative),
        }
        for name, relative in ARTIFACTS.items()
    }

    evidence = {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "recommendation_manifest": str(DATA.relative_to(PROJECT_ROOT)),
        "recommendation_count": len(rows),
        "status_counts": dict(sorted(status_counts.items())),
        "boundary_counts": dict(sorted(boundary_counts.items())),
        "recommendations": rows,
        "artifacts": artifacts,
        "git_dirty_summary": git_dirty_summary(),
        "human_testing_boundary": "No physical human playthrough is claimed by this evidence.",
        "credential_boundary": "Apple Developer ID signing and notarization require external credentials.",
    }

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    latest = OUT_DIR / "nohuman_next20_evidence_latest.json"
    stamped = OUT_DIR / f"nohuman_next20_evidence_{stamp}.json"
    text = json.dumps(evidence, indent=2, sort_keys=True)
    latest.write_text(text + "\n", encoding="utf-8")
    stamped.write_text(text + "\n", encoding="utf-8")

    print(f"[generate_nohuman_next20_evidence] wrote {latest}")
    print(f"[generate_nohuman_next20_evidence] recommendations={len(rows)} status_counts={dict(status_counts)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
