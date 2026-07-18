#!/usr/bin/env python3
"""Static verifier for the terminal post-solve debrief slice."""

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


terminal_cpp = read(SRC / "CodeTerminalWidget.cpp")
curriculum_manifest = read(DATA / "curriculum_feedback_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "TERMINAL_POST_SOLVE_DEBRIEF_SLICE.md")

validate_body = function_body(terminal_cpp, "void UCodeTerminalWidget::RunValidation")

check_all(
    terminal_cpp,
    [
        "GetConceptProofForChallenge",
        "GetLanguageTransferForChallenge",
        "GetNextPracticeRepForChallenge",
        "BuildPostSolveAfterActionDebrief",
        "BuildRepairDebrief",
        "POST-SOLVE DEBRIEF",
        "Concept proof:",
        "Language transfer (%s):",
        "World follow-up:",
        "Save continuity:",
        "Next practice:",
        "REPAIR DEBRIEF",
        "Active target:",
        "World state: combat remains paused in the safehouse until the survivor route unlocks.",
        "UCodeRescueGameInstance::MakeLanguageSaveSlotName(Language)",
        "GI->GetLanguageProgressSummary()",
        "FMath::Max(1, CityIndex + 1)",
    ],
    "terminal implementation must define post-solve and repair debrief helpers",
)
check_all(
    validate_body,
    [
        "BuildPostSolveAfterActionDebrief(",
        "BuildRepairDebrief(",
        "TerminalActor->CityIndex",
        "bUsedHintThisAttempt",
        "ResearchReward",
        "Intel Reward: survivor whereabouts uploaded",
        "Learning Profile:",
        "Language Practice:",
    ],
    "validation flow must append debriefs while preserving existing reward/profile output",
)
check_all(
    curriculum_manifest,
    [
        "PostSolveExplanation",
        "BuildPostSolveAfterActionDebrief + Terminal success summary",
        "concept proof, language transfer, survivor intel, save continuity, and next practice",
        "RepairDebrief",
        "BuildRepairDebrief + failed checks",
    ],
    "curriculum feedback manifest must document success and repair debrief coverage",
)
check_all(
    onboarding,
    [
        "POST-SOLVE DEBRIEF or REPAIR DEBRIEF",
        "next validation move",
    ],
    "first-ten-minutes onboarding must include debrief expectations",
)
check_all(
    visual_manifest,
    [
        "CodingTerminalSurface",
        "post-solve/repair debrief output",
    ],
    "visual regression targets must include debrief output",
)
check_all(
    human_qa,
    [
        "concept proof, language transfer, save continuity, and survivor-route follow-up",
    ],
    "human QA checklist must ask reviewers to inspect the debrief copy",
)
check_all(
    creative_plan,
    [
        "terminal post-solve after-action debrief",
        "verify_terminal_post_solve_debrief_slice_pass.py",
    ],
    "creative development inclusion plan must track the slice",
)
check("verify_terminal_post_solve_debrief_slice_pass.py" in full_qa,
      "full QA must run the terminal post-solve debrief verifier")
check("verify_terminal_post_solve_debrief_slice_pass.py" in local_ci,
      "local CI must run the terminal post-solve debrief verifier")
check("Terminal post-solve debrief slice" in progress,
      "progress log must document the terminal post-solve debrief slice")
check_all(
    slice_doc,
    [
        "Terminal Post-Solve Debrief Slice",
        "POST-SOLVE DEBRIEF",
        "REPAIR DEBRIEF",
        "Human QA Notes",
        "Validation",
    ],
    "slice doc must explain implementation, verification, and human QA",
)

if errors:
    for error in errors:
        print(f"[verify_terminal_post_solve_debrief_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_terminal_post_solve_debrief_slice_pass] PASS: terminal post-solve debrief slice verified")
