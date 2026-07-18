#!/usr/bin/env python3
"""verify_fading_review_trail_photomode_2026_07_04c.py

Gate for the 2026-07-04 part-4 slice: worked-example fading (item 31), spaced
repetition (29), terminal guidance trail (45), photo mode (50), launcher-help fix.
"""
from __future__ import annotations
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "Source" / "CodeRescueUnreal"
FAILURES: list[str] = []


def check(cond: bool, msg: str) -> None:
    print(f"[verify_fading_review_trail_photomode_2026_07_04c] {'PASS' if cond else 'FAIL'}: {msg}")
    if not cond:
        FAILURES.append(msg)


def has(path: Path, *needles: str) -> bool:
    try:
        t = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return False
    return all(n in t for n in needles)


term = SRC / "CodeTerminalWidget.cpp"
gm = SRC / "CodeRescueGameMode.cpp"
ch_h = SRC / "CodeRescueCharacter.h"
ch_c = SRC / "CodeRescueCharacter.cpp"

# item 31 — worked-example fading
check(has(term, "worked-example FADING", "Fading scaffold (1 solve in this concept):",
          "Scaffold faded (%d solves in this concept)"),
      "item 31: worked example fades full -> cue -> recall by concept solve count")
check(has(term, "ConceptSolves = Progress.SuccessfulValidations;"),
      "item 31: fade level reads saved concept progress")

# item 29 — spaced repetition
check(has(term, "SPACED REPETITION", "CityIndex % 3 == 2", "[SpacedReview]"),
      "item 29: every third city reviews the weakest concept with logging")
check(has(term, "WorstRate = 0.5f", "Attempts >= 2"),
      "item 29: weakness threshold is <50% success across >=2 attempts")

# item 45 — guidance trail
check(has(gm, "GuidanceTrail", "TrailMissions[CityIndex].TerminalId == Id", "[GuidanceTrail]"),
      "item 45: breadcrumb trail spawns for the MAIN terminal only, with logging")
check(has(gm, "GroundZAt(TrailPoint + FVector(0, 0, 200.0f)"),
      "item 45: trail strips are ground-snapped")

# item 50 — photo mode
check(has(ch_h, "bPhotoModeActive", "PhotoModeHiddenWidgets", "TogglePhotoMode"),
      "item 50: character owns photo-mode state with per-widget visibility restore")
# 2026-07-17 migration: the F10 binding is deliberately GONE. An accidental
# press (F10 sits beside the F12 screenshot key) put the whole game into 12%
# time dilation with the HUD hidden and no indicator — Kenny reported "the
# game is STILL running EXTREMELY slowly" and lost a session to it. The
# feature function remains for a future MENU entry; BeginPlay force-restores
# dilation so no stale slow-motion can survive a spawn.
check("BindKey(EKeys::F10" not in ch_c,
      "item 50 rev: NO hotkey may toggle photo mode (12%-speed trap)")
check(has(ch_c, "TogglePhotoMode", "SetGlobalTimeDilation(this, 1.0f)"),
      "item 50 rev: photo mode retained for menus; spawn restores dilation")
check(has(ch_c, "SetGlobalTimeDilation(this, 0.12f)", "SetGlobalTimeDilation(this, 1.0f)",
          "GetAllWidgetsOfClass"),
      "item 50: photo mode hides viewport widgets and slows time, restoring both")
check(has(ch_c, "IsUIOpen())"),
      "item 50: photo mode refuses to trigger under modal UI")

# launcher help honesty
check(has(ROOT / "Run_Character_World_Demo.command",
          "number keys 1-0 select WEAPONS", "F10 photo mode"),
      "launcher help text corrected (cameras C/V, weapons on numbers, F10 photo)")

print()
if FAILURES:
    print(f"[verify_fading_review_trail_photomode_2026_07_04c] {len(FAILURES)} FAILURE(S)")
    sys.exit(1)
print("[verify_fading_review_trail_photomode_2026_07_04c] ALL CHECKS PASSED")
