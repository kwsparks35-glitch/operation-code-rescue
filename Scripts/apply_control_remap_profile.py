#!/usr/bin/env python3
"""Create or validate a reviewable control-remap profile from the manifest.

This script intentionally does not rewrite C++ direct bindings. It exports the
current control contract to JSON so designers can review, edit, and then port
approved bindings into Config/DefaultInput.ini plus the direct C++ BindKey
section together.
"""

from __future__ import annotations

import csv
import json
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
MANIFEST = PROJECT_ROOT / "Content/CodeRescueData/control_remap_manifest.tsv"
OUT_DIR = PROJECT_ROOT / "Saved/Config/ControlProfiles"


def main() -> int:
    if not MANIFEST.exists():
        print(f"[apply_control_remap_profile] FAIL: missing {MANIFEST}")
        return 1
    with MANIFEST.open(encoding="utf-8", newline="") as fh:
        rows = list(csv.DictReader(fh, delimiter="\t"))
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    direct = [row for row in rows if "Direct C++" in row.get("remap_status", "")]
    config_backed = [row for row in rows if "Config-backed" in row.get("remap_status", "")]
    mixed = [row for row in rows if "Config-backed" in row.get("remap_status", "") and "Direct C++" in row.get("remap_status", "")]
    out = OUT_DIR / "default_controls_profile.json"
    payload = {
        "profile_name": "Default",
        "profile_status": "reviewable contract",
        "implementation_note": (
            "The current game uses stable config-backed axes plus direct C++ key bindings. "
            "This JSON is a review/apply contract; arbitrary in-game key capture should be "
            "enabled only after direct bindings are routed through a shared action table."
        ),
        "counts": {
            "total_controls": len(rows),
            "config_backed_controls": len(config_backed),
            "direct_cpp_controls": len(direct),
            "mixed_config_and_direct_controls": len(mixed),
        },
        "controls": rows,
    }
    out.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"[apply_control_remap_profile] wrote {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
