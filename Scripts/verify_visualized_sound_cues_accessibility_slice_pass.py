#!/usr/bin/env python3
"""Static verifier for the visualized sound cues accessibility slice."""

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


save_h = read(SRC / "CodeRescueSaveGame.h")
gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
settings_h = read(SRC / "CodeRescueSettingsWidget.h")
settings_cpp = read(SRC / "CodeRescueSettingsWidget.cpp")
hud_h = read(SRC / "CodeRescueHUDWidget.h")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
manifest = read(DATA / "visualized_sound_cues_accessibility_manifest.tsv")
accessibility = read(DATA / "accessibility_settings_manifest.tsv")
audio_manifest = read(DATA / "audio_coverage_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "VISUALIZED_SOUND_CUES_ACCESSIBILITY_SLICE.md")

accessibility_summary = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetAccessibilitySummary")
sound_summary = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetVisualizedSoundCueSummary")
save_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::SavePersistentRunInternal")
load_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::LoadPersistentRun")
settings_construct = function_body(settings_cpp, "void UCodeRescueSettingsWidget::NativeConstruct")
refresh_readouts = function_body(settings_cpp, "void UCodeRescueSettingsWidget::RefreshReadouts")
reset_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnResetAccessibilityClicked")
sync_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::SyncCachedControls")
apply_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnApplyClicked")
hud_construct = function_body(hud_cpp, "void UCodeRescueHUDWidget::NativeConstruct")
refresh_hud = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")

check("bool bVisualizeSoundCues = true" in save_h,
      "save game must persist the visualized sound cue setting")
check_all(
    gi_h,
    [
        "bool bVisualizeSoundCues = true",
        "GetVisualizedSoundCueSummary",
    ],
    "game instance header must expose visualized sound cue state and summary",
)
check_all(
    accessibility_summary + sound_summary,
    [
        "Sound cues %s",
        "Sound cues hidden",
        "Sound cues | threat",
        "ReactiveThreatMusicState",
        "CityAmbientZoneLabel",
        "bSubtitlesEnabled",
        "SfxVolume",
        "MusicVolume",
    ],
    "game instance summaries must expose visualized sound cues, threat, ambient, captions, and mix",
)
check_all(
    save_body + load_body,
    [
        "Save->bVisualizeSoundCues = bVisualizeSoundCues",
        "bVisualizeSoundCues = Save->bVisualizeSoundCues",
    ],
    "persistent save/load must carry the visualized sound cue setting",
)
check_all(
    settings_h,
    [
        "OnVisualizeSoundCuesChanged",
        "VisualizeSoundCuesCheck",
        "bCachedVisualizeSoundCues",
    ],
    "settings header must expose visualized sound cue controls",
)
check_all(
    settings_construct + refresh_readouts + reset_body + sync_body + apply_body,
    [
        "GI->bVisualizeSoundCues",
        "Visualize Sound Cues",
        "OnVisualizeSoundCuesChanged",
        "Sound cues %s",
        "bCachedVisualizeSoundCues = true",
        "VisualizeSoundCuesCheck->SetIsChecked",
        "GI->bVisualizeSoundCues = bCachedVisualizeSoundCues",
    ],
    "settings runtime must restore, preview, reset, sync, and apply visualized sound cues",
)
check("UTextBlock* SoundCueText" in hud_h,
      "HUD header must hold the sound cue text block")
check_all(
    hud_construct + refresh_hud,
    [
        "VisualizedSoundCueText",
        "SoundCueText",
        "SOUND CUES",
        "GI->bVisualizeSoundCues",
        "GI->ReactiveThreatMusicIntensity",
        "GI->ReactiveThreatMusicState",
        "GI->CityAmbientZoneIntensity",
        "GI->CityAmbientZoneLabel",
        "GI->bSubtitlesEnabled",
        "GI->bHighContrastHUD",
    ],
    "HUD must render visible threat, ambient, and caption state when enabled",
)
check_all(
    manifest,
    [
        "visualized_sound_cues_toggle",
        "threat_music_visualization",
        "ambient_zone_visualization",
        "caption_state_visualization",
        "verify_visualized_sound_cues_accessibility_slice_pass.py",
    ],
    "slice manifest must document visualized sound cue signals",
)
check("VisualizedSoundCuesAccessibility" in accessibility,
      "accessibility settings manifest must include visualized sound cues")
check("visualized_sound_cues" in audio_manifest,
      "audio coverage manifest must include visualized sound cues")
check("visualized sound cues" in creative_plan,
      "creative inclusion plan must include visualized sound cues")
check("VisualizedSoundCues" in visual_targets,
      "visual regression targets must include visualized sound cues")
check("VisualizedSoundCues" in human_qa,
      "human QA checklist must include visualized sound cues")
check("verify_visualized_sound_cues_accessibility_slice_pass.py" in full_qa,
      "full QA must run the visualized sound cues verifier")
check("verify_visualized_sound_cues_accessibility_slice_pass.py" in local_ci,
      "local CI must run the visualized sound cues verifier")
check("Visualized sound cues accessibility slice" in progress,
      "progress log must document the visualized sound cues slice")
check_all(
    slice_doc,
    [
        "TOP_50_RECOMMENDATIONS",
        "visualize sounds",
        "bVisualizeSoundCues",
        "GetVisualizedSoundCueSummary",
        "SoundCueText",
        "selected-language",
    ],
    "slice doc must map the implementation to the guidance and QA artifacts",
)

if errors:
    for error in errors:
        print(f"[verify_visualized_sound_cues_accessibility_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_visualized_sound_cues_accessibility_slice_pass] PASS: visualized sound cues accessibility verified")
