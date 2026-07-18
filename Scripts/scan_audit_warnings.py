#!/usr/bin/env python3
"""Scan Unreal smoke logs for playability-blocking warnings.

The scanner is intentionally stricter than a casual grep: known immediate-quit
navigation/crowd diagnostics, unattended macOS audio-device query noise, and the
Development-only UMG preview-viewport hit-proxy diagnostic are reported as
allowed warnings, while missing objects, linker/load errors, fatals, exceptions,
and stale asset references fail the audit. UE may also warn when its packaged-app
log rotation tries to delete a backup log that has already been removed between
immediate smoke runs; that housekeeping message is non-playable state and
remains allowed.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys


BLOCKING_PATTERNS = (
    "Fatal",
    "Error:",
    "Exception",
    "LogLinker: Warning",
    "LoadErrors",
    "Failed to find object",
    "/Engine/EngineMeshes/Humanoid",
    "SM_postapo_bridge_001",
    "SKM_ZombieFemaleClothingCasual01",
)

ALLOWED_WARNING_FRAGMENTS = (
    "LogNavigationDirtyArea: Warning: Skipped some dirty area creation",
    "LogCrowdFollowing: Warning: Unable to find RecastNavMesh instance",
    "LogAudioMixerAudioUnit: Warning: Error querying Sample Rate",
    "LogClient: Warning: Consoles don't need hitproxy storage - wasting memory!?",
    "LogFileManager: Warning: Error deleting file:",
)


def classify_line(line: str, strict: bool) -> str | None:
    if any(pattern in line for pattern in BLOCKING_PATTERNS):
        return "blocker"
    if "Warning" in line:
        if not strict and any(fragment in line for fragment in ALLOWED_WARNING_FRAGMENTS):
            return "allowed-warning"
        return "unexpected-warning"
    return None


def scan_log(path: Path, strict: bool) -> tuple[list[str], list[str]]:
    blockers: list[str] = []
    allowed: list[str] = []
    if not path.exists():
        blockers.append(f"{path}: missing log file")
        return blockers, allowed

    for index, line in enumerate(path.read_text(encoding="utf-8", errors="ignore").splitlines(), start=1):
        result = classify_line(line, strict)
        if result == "blocker" or result == "unexpected-warning":
            blockers.append(f"{path}:{index}: {line}")
        elif result == "allowed-warning":
            allowed.append(f"{path}:{index}: {line}")
    return blockers, allowed


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Scan Code Rescue Unreal audit smoke logs.")
    parser.add_argument("logs", nargs="*", type=Path, help="Log files to scan.")
    parser.add_argument("--strict", action="store_true", help="Fail even on currently allowed NullRHI nav/crowd warnings.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    logs = args.logs or [Path("Saved/Logs/HeadlessComprehensiveAuditSmoke.log")]
    all_blockers: list[str] = []
    all_allowed: list[str] = []

    for log in logs:
        blockers, allowed = scan_log(log, args.strict)
        all_blockers.extend(blockers)
        all_allowed.extend(allowed)

    for line in all_allowed:
        print(f"[cr-log-scan] allowed warning: {line}")

    if all_blockers:
        for line in all_blockers:
            print(f"[cr-log-scan] BLOCKER: {line}", file=sys.stderr)
        print(f"[cr-log-scan] failed with {len(all_blockers)} blocking issue(s)", file=sys.stderr)
        return 1

    print(f"[cr-log-scan] passed: scanned {len(logs)} log file(s), {len(all_allowed)} allowed warning(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
