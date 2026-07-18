#!/usr/bin/env python3
"""Static verifier for the terminal diegetic restyle slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
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
curriculum_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/curriculum_feedback_manifest.tsv")
access_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
visual_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "TERMINAL_DIEGETIC_RESTYLE_SLICE.md")

construct_body = function_body(terminal_cpp, "void UCodeTerminalWidget::NativeConstruct")
refresh_body = function_body(terminal_cpp, "void UCodeTerminalWidget::RefreshText")
diagnostics_body = function_body(terminal_cpp, "void UCodeTerminalWidget::SetDiagnosticsState")
reset_body = function_body(terminal_cpp, "void UCodeTerminalWidget::ResetToStarterCode")
hint_body = function_body(terminal_cpp, "void UCodeTerminalWidget::OnHintClicked")
validate_body = function_body(terminal_cpp, "void UCodeTerminalWidget::RunValidation")
matlab_body = function_body(terminal_cpp, "void UCodeTerminalWidget::OnMATLABClicked")

check_all(
    terminal_h,
    [
        "class UBorder",
        "UTextBlock* TerminalStatusText",
        "UBorder* PanelFrame",
        "UBorder* CodeEditorFrame",
        "UBorder* OutputFrame",
        "UTextBlock* DiagnosticsHeaderText",
        "SetDiagnosticsState(const FString& Label, const FLinearColor& Color)",
    ],
    "terminal header must persist restyle chrome widgets",
)
check_all(
    terminal_cpp,
    [
        "#include \"Components/HorizontalBox.h\"",
        "#include \"Components/HorizontalBoxSlot.h\"",
        "TerminalPanelFill",
        "TerminalEditorFill",
        "TerminalToolchainStateLine",
        "CodeRescueUI::Theme().bHighContrast",
        "safe in-engine fallback",
        "external compiler detected",
    ],
    "terminal implementation must define high-contrast fills and toolchain status",
)
check_all(
    construct_body,
    [
        "PanelFrame = WidgetTree->ConstructWidget<UBorder>",
        "TerminalStatusText = WidgetTree->ConstructWidget<UTextBlock>",
        "CodeEditorFrame = WidgetTree->ConstructWidget<UBorder>",
        "CodeEditorFrame->SetContent(CodeBox)",
        "OutputFrame = WidgetTree->ConstructWidget<UBorder>",
        "DiagnosticsHeaderText = WidgetTree->ConstructWidget<UTextBlock>",
        "TerminalPrimaryActionRow",
        "TerminalUtilityActionRow",
        "AddChildToHorizontalBox",
        "SetSize(FSlateChildSize(ESlateSizeRule::Fill))",
    ],
    "terminal construct path must create framed editor/diagnostics panes and grouped action rows",
)
check_all(
    refresh_body,
    [
        "CodeRescueUI::StylePanel(",
        "TerminalPanelFill()",
        "TerminalEditorFill()",
        "TerminalToolchainStateLine(GI->SelectedLanguage)",
        "SAFEHOUSE TERMINAL LINK",
        "combat paused for coding",
        "survivor route unlocked",
        "SetDiagnosticsState(",
        "READY | awaiting validation",
        "SOLVED | survivor intel uploaded",
    ],
    "terminal refresh must update language, toolchain, frame styling, and diagnostics state",
)
check_all(
    diagnostics_body,
    [
        "DIAGNOSTICS | %s",
        "CodeRescueUI::StyleText(DiagnosticsHeaderText",
        "CodeRescueUI::StylePanel(OutputFrame",
    ],
    "diagnostics helper must style and relabel the diagnostics pane",
)
check_all(
    reset_body + hint_body + validate_body + matlab_body,
    [
        "READY | starter restored",
        "HINTS | no ResearchPoints remaining",
        "HINTS | support uploaded",
        "READY | no code detected",
        "LIMIT | code too long",
        "PASS | survivor intel uploaded",
        "REPAIR | failed checks detected",
        "MATLAB | launch requested",
        "MATLAB | launch unavailable",
    ],
    "terminal actions must drive explicit diagnostics states",
)
check_all(
    curriculum_manifest,
    [
        "TerminalDiegeticRestyle",
        "TerminalStatusText + CodeEditorFrame + DiagnosticsHeaderText",
        "code editor, diagnostics, and action grouping",
    ],
    "curriculum feedback manifest must document terminal restyle coverage",
)
check_all(
    access_manifest,
    [
        "TerminalDiegeticAccessibility",
        "high-contrast panel/editor fills",
        "explicit diagnostics state text",
    ],
    "accessibility manifest must document terminal restyle coverage",
)
check_all(
    visual_manifest,
    [
        "CodingTerminalSurface",
        "framed code editor",
        "diagnostics header",
    ],
    "visual regression targets must include the terminal surface",
)
check("verify_terminal_diegetic_restyle_slice_pass.py" in full_qa,
      "full QA must run the terminal diegetic restyle verifier")
check("verify_terminal_diegetic_restyle_slice_pass.py" in local_ci,
      "local CI must run the terminal diegetic restyle verifier")
check("Terminal diegetic restyle slice" in progress,
      "progress log must document the terminal diegetic restyle slice")
check_all(
    slice_doc,
    [
        "Terminal Diegetic Restyle Slice",
        "TerminalStatusText",
        "CodeEditorFrame",
        "DiagnosticsHeaderText",
        "High Contrast HUD",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, player impact, verification, and human QA",
)

if errors:
    for error in errors:
        print(f"[verify_terminal_diegetic_restyle_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_terminal_diegetic_restyle_slice_pass] PASS: terminal diegetic restyle verified")
