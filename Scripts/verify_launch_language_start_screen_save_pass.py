#!/usr/bin/env python3
"""Static verifier for the launch-screen language selection and save contract."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def check(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        errors.append(f"missing function {signature}")
        return ""
    brace = source.find("{", start)
    if brace < 0:
        errors.append(f"missing function body for {signature}")
        return ""
    depth = 0
    for idx in range(brace, len(source)):
        if source[idx] == "{":
            depth += 1
        elif source[idx] == "}":
            depth -= 1
            if depth == 0:
                _cr_body = source[brace : idx + 1]  # 2026-07-04 BuildWidgetTreeNow migration
                if "::NativeConstruct" in signature and "BuildWidgetTreeNow();" in _cr_body:
                    return function_body(source, signature.replace("::NativeConstruct", "::BuildWidgetTreeNow"))
                return _cr_body
    errors.append(f"unterminated function body for {signature}")
    return ""


game_instance_h = read(SRC / "CodeRescueGameInstance.h")
game_instance_cpp = read(SRC / "CodeRescueGameInstance.cpp")
main_menu_h = read(SRC / "CodeRescueMainMenuWidget.h")
main_menu_cpp = read(SRC / "CodeRescueMainMenuWidget.cpp")
game_mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
terminal_widget_cpp = read(SRC / "CodeTerminalWidget.cpp")
language_station_cpp = read(SRC / "LanguageStationActor.cpp")
save_game_h = read(SRC / "CodeRescueSaveGame.h")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")

for token in (
    "MakeLanguageSaveSlotName",
    "DoesLanguageSaveExist",
    "GetLanguageSaveSummary",
    "StartFreshLanguageRun",
    "ResumeLanguageRun",
):
    check(token in game_instance_h and token in game_instance_cpp, f"game instance must expose {token}")

for slot_token in (
    "OperationCodeRescue_Language_Java",
    "OperationCodeRescue_Language_C",
    "OperationCodeRescue_Language_CPlus",
    "OperationCodeRescue_Language_Cpp",
    "OperationCodeRescue_Language_Python",
    "OperationCodeRescue_Language_MATLAB",
):
    check(slot_token in game_instance_cpp, f"language save slot missing {slot_token}")

fresh_body = function_body(game_instance_cpp, "bool UCodeRescueGameInstance::StartFreshLanguageRun")
resume_body = function_body(game_instance_cpp, "bool UCodeRescueGameInstance::ResumeLanguageRun")
check("SaveSlotName = MakeLanguageSaveSlotName(Language);" in fresh_body, "fresh language run must select a language-specific save slot")
check("ResetRun();" in fresh_body and "SelectedLanguage = Language;" in fresh_body, "fresh language run must reset and lock the chosen language")
check("SavePersistentRun" not in fresh_body, "fresh language run must not capture/save launch-scene world state before gameplay")
check("SaveSlotName = MakeLanguageSaveSlotName(Language);" in resume_body, "resume language run must load from the language-specific save slot")
check("LoadPersistentRun();" in resume_body and "SelectedLanguage = Language;" in resume_body, "resume language run must load progress and re-lock the chosen language")

for token in (
    "ResumeJavaBtn",
    "ResumeCBtn",
    "ResumeCPlusBtn",
    "ResumeCppBtn",
    "ResumePythonBtn",
    "ResumeMATLABBtn",
    "OnResumeJavaClicked",
    "OnResumeCClicked",
    "OnResumeCPlusClicked",
    "OnResumeCppClicked",
    "OnResumePythonClicked",
    "OnResumeMATLABClicked",
):
    check(token in main_menu_h and token in main_menu_cpp, f"main menu must wire {token}")

for token in ("CPlusLanguageBtn", "OnCPlusLanguageClicked"):
    check(token in main_menu_h and token in main_menu_cpp, f"main menu must wire {token}")

for language in ("Java", "C", "CPlus", "Cpp", "Python", "MATLAB"):
    check(f"NewLanguageRunLabel(ECodingLanguage::{language})" in main_menu_cpp, f"launch menu must offer new {language} run")
    check(f"ResumeLanguageRunLabel(GI, ECodingLanguage::{language})" in main_menu_cpp, f"launch menu must offer resume {language} save")

check("NEW %s RUN" in main_menu_cpp and "RESUME %s SAVE" in main_menu_cpp, "launch menu labels must explicitly say new and resume")
check("SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock)" in main_menu_cpp, "launch menu must keep mouse selection usable")
check("StartLanguageRun(ECodingLanguage::Java)" in main_menu_cpp and "ResumeLanguageRun(ECodingLanguage::Java)" in main_menu_cpp,
      "language buttons must start and resume through explicit launch helpers")
check("StartLanguageRun(ECodingLanguage::CPlus)" in main_menu_cpp and "ResumeLanguageRun(ECodingLanguage::CPlus)" in main_menu_cpp,
      "C+ must have explicit start and resume launch helpers")
check("GI->SaveSlotName = UCodeRescueGameInstance::MakeLanguageSaveSlotName(Language);" in main_menu_cpp,
      "non-launch language selection must point continue/new-game actions at the language profile")

launch_scene = function_body(game_mode_cpp, "void ACodeRescueGameMode::SpawnLaunchLanguageSelectionScene")
check("SetLaunchLanguageOnly(true)" in game_mode_cpp, "game mode must show the dedicated launch language widget before active play")
check("SELECT CODING LANGUAGE" in launch_scene and "NEW RUN OR RESUME SAVE" in launch_scene,
      "launch scene must include readable fallback instructions")
check("TRACK ONLY" in launch_scene, "launch scene must visibly identify language tracks")
for label in ("JAVA", "PYTHON", "C", "C+", "C++", "MATLAB"):
    check(f'TEXT("{label}")' in launch_scene, f"launch scene must show {label} track pedestal")
check("Terminal->Challenge.Language = GI->SelectedLanguage;" in game_mode_cpp,
      "spawned terminals must be locked to the selected language before UI opens")
check("GI && GI->SelectedLanguage != Language" in game_mode_cpp,
      "off-track language stations must not spawn during active play")
check("GI->SelectedLanguage == Language" in language_station_cpp,
      "language stations must persist only the active selected track")
check("TerminalActor->Challenge.Language = GI->SelectedLanguage;" in terminal_widget_cpp,
      "terminal UI must refresh challenges to the selected language")
check("LastSelectedLanguage" in save_game_h, "save game must persist the selected language")

check("verify_launch_language_start_screen_save_pass.py" in full_qa,
      "full QA must run the launch language save verifier")
check("verify_launch_language_start_screen_save_pass.py" in local_ci,
      "local CI readiness must run the launch language save verifier")

if errors:
    for error in errors:
        print(f"[verify_launch_language_start_screen_save_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_launch_language_start_screen_save_pass] PASS: launch language selection and save contract verified")
