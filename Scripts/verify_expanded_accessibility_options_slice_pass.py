#!/usr/bin/env python3
"""Static verifier for the expanded accessibility options slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"
SOURCE_DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-25"

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


settings_h = read(SRC / "CodeRescueSettingsWidget.h")
settings_cpp = read(SRC / "CodeRescueSettingsWidget.cpp")
game_instance_h = read(SRC / "CodeRescueGameInstance.h")
game_instance_cpp = read(SRC / "CodeRescueGameInstance.cpp")
save_game_h = read(SRC / "CodeRescueSaveGame.h")
access_manifest = read(DATA / "accessibility_settings_manifest.tsv")
expanded_manifest = read(DATA / "expanded_accessibility_options_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC / "EXPANDED_ACCESSIBILITY_OPTIONS_SLICE.md")
top50 = read(SOURCE_DOC / "TOP_50_RECOMMENDATIONS_2026-06-25.md")
ux_guide = read(SOURCE_DOC / "UX_OVERHAUL_GUIDE.md")
progress = read(PROJECT_ROOT / "progress.md")
self_source = read(PROJECT_ROOT / "Scripts/verify_expanded_accessibility_options_slice_pass.py")


check_all(
    settings_h,
    [
        "OnSubtitleScaleChanged",
        "OnUITextScaleChanged",
        "OnAimAssistChanged",
        "OnColorblindModeClicked",
        "OnReducedMotionChanged",
        "OnSimplifiedHintsChanged",
        "OnResetAccessibilityClicked",
        "OnExportControlsClicked",
        "AudioReadoutText",
        "GameplayReadoutText",
        "AccessibilityReadoutText",
        "ControlProfileText",
    ],
    "settings header must expose expanded accessibility controls and readouts",
)

check_all(
    settings_cpp,
    [
        "Subtitle Size",
        "UI Text Size",
        "Aim Assist Strength",
        "High Contrast HUD",
        "Color Vision Mode",
        "Reduced Motion",
        "Simplified Input Hints",
        "Reset Accessibility Defaults",
        "Export Control Profile",
        "RefreshReadouts",
        "RefreshColorblindButtonLabel",
        "SyncCachedControls",
        "RefreshActiveColorVisionPostProcess",
        "UCodeRescueSubtitlesWidget::RefreshAccessibilityState",
        "UCodeRescueDamageFeedbackWidget::RefreshAccessibilityState",
        "CodeRescueUI::Theme().bHighContrast",
        "CodeRescueUI::Theme().bReducedMotion",
        "CodeRescueUI::Theme().TextScale",
        "SavePersistentRun",
        "GetControlProfileSummary",
        "ExportControlProfileReviewFile",
    ],
    "settings implementation must wire controls, live refresh, persistence, and export",
)

check_all(
    game_instance_h + save_game_h,
    [
        "bSubtitlesEnabled",
        "ColorblindMode",
        "SubtitleScale",
        "UITextScale",
        "GetUITextScale",
        "bHighContrastHUD",
        "bReducedMotion",
        "bSimplifiedInputHints",
        "AimAssistScale",
        "MasterVolume",
        "SfxVolume",
        "MusicVolume",
        "ControlProfileName",
        "ControlProfileExportCount",
        "GetAccessibilitySummary",
        "GetAudioMixSummary",
        "ApplyAudioMixSettings",
    ],
    "game instance and save game must persist expanded accessibility/audio/control state",
)

check_all(
    game_instance_cpp,
    [
        "GetAccessibilitySummary",
        "Save->bSubtitlesEnabled",
        "Save->ColorblindMode",
        "Save->SubtitleScale",
        "Save->UITextScale",
        "Save->bHighContrastHUD",
        "Save->bReducedMotion",
        "Save->bSimplifiedInputHints",
        "Save->AimAssistScale",
        "Save->ControlProfileName",
        "Save->ControlProfileExportCount",
        "bSubtitlesEnabled = Save->bSubtitlesEnabled",
        "ColorblindMode = Save->ColorblindMode",
        "SubtitleScale = FMath::Clamp(Save->SubtitleScale",
        "UITextScale = FMath::Clamp(Save->UITextScale",
        "bHighContrastHUD = Save->bHighContrastHUD",
        "bReducedMotion = Save->bReducedMotion",
        "bSimplifiedInputHints = Save->bSimplifiedInputHints",
        "AimAssistScale = FMath::Clamp(Save->AimAssistScale",
        "ControlProfileName = Save->ControlProfileName",
        "ControlProfileExportCount = FMath::Max(0, Save->ControlProfileExportCount)",
    ],
    "game instance implementation must save and restore expanded accessibility state",
)

check_all(
    access_manifest,
    [
        "Subtitles",
        "SubtitleSize",
        "SubtitleLiveRefresh",
        "HighContrastHUD",
        "ReducedMotion",
        "AimAssist",
        "ColorSafeMarkers",
        "SettingsReadouts",
        "ResetAccessibility",
        "DamageFeedbackAccessibility",
        "ObjectiveJournalAccessibility",
        "TerminalDiegeticAccessibility",
        "EndStateLanguageRunAccessibility",
    ],
    "accessibility manifest must cover controls plus affected runtime surfaces",
)

check_all(
    expanded_manifest,
    [
        "Subtitles",
        "Subtitle size",
        "UI text size",
        "High contrast",
        "Color vision",
        "Reduced motion",
        "Simplified input hints",
        "Aim assist",
        "Reset defaults",
        "Settings readouts",
        "Control profile export",
        "verify_expanded_accessibility_options_slice_pass.py",
    ],
    "expanded accessibility manifest must list every player-facing option",
)

check_all(
    creative_plan,
    [
        "expanded accessibility options",
        "verify_expanded_accessibility_options_slice_pass.py",
        "verify_settings_audio_accessibility_slice_pass.py",
        "verify_subtitle_accessibility_live_refresh_slice_pass.py",
        "verify_damage_feedback_accessibility_slice_pass.py",
        "verify_objective_journal_accessibility_slice_pass.py",
        "manual accessibility QA",
    ],
    "creative plan must route the P2 accessibility row through concrete verifiers",
)

check_all(
    human_qa,
    [
        "Accessibility",
        "color vision",
        "reset defaults",
        "control profile export",
        "observable runtime effect",
    ],
    "human QA must include the expanded settings review",
)

check_all(
    visual_targets,
    [
        "SettingsAccessibility",
        "subtitle/aim/color vision/readout controls",
        "720p",
    ],
    "visual targets must keep the settings accessibility screenshot target",
)

check_all(
    onboarding,
    [
        "settings",
        "queued accessibility/audio/gameplay choices",
    ],
    "first-ten-minutes onboarding must mention settings readout expectations",
)

check("verify_expanded_accessibility_options_slice_pass.py" in full_qa,
      "full QA must run the expanded accessibility verifier")
check("verify_expanded_accessibility_options_slice_pass.py" in local_ci,
      "local CI must run the expanded accessibility verifier")

check_all(
    slice_doc,
    [
        "Expanded Accessibility Options Slice",
        "Runtime Coverage",
        "Source Guidance",
        "Boundaries",
        "Validation",
        "mono mix",
        "visualized-sound",
    ],
    "slice documentation must explain coverage and audio-production boundaries",
)

check_all(
    top50,
    [
        "Accessibility actually drives the UI",
        "High-contrast",
        "reduced-motion",
        "text",
        "Mix & accessibility",
    ],
    "Top 50 source guidance must include accessibility recommendations",
)

check_all(
    ux_guide,
    [
        "Accessibility is built in",
        "mirrored from the player's saved settings",
        "high contrast",
        "bReducedMotion",
        "TextScale",
    ],
    "UX guide must include shared accessibility theme guidance",
)

check_all(
    progress,
    [
        "Expanded accessibility options slice",
        "expanded_accessibility_options_manifest.tsv",
        "verify_expanded_accessibility_options_slice_pass.py",
    ],
    "progress log must record the expanded accessibility options slice",
)

check("verify_expanded_accessibility_options_slice_pass.py" in self_source,
      "static verifier should identify itself")

if errors:
    print("[verify_expanded_accessibility_options_slice_pass] FAIL")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("[verify_expanded_accessibility_options_slice_pass] PASS: expanded accessibility options verified")
