#!/usr/bin/env python3
"""Static verifier for selected-language terminal flow and save continuity."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"

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


game_instance_h = read(SRC / "CodeRescueGameInstance.h")
game_instance_cpp = read(SRC / "CodeRescueGameInstance.cpp")
main_menu_cpp = read(SRC / "CodeRescueMainMenuWidget.cpp")
game_mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
language_station_cpp = read(SRC / "LanguageStationActor.cpp")
terminal_cpp = read(SRC / "CodeTerminalWidget.cpp")
code_runner_cpp = read(SRC / "CodeRunnerLibrary.cpp")
manifest = read(DATA / "selected_language_terminal_flow_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC / "SELECTED_LANGUAGE_TERMINAL_FLOW_SLICE.md")
progress = read(PROJECT_ROOT / "progress.md")

fresh_body = function_body(game_instance_cpp, "bool UCodeRescueGameInstance::StartFreshLanguageRun")
resume_body = function_body(game_instance_cpp, "bool UCodeRescueGameInstance::ResumeLanguageRun")
save_body = function_body(game_instance_cpp, "bool UCodeRescueGameInstance::SavePersistentRun")
save_internal_body = function_body(game_instance_cpp, "bool UCodeRescueGameInstance::SavePersistentRunInternal")
attempt_body = function_body(game_instance_cpp, "void UCodeRescueGameInstance::RecordTerminalAttempt")
solved_body = function_body(game_instance_cpp, "void UCodeRescueGameInstance::RecordTerminalSolved")
refresh_body = function_body(terminal_cpp, "void UCodeTerminalWidget::RefreshText")
validate_body = function_body(terminal_cpp, "void UCodeTerminalWidget::RunValidation")

check_all(
    main_menu_cpp,
    [
        "CHOOSE CODING LANGUAGE",
        "This screen appears every time the game starts.",
        "NewLanguageRunLabel(ECodingLanguage::Java)",
        "NewLanguageRunLabel(ECodingLanguage::C)",
        "NewLanguageRunLabel(ECodingLanguage::CPlus)",
        "NewLanguageRunLabel(ECodingLanguage::Cpp)",
        "NewLanguageRunLabel(ECodingLanguage::Python)",
        "NewLanguageRunLabel(ECodingLanguage::MATLAB)",
        "ResumeLanguageRunLabel(GI, ECodingLanguage::Java)",
        "ResumeLanguageRunLabel(GI, ECodingLanguage::C)",
        "ResumeLanguageRunLabel(GI, ECodingLanguage::CPlus)",
        "ResumeLanguageRunLabel(GI, ECodingLanguage::Cpp)",
        "ResumeLanguageRunLabel(GI, ECodingLanguage::Python)",
        "ResumeLanguageRunLabel(GI, ECodingLanguage::MATLAB)",
        "StartLanguageRun(ECodingLanguage::CPlus)",
        "ResumeLanguageRun(ECodingLanguage::CPlus)",
    ],
    "launch menu must offer new/resume actions for every supported language",
)

check_all(
    game_instance_h + game_instance_cpp,
    [
        "MakeLanguageSaveSlotName",
        "DoesLanguageSaveExist",
        "GetLanguageSaveSummary",
        "StartFreshLanguageRun",
        "ResumeLanguageRun",
        "OperationCodeRescue_Language_Java",
        "OperationCodeRescue_Language_C",
        "OperationCodeRescue_Language_CPlus",
        "OperationCodeRescue_Language_Cpp",
        "OperationCodeRescue_Language_Python",
        "OperationCodeRescue_Language_MATLAB",
    ],
    "game instance must keep per-language save helpers and slots",
)
check("SaveSlotName = MakeLanguageSaveSlotName(Language);" in fresh_body,
      "fresh language run must target the chosen language save slot")
check("ResetRun();" in fresh_body and "SelectedLanguage = Language;" in fresh_body,
      "fresh language run must reset and lock the chosen language")
check("SaveSlotName = MakeLanguageSaveSlotName(Language);" in resume_body,
      "resume language run must target the chosen language save slot")
check("LoadPersistentRun();" in resume_body and "SelectedLanguage = Language;" in resume_body,
      "resume language run must load then re-lock the chosen language")
check("SavePersistentRunInternal(true)" in save_body and "Save->LastSelectedLanguage = SelectedLanguage;" in save_internal_body,
      "save game must persist the selected language")
check("SavePersistentRun();" in attempt_body,
      "failed or partial terminal attempts must save language progress")
check("SavePersistentRun();" in solved_body and "MarkTerminalSolved(MissionId)" in solved_body,
      "terminal solves must persist selected-language solve state")

check_all(
    game_mode_cpp + language_station_cpp + refresh_body,
    [
        "Terminal->Challenge.Language = GI->SelectedLanguage;",
        "GI && GI->SelectedLanguage != Language",
        "GI->SelectedLanguage == Language",
        "TerminalActor->Challenge.Language = GI->SelectedLanguage;",
        "TerminalActor->Challenge.StarterCode = MakeStarterForLanguage",
        "LOCKED TRACK:",
        "MakeLanguageSaveSlotName",
        "Resume it from the start screen next launch",
    ],
    "active play must keep terminals and stations locked to the selected language",
)

check_all(
    validate_body,
    [
        "Language Run Lock:",
        "Track: %s only",
        "Save profile updated: %s",
        "Start screen resume uses this same %s profile",
        "GetTerminalLanguageLabel(TerminalActor->Challenge.Language)",
        "UCodeRescueGameInstance::MakeLanguageSaveSlotName(TerminalActor->Challenge.Language)",
        "GI->RecordTerminalAttempt",
        "GI->RecordTerminalSolved",
        "BuildPostSolveAfterActionDebrief",
        "BuildRepairDebrief",
    ],
    "terminal validation must name the locked language save profile and preserve debriefs",
)

check_all(
    code_runner_cpp,
    [
        "bUsesOutputBufferMirror",
        "input[",
        "- 1 -",
        "output[",
        "TEXT(\"'\\\\0'\")",
        "Reverses via a built-in, mirrored output buffer, or backward-iterating loop",
    ],
    "in-engine validator must accept valid C reverse output-buffer solutions",
)

check_all(
    manifest,
    [
        "Start screen language choice",
        "Language-specific save slots",
        "Active play language lock",
        "Terminal locked-track cue",
        "Validation save confirmation",
        "Curriculum shape fallback",
        "C reverse output-buffer solutions",
        "Language Run Lock",
        "verify_selected_language_terminal_flow_slice_pass.py",
    ],
    "selected-language terminal manifest must describe runtime contracts and validation",
)

check_all(
    creative_plan,
    [
        "selected language terminal flow",
        "verify_selected_language_terminal_flow_slice_pass.py",
        "verify_launch_language_start_screen_save_pass.py",
        "verify_terminal_language_track_ux_pass.py",
        "manual language save/resume review",
    ],
    "creative plan must route selected-language flow through focused validation",
)

check_all(
    human_qa,
    [
        "SelectedLanguageTerminalFlow",
        "Java, C, C+, C++, Python, and MATLAB",
        "Language Run Lock",
        "attempts/solves update only that language save profile",
    ],
    "human QA checklist must include selected-language flow coverage",
)

check_all(
    visual_targets,
    [
        "SelectedLanguageTerminalFlow",
        "Start screen language rows",
        "terminal LOCKED TRACK banner",
        "Language Run Lock validation output",
        "language save summary",
    ],
    "visual regression targets must include selected-language terminal flow capture",
)

check_all(
    slice_doc,
    [
        "Selected Language Terminal Flow Slice",
        "Language Run Lock",
        "Java, C, C+, C++, Python, and MATLAB",
        "per-language save isolation",
        "C reverse output-buffer fallback support",
        "future start-screen resume",
    ],
    "slice documentation must explain selected-language terminal behavior and validation",
)

check("verify_selected_language_terminal_flow_slice_pass.py" in full_qa,
      "full QA must run selected-language terminal verifier")
check("verify_selected_language_terminal_flow_slice_pass.py" in local_ci,
      "local CI must run selected-language terminal verifier")
check("Selected language terminal flow slice" in progress,
      "progress log must document this slice")

if errors:
    for error in errors:
        print(f"[verify_selected_language_terminal_flow_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_selected_language_terminal_flow_slice_pass] PASS: selected-language terminal flow verified")
