#!/usr/bin/env python3
"""Static verifier for the HUD first-ten-minutes field checklist slice."""

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


hud_h = read(SRC / "CodeRescueHUDWidget.h")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
field_manifest = read(DATA / "field_checklist_hud_manifest.tsv")
objective_manifest = read(DATA / "objective_route_toast_clarity_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
access_manifest = read(DATA / "accessibility_settings_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC / "FIELD_CHECKLIST_HUD_SLICE.md")
ledger = read(DOC / "CREATIVE_DEVELOPMENT_IMPLEMENTATION_LEDGER.md")
progress = read(PROJECT_ROOT / "progress.md")

construct_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::NativeConstruct")
refresh_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")

check_all(
    hud_h,
    [
        "FieldChecklistText",
        "NavigationStripText",
    ],
    "HUD header must declare the field checklist text beside navigation",
)
check_all(
    construct_body,
    [
        "BespokeFieldChecklistPanel",
        "FirstTenMinutesFieldChecklistText",
        "FIRST TEN MINUTES FIELD CHECKLIST",
        "FieldChecklistText->SetAutoWrapText(true)",
        "ChecklistPanelSlot->SetPosition(FVector2D(-12.0f, 342.0f))",
        "ChecklistSlot->SetSize(FVector2D(310.0f, 116.0f))",
        "ChecklistFont.Size = 14",
    ],
    "HUD construction must add a compact wrapped checklist panel under navigation",
)
check_all(
    refresh_body,
    [
        "if (FieldChecklistText)",
        "FIRST TEN MINUTES FIELD CHECKLIST",
        "Track: %s only",
        "UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage)",
        "GI->DoesLanguageSaveExist(GI->SelectedLanguage)",
        "start-screen Resume ready",
        "autosaves after progress",
        "protected terminal -> survivor marker -> extraction",
        "GI->SolvedTerminalIds.Contains(Mission->TerminalId)",
        "GI->RescuedSurvivorNames.Contains(Mission->SurvivorName)",
        "E open terminal | Ctrl+P practice | VALIDATE CODE",
        "Follow survivor marker to %s",
        "Return to helipad extraction/debrief",
        "T route | Backspace/F8 recover | J journal",
        "T route | J survivor intel | P/Esc save",
        "T route | J recap | P/Esc save",
        "GI->bHighContrastHUD",
    ],
    "HUD refresh must derive language-save, route phase, controls, and accessibility state",
)
check_all(
    field_manifest + objective_manifest,
    [
        "HUD field checklist",
        "FieldChecklistText",
        "FIRST TEN MINUTES FIELD CHECKLIST",
        "Language save continuity",
        "Route phase clarity",
        "Control recovery",
        "protected terminal -> survivor marker -> extraction",
        "verify_field_checklist_hud_slice_pass.py",
    ],
    "field checklist and objective manifests must describe runtime behavior and verification",
)
check_all(
    creative_plan + full_qa + local_ci,
    [
        "objective route toast clarity",
        "verify_field_checklist_hud_slice_pass.py",
    ],
    "creative plan and QA entry points must run the field checklist verifier",
)
check_all(
    access_manifest + onboarding + visual_targets + human_qa,
    [
        "FieldChecklistHUDAccessibility",
        "FieldChecklistHUD",
        "HUD FIRST TEN MINUTES FIELD CHECKLIST",
        "FIRST TEN MINUTES FIELD CHECKLIST",
        "protected terminal -> survivor marker -> extraction",
        "start-screen Resume",
        "E/Ctrl+P",
        "Backspace/F8",
        "P/Esc",
    ],
    "accessibility, onboarding, visual, and human QA records must cover the field checklist",
)
check_all(
    slice_doc + ledger + progress,
    [
        "FIELD_CHECKLIST_HUD_SLICE.md",
        "Field Checklist HUD Slice",
        "FIRST TEN MINUTES FIELD CHECKLIST",
        "verify_field_checklist_hud_slice_pass.py",
        "155 named verifier references",
        "110 unique verifier scripts",
    ],
    "slice doc, ledger, and progress must record field checklist work and updated counts",
)

if errors:
    for error in errors:
        print(f"[verify_field_checklist_hud_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_field_checklist_hud_slice_pass] PASS: field checklist HUD verified")
