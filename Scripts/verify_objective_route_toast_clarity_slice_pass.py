#!/usr/bin/env python3
"""Static verifier for the objective route toast clarity slice."""

from pathlib import Path
import sys

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"

errors: list[str] = []


def read(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except FileNotFoundError:
        errors.append(f"missing file: {path.relative_to(PROJECT_ROOT)}")
        return ""


def check(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def check_all(haystack: str, needles: list[str], message: str) -> None:
    missing = [needle for needle in needles if needle not in haystack]
    if missing:
        errors.append(f"{message}; missing {missing}")


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        errors.append(f"missing function: {signature}")
        return ""
    brace = source.find("{", start)
    if brace < 0:
        errors.append(f"missing function body: {signature}")
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
    errors.append(f"unterminated function: {signature}")
    return ""


hud_h = read(SRC / "CodeRescueHUDWidget.h")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
manifest = read(DATA / "objective_route_toast_clarity_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
curriculum = read(DATA / "curriculum_feedback_manifest.tsv")
accessibility = read(DATA / "accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC / "OBJECTIVE_ROUTE_TOAST_CLARITY_SLICE.md")
progress = read(PROJECT_ROOT / "progress.md")

construct_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::NativeConstruct")
trigger_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::TriggerObjectiveRouteToast")
refresh_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshObjectiveRouteToast")
hud_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")

check_all(
    hud_h,
    [
        "class UWorld",
        "ObjectiveToastText",
        "bObjectiveToastStateSeeded",
        "LastObservedSolvedTerminalCount",
        "LastObservedRescuedSurvivorCount",
        "LastObservedCodingScore",
        "LastObservedSaveWallSeconds",
        "ObjectiveToastMessage",
        "ObjectiveToastColor",
        "ObjectiveToastStartSeconds",
        "ObjectiveToastDurationSeconds",
        "TriggerObjectiveRouteToast",
        "RefreshObjectiveRouteToast",
    ],
    "HUD header must declare objective route toast widget, cached state, and helpers",
)

check_all(
    construct_body,
    [
        "ObjectiveRouteToastText",
        "ObjectiveToastText->SetAutoWrapText(true)",
        "ObjectiveToastText->SetJustification(ETextJustify::Center)",
        "CodeRescueUI::StyleText(ObjectiveToastText, CodeRescueUI::EType::Subheading",
        "ToastSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f))",
        "ToastSlot->SetSize(FVector2D(780.0f, 54.0f))",
    ],
    "HUD construction must add a centered, styled objective route toast text block",
)

check_all(
    trigger_body,
    [
        "Message.IsEmpty()",
        "ObjectiveToastMessage = Message",
        "ObjectiveToastColor = Color",
        "ObjectiveToastStartSeconds = World->TimeSeconds",
        "ObjectiveToastDurationSeconds = FMath::Max(0.5f, DurationSeconds)",
    ],
    "TriggerObjectiveRouteToast must store text, color, start time, and duration safely",
)

check_all(
    refresh_body,
    [
        "GI->SolvedTerminalIds.Num()",
        "GI->RescuedSurvivorNames.Num()",
        "GI->CodingScore",
        "GI->LastSaveWallSeconds",
        "bObjectiveToastStateSeeded = true",
        "LastObservedSolvedTerminalCount = SolvedCount",
        "LastObservedRescuedSurvivorCount = RescuedCount",
        "LastObservedCodingScore = CurrentScore",
        "LastObservedSaveWallSeconds = SaveStamp",
        "SolvedCount > LastObservedSolvedTerminalCount",
        "OBJECTIVE UPDATED | Terminal solved | Survivor route open",
        "ScoreDelta",
        "RescuedCount > LastObservedRescuedSurvivorCount",
        "OBJECTIVE UPDATED | Survivor rescued | Extraction ready | Language save refreshed",
        "CHECKPOINT SAVED | %s run can resume from the start screen",
        "GI->GetLanguageName()",
        "GI->bHighContrastHUD",
        "GI->bReducedMotion",
        "ObjectiveToastText->SetText(FText::FromString(ObjectiveToastMessage))",
        "ObjectiveToastDurationSeconds - ToastElapsed",
        "GI->bVisualizeSoundCues",
        "SoundCueText->SetColorAndOpacity",
    ],
    "RefreshObjectiveRouteToast must observe progress, emit route/save messages, and honor accessibility state",
)

check(
    "RefreshObjectiveRouteToast(GI, World)" in hud_body,
    "RefreshHUD must call RefreshObjectiveRouteToast during the normal HUD refresh",
)

check_all(
    manifest,
    [
        "HUD objective route toast",
        "Terminal solve acknowledgement",
        "Survivor rescue acknowledgement",
        "Checkpoint resume acknowledgement",
        "Accessibility treatment",
        "verify_objective_route_toast_clarity_slice_pass.py",
    ],
    "objective route toast manifest must document runtime hooks, player effects, and validation",
)

check_all(
    creative_plan + visual_targets + human_qa,
    [
        "objective route toast clarity",
        "ObjectiveRouteToasts",
        "verify_objective_route_toast_clarity_slice_pass.py",
        "Terminal solved",
        "Extraction ready",
        "start-screen",
    ],
    "creative, visual, and human QA manifests must include objective route toast coverage",
)

check_all(
    onboarding + curriculum + accessibility,
    [
        "ObjectiveRouteToast",
        "route toast",
        "CHECKPOINT SAVED",
        "UI text",
        "Reduced Motion",
    ],
    "onboarding, curriculum, and accessibility manifests must describe route-toast player and accessibility coverage",
)

check(
    "verify_objective_route_toast_clarity_slice_pass.py" in full_qa,
    "full QA must run the objective route toast verifier",
)
check(
    "verify_objective_route_toast_clarity_slice_pass.py" in local_ci,
    "local CI must run the objective route toast verifier",
)

check_all(
    slice_doc,
    [
        "Objective Route Toast Clarity Slice",
        "ObjectiveRouteToastText",
        "TriggerObjectiveRouteToast",
        "RefreshObjectiveRouteToast",
        "Terminal solved",
        "Survivor rescued",
        "CHECKPOINT SAVED",
        "verify_objective_route_toast_clarity_slice_pass.py",
    ],
    "slice documentation must describe implementation, player impact, and validation",
)

check_all(
    progress,
    [
        "Objective route toast clarity slice",
        "ObjectiveRouteToastText",
        "verify_objective_route_toast_clarity_slice_pass.py",
    ],
    "progress log must document the objective route toast clarity slice",
)

if errors:
    print("[verify_objective_route_toast_clarity_slice_pass] FAIL")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("[verify_objective_route_toast_clarity_slice_pass] PASS: objective route toast clarity verified")
