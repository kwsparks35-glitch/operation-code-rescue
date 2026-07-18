#!/usr/bin/env python3
"""Static verifier for the launch, posture, narration, HUD, and architecture readability fixes."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DOC = PROJECT_ROOT / "Documentation"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def check(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


game_mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
game_mode_h = read(SRC / "CodeRescueGameMode.h")
main_menu_cpp = read(SRC / "CodeRescueMainMenuWidget.cpp")
main_menu_h = read(SRC / "CodeRescueMainMenuWidget.h")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
hud_h = read(SRC / "CodeRescueHUDWidget.h")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
progress = read(PROJECT_ROOT / "progress.md")
pass_doc = read(DOC / "improvement_pass_2026-06-19/45_PLAYABILITY_READABILITY_FIX_PASS.md")

# Launch-language chooser must put readable labels in the screen-space widget
# and keep a readable in-world fallback for screenshot paths that do not capture
# UMG. This supersedes the older "symbols only" launch-stage rule because the
# current start-screen requirement is explicit, selectable language identity.
launch_scene = game_mode_cpp.split("void ACodeRescueGameMode::SpawnLaunchLanguageSelectionScene")[1].split(
    "\n}\n", 1)[0] if "void ACodeRescueGameMode::SpawnLaunchLanguageSelectionScene" in game_mode_cpp else ""
check("SetLaunchLanguageOnly(true)" in game_mode_cpp and "bLaunchLanguageOnly" in main_menu_h,
      "launch must show the dedicated language-only UI before active play")
check("Launch-only language widget ready" in main_menu_cpp,
      "launch-only widget must log that language buttons are ready")
check("LaunchLanguageCamera" in launch_scene and "Launch Language Solid Platform" in launch_scene,
      "launch scene must still render a nonverbal backdrop through the dedicated launch camera")
check("SELECT CODING LANGUAGE" in launch_scene and "NEW RUN OR RESUME SAVE" in launch_scene,
      "launch scene must render a readable fallback explaining new/resume language selection")
for language_label in ("TEXT(\"JAVA\")", "TEXT(\"C\")", "TEXT(\"C++\")", "TEXT(\"PYTHON\")", "TEXT(\"MATLAB\")"):
    check(language_label in launch_scene, f"launch scene must include fallback label source for {language_label}")
check("TRACK ONLY" in launch_scene, "launch scene must label each visible language as the selected-track target")

# Player posture: camera pitch may move the camera, never the character capsule.
check("bUseControllerRotationPitch = false;" in character_cpp,
      "character must keep controller pitch off the capsule")
check("CameraPerspective = 1;" in character_cpp,
      "fresh play must start in readable third-person")
check("prone/flying posture" in character_cpp,
      "posture fix must be documented in code near the camera default")

# Narration: intelligible speech is the default, generated cooked cues are opt-in.
check("bool bPreferCookedRadioBriefingCues = false;" in game_mode_h,
      "cooked/generated narration cues must be opt-in by default")
check("-v Samantha -r 165" in game_mode_cpp,
      "runtime radio narration must default to clear macOS Samantha speech at a moderate rate")
check("UseCookedRadioVoice" in game_mode_cpp and "bCookedRadioVoiceAllowed" in game_mode_cpp,
      "generated cooked narration must remain available only behind an explicit opt-in")
check("UCodeRescueSubtitlesWidget::Push(Mission.RadioBriefing, 12.0f)" in game_mode_cpp,
      "subtitles must remain the longer guaranteed radio baseline")

# HUD: health/navigation/weapons/items must be explicit, not hidden in prose.
check("NavigationStripText" in hud_h and "WeaponStripText" in hud_h,
      "HUD must own explicit navigation and weapon strip fields")
check("BespokeNavigationPanel" in hud_cpp and "NAVIGATION" in hud_cpp,
      "HUD must render a persistent navigation panel")
check("WEAPON SLOT" in hud_cpp and "1-0 select, Wheel/[ ] cycle" in hud_cpp,
      "HUD must render weapon/ammo/item cycling instructions")
check("PLAYER HEALTH  %.0f / %.0f" in hud_cpp and "HealthBar->SetPercent" in hud_cpp,
      "HUD must render visible numeric health and a health bar")

# Architecture clarity: purpose-coded route and nonblocking decorative clutter.
check("SpawnPurposeClarityLayer" in game_mode_h and "CodeRescueArchitectureClarity" in game_mode_cpp,
      "game mode must define and log the architecture clarity layer")
check("PurposeCodedArchitecture" in game_mode_cpp and "CriticalPathNonBlockingArchitecture" in game_mode_cpp,
      "architecture pass must tag purpose-coded and route-adjacent nonblocking actors")
check("NAVIGATION LEGEND" in game_mode_cpp and "OBJECTIVE 2" in game_mode_cpp and "EXTRACTION" in game_mode_cpp,
      "architecture pass must add readable functional route labels")
check("SpawnPurposeClarityLayer(Mission, CityIndex, Origin, CityLabel);" in game_mode_cpp,
      "campaign city spawning must run the architecture clarity layer")

# Documentation and QA registration.
check("verify_june19_playability_readability_fix_pass.py" in full_qa,
      "full QA must run this regression verifier")
check("Playability readability fix" in progress,
      "progress ledger must document the fix pass")
check("CodeRescueArchitectureClarity" in pass_doc and "bPreferCookedRadioBriefingCues" in pass_doc,
      "pass documentation must capture architecture and narration decisions")

if errors:
    for error in errors:
        print(f"[verify_june19_playability_readability_fix_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_june19_playability_readability_fix_pass] PASS: playability readability fixes verified")
