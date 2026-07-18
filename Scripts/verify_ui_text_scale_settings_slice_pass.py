#!/usr/bin/env python3
"""Static verifier for the independent UI text scale settings slice."""

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


save_h = read(SRC / "CodeRescueSaveGame.h")
gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
settings_h = read(SRC / "CodeRescueSettingsWidget.h")
settings_cpp = read(SRC / "CodeRescueSettingsWidget.cpp")
subtitles_cpp = read(SRC / "CodeRescueSubtitlesWidget.cpp")

widget_files = [
    "CodeRescueHUDWidget.cpp",
    "CodeRescuePauseWidget.cpp",
    "CodeRescueTutorialWidget.cpp",
    "CodeTerminalWidget.cpp",
    "CodeRescueObjectiveJournalWidget.cpp",
    "CodeRescueMinimapWidget.cpp",
    "CodeRescueSkillTreeWidget.cpp",
    "CodeRescueSaveSlotsWidget.cpp",
    "CityFastTravelWidget.cpp",
    "CodeRescueDamageFeedbackWidget.cpp",
    "CodeRescueVictoryWidget.cpp",
    "CodeRescueDeathWidget.cpp",
]
widget_sources = {name: read(SRC / name) for name in widget_files}

manifest = read(DATA / "ui_text_scale_settings_manifest.tsv")
access_manifest = read(DATA / "accessibility_settings_manifest.tsv")
expanded_manifest = read(DATA / "expanded_accessibility_options_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC / "UI_TEXT_SCALE_SETTINGS_SLICE.md")
progress = read(PROJECT_ROOT / "progress.md")

check_all(
    save_h + gi_h,
    [
        "float SubtitleScale = 1.0f",
        "float UITextScale = 1.0f",
        "float GetUITextScale() const",
        "FString GetUITextScaleSummary() const",
    ],
    "save/game instance headers must declare independent subtitle and UI scale fields",
)

check_all(
    gi_cpp,
    [
        "GetUITextScale()",
        "return FMath::Clamp(UITextScale, 0.80f, 1.75f)",
        "GetUITextScaleSummary()",
        "Save->SubtitleScale = FMath::Clamp(SubtitleScale, 0.75f, 1.75f)",
        "Save->UITextScale = GetUITextScale()",
        "SubtitleScale = FMath::Clamp(Save->SubtitleScale, 0.75f, 1.75f)",
        "UITextScale = FMath::Clamp(Save->UITextScale, 0.80f, 1.75f)",
        "TEXT(\"Subtitles %s %.1fx | UI %.1fx",
    ],
    "game instance implementation must clamp, summarize, save, and restore UI text scale independently",
)

check_all(
    settings_h + settings_cpp,
    [
        "OnSubtitleScaleChanged",
        "OnUITextScaleChanged",
        "SubtitleScaleSlider",
        "UITextScaleSlider",
        "CachedSubtitleScale",
        "CachedUITextScale",
        "AddRow(TEXT(\"Subtitle Size\"), SubtitleScaleSlider)",
        "AddRow(TEXT(\"UI Text Size\"), UITextScaleSlider)",
        "CachedUITextScale = GI->GetUITextScale()",
        "CachedUITextScale = 1.0f",
        "UITextScaleSlider->SetValue",
        "GI->SubtitleScale = FMath::Clamp(CachedSubtitleScale, 0.75f, 1.75f)",
        "GI->UITextScale = FMath::Clamp(CachedUITextScale, 0.80f, 1.75f)",
        "CodeRescueUI::Theme().TextScale = GI->GetUITextScale()",
        "GI->GetUITextScaleSummary()",
        "UI %.2fx",
    ],
    "settings widget must expose, preview, reset, apply, and report the independent UI text scale",
)

for name, source in widget_sources.items():
    check(
        "CodeRescueUI::Theme().TextScale = GI->GetUITextScale()" in source,
        f"{name} must use GetUITextScale() for shared UI theme text scale",
    )
    check(
        "CodeRescueUI::Theme().TextScale = FMath::Clamp(GI->SubtitleScale" not in source
        and "CodeRescueUI::Theme().TextScale = GI->SubtitleScale" not in source,
        f"{name} must not use SubtitleScale for shared UI theme text scale",
    )

check_all(
    subtitles_cpp,
    [
        "SubtitleScale = FMath::Clamp(GI->SubtitleScale, 0.75f, 1.75f)",
        "SubtitleFont.Size = FMath::RoundToInt(BaseSubtitleFont.Size * SubtitleScale)",
    ],
    "subtitle overlay must remain governed by SubtitleScale",
)
check("GI->GetUITextScale()" not in subtitles_cpp,
      "subtitle overlay should not read the UI text scale helper")

check_all(
    manifest,
    [
        "Settings menu",
        "Selected-language save",
        "Shared UI theme",
        "Subtitle overlay",
        "Reset defaults",
        "UITextScale",
        "SubtitleScale",
    ],
    "UI text scale manifest must document settings, save, theme, subtitle, and reset boundaries",
)

check_all(
    access_manifest + expanded_manifest,
    [
        "UITextSize",
        "UI text size",
        "UI Text Size slider",
        "UITextScale",
        "without changing subtitle",
    ],
    "accessibility manifests must document the independent UI text scale option",
)

check_all(
    creative_plan + visual_targets + human_qa,
    [
        "verify_ui_text_scale_settings_slice_pass.py",
        "UITextScaleSettings",
        "UI Text Size",
        "Subtitle Size",
    ],
    "creative, visual, and human QA manifests must include UI text scale review coverage",
)

check("verify_ui_text_scale_settings_slice_pass.py" in full_qa,
      "full QA must run the UI text scale verifier")
check("verify_ui_text_scale_settings_slice_pass.py" in local_ci,
      "local CI must run the UI text scale verifier")

check_all(
    slice_doc,
    [
        "UI Text Scale Settings Slice",
        "SubtitleScale",
        "UITextScale",
        "GetUITextScale()",
        "UI Text Size",
        "verify_ui_text_scale_settings_slice_pass.py",
    ],
    "slice documentation must describe the independent UI text scale work and validation",
)

check_all(
    progress,
    [
        "UI text scale settings slice",
        "UITextScale",
        "verify_ui_text_scale_settings_slice_pass.py",
    ],
    "progress log must document the UI text scale settings slice",
)

if errors:
    print("[verify_ui_text_scale_settings_slice_pass] FAIL")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("[verify_ui_text_scale_settings_slice_pass] PASS: independent UI text scale settings verified")
