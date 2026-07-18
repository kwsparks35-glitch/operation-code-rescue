#!/usr/bin/env python3
"""Static verifier for the language profile recap slice."""

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


gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
journal_h = read(SRC / "CodeRescueObjectiveJournalWidget.h")
journal_cpp = read(SRC / "CodeRescueObjectiveJournalWidget.cpp")

manifest = read(DATA / "language_profile_recap_manifest.tsv")
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
slice_doc = read(DOC_DIR / "LANGUAGE_PROFILE_RECAP_SLICE.md")
ledger = read(DOC_DIR / "CREATIVE_DEVELOPMENT_IMPLEMENTATION_LEDGER.md")

recap_body = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetLanguageProfileRecapSummary")
recommend_body = function_body(gi_cpp, "FString BuildLanguageProfileReviewRecommendation")
construct_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::NativeConstruct")
refresh_body = function_body(journal_cpp, "void UCodeRescueObjectiveJournalWidget::RefreshJournal")

check("GetLanguageProfileRecapSummary" in gi_h,
      "game instance header must expose the language profile recap helper")
check_all(
    recap_body,
    [
        "LANGUAGE PROFILE RECAP",
        "Profile stats",
        "Run stats",
        "Stage recap",
        "Save-slot preview",
        "ReviewRecommendation",
        "LanguageSolveCounts",
        "LanguageAttemptCounts",
        "LanguageNoHintSolveCounts",
        "GetLearningMasteryTitle",
        "GetDifficultyDisplayName",
        "FCodeRescueCampaign::GetFirstIncompleteCityIndex",
        "MakeLanguageSaveSlotName",
        "DoesLanguageSaveExist",
        "Start-screen Resume",
        "ResearchPoints",
        "CodingScore",
        "RescueCount",
        "KillCount",
        "HeadshotCount",
        "DeathCount",
        "RunSeconds",
    ],
    "recap helper must expose stage, profile stats, recommendation, run stats, and save-slot preview",
)
check_all(
    recommend_body,
    [
        "LanguageAttempts <= 0",
        "LanguageSolves <= 0",
        "SuccessRate < 50",
        "LanguageNoHintSolves < FMath::Max",
        "bCampaignComplete",
        "Review recommendation",
        "start screen",
        "reopen the challenge replay brief",
        "Story or Easy",
        "without hints",
        "first-try full-score",
        "next city stage",
    ],
    "review recommendation helper must adapt to major learning profile states",
)
check("UTextBlock* LanguageProfileRecapText" in journal_h,
      "journal header must keep a language profile recap text widget")
check_all(
    construct_body,
    [
        "LanguageProfileRecapText = WidgetTree->ConstructWidget<UTextBlock>",
        "LanguageProfileRecapText",
        "LanguageProfileRecapText->SetAutoWrapText(true)",
        "Box->AddChildToVerticalBox(LanguageProfileRecapText)",
    ],
    "journal construction must add the language profile recap readout",
)
check_all(
    refresh_body,
    [
        "GI->GetLanguageProfileRecapSummary()",
        "LANGUAGE PROFILE RECAP",
        "stage recap, profile stats, recommendation, and save-slot preview",
        "LanguageProfileRecapText->SetAutoWrapText(true)",
    ],
    "journal refresh must render the language profile recap",
)
check_all(
    manifest,
    [
        "LanguageProfileRecapText",
        "StageRecap",
        "ProfileStats",
        "ReviewRecommendation",
        "SaveSlotPreview",
        "verify_language_profile_recap_slice_pass.py",
    ],
    "language profile recap manifest must document runtime surfaces and saved state",
)
check_all(
    curriculum,
    [
        "LanguageProfileRecap",
        "GetLanguageProfileRecapSummary + Objective journal LANGUAGE PROFILE RECAP",
        "profile stats, stage recap, review recommendation, and save-slot preview",
    ],
    "curriculum feedback manifest must include profile recap coverage",
)
check_all(
    inventory_journal,
    [
        "LanguageProfileRecapText",
        "GetLanguageProfileRecapSummary",
        "LANGUAGE PROFILE RECAP shows selected-language mastery",
        "verify_language_profile_recap_slice_pass.py",
    ],
    "inventory/journal manifest must include language profile recap as a named surface",
)
check_all(
    accessibility,
    [
        "LanguageProfileRecapAccessibility",
        "LanguageProfileRecapText + GetLanguageProfileRecapSummary",
        "stage recap, profile stats, review recommendation, and save-slot preview",
    ],
    "accessibility manifest must document the text-first profile recap",
)
check_all(
    onboarding,
    [
        "LANGUAGE PROFILE RECAP",
        "stage recap/profile stats/save-slot preview",
        "Review mastery",
    ],
    "first-ten-minutes onboarding must guide reviewers to the profile recap",
)
check_all(
    visual,
    [
        "LanguageProfileRecap",
        "Journal LANGUAGE PROFILE RECAP",
        "stage recap, review recommendation, and save-slot preview",
    ],
    "visual regression targets must include language profile recap review",
)
check_all(
    human_qa,
    [
        "LanguageProfileRecap",
        "Solve one terminal, rescue one survivor",
        "profile stats, run stats, stage recap, review recommendation, save-slot preview",
    ],
    "human QA checklist must include save/resume review of the profile recap",
)
check_all(
    creative_plan,
    [
        "terminal post-solve after-action debrief",
        "verify_language_profile_recap_slice_pass.py",
        "manual language profile recap resume review",
    ],
    "creative inclusion plan must route terminal debrief through profile recap review",
)
check("verify_language_profile_recap_slice_pass.py" in full_qa,
      "full QA must run the language profile recap verifier")
check("verify_language_profile_recap_slice_pass.py" in local_ci,
      "local CI must run the language profile recap verifier")
check("Language profile recap slice" in progress,
      "progress log must document the language profile recap slice")
check_all(
    slice_doc,
    [
        "Language Profile Recap Slice",
        "GetLanguageProfileRecapSummary",
        "LanguageProfileRecapText",
        "profile stats",
        "stage recap",
        "review recommendation",
        "save-slot preview",
        "Validation",
    ],
    "slice doc must explain implementation, player impact, validation, and human QA",
)
check_all(
    ledger,
    [
        "155 named verifier references",
        "110 unique verifier scripts",
        "LANGUAGE_PROFILE_RECAP_SLICE.md",
        "verify_language_profile_recap_slice_pass.py",
    ],
    "creative ledger must include the new language profile recap verifier and counts",
)

if errors:
    for error in errors:
        print(f"[verify_language_profile_recap_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_language_profile_recap_slice_pass] PASS: language profile recap verified")
