#!/usr/bin/env python3
"""Static verifier for the terminal reward choice kiosk slice."""

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


terminal_h = read(SRC / "CodeTerminalWidget.h")
terminal_cpp = read(SRC / "CodeTerminalWidget.cpp")
gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
save_h = read(SRC / "CodeRescueSaveGame.h")
reward_manifest = read(DATA / "terminal_reward_choice_kiosk_manifest.tsv")
curriculum_manifest = read(DATA / "curriculum_feedback_manifest.tsv")
selected_manifest = read(DATA / "selected_language_terminal_flow_manifest.tsv")
access_manifest = read(DATA / "accessibility_settings_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "TERMINAL_REWARD_CHOICE_KIOSK_SLICE.md")
ledger = read(DOC_DIR / "CREATIVE_DEVELOPMENT_IMPLEMENTATION_LEDGER.md")

construct_body = function_body(terminal_cpp, "void UCodeTerminalWidget::NativeConstruct")
run_body = function_body(terminal_cpp, "void UCodeTerminalWidget::RunValidation")
bypass_body = function_body(terminal_cpp, "void UCodeTerminalWidget::OnBypassClicked")
claim_widget_body = function_body(terminal_cpp, "void UCodeTerminalWidget::ClaimRewardChoice")
update_buttons_body = function_body(terminal_cpp, "void UCodeTerminalWidget::UpdateRewardChoiceButtons")
eligible_body = function_body(gi_cpp, "void UCodeRescueGameInstance::MarkTerminalRewardChoiceEligible")
available_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::IsTerminalRewardChoiceAvailable")
claim_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::ClaimTerminalRewardChoice")
summary_body = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetTerminalRewardChoiceSummary")
save_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::SavePersistentRunInternal")
load_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::LoadPersistentRun")
reset_body = function_body(gi_cpp, "void UCodeRescueGameInstance::ResetRun")

check_all(
    save_h + gi_h,
    [
        "RewardChoiceEligibleTerminalIds",
        "ClaimedTerminalRewardChoiceIds",
        "LastTerminalRewardChoiceTerminalId",
        "LastTerminalRewardChoiceId",
        "LastTerminalRewardChoiceSummary",
        "bHasTerminalRewardChoiceState",
        "MarkTerminalRewardChoiceEligible",
        "IsTerminalRewardChoiceAvailable",
        "HasClaimedTerminalRewardChoice",
        "ClaimTerminalRewardChoice",
        "GetTerminalRewardChoiceSummary",
    ],
    "save game and game instance must expose reward choice eligibility, claim, and summary state",
)
check_all(
    save_body + load_body + reset_body,
    [
        "Save->RewardChoiceEligibleTerminalIds = RewardChoiceEligibleTerminalIds",
        "Save->ClaimedTerminalRewardChoiceIds = ClaimedTerminalRewardChoiceIds",
        "Save->LastTerminalRewardChoiceSummary = LastTerminalRewardChoiceSummary",
        "RewardChoiceEligibleTerminalIds = Save->RewardChoiceEligibleTerminalIds",
        "ClaimedTerminalRewardChoiceIds = Save->ClaimedTerminalRewardChoiceIds",
        "RewardChoiceEligibleTerminalIds.RemoveAll",
        "ClaimedTerminalRewardChoiceIds.RemoveAll",
        "RewardChoiceEligibleTerminalIds.Reset()",
        "ClaimedTerminalRewardChoiceIds.Reset()",
    ],
    "reward choice state must save, load, sanitize, and reset with the selected-language profile",
)
check_all(
    construct_body + terminal_h,
    [
        "RewardChoiceActionRow",
        "RewardChoiceResearchButton",
        "RewardChoiceFieldKitButton",
        "RewardChoiceCraftingButton",
        "REWARD: RESEARCH +2 RP",
        "REWARD: FIELD KIT",
        "REWARD: CRAFTING CACHE",
        "OnRewardResearchClicked",
        "OnRewardFieldKitClicked",
        "OnRewardCraftingClicked",
    ],
    "terminal UI must create three explicit reward choice buttons",
)
check_all(
    run_body,
    [
        "Result.bSuccess && !bPracticeOnly",
        "GI->MarkTerminalRewardChoiceEligible(TerminalActor->Challenge.Id)",
        "GI->GetTerminalRewardChoiceSummary(TerminalActor->Challenge.Id)",
        "Intel Reward: survivor whereabouts uploaded",
    ],
    "live validation success must unlock and surface the reward kiosk",
)
check("MarkTerminalRewardChoiceEligible" not in bypass_body,
      "bypass flow must not unlock reward choice eligibility")
check_all(
    eligible_body + available_body,
    [
        "HasClaimedTerminalRewardChoice(TerminalId)",
        "RewardChoiceEligibleTerminalIds.AddUnique(TerminalId)",
        "SolvedTerminalIds.Contains(TerminalId)",
        "RewardChoiceEligibleTerminalIds.Contains(TerminalId)",
        "!HasClaimedTerminalRewardChoice(TerminalId)",
    ],
    "eligibility helpers must require live-solve state and exclude claimed terminals",
)
check_all(
    claim_body,
    [
        "IsTerminalRewardChoiceAvailable(TerminalId)",
        "ResearchPoints += 2",
        "Player->AddAmmo(45)",
        "Player->AddMedkits(1)",
        "Player->AddArmorPlates(1)",
        "Player->GrantScrap(5)",
        "Player->AddBypassKits(1)",
        "RewardChoiceEligibleTerminalIds.Remove(TerminalId)",
        "ClaimedTerminalRewardChoiceIds.AddUnique(TerminalId)",
        "REWARD CHOICE KIOSK",
        "Start-screen Resume keeps this one-time choice",
        "SavePersistentRun()",
    ],
    "claim helper must grant one of three rewards, mark claimed, summarize, and save",
)
check_all(
    claim_widget_body + update_buttons_body + summary_body,
    [
        "GI->ClaimTerminalRewardChoice",
        "UpdateRewardChoiceButtons()",
        "GI->IsTerminalRewardChoiceAvailable",
        "RewardResearchButton->SetIsEnabled",
        "RewardFieldKitButton->SetIsEnabled",
        "RewardCraftingButton->SetIsEnabled",
        "Reward already claimed",
        "Choose one: Research Boost",
        "Practice runs and bypass-kit solves do not unlock the kiosk.",
    ],
    "widget claim and summary paths must show pending/claimed/unavailable state and disable duplicates",
)
check_all(
    reward_manifest + curriculum_manifest + selected_manifest + access_manifest,
    [
        "RewardChoiceActionRow",
        "RewardChoiceEligibility",
        "RewardChoiceClaim",
        "RewardChoiceResume",
        "RewardChoiceKiosk",
        "Reward choice kiosk",
        "TerminalRewardChoiceAccessibility",
        "verify_terminal_reward_choice_kiosk_slice_pass.py",
    ],
    "data manifests must document reward choice runtime, selected-language, accessibility, and verification coverage",
)
check_all(
    visual_targets + human_qa + onboarding,
    [
        "TerminalRewardChoiceKiosk",
        "REWARD CHOICE KIOSK",
        "Reward choice is unavailable after practice/bypass",
        "choose one reward after live solve",
        "Research Boost",
        "Field Kit",
        "Crafting Cache",
    ],
    "visual, human QA, and onboarding records must cover reward choice behavior",
)
check_all(
    creative_plan,
    [
        "terminal post-solve after-action debrief",
        "manual terminal practice-only review plus verify_terminal_reward_choice_kiosk_slice_pass.py",
        "manual reward resume review",
    ],
    "creative plan must append the reward-choice verifier to terminal post-solve work",
)
check("verify_terminal_reward_choice_kiosk_slice_pass.py" in full_qa,
      "full QA must run the reward choice verifier")
check("verify_terminal_reward_choice_kiosk_slice_pass.py" in local_ci,
      "local CI must run the reward choice verifier")
check("Terminal reward choice kiosk slice" in progress,
      "progress log must document the reward choice kiosk slice")
check_all(
    slice_doc,
    [
        "Terminal Reward Choice Kiosk Slice",
        "reward choice kiosk",
        "REWARD: RESEARCH +2 RP",
        "REWARD: FIELD KIT",
        "REWARD: CRAFTING CACHE",
        "ClaimTerminalRewardChoice",
        "Practice runs do not unlock",
        "Bypass-kit solves do not unlock",
        "Verification",
    ],
    "slice doc must explain implementation, save safety, validation, and human QA",
)
check_all(
    ledger,
    [
        "155 named verifier references",
        "110 unique verifier scripts",
        "TERMINAL_REWARD_CHOICE_KIOSK_SLICE.md",
        "verify_terminal_reward_choice_kiosk_slice_pass.py",
    ],
    "creative ledger must include the terminal reward verifier and updated counts",
)

if errors:
    for error in errors:
        print(f"[verify_terminal_reward_choice_kiosk_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_terminal_reward_choice_kiosk_slice_pass] PASS: terminal reward choice kiosk verified")
