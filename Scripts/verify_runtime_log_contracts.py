#!/usr/bin/env python3
"""Assert profile-aware runtime breadcrumbs in Unreal smoke logs.

This verifier promotes the most important manual log-review checks into a
repeatable contract. Curated production boots and development-showcase boots
have intentionally different world layers, so each profile has its own
required evidence while sharing the safe-learning, confinement, and access
contracts. Logs must also remain free of known dynamic-material regressions.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys


COMMON_REQUIRED_FRAGMENTS = (
    "[CodeRescueArenaConfinement]",
    "Backspace recovery guidance",
    "[CodeRescueSafeLearning]",
    "[CodeRescueEntryAccess]",
)

CURATED_REQUIRED_FRAGMENTS = (
    "[CodeRescueUSCityRealization]",
    "[FirstLevelCombatArtV4]",
    "[ProductionWorld]",
    "curated=1 development_showcases=0",
    "[CodeRescueEncounterDirector]",
    "[ProductionPresentation]",
    "[FirstLevelAim]",
)

SHOWCASE_REQUIRED_FRAGMENTS = (
    "[CodeRescueUSCityIdentity]",
    "signature='harbor statue silhouette and dense island skyline'",
    "districts='waterfront or beach approach | transit stop and rail/bus corridor | historic core and stoop row'",
    "[CodeRescueUnrealSystems]",
    "[CodeRescuePublicDemoQuality]",
    "[CodeRescueCreativeImplementation]",
    "Nanite/SM6 review gates",
    "Mac LOD/texture/shader asset budget gates",
)

FORBIDDEN_FRAGMENTS = (
    "LogMaterial: Warning",
    "not a valid parent for MaterialInstanceDynamic",
    "MID_MID_",
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Verify Code Rescue runtime log contracts.")
    parser.add_argument("log", type=Path, help="Unreal runtime smoke log to inspect.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    log_path = args.log
    if not log_path.exists():
        print(f"[cr-runtime-log-contracts] BLOCKER: missing log {log_path}", file=sys.stderr)
        return 1

    text = log_path.read_text(encoding="utf-8", errors="ignore")
    blockers: list[str] = []

    b_curated_production = "curated=1 development_showcases=0" in text
    required_fragments = COMMON_REQUIRED_FRAGMENTS + (
        CURATED_REQUIRED_FRAGMENTS if b_curated_production else SHOWCASE_REQUIRED_FRAGMENTS
    )

    for fragment in required_fragments:
        if fragment not in text:
            blockers.append(f"missing required runtime marker: {fragment}")

    for fragment in FORBIDDEN_FRAGMENTS:
        if fragment in text:
            blockers.append(f"forbidden runtime log fragment present: {fragment}")

    if blockers:
        for blocker in blockers:
            print(f"[cr-runtime-log-contracts] BLOCKER: {blocker}", file=sys.stderr)
        return 1

    profile = "curated-production" if b_curated_production else "development-showcase"
    print(f"[cr-runtime-log-contracts] passed ({profile}): {log_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
