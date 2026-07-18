#!/usr/bin/env python3
"""Static verifier for the terminal practice-only run slice."""

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
practice_manifest = read(DATA / "terminal_practice_run_manifest.tsv")
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
slice_doc = read(DOC_DIR / "TERMINAL_PRACTICE_RUN_SLICE.md")
ledger = read(DOC_DIR / "CREATIVE_DEVELOPMENT_IMPLEMENTATION_LEDGER.md")

construct_body = function_body(terminal_cpp, "void UCodeTerminalWidget::NativeConstruct")
validate_click_body = function_body(terminal_cpp, "void UCodeTerminalWidget::OnValidateClicked")
practice_click_body = function_body(terminal_cpp, "void UCodeTerminalWidget::OnPracticeClicked")
run_body = function_body(terminal_cpp, "void UCodeTerminalWidget::RunValidation")
refresh_body = function_body(terminal_cpp, "void UCodeTerminalWidget::RefreshText")
key_body = function_body(terminal_cpp, "FReply UCodeTerminalWidget::NativeOnKeyDown")
init_body = function_body(terminal_cpp, "void UCodeTerminalWidget::InitializeTerminal")

check_all(
    terminal_h,
    [
        "UButton* PracticeButton",
        "void OnPracticeClicked()",
        "int32 PracticeRunCount = 0",
        "void RunValidation(bool bPracticeOnly)",
    ],
    "terminal header must expose the practice button, click handler, counter, and shared validation path",
)
check_all(
    construct_body,
    [
        "PracticeButton = WidgetTree->ConstructWidget<UButton>",
        "TEXT(\"PracticeRunButton\")",
        "TEXT(\"PracticeRunLabel\")",
        "PRACTICE RUN [Ctrl+P]",
        "PracticeButton->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnPracticeClicked)",
        "Practice run checks code without saving or opening the route",
    ],
    "terminal construction must create a visible practice-only action",
)
check_all(
    validate_click_body,
    ["RunValidation(false)"],
    "live validate click must call the non-practice validation path",
)
check_all(
    practice_click_body,
    ["RunValidation(true)"],
    "practice click must call the practice validation path",
)
check_all(
    run_body,
    [
        "if (bPracticeOnly)",
        "++PracticeRunCount",
        "++SessionAttemptCount",
        "const bool bFirstTry = !bPracticeOnly",
        "GI && !bPracticeOnly",
        "Result.bSuccess && !bPracticeOnly",
        "PRACTICE PASS | no route advanced",
        "PRACTICE REPAIR | no save penalty",
        "PRACTICE RUN - NO SAVE ADVANCE",
        "PRACTICE-ONLY DEBRIEF",
        "No terminal solve, survivor route, ResearchPoints, coding score, attempts, streaks, survivor intel archive, or save profile were advanced.",
        "Use VALIDATE CODE when ready to commit",
        "Practice Run Lock:",
        "Save profile unchanged",
        "practice-only terminal option does not advance campaign progress",
        "Practice safety: no route opened, no save write, no ResearchPoints awarded, and live validation remains available.",
    ],
    "practice validation path must run the validator without persistent progression, rewards, or route reveal",
)
check_all(
    refresh_body + init_body + key_body,
    [
        "PracticeRunCount = 0",
        "PracticeButton->SetIsEnabled(!bAlreadySolved)",
        "EKeys::P && bCommandModifier",
        "OnPracticeClicked()",
    ],
    "practice state must reset per terminal, disable after solve, and support Ctrl+P",
)
check_all(
    practice_manifest,
    [
        "PracticeRunButton",
        "PracticeOnlyDebrief",
        "PracticeRunLock",
        "PracticeShortcut",
        "PRACTICE RUN [Ctrl+P]",
        "No persistent counters changed",
        "verify_terminal_practice_run_slice_pass.py",
    ],
    "practice manifest must document surfaces, save safety, and verification",
)
check_all(
    curriculum_manifest + selected_manifest + access_manifest,
    [
        "PracticeOnlyTerminal",
        "Practice-only terminal option",
        "TerminalPracticeRunAccessibility",
        "without save advance, route unlock, rewards, attempts, streaks, or survivor intel upload",
        "only VALIDATE CODE updates the save and survivor route",
        "PRACTICE-ONLY DEBRIEF",
    ],
    "data manifests must document practice-only curriculum, selected-language flow, and accessibility",
)
check_all(
    visual_targets + human_qa + onboarding,
    [
        "TerminalPracticeRun",
        "PRACTICE RUN - NO SAVE ADVANCE",
        "Practice Run Lock",
        "live validation remains available",
        "use PRACTICE RUN [Ctrl+P]",
        "Use practice run before committing",
    ],
    "visual, human QA, and onboarding records must cover the practice-only flow",
)
check_all(
    creative_plan,
    [
        "terminal post-solve after-action debrief",
        "manual persistent learning debrief resume review plus manual challenge replay resume review",
        "manual language profile recap resume review plus verify_terminal_practice_run_slice_pass.py",
        "manual terminal practice-only review",
    ],
    "creative plan must append the practice verifier while preserving older manual resume review wording",
)
check("verify_terminal_practice_run_slice_pass.py" in full_qa,
      "full QA must run the terminal practice verifier")
check("verify_terminal_practice_run_slice_pass.py" in local_ci,
      "local CI must run the terminal practice verifier")
check("Terminal practice run slice" in progress,
      "progress log must document the terminal practice run slice")
check_all(
    slice_doc,
    [
        "Terminal Practice Run Slice",
        "practice-only terminal option",
        "RunValidation(bool bPracticeOnly)",
        "PRACTICE RUN [Ctrl+P]",
        "PRACTICE RUN - NO SAVE ADVANCE",
        "PRACTICE-ONLY DEBRIEF",
        "Only `VALIDATE CODE` commits those outcomes.",
        "Verification",
    ],
    "slice doc must explain implementation, save safety, validation, and human QA",
)
check_all(
    ledger,
    [
        "155 named verifier references",
        "110 unique verifier scripts",
        "TERMINAL_PRACTICE_RUN_SLICE.md",
        "verify_terminal_practice_run_slice_pass.py",
    ],
    "creative ledger must include the terminal practice verifier and updated counts",
)

if errors:
    for error in errors:
        print(f"[verify_terminal_practice_run_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_terminal_practice_run_slice_pass] PASS: terminal practice run verified")
