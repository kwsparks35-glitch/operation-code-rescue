#!/usr/bin/env python3
"""Static verifier for the fail-safe objective board journal slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
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


game_h = read(SRC / "CodeRescueGameInstance.h")
game_cpp = read(SRC / "CodeRescueGameInstance.cpp")
journal_h = read(SRC / "CodeRescueObjectiveJournalWidget.h")
journal_cpp = read(SRC / "CodeRescueObjectiveJournalWidget.cpp")
board_manifest = read(DATA / "fail_safe_objective_board_manifest.tsv")
inventory_manifest = read(DATA / "inventory_map_journal_manifest.tsv")
access_manifest = read(DATA / "accessibility_settings_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "FAIL_SAFE_OBJECTIVE_BOARD_SLICE.md")
ledger = read(DOC_DIR / "CREATIVE_DEVELOPMENT_IMPLEMENTATION_LEDGER.md")

board_body = function_body(game_cpp, "FString UCodeRescueGameInstance::GetFailSafeObjectiveBoardSummary")
construct_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::NativeConstruct")
refresh_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::RefreshJournal")

check_all(
    game_h,
    [
        "GetFailSafeObjectiveBoardSummary",
        "BlueprintCallable",
        "BlueprintPure",
        "Category=\"Objectives\"",
    ],
    "game instance header must expose a pure fail-safe objective board summary",
)
check_all(
    board_body,
    [
        "FAIL-SAFE OBJECTIVE BOARD",
        "MakeLanguageSaveSlotName(SelectedLanguage)",
        "DoesLanguageSaveExist(SelectedLanguage)",
        "FCodeRescueCampaign::GetFirstIncompleteCityIndex(this)",
        "FCodeRescueCampaign::IsCityCompleted(this, i)",
        "SolvedTerminalIds.Contains",
        "RescuedSurvivorNames.Contains",
        "Start-screen Resume",
        "protected terminal",
        "survivor route",
        "extraction/debrief",
        "Return markers: T objective jump | Backspace/F8 safe recovery | J journal | P/Esc pause/save",
        "protected terminal suppresses combat pressure",
        "Next action",
    ],
    "fail-safe summary must be derived from saved language progress and expose recovery guidance",
)
check_all(
    journal_h,
    [
        "UTextBlock* FailSafeObjectiveBoardText",
        "UTextBlock* RouteMapText",
    ],
    "journal header must own the fail-safe board text block",
)
check_all(
    construct_body,
    [
        "FailSafeObjectiveBoardText = WidgetTree->ConstructWidget<UTextBlock>",
        "TEXT(\"FailSafeObjectiveBoardText\")",
        "FailSafeSlot->SetPadding",
    ],
    "journal construction must create a named fail-safe board surface",
)
check_all(
    refresh_body,
    [
        "FailSafeObjectiveBoardText->SetText",
        "GI->GetFailSafeObjectiveBoardSummary()",
        "Resume a selected-language run",
        "active route, return markers, recovery controls, and start-screen Resume state",
        "FailSafeObjectiveBoardText->SetAutoWrapText(true)",
        "GI && GI->bHighContrastHUD",
    ],
    "journal refresh must populate and style the fail-safe board from live game state",
)
check_all(
    board_manifest,
    [
        "FailSafeObjectiveBoardText",
        "ReturnMarkers",
        "ProtectedTerminalSafety",
        "ResumeContinuity",
        "T objective jump",
        "Backspace/F8 safe recovery",
        "P/Esc pause/save",
        "start-screen Resume",
        "verify_fail_safe_objective_board_slice_pass.py",
    ],
    "fail-safe objective board manifest must document runtime surface, saved state, and verification",
)
check_all(
    inventory_manifest,
    [
        "FailSafeObjectiveBoardText",
        "GetFailSafeObjectiveBoardSummary",
        "active route phase",
        "safe recovery controls",
        "protected terminal safety",
    ],
    "inventory journal manifest must include the fail-safe board row",
)
check_all(
    access_manifest,
    [
        "InventoryMapJournalAccessibility",
        "LanguageSaveContinuityText + RouteMapReadoutText + FieldInventoryReadoutText",
        "FailSafeObjectiveBoardText",
        "FailSafeObjectiveBoardAccessibility",
        "text-first return markers",
        "protected terminal safety",
        "text-first inventory, route, save, and intel summaries",
    ],
    "accessibility manifest must preserve inventory coverage and add fail-safe board accessibility",
)
check(
    "inventory map and journal polish" in creative_plan
    and "verify_inventory_map_journal_polish_slice_pass.py plus verify_minimap_route_readability_slice_pass.py plus verify_objective_journal_accessibility_slice_pass.py plus packaged smoke plus manual UI pass plus verify_fail_safe_objective_board_slice_pass.py" in creative_plan,
    "creative development plan must route the P1 UI row through the fail-safe board verifier",
)
check_all(
    visual_targets,
    [
        "FailSafeObjectiveBoard",
        "FAIL-SAFE OBJECTIVE BOARD card",
        "Backspace/F8 recovery",
        "protected terminal safety",
    ],
    "visual regression targets must include the fail-safe objective board",
)
check_all(
    human_qa,
    [
        "FailSafeObjectiveBoard",
        "open J before terminal solve",
        "start-screen Resume availability",
        "T objective jump, Backspace/F8 safe recovery, J journal, P/Esc pause/save",
        "InventoryMapJournalPolish",
        "language save, route map, field inventory, survivor intel",
    ],
    "human QA checklist must include the fail-safe board and preserve inventory journal review",
)
check_all(
    onboarding,
    [
        "journal FAIL-SAFE OBJECTIVE BOARD",
        "return markers, protected terminal safety, and start-screen Resume state",
        "T objective jump, Backspace/F8 safe recovery, and P/Esc pause/save",
        "journal field ops readout",
        "language save, route map, field inventory, survivor intel",
    ],
    "first-ten-minutes checklist must teach the fail-safe board without losing existing journal guidance",
)
check("verify_fail_safe_objective_board_slice_pass.py" in full_qa,
      "full QA must run the fail-safe objective board verifier")
check("verify_fail_safe_objective_board_slice_pass.py" in local_ci,
      "local CI must run the fail-safe objective board verifier")
check("Fail-safe objective board slice" in progress,
      "progress log must document the fail-safe objective board slice")
check_all(
    slice_doc,
    [
        "Fail-Safe Objective Board Slice",
        "GetFailSafeObjectiveBoardSummary",
        "FailSafeObjectiveBoardText",
        "Start-screen Resume",
        "T objective jump",
        "Backspace/F8 safe recovery",
        "protected terminal",
        "Verification",
    ],
    "slice doc must explain implementation, player impact, validation, and human QA",
)
check_all(
    ledger,
    [
        "155 named verifier references",
        "110 unique verifier scripts",
        "FAIL_SAFE_OBJECTIVE_BOARD_SLICE.md",
        "verify_fail_safe_objective_board_slice_pass.py",
    ],
    "creative ledger must include the fail-safe objective board verifier and updated counts",
)

if errors:
    for error in errors:
        print(f"[verify_fail_safe_objective_board_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_fail_safe_objective_board_slice_pass] PASS: fail-safe objective board verified")
