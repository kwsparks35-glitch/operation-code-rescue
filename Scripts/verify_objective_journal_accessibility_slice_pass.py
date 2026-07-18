#!/usr/bin/env python3
"""Static verifier for the objective journal accessibility slice."""

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


journal_h = read(SRC / "CodeRescueObjectiveJournalWidget.h")
journal_cpp = read(SRC / "CodeRescueObjectiveJournalWidget.cpp")
access_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
onboarding = read(PROJECT_ROOT / "Content/CodeRescueData/first_ten_minutes_onboarding.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "OBJECTIVE_JOURNAL_ACCESSIBILITY_SLICE.md")

construct_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::NativeConstruct")
refresh_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::RefreshJournal")

check_all(
    journal_h,
    [
        "class UBorder",
        "UTextBlock* TitleText",
        "UTextBlock* SummaryText",
        "UBorder* PanelBorder",
    ],
    "journal header must store styled panel, title, and summary widgets",
)
check_all(
    journal_cpp,
    [
        "#include \"CodeRescueUITheme.h\"",
        "#include \"Components/Border.h\"",
        "MirrorJournalThemeFromSettings",
        "JournalStateColor",
        "JournalStateLabel",
        "GI->bHighContrastHUD",
        "GI->bReducedMotion",
        "GI->GetUITextScale()",
    ],
    "journal implementation must mirror saved accessibility theme state",
)
check_all(
    construct_body,
    [
        "Blur->SetBlurStrength(CodeRescueUI::Theme().bReducedMotion ? 1.0f : 3.0f)",
        "PanelBorder = WidgetTree->ConstructWidget<UBorder>",
        "CodeRescueUI::StylePanel(PanelBorder",
        "TEXT(\"OPERATION ROUTE JOURNAL\")",
        "SummaryText = WidgetTree->ConstructWidget<UTextBlock>",
        "SummaryText->SetAutoWrapText(true)",
        "CodeRescueUI::StyleText(SummaryText",
    ],
    "journal construct path must build a themed title and summary panel",
)
check_all(
    refresh_body,
    [
        "MirrorJournalThemeFromSettings(GI)",
        "const FCodeRescueCityMission* ActiveMission",
        "CompletedCount",
        "GI->SolvedTerminalIds.Contains(ActiveMission->TerminalId)",
        "GI->RescuedSurvivorNames.Contains(ActiveMission->SurvivorName)",
        "GI->GetLanguageName()",
        "GI->GetAccessibilitySummary()",
        "JournalStateLabel(bDone, bActive, bUnlocked)",
        "JournalStateColor(bDone, bActive, bUnlocked, GI && GI->bHighContrastHUD)",
        "TEXT(\"[%s] %03d.",
        "Row->SetAutoWrapText(true)",
        "CodeRescueUI::ScaledSize(CodeRescueUI::EType::BodySmall)",
        "MissionScrollBox->ScrollWidgetIntoView(ActiveRow)",
    ],
    "journal refresh must expose active route state, saved language, explicit labels, and scalable rows",
)
check_all(
    journal_cpp,
    [
        "TEXT(\"DONE\")",
        "TEXT(\"ACTIVE\")",
        "TEXT(\"OPEN\")",
        "TEXT(\"LOCKED\")",
        "TEXT(\"Secure %s terminal\")",
        "TEXT(\"Rescue %s\")",
        "TEXT(\"Extract and debrief\")",
    ],
    "journal state vocabulary must be readable without relying on symbols",
)
check_all(
    access_manifest,
    [
        "ObjectiveJournalAccessibility",
        "bHighContrastHUD + UITextScale + bReducedMotion",
        "text-first state labels",
        "active objective summary",
    ],
    "accessibility manifest must document objective journal coverage",
)
check_all(
    onboarding,
    [
        "Journal shows active route summary",
        "text-first active/completed state labels",
    ],
    "first-ten-minutes onboarding must point reviewers at the improved journal behavior",
)
check("verify_objective_journal_accessibility_slice_pass.py" in full_qa,
      "full QA must run the objective journal accessibility verifier")
check("verify_objective_journal_accessibility_slice_pass.py" in local_ci,
      "local CI must run the objective journal accessibility verifier")
check("Objective journal accessibility slice" in progress,
      "progress log must document the objective journal accessibility slice")
check_all(
    slice_doc,
    [
        "Objective Journal Accessibility Slice",
        "OPERATION ROUTE JOURNAL",
        "DONE",
        "ACTIVE",
        "Reduced Motion",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, player impact, verification, and human QA",
)

if errors:
    for error in errors:
        print(f"[verify_objective_journal_accessibility_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_objective_journal_accessibility_slice_pass] PASS: objective journal accessibility verified")
