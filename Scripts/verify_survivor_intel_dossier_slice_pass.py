#!/usr/bin/env python3
"""Static verifier for the survivor intel dossier slice."""

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


journal_h = read(SRC / "CodeRescueObjectiveJournalWidget.h")
journal_cpp = read(SRC / "CodeRescueObjectiveJournalWidget.cpp")
curriculum_manifest = read(DATA / "curriculum_feedback_manifest.tsv")
access_manifest = read(DATA / "accessibility_settings_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "SURVIVOR_INTEL_DOSSIER_SLICE.md")

construct_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::NativeConstruct")
refresh_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::RefreshJournal")
dossier_body = function_body(journal_cpp, "FString BuildSurvivorIntelDossier")

check("UTextBlock* IntelText" in journal_h,
      "journal header must store the survivor intel dossier text widget")
check_all(
    journal_cpp,
    [
        "FindJournalMissionProgress",
        "SurvivorIntelStatusLabel",
        "SurvivorIntelNextStep",
        "BuildSurvivorIntelDossier",
        "SURVIVOR INTEL DOSSIER",
        "LOCKED - solve terminal to decrypt",
        "ROUTE OPEN - survivor marker broadcasting",
        "RESCUED - extraction debrief ready",
    ],
    "journal implementation must define survivor intel dossier helpers and status vocabulary",
)
check_all(
    construct_body,
    [
        "IntelText = WidgetTree->ConstructWidget<UTextBlock>",
        "SurvivorIntelDossierText",
        "IntelText->SetAutoWrapText(true)",
        "Box->AddChildToVerticalBox(IntelText)",
    ],
    "journal construction must add the dossier card below the summary",
)
check_all(
    refresh_body,
    [
        "BuildSurvivorIntelDossier(ActiveMission, GI)",
        "JournalStateColor(bSurvivorRescued, bTerminalSolved, ActiveMission != nullptr",
        "GI->SolvedTerminalIds.Contains(ActiveMission->TerminalId)",
        "GI->RescuedSurvivorNames.Contains(ActiveMission->SurvivorName)",
    ],
    "journal refresh must bind dossier status to terminal and survivor progress",
)
check_all(
    dossier_body,
    [
        "Mission->SurvivorName",
        "Mission->LandmarkName",
        "Mission->DistrictStyle",
        "Mission->NovelGameplayDetail",
        "Progress->Attempts",
        "Progress->BestScore",
        "GI->GetLanguageProgressSummary()",
        "Next step:",
    ],
    "dossier text must include contact, place, lesson payoff, validation record, language run, and next step",
)
check_all(
    curriculum_manifest,
    [
        "SurvivorIntelDossier",
        "active survivor contact, location, lesson payoff, validation record, language run, and next step",
    ],
    "curriculum feedback manifest must document survivor intel dossier coverage",
)
check_all(
    access_manifest,
    [
        "ObjectiveJournalAccessibility",
        "survivor intel dossier",
    ],
    "accessibility manifest must keep the dossier under journal accessibility coverage",
)
check_all(
    onboarding,
    [
        "journal dossier changes from ROUTE OPEN to RESCUED",
        "survivor intel dossier",
    ],
    "first-ten-minutes onboarding must include dossier expectations",
)
check_all(
    visual_manifest,
    [
        "SurvivorIntelDossier",
        "survivor status, contact, location, lesson payoff, validation line, language run, and next step",
    ],
    "visual regression targets must include the survivor intel dossier",
)
check("journal survivor dossier updates after terminal solve and rescue" in human_qa,
      "human QA checklist must ask reviewers to inspect dossier state changes")
check_all(
    creative_plan,
    [
        "survivor intel dossier UI",
        "verify_survivor_intel_dossier_slice_pass.py",
    ],
    "creative development inclusion plan must track the survivor intel dossier slice",
)
check("verify_survivor_intel_dossier_slice_pass.py" in full_qa,
      "full QA must run the survivor intel dossier verifier")
check("verify_survivor_intel_dossier_slice_pass.py" in local_ci,
      "local CI must run the survivor intel dossier verifier")
check("Survivor intel dossier slice" in progress,
      "progress log must document the survivor intel dossier slice")
check_all(
    slice_doc,
    [
        "Survivor Intel Dossier Slice",
        "SURVIVOR INTEL DOSSIER",
        "ROUTE OPEN",
        "RESCUED",
        "Human QA Notes",
        "Validation",
    ],
    "slice doc must explain implementation, verification, and human QA",
)

if errors:
    for error in errors:
        print(f"[verify_survivor_intel_dossier_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_survivor_intel_dossier_slice_pass] PASS: survivor intel dossier slice verified")
