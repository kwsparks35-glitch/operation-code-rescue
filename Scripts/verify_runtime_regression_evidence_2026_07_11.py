#!/usr/bin/env python3
"""Verify final editor/package evidence for the July 11 regression pass."""

from pathlib import Path
import plistlib
import re


ROOT = Path(__file__).resolve().parents[1]
LOGS = ROOT / "Saved" / "Logs"
APP = ROOT / "PackagedMac" / "Mac" / "CodeRescueUnreal.app"


def read_log(name: str) -> str:
    return (LOGS / name).read_text(encoding="utf-8", errors="replace")


challenge = read_log("FirstLevelChallengeRegressionV7Final.log")
ground = read_log("CampaignGroundRecoveryRegressionV7Final.log")
terminal = read_log("TerminalContrastReviewV7Final.log")
editor_integrated = read_log("FirstLevelIntegratedRegressionV7Metal.log")
packaged_ground = read_log("PackagedCampaignGroundRecoveryV7.log")
packaged_integrated = read_log("PackagedFirstLevelIntegratedV7.log")
launch = read_log("PackagedNormalLaunchGateV7.log")

integrated_marker = (
    "[FirstLevelIntegratedAudit] COMPLETE PASS world=1 access=1 ground=1 "
    "population=1 sky=1 day_period=1 challenges=1 alternate_solution=1 "
    "progression=1 supplies=1 target_lock=1 combat=1 corpse=1 animation=1 "
    "reader=1 armory=1 pause_mouse=1 crafting=1"
)

saved_renewable = re.search(r"saved_renewable_deaths_ignored=(\d+)", packaged_ground)
population = re.search(r"city=1 renewable_alive=(\d+)", packaged_ground)

with (APP / "Contents" / "Info.plist").open("rb") as handle:
    info = plistlib.load(handle)

bad_tokens = (
    "COMPLETE FAIL",
    "LogAnimation: Warning",
    "Assertion failed",
    "ensure condition failed",
    "Fatal error",
    "Failed to load package",
)
final_logs = (
    challenge,
    ground,
    terminal,
    editor_integrated,
    packaged_ground,
    packaged_integrated,
    launch,
)

checks: list[tuple[bool, str]] = [
    ("validators=60/60 alternate_solutions=1 external_available=51" in challenge,
     "external 60-solution matrix and alternate case"),
    ("prefix_decrement=1 score=100" in challenge,
     "reported prefix-decrement C++ solution scores 100"),
    ("[CampaignGroundRecoveryAudit] COMPLETE PASS city=1" in ground
     and "recovered_relative_z=92.00" in ground,
     "editor second-city recovery"),
    ("[TerminalContrastAudit] COMPLETE PASS ratio=18.89" in terminal
     and "[TerminalContrastReview] COMPLETE PASS" in terminal,
     "terminal numeric and Metal render contrast"),
    (integrated_marker in editor_integrated,
     "editor single-session integrated acceptance"),
    (saved_renewable is not None and int(saved_renewable.group(1)) > 0,
     "packaged run exercised historical renewable deaths"),
    (population is not None and int(population.group(1)) >= 6,
     "packaged city retains renewable enemies"),
    ("[RadioVoiceArbiter] previous_voice_stopped=1" in packaged_ground,
     "packaged city switch stops prior narrator"),
    ("[CampaignGroundRecoveryAudit] COMPLETE PASS city=1" in packaged_ground,
     "packaged real-save ground recovery"),
    (integrated_marker in packaged_integrated,
     "packaged single-session integrated acceptance"),
    (all(label in launch for label in (
        "station spawned: JAVA", "station spawned: PYTHON", "station spawned: C\n",
        "station spawned: C+", "station spawned: C++", "station spawned: MATLAB")),
     "normal launch exposes all six language stations"),
    ("Launch-only language widget ready: Java, C, C+, C++, Python, MATLAB" in launch
     and "Showing launch language chooser before active play" in launch,
     "normal launch displays the language chooser"),
    ("[ProductionWorld]" not in launch
     and "[FirstLevelIntegratedAudit]" not in launch
     and "[RadioVoiceArbiter]" not in launch,
     "normal launch does not auto-start gameplay or narration"),
    (all(token not in text for text in final_logs for token in bad_tokens),
     "final evidence logs contain no failure, fatal, ensure, or animation warning"),
    (APP.is_dir(), "packaged app exists"),
    (info.get("CFBundleIdentifier") == "com.operationcoderescue.CodeRescueUnreal",
     "final bundle identifier"),
    # 2026-07-11 pattern refresh: the shipped bundle version increments with
    # every repackage (the .198 pin broke as soon as the afternoon fix pass
    # shipped .199). Require the same changelist family at or ABOVE the
    # morning pass's floor instead of one literal version.
    ((lambda v: v.startswith("51494982.0.") and v.split(".")[-1].isdigit()
      and int(v.split(".")[-1]) >= 198)(str(info.get("CFBundleVersion", ""))),
     "final bundle version"),
]

failed = [label for passed, label in checks if not passed]
for passed, label in checks:
    print(f"[{'PASS' if passed else 'FAIL'}] {label}")

if failed:
    raise SystemExit(f"{len(failed)} runtime evidence check(s) failed")

print(f"[PASS] 2026-07-11 runtime evidence ({len(checks)}/{len(checks)})")
