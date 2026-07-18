#!/usr/bin/env python3
"""Static verifier for the pause-menu difficulty matrix clarity slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DOC_DIR = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def check(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def check_all(source: str, tokens: list[str], message: str) -> None:
    missing = [token for token in tokens if token not in source]
    if missing:
        errors.append(f"{message}: missing {', '.join(missing)}")


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        errors.append(f"missing function {signature}")
        return ""
    brace = source.find("{", start)
    if brace < 0:
        errors.append(f"missing body for {signature}")
        return ""
    depth = 0
    for idx in range(brace, len(source)):
        char = source[idx]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                _cr_body = source[brace : idx + 1]  # 2026-07-04 BuildWidgetTreeNow migration
                if "::NativeConstruct" in signature and "BuildWidgetTreeNow();" in _cr_body:
                    return function_body(source, signature.replace("::NativeConstruct", "::BuildWidgetTreeNow"))
                return _cr_body
    errors.append(f"unterminated function {signature}")
    return ""


pause_h = read(SRC / "CodeRescuePauseWidget.h")
pause_cpp = read(SRC / "CodeRescuePauseWidget.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "PAUSE_DIFFICULTY_MATRIX_SLICE.md")
difficulty_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/difficulty_presets.tsv")
onboarding_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/first_ten_minutes_onboarding.tsv")

construct_body = function_body(pause_cpp, "void UCodeRescuePauseWidget::NativeConstruct")
refresh_body = function_body(pause_cpp, "void UCodeRescuePauseWidget::RefreshDifficultyLabel")
click_body = function_body(pause_cpp, "void UCodeRescuePauseWidget::OnDifficultyClicked")
health_body = function_body(pause_cpp, "float DifficultyHealthMultiplier")
damage_body = function_body(pause_cpp, "float DifficultyDamageMultiplier")
intent_body = function_body(pause_cpp, "FString DifficultyIntent")
expectation_body = function_body(pause_cpp, "FString DifficultyFirstTenMinutesExpectation")

check("DifficultyDetailText" in pause_h and "DifficultyDetailText" in pause_cpp,
      "pause widget must own a difficulty detail text block")
check_all(
    construct_body,
    [
        "CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD",
        "CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion",
        "CodeRescueUI::Theme().TextScale = GI->GetUITextScale()",
        # 2026-07-11 refresh: the 07-10 field-armory redesign replaced the
        # grouped-prefix labels with plain action labels and a 10.0f blur.
        "CodeRescueUI::Theme().bReducedMotion ? 4.0f : 10.0f",
        "RESUME  [P / ESC]",
        "SAVE NOW",
        "LOAD LAST SAVE",
        "MANAGE SAVE SLOTS",
        "BALANCE: DIFFICULTY",
        "SETTINGS",
        "REPLAY TUTORIAL",
        "CRAFTING WORKBENCH",
        "SKILL TREE",
        "QUIT TO DESKTOP",
    ],
    "pause menu must mirror theme settings and expose grouped information architecture labels",
)
check_all(
    refresh_body,
    [
        "Story -> Easy -> Normal -> Hard -> Survival -> Nightmare",
        "Zombie health x%.2f",
        "Zombie damage x%.2f",
        "DifficultyHealthMultiplier(D)",
        "DifficultyDamageMultiplier(D)",
        "DifficultyIntent(D)",
        "DifficultyFirstTenMinutesExpectation(D)",
        "First-ten-minutes expectation",
        "Saved immediately",
    ],
    "difficulty refresh must show cycle order, multipliers, intent, first-ten-minutes guidance, and save effect",
)
for token in ("Story", "Easy", "Hard", "Survival", "Nightmare", "1.00f"):
    check(token in health_body and token in damage_body, f"difficulty multiplier tables must include {token}")
check_all(
    intent_body + expectation_body,
    [
        "Learning-first route",
        "Approachable survival pressure",
        "Default balance",
        "Experienced survival-horror pressure",
        "Resource-aware high-pressure",
        "Repeat-player challenge mode",
        "First-time players",
        "single-hit protection",
    ],
    "difficulty copy must cover all six preset intents and QA expectations",
)
check_all(
    click_body,
    [
        "EGameDifficulty::Story",
        "EGameDifficulty::Easy",
        "EGameDifficulty::Normal",
        "EGameDifficulty::Hard",
        "EGameDifficulty::Survival",
        "EGameDifficulty::Nightmare",
        "GI->SavePersistentRun()",
        "RefreshDifficultyLabel()",
    ],
    "difficulty click must cycle every preset, save immediately, and refresh the matrix",
)
check_all(
    difficulty_manifest,
    ["Story", "Easy", "Normal", "Hard", "Survival", "Nightmare", "First-time players"],
    "difficulty manifest must remain aligned with the in-menu matrix",
)
check("terminal" in onboarding_manifest and "Review mastery" in onboarding_manifest,
      "first-ten-minutes onboarding manifest must remain available for matrix guidance")
check("verify_pause_difficulty_matrix_slice_pass.py" in full_qa,
      "full QA must run the pause difficulty matrix verifier")
check("verify_pause_difficulty_matrix_slice_pass.py" in local_ci,
      "local CI must run the pause difficulty matrix verifier")
check("Pause difficulty matrix slice" in progress,
      "progress log must document the pause difficulty matrix slice")
check_all(
    slice_doc,
    [
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "difficulty_presets",
        "first_ten_minutes_onboarding",
        "Story",
        "Nightmare",
        "pause menu",
        "saved immediately",
    ],
    "slice doc must map the pause difficulty matrix work to the June 25 guidance",
)

if errors:
    for error in errors:
        print(f"[verify_pause_difficulty_matrix_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_pause_difficulty_matrix_slice_pass] PASS: pause difficulty matrix slice verified")
