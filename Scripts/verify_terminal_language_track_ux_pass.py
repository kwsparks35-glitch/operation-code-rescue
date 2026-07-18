#!/usr/bin/env python3
"""Static verifier for language-locked terminal UX and themed terminal states."""

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
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")

native_construct = function_body(terminal_cpp, "void UCodeTerminalWidget::NativeConstruct")
refresh_text = function_body(terminal_cpp, "void UCodeTerminalWidget::RefreshText")
validate_clicked = function_body(terminal_cpp, "void UCodeTerminalWidget::RunValidation")

check('"CodeRescueUITheme.h"' in terminal_cpp, "terminal must include the shared UI theme")
check("UTextBlock* LanguageLockText" in terminal_h, "terminal must own a language-lock text block")
check("LanguageLockText = WidgetTree->ConstructWidget" in native_construct, "terminal must construct the language-lock cue")
check("LOCKED TRACK:" in refresh_text, "terminal must visibly identify the selected language track")
check("MakeLanguageSaveSlotName" in refresh_text, "terminal must show the language-specific save profile")
check("Progress, hints, attempts, and solves remain in this language" in refresh_text,
      "terminal must explain that progress remains in the chosen language")
check("Resume it from the start screen next launch" in refresh_text,
      "terminal must connect language saves back to the start-screen resume flow")
check("TerminalActor->Challenge.Language = GI->SelectedLanguage;" in refresh_text,
      "terminal must continue forcing challenges to the selected language")
check("MirrorTerminalThemeFromSettings" in terminal_cpp and "StyleTerminalCodeEditor" in terminal_cpp,
      "terminal must mirror accessibility theme settings and style the code editor")
check("FCoreStyle::GetDefaultFontStyle" in terminal_cpp and "FName(TEXT(\"Mono\"))" in terminal_cpp,
      "terminal code editor must use a monospace font")
check("StyleTerminalButton" in native_construct and "CodeRescueUI::StylePrimaryButton" in terminal_cpp,
      "terminal buttons must use shared themed button styling")
check("REVEAL HINT [Ctrl+H]" in native_construct,
      "hint button label must match the implemented Ctrl/Command+H shortcut")
check("SetTerminalOutput(OutputText, Output, TerminalOutputColor(Result.bSuccess));" in validate_clicked,
      "validation output must color-code success vs repair-needed states")
check("CodeRescueUI::Color::DangerBright()" in terminal_cpp and "CodeRescueUI::Color::TerminalGreenBright()" in terminal_cpp,
      "terminal must use semantic success/error colors")
check("verify_terminal_language_track_ux_pass.py" in full_qa,
      "full QA must run the terminal language UX verifier")
check("verify_terminal_language_track_ux_pass.py" in local_ci,
      "local CI readiness must run the terminal language UX verifier")
check("Terminal language-track UX" in progress,
      "progress log must document the terminal language-track UX pass")

if errors:
    for error in errors:
        print(f"[verify_terminal_language_track_ux_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_terminal_language_track_ux_pass] PASS: terminal language-track UX and themed states verified")
