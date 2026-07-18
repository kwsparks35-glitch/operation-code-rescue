#!/usr/bin/env python3
"""Static verifier for the first-session route preview start-screen slice."""

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
main_menu_h = read(SRC / "CodeRescueMainMenuWidget.h")
main_menu_cpp = read(SRC / "CodeRescueMainMenuWidget.cpp")
game_mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
preview_manifest = read(DATA / "first_session_route_preview_manifest.tsv")
selected_manifest = read(DATA / "selected_language_terminal_flow_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
access_manifest = read(DATA / "accessibility_settings_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC_DIR / "FIRST_SESSION_ROUTE_PREVIEW_SLICE.md")
ledger = read(DOC_DIR / "CREATIVE_DEVELOPMENT_IMPLEMENTATION_LEDGER.md")
progress = read(PROJECT_ROOT / "progress.md")

preview_body = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetFirstSessionRoutePreviewSummary")
roster_body = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetLaunchLanguageSaveRosterSummary")
refresh_body = function_body(main_menu_cpp, "void UCodeRescueMainMenuWidget::RefreshLanguageText")
launch_scene_body = function_body(game_mode_cpp, "void ACodeRescueGameMode::SpawnLaunchLanguageSelectionScene")

check_all(
    gi_h + preview_body,
    [
        "GetFirstSessionRoutePreviewSummary",
        "Category=\"Launch\"",
        "FCodeRescueCampaign::GetMissions()",
        "MakeLanguageSaveSlotName(Language)",
        "DoesLanguageSaveExist(Language)",
        "GetLanguageSaveSummary(Language)",
        "FIRST-SESSION ROUTE PREVIEW",
        "protected terminal -> survivor marker -> extraction",
        "FirstMission.TerminalTitle",
        "FirstMission.CurriculumStageName",
        "FirstMission.CurriculumFocus",
        "FirstMission.LandmarkName",
        "FirstMission.SurvivorName",
        "beginner/normal/challenge tuning band",
        "Start-screen choice remains first",
    ],
    "game instance must expose a reusable selected-language first route preview",
)
check("Missions[0]" in preview_body and "Resume available:" in preview_body,
      "route preview must use the first campaign mission and save/resume state")
check_all(
    gi_h + roster_body,
    [
        "GetLaunchLanguageSaveRosterSummary",
        "LANGUAGE SAVE ROSTER",
        "RESUME AVAILABLE",
        "NEW RUN READY",
        "ECodingLanguage::Java",
        "ECodingLanguage::C",
        "ECodingLanguage::CPlus",
        "ECodingLanguage::Cpp",
        "ECodingLanguage::Python",
        "ECodingLanguage::MATLAB",
        "DoesLanguageSaveExist(Language)",
    ],
    "game instance must expose a compact all-language launch save roster",
)

check_all(
    main_menu_h + main_menu_cpp + refresh_body,
    [
        "FirstSessionRoutePreviewText",
        "UTextBlock::StaticClass()",
        "SetAutoWrapText(true)",
        "CodeRescueUI::Color::TerminalGreen()",
        "GetFirstSessionRoutePreviewSummary(Language)",
        "GetLaunchLanguageSaveRosterSummary()",
        "LANGUAGE SAVE ROSTER",
        "Preview uses the current/default profile",
        "each row deploys only the clicked language",
        "This screen appears every time the game starts.",
    ],
    "launch-only main menu must show the route preview before gameplay",
)
check("if (bLaunchLanguageOnly)" in main_menu_cpp and "FirstSessionRoutePreviewText" in main_menu_h,
      "route preview text should be launch-screen scoped")

check_all(
    launch_scene_body,
    [
        "FIRST-SESSION ROUTE PREVIEW",
        "protected terminal -> survivor marker -> extraction",
        "each save is language-only",
    ],
    "fallback 3D launch scene must mirror the route preview contract",
)

check_all(
    preview_manifest + selected_manifest,
    [
        "First-session route preview",
        "Launch route preview",
        "Reusable summary",
        "Language-only save contract",
        "All-language save roster",
        "Route phase clarity",
        "FirstSessionRoutePreviewText",
        "GetFirstSessionRoutePreviewSummary",
        "GetLaunchLanguageSaveRosterSummary",
        "LANGUAGE SAVE ROSTER",
        "FIRST-SESSION ROUTE PREVIEW",
        "protected terminal -> survivor marker -> extraction",
        "verify_first_session_route_preview_slice_pass.py",
    ],
    "route preview manifests must describe runtime, save, route, and verifier coverage",
)
check_all(
    access_manifest + onboarding + visual_targets + human_qa,
    [
        "FirstSessionRoutePreviewAccessibility",
        "FirstSessionRoutePreview",
        "FIRST-SESSION ROUTE PREVIEW",
        "LANGUAGE SAVE ROSTER",
        "protected terminal -> survivor marker -> extraction",
        "language-only save slot",
        "Java, C, C+, C++, Python, and MATLAB",
        "RESUME AVAILABLE",
        "NEW RUN READY",
        "before active play",
    ],
    "accessibility, onboarding, visual, and human QA records must cover the start-screen preview",
)
check_all(
    creative_plan + full_qa + local_ci,
    [
        "verify_first_session_route_preview_slice_pass.py",
        "selected language terminal flow",
    ],
    "creative plan and QA entry points must run the route preview verifier",
)
check_all(
    slice_doc + ledger + progress,
    [
        "FIRST_SESSION_ROUTE_PREVIEW_SLICE.md",
        "First Session Route Preview Slice",
        "FIRST-SESSION ROUTE PREVIEW",
        "LANGUAGE SAVE ROSTER",
        "verify_first_session_route_preview_slice_pass.py",
        "155 named verifier references",
        "110 unique verifier scripts",
    ],
    "documentation, ledger, and progress log must record route preview work and updated counts",
)

if errors:
    for error in errors:
        print(f"[verify_first_session_route_preview_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_first_session_route_preview_slice_pass] PASS: first-session route preview verified")
