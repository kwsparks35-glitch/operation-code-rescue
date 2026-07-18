#!/usr/bin/env python3
"""Static verifier for the persistent learning debrief slice."""

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

manifest = read(DATA / "persistent_learning_debrief_manifest.tsv")
curriculum = read(DATA / "curriculum_feedback_manifest.tsv")
inventory_journal = read(DATA / "inventory_map_journal_manifest.tsv")
accessibility = read(DATA / "accessibility_settings_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "PERSISTENT_LEARNING_DEBRIEF_SLICE.md")

reset_body = function_body(gi_cpp, "void UCodeRescueGameInstance::ResetRun")
save_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::SavePersistentRunInternal")
load_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::LoadPersistentRun")
record_body = function_body(gi_cpp, "void UCodeRescueGameInstance::RecordLearningDebrief")
summary_body = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetLearningDebriefJournalSummary")
validate_body = function_body(terminal_cpp, "void UCodeTerminalWidget::RunValidation")
bypass_body = function_body(terminal_cpp, "void UCodeTerminalWidget::OnBypassClicked")
construct_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::NativeConstruct")
refresh_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::RefreshJournal")

debrief_fields = [
    "LastLearningDebriefChallengeId",
    "LastLearningDebriefConcept",
    "LastLearningDebriefLanguage",
    "LastLearningDebriefSummary",
    "LastLearningDebriefScore",
    "bHasLearningDebriefState",
]

check_all(save_h, debrief_fields, "save game must persist the latest selected-language learning debrief")
check_all(
    gi_h,
    debrief_fields + [
        "RecordLearningDebrief",
        "GetLearningDebriefJournalSummary",
    ],
    "game instance header must expose learning debrief runtime state and helpers",
)
check_all(reset_body, debrief_fields, "fresh language runs must clear previous learning debrief state")
check_all(save_body, [f"Save->{field} = {field}" for field in debrief_fields],
          "save path must serialize learning debrief state")
check_all(load_body, [f"{field} = Save->{field}" for field in debrief_fields[:4]] + [
    "LastLearningDebriefScore = FMath::Max(0, Save->LastLearningDebriefScore)",
    "bHasLearningDebriefState = Save->bHasLearningDebriefState && !LastLearningDebriefSummary.IsEmpty()",
], "load path must restore learning debrief state")
check_all(
    record_body,
    [
        "CompactSummary",
        "CompactSummary.Len() > 900",
        "LastLearningDebriefChallengeId = ChallengeId",
        "LastLearningDebriefConcept",
        "LastLearningDebriefLanguage",
        "LastLearningDebriefScore = FMath::Clamp(Score, 0, 100)",
        "LastLearningDebriefSummary = CompactSummary",
        "bHasLearningDebriefState = true",
        "SavePersistentRun()",
    ],
    "record helper must compact, store, flag, and save the debrief",
)
check_all(
    summary_body,
    [
        "LAST LEARNING DEBRIEF",
        "No solved terminal debrief saved yet",
        "Track: %s | Challenge: %s | Concept: %s | Score: %d | Slot: %s",
        "LastLearningDebriefSummary",
    ],
    "journal summary helper must expose empty and populated debrief states",
)
check_all(
    validate_body,
    [
        "const FString PostSolveDebrief = BuildPostSolveAfterActionDebrief",
        "GI->RecordLearningDebrief(",
        "GetConceptLabelForChallenge(TerminalActor->Challenge.Id)",
        "GetTerminalLanguageLabel(TerminalActor->Challenge.Language)",
        "Result.Score",
        "PostSolveDebrief",
    ],
    "successful terminal validation must persist the generated post-solve debrief",
)
check_all(
    bypass_body,
    [
        "GI->RecordLearningDebrief(",
        "ASSISTED LEARNING DEBRIEF",
        "Bypass kit opened the survivor route",
        "clean-solve rewards are disabled",
        "GetLanguageTransferForChallenge(ChallengeId, Language)",
    ],
    "bypass terminal flow must persist an assisted learning debrief",
)
check("UTextBlock* LearningDebriefText" in journal_h,
      "journal header must keep a learning debrief text widget")
check_all(
    construct_body,
    [
        "LearningDebriefText = WidgetTree->ConstructWidget<UTextBlock>",
        "LearningDebriefReadoutText",
        "LearningDebriefText->SetAutoWrapText(true)",
        "Box->AddChildToVerticalBox(LearningDebriefText)",
    ],
    "journal construction must add the persistent learning debrief readout",
)
check_all(
    refresh_body,
    [
        "GI->GetLearningDebriefJournalSummary()",
        "LAST LEARNING DEBRIEF",
        "LearningDebriefText->SetAutoWrapText(true)",
        "CodeRescueUI::Color::TerminalGreen()",
    ],
    "journal refresh must show the saved learning debrief with text-first styling",
)
check_all(
    manifest,
    [
        "CleanSolveDebrief",
        "AssistedBypassDebrief",
        "JournalDebriefReadout",
        "StartScreenResumeContinuity",
    ],
    "persistent learning debrief manifest must document runtime and resume contracts",
)
check_all(
    curriculum,
    [
        "PersistentLearningDebrief",
        "RecordLearningDebrief + Objective journal LAST LEARNING DEBRIEF",
        "relaunch from the start screen resume slot",
    ],
    "curriculum feedback manifest must include persistent debrief coverage",
)
check_all(
    inventory_journal,
    [
        "LearningDebriefReadoutText",
        "GetLearningDebriefJournalSummary",
        "LastLearningDebriefChallengeId",
        "verify_persistent_learning_debrief_slice_pass.py",
    ],
    "inventory/journal manifest must include the learning debrief readout",
)
check_all(
    accessibility,
    [
        "PersistentLearningDebriefAccessibility",
        "LearningDebriefReadoutText + GetLearningDebriefJournalSummary",
        "selected-language run can review the latest terminal takeaway",
    ],
    "accessibility manifest must include text-first persistent debrief coverage",
)
check_all(
    onboarding,
    [
        "journal LAST LEARNING DEBRIEF stores the latest selected-language takeaway",
        "LAST LEARNING DEBRIEF",
    ],
    "first-ten-minutes onboarding must tell reviewers to inspect the saved debrief",
)
check_all(
    visual,
    [
        "PersistentLearningDebrief",
        "Journal LAST LEARNING DEBRIEF",
        "Closing the terminal or resuming from the start-screen language slot",
    ],
    "visual regression targets must include the persistent debrief surface",
)
check_all(
    human_qa,
    [
        "PersistentLearningDebrief",
        "save/quit, relaunch from the start-screen Resume action",
        "saved post-solve or assisted learning takeaway before and after resume",
    ],
    "human QA checklist must require save/resume review of the learning debrief",
)
check_all(
    creative_plan,
    [
        "terminal post-solve after-action debrief",
        "verify_persistent_learning_debrief_slice_pass.py",
        "manual persistent learning debrief resume review",
    ],
    "creative inclusion plan must route terminal debrief through persistent review",
)
check("verify_persistent_learning_debrief_slice_pass.py" in full_qa,
      "full QA must run the persistent learning debrief verifier")
check("verify_persistent_learning_debrief_slice_pass.py" in local_ci,
      "local CI must run the persistent learning debrief verifier")
check("Persistent learning debrief slice" in progress,
      "progress log must document the persistent learning debrief slice")
check_all(
    slice_doc,
    [
        "Persistent Learning Debrief Slice",
        "RecordLearningDebrief",
        "LAST LEARNING DEBRIEF",
        "ASSISTED LEARNING DEBRIEF",
        "Human QA Notes",
    ],
    "slice doc must explain source guidance, runtime implementation, and QA",
)

if errors:
    for error in errors:
        print(f"[verify_persistent_learning_debrief_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_persistent_learning_debrief_slice_pass] PASS: persistent learning debrief verified")
