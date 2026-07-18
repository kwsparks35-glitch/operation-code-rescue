#!/usr/bin/env python3
"""Static verifier for the save-backed survivor intel archive slice."""

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


save_h = read(SRC / "CodeRescueSaveGame.h")
gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
terminal_cpp = read(SRC / "CodeTerminalWidget.cpp")
journal_h = read(SRC / "CodeRescueObjectiveJournalWidget.h")
journal_cpp = read(SRC / "CodeRescueObjectiveJournalWidget.cpp")
archive_manifest = read(DATA / "survivor_intel_archive_manifest.tsv")
curriculum_manifest = read(DATA / "curriculum_feedback_manifest.tsv")
inventory_manifest = read(DATA / "inventory_map_journal_manifest.tsv")
access_manifest = read(DATA / "accessibility_settings_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
ledger = read(DOC_DIR / "CREATIVE_DEVELOPMENT_IMPLEMENTATION_LEDGER.md")
slice_doc = read(DOC_DIR / "SURVIVOR_INTEL_ARCHIVE_SLICE.md")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")

reset_body = function_body(gi_cpp, "void UCodeRescueGameInstance::ResetRun")
save_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::SavePersistentRunInternal")
load_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::LoadPersistentRun")
record_body = function_body(gi_cpp, "void UCodeRescueGameInstance::RecordSurvivorIntelDossier")
archive_summary_body = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetSurvivorIntelArchiveSummary")
rescue_body = function_body(gi_cpp, "void UCodeRescueGameInstance::MarkSurvivorRescued")
terminal_upload_body = function_body(terminal_cpp, "static void RecordSurvivorIntelArchiveForTerminal")
terminal_summary_body = function_body(terminal_cpp, "static FString BuildSurvivorIntelArchiveText")
bypass_body = function_body(terminal_cpp, "void UCodeTerminalWidget::OnBypassClicked")
validate_body = function_body(terminal_cpp, "void UCodeTerminalWidget::RunValidation")
journal_construct_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::NativeConstruct")
journal_refresh_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::RefreshJournal")

archive_fields = [
    "LastSurvivorIntelTerminalId",
    "LastSurvivorIntelSurvivorName",
    "LastSurvivorIntelCityLabel",
    "LastSurvivorIntelLanguage",
    "LastSurvivorIntelStatus",
    "LastSurvivorIntelSummary",
    "LastSurvivorIntelScore",
    "bHasSurvivorIntelArchiveState",
]

check_all(save_h, archive_fields, "save game must persist all survivor intel archive fields")
check_all(gi_h, archive_fields, "game instance must expose survivor intel archive runtime fields")
check_all(
    gi_h,
    ["RecordSurvivorIntelDossier", "GetSurvivorIntelArchiveSummary"],
    "game instance header must declare archive write/read helpers",
)
check_all(reset_body, archive_fields, "fresh language runs must clear survivor intel archive state")
check_all(save_body, [f"Save->{field}" for field in archive_fields], "save path must serialize archive fields")
check_all(load_body, [f"Save->{field}" for field in archive_fields], "load path must restore archive fields")
check_all(
    load_body,
    ["!LastSurvivorIntelTerminalId.IsEmpty()", "!LastSurvivorIntelSummary.IsEmpty()"],
    "load path must reject incomplete legacy archive state",
)
check_all(
    record_body,
    [
        "TerminalId.IsEmpty()",
        "SurvivorName.IsEmpty()",
        "Summary.IsEmpty()",
        "CompactSummary.ReplaceInline",
        "CompactSummary.Len() > 720",
        "FMath::Clamp(Score, 0, 100)",
        "ROUTE OPEN - survivor marker broadcasting",
        "bHasSurvivorIntelArchiveState = true",
        "SavePersistentRun()",
    ],
    "record helper must compact, validate, clamp, mark, and save archive intel",
)
check_all(
    archive_summary_body,
    [
        "SURVIVOR INTEL ARCHIVE",
        "No survivor intel has been uploaded yet",
        "Track: %s | Status: %s | Score: %d | Slot: %s",
        "Contact: %s | Terminal: %s | Route: %s",
        "LastSurvivorIntelSummary",
    ],
    "journal summary helper must expose empty and populated archive readouts",
)
check_all(
    rescue_body,
    [
        "bHasSurvivorIntelArchiveState",
        "LastSurvivorIntelSurvivorName == SurvivorName",
        "RESCUED - extraction debrief ready",
        "Archive update: survivor rescued; extraction and debrief route are ready.",
    ],
    "survivor rescue must refresh the matching saved archive status",
)
check_all(
    terminal_cpp,
    [
        '#include "CodeRescueCampaign.h"',
        "FindTerminalMissionForIntel",
        "BuildSurvivorIntelArchiveText",
        "RecordSurvivorIntelArchiveForTerminal",
        "FCodeRescueCampaign::GetMissions()",
        "FCodeRescueCampaign::GetSurvivorArchetypeProfile",
    ],
    "terminal must use campaign mission data for archive upload",
)
check_all(
    terminal_summary_body,
    [
        "Intel upload:",
        "assisted field repair",
        "clean terminal solve",
        "Contact role:",
        "Need:",
        "Rescue value:",
        "Lesson payoff:",
        "resume this language slot",
        "Next step: follow the survivor marker",
    ],
    "terminal archive summary must describe route, contact, lesson payoff, and resume continuity",
)
check_all(
    terminal_upload_body,
    [
        "FindTerminalMissionForIntel(CityIndex, ChallengeId)",
        "RecordSurvivorIntelDossier",
        "ROUTE OPEN - survivor marker broadcasting",
        "BuildSurvivorIntelArchiveText",
    ],
    "terminal upload helper must write archive state through the game instance",
)
check_all(
    bypass_body,
    [
        "RecordLearningDebrief",
        "ASSISTED LEARNING DEBRIEF",
        "RecordSurvivorIntelArchiveForTerminal",
        "true",
    ],
    "bypass path must upload assisted survivor intel archive state",
)
check_all(
    validate_body,
    [
        "BuildPostSolveAfterActionDebrief",
        "RecordLearningDebrief",
        "RecordSurvivorIntelArchiveForTerminal",
        "false",
    ],
    "clean solve path must upload clean survivor intel archive state",
)
check("UTextBlock* IntelArchiveText" in journal_h,
      "journal header must store the survivor intel archive text widget")
check_all(
    journal_construct_body,
    [
        "IntelArchiveText = WidgetTree->ConstructWidget<UTextBlock>",
        "SurvivorIntelArchiveText",
        "IntelArchiveText->SetAutoWrapText(true)",
        "Box->AddChildToVerticalBox(IntelArchiveText)",
    ],
    "journal construction must add the archive readout below the dossier",
)
check_all(
    journal_refresh_body,
    [
        "GI->GetSurvivorIntelArchiveSummary()",
        "SURVIVOR INTEL ARCHIVE",
        "Profile unavailable",
        "IntelArchiveText->SetAutoWrapText(true)",
    ],
    "journal refresh must bind archive text to selected-language saved state",
)
check_all(
    archive_manifest,
    [
        "TerminalCleanSolveUpload",
        "TerminalBypassUpload",
        "JournalArchiveCard",
        "RescueStatusRefresh",
        "LanguageResumePersistence",
        "verify_survivor_intel_archive_slice_pass.py",
    ],
    "survivor intel archive manifest must cover solve, bypass, journal, rescue, and resume paths",
)
check_all(
    curriculum_manifest,
    [
        "SurvivorIntelArchive",
        "Latest terminal success or assisted bypass writes",
        "selected-language save",
    ],
    "curriculum feedback manifest must document survivor intel archive behavior",
)
check_all(
    inventory_manifest,
    [
        "SurvivorIntelArchiveText",
        "GetSurvivorIntelArchiveSummary",
        "LastSurvivorIntelTerminalId",
    ],
    "inventory/map/journal manifest must include the archive card and persisted fields",
)
check_all(
    access_manifest,
    [
        "SurvivorIntelArchiveAccessibility",
        "SurvivorIntelArchiveText",
        "GetSurvivorIntelArchiveSummary",
    ],
    "accessibility manifest must include the archive readout",
)
check_all(
    onboarding,
    [
        "SURVIVOR INTEL ARCHIVE",
        "RESCUED dossier/archive after resume",
    ],
    "first-ten-minutes onboarding must include archive resume expectations",
)
check_all(
    visual_manifest,
    [
        "SurvivorIntelArchive",
        "latest terminal, contact, route, score",
        "start-screen language slot",
    ],
    "visual regression targets must include the archive card",
)
check_all(
    human_qa,
    [
        "SurvivorIntelArchive",
        "save/quit",
        "start-screen Resume action",
        "updates to RESCUED",
    ],
    "human QA checklist must require archive resume and rescue validation",
)
check_all(
    creative_plan,
    [
        "survivor intel dossier UI",
        "resume-safe survivor intel archive",
        "verify_survivor_intel_archive_slice_pass.py",
        "manual survivor intel archive resume review",
    ],
    "creative plan must track archive validation under the survivor intel inclusion",
)
check_all(
    ledger,
    [
        "155 named verifier references",
        "110 unique verifier scripts",
        "SURVIVOR_INTEL_ARCHIVE_SLICE.md",
        "verify_survivor_intel_archive_slice_pass.py",
        "SURVIVOR INTEL ARCHIVE",
    ],
    "creative-development ledger must include archive doc, verifier, and counts",
)
check("verify_survivor_intel_archive_slice_pass.py" in full_qa,
      "full QA must run the survivor intel archive verifier")
check("verify_survivor_intel_archive_slice_pass.py" in local_ci,
      "local CI must run the survivor intel archive verifier")
check_all(
    slice_doc,
    [
        "Survivor Intel Archive Slice",
        "SURVIVOR INTEL ARCHIVE",
        "RecordSurvivorIntelDossier",
        "start screen",
        "selected-language save",
        "Human QA Notes",
        "Validation",
    ],
    "slice doc must explain archive implementation, validation, and human QA",
)
check_all(
    progress,
    [
        "Survivor intel archive slice",
        "SURVIVOR INTEL ARCHIVE",
        "verify_survivor_intel_archive_slice_pass.py",
    ],
    "progress log must document survivor intel archive work",
)

if errors:
    for error in errors:
        print(f"[verify_survivor_intel_archive_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_survivor_intel_archive_slice_pass] PASS: survivor intel archive verified")
