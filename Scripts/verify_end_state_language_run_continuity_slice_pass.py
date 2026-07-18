#!/usr/bin/env python3
"""Static verifier for the end-state language run continuity slice."""

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


death_h = read(SRC / "CodeRescueDeathWidget.h")
death_cpp = read(SRC / "CodeRescueDeathWidget.cpp")
victory_h = read(SRC / "CodeRescueVictoryWidget.h")
victory_cpp = read(SRC / "CodeRescueVictoryWidget.cpp")
access_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
visual_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
safe_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/safe_learning_city_controls_manifest.tsv")
qa_checklist = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
onboarding = read(PROJECT_ROOT / "Content/CodeRescueData/first_ten_minutes_onboarding.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "END_STATE_LANGUAGE_RUN_CONTINUITY_SLICE.md")

death_construct = function_body(death_cpp, "void UCodeRescueDeathWidget::NativeConstruct")
death_summary = function_body(death_cpp, "FString BuildDeathLanguageSummary")
death_stats = function_body(death_cpp, "FString BuildDeathStatsText")
victory_construct = function_body(victory_cpp, "void UCodeRescueVictoryWidget::NativeConstruct")
victory_summary = function_body(victory_cpp, "FString BuildVictoryLanguageSummary")
victory_stats = function_body(victory_cpp, "FString BuildVictoryStatsText")
victory_quit = function_body(victory_cpp, "void UCodeRescueVictoryWidget::OnQuitClicked")

check_all(
    death_h,
    [
        "UTextBlock* SummaryText",
        "SaveAndQuitButton",
        "OnSaveAndQuitClicked",
    ],
    "death header must expose the language summary and preserve recovery buttons",
)
check_all(
    victory_h,
    [
        "UTextBlock* SummaryText",
        "RestartButton",
        "QuitButton",
    ],
    "victory header must expose the language summary and preserve end-state buttons",
)
check_all(
    death_cpp,
    [
        "#include \"Components/ScrollBox.h\"",
        "MirrorDeathThemeFromSettings",
        "FormatDeathRunTime",
        "BuildDeathLanguageSummary",
        "BuildDeathStatsText",
        "CodeRescueUI::Theme().bHighContrast",
        "CodeRescueUI::Theme().bReducedMotion",
        "GI->GetUITextScale()",
    ],
    "death widget must mirror accessibility settings and use end-state helpers",
)
check_all(
    victory_cpp,
    [
        "#include \"Components/ScrollBox.h\"",
        "MirrorVictoryThemeFromSettings",
        "FormatVictoryRunTime",
        "BuildVictoryLanguageSummary",
        "BuildVictoryStatsText",
        "CodeRescueUI::Theme().bHighContrast",
        "CodeRescueUI::Theme().bReducedMotion",
        "GI->GetUITextScale()",
    ],
    "victory widget must mirror accessibility settings and use end-state helpers",
)
check_all(
    death_summary,
    [
        "Active language: %s | Start-screen resume: %s",
        "MakeLanguageSaveSlotName(GI->SelectedLanguage)",
        "GetLanguageProgressSummary()",
    ],
    "death summary must show active language, start-screen resume slot, and language progress",
)
check_all(
    victory_summary,
    [
        "Completed language: %s | Start-screen resume: %s",
        "MakeLanguageSaveSlotName(GI->SelectedLanguage)",
        "GetLanguageProgressSummary()",
    ],
    "victory summary must show completed language, start-screen resume slot, and language progress",
)
check_all(
    death_stats,
    [
        "Research points: %d",
        "Run time: %s | Deaths: %d | Headshots: %d",
        "FormatDeathRunTime(GI->RunSeconds)",
    ],
    "death stats must include expanded run-state details",
)
check_all(
    victory_stats,
    [
        "Research points remaining: %d",
        "Run time: %s | Deaths: %d | Headshots: %d",
        "FormatVictoryRunTime(GI->RunSeconds)",
    ],
    "victory stats must include expanded run-state details",
)
check_all(
    death_construct,
    [
        "DeathEndStateScroll",
        "Blur->SetBlurStrength((GI && GI->bReducedMotion) ? 4.0f : 14.0f)",
        "DeathLanguageSummary",
        "RESUME FROM LANGUAGE SAVE",
        "START FRESH LANGUAGE RUN (delete this save)",
        "SAVE THIS LANGUAGE RUN AND QUIT",
    ],
    "death construct path must build scrollable reduced-motion language-run recovery UI",
)
check_all(
    victory_construct,
    [
        "VictoryEndStateScroll",
        "Blur->SetBlurStrength((GI && GI->bReducedMotion) ? 3.0f : 10.0f)",
        "VictoryLanguageSummary",
        "START NEW LANGUAGE RUN",
        "SAVE COMPLETION AND QUIT",
        "GI->SavePersistentRun()",
    ],
    "victory construct path must build scrollable reduced-motion language-run completion UI and save completion",
)
check_all(
    victory_quit,
    [
        "GI->SavePersistentRun()",
        "QuitGame",
    ],
    "victory quit must preserve completed language run before closing",
)
check_all(
    access_manifest,
    [
        "EndStateLanguageRunAccessibility",
        "Death and victory end-state overlays",
        "active language resume slot",
    ],
    "accessibility manifest must document death/victory end-state coverage",
)
check_all(
    visual_manifest,
    [
        "EndStateLanguageRun",
        "Death/victory title",
        "resume behavior clear",
    ],
    "visual regression targets must include end-state language-run screens",
)
check_all(
    safe_manifest,
    [
        "resume from language save",
        "start-screen language resume contract",
    ],
    "safe-learning manifest must document language-aware death recovery",
)
check_all(
    qa_checklist,
    [
        "death/victory language-run summaries",
        "start-screen language resume slot",
    ],
    "human QA checklist must require end-state language-run review",
)
check_all(
    onboarding,
    [
        "end-state appears",
        "language resume summary",
        "death/victory screens show the start-screen language resume slot",
    ],
    "first ten minutes onboarding must mention end-state language resume summaries",
)
check("verify_end_state_language_run_continuity_slice_pass.py" in full_qa,
      "full QA must run the end-state language continuity verifier")
check("verify_end_state_language_run_continuity_slice_pass.py" in local_ci,
      "local CI must run the end-state language continuity verifier")
check("End state language run continuity slice" in progress,
      "progress log must document the end-state language run continuity slice")
check_all(
    slice_doc,
    [
        "End State Language Run Continuity Slice",
        "CodeRescueDeathWidget",
        "CodeRescueVictoryWidget",
        "MakeLanguageSaveSlotName",
        "SavePersistentRun",
        "SAVE THIS LANGUAGE RUN AND QUIT",
        "SAVE COMPLETION AND QUIT",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, player impact, verification, and human QA",
)

if errors:
    for error in errors:
        print(f"[verify_end_state_language_run_continuity_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_end_state_language_run_continuity_slice_pass] PASS: end-state language run continuity verified")
