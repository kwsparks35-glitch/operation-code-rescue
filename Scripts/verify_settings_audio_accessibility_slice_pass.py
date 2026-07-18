#!/usr/bin/env python3
"""Static verifier for the settings audio/accessibility persistence slice."""

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


save_h = read(SRC / "CodeRescueSaveGame.h")
gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
settings_h = read(SRC / "CodeRescueSettingsWidget.h")
settings_cpp = read(SRC / "CodeRescueSettingsWidget.cpp")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
survivor_cpp = read(SRC / "SurvivorActor.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "SETTINGS_AUDIO_ACCESSIBILITY_SLICE.md")

settings_construct = function_body(settings_cpp, "void UCodeRescueSettingsWidget::NativeConstruct")
refresh_readouts = function_body(settings_cpp, "void UCodeRescueSettingsWidget::RefreshReadouts")
reset_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnResetAccessibilityClicked")
apply_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnApplyClicked")
sync_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::SyncCachedControls")
audio_summary = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetAudioMixSummary")
apply_audio = function_body(gi_cpp, "void UCodeRescueGameInstance::ApplyAudioMixSettings")
play_music = function_body(gi_cpp, "void UCodeRescueGameInstance::PlayMusic")
load_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::LoadPersistentRun")
save_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::SavePersistentRunInternal")

check_all(
    save_h,
    [
        "float MasterVolume = 1.0f",
        "float SfxVolume = 1.0f",
        "float MusicVolume = 1.0f",
    ],
    "save game must persist the audio mix",
)
check_all(
    gi_h,
    [
        "float MasterVolume = 1.0f",
        "float SfxVolume = 1.0f",
        "float MusicVolume = 1.0f",
        "GetAudioMixSummary",
        "GetSfxVolumeScalar",
        "GetMusicVolumeScalar",
        "ApplyAudioMixSettings",
    ],
    "game instance must expose audio mix runtime fields and helpers",
)
check_all(
    audio_summary + apply_audio + play_music,
    [
        "Audio mix | Master",
        "SetTransientPrimaryVolume(MasterVolume)",
        "RefreshReactiveThreatMusicVolume()",
        "MusicComponent->FadeIn(FMath::Max(0.0f, FadeInDuration), 0.45f * GetMusicVolumeScalar() * GetReactiveThreatMusicScalar())",
    ],
    "game instance must summarize and apply master/music volume through the reactive-safe music path",
)
check_all(
    save_body + load_body,
    [
        "Save->MasterVolume = FMath::Clamp(MasterVolume",
        "Save->SfxVolume = FMath::Clamp(SfxVolume",
        "Save->MusicVolume = FMath::Clamp(MusicVolume",
        "MasterVolume = FMath::Clamp(Save->MasterVolume",
        "SfxVolume = FMath::Clamp(Save->SfxVolume",
        "MusicVolume = FMath::Clamp(Save->MusicVolume",
        "ApplyAudioMixSettings()",
    ],
    "persistent save/load must carry and reapply audio settings",
)
check_all(
    settings_h,
    [
        "AudioReadoutText",
        "GameplayReadoutText",
        "AccessibilityReadoutText",
        "ResetAccessibilityButton",
        "OnResetAccessibilityClicked",
        "RefreshReadouts",
        "SyncCachedControls",
    ],
    "settings header must expose readout and reset controls",
)
check_all(
    settings_construct,
    [
        "GI->MasterVolume",
        "GI->SfxVolume",
        "GI->MusicVolume",
        "AudioReadoutText",
        "GameplayReadoutText",
        "AccessibilityReadoutText",
        "Reset Accessibility Defaults",
        "RefreshReadouts()",
    ],
    "settings construct must restore saved audio values and create readouts",
)
check_all(
    refresh_readouts,
    [
        "Audio mix | Master",
        "Display and aim",
        "Accessibility | Subtitles",
        "UI %.2fx",
        "CachedMaster",
        "CachedSfx",
        "CachedMusic",
        "CachedFov",
        "CachedAimAssistScale",
    ],
    "settings readouts must summarize queued audio, gameplay, and accessibility values",
)
check_all(
    reset_body + sync_body,
    [
        "bCachedSubtitles = true",
        "CachedSubtitleScale = 1.0f",
        "CachedUITextScale = 1.0f",
        "bCachedHighContrast = false",
        "bCachedReducedMotion = false",
        "bCachedSimplifiedHints = false",
        "CachedAimAssistScale = 1.0f",
        "Accessibility defaults queued. Apply to save.",
        "SubtitleScaleSlider->SetValue",
        "UITextScaleSlider->SetValue",
        "AimAssistSlider->SetValue",
        "ReducedMotionCheck->SetIsChecked",
    ],
    "reset must queue safe accessibility defaults and sync visible controls",
)
check_all(
    apply_body,
    [
        "GI->MasterVolume = FMath::Clamp(CachedMaster",
        "GI->SfxVolume = FMath::Clamp(CachedSfx",
        "GI->MusicVolume = FMath::Clamp(CachedMusic",
        "GI->UITextScale = FMath::Clamp(CachedUITextScale",
        "GI->ApplyAudioMixSettings()",
        "GI->SavePersistentRun()",
        "GI->GetAudioMixSummary()",
        "GI->GetUITextScaleSummary()",
    ],
    "settings apply must commit audio values, apply live mix, save, and report the mix",
)
check("RefreshReadouts();" in settings_cpp and settings_cpp.count("RefreshReadouts();") >= 12,
      "slider/toggle callbacks should refresh the readout state")
check("GetRuntimeSfxVolume" in character_cpp and character_cpp.count("GetRuntimeSfxVolume(this)") >= 7,
      "player weapon cue playback must use the SFX scalar")
check("GetRuntimeSfxVolume" in zombie_cpp and zombie_cpp.count("GetRuntimeSfxVolume(this)") >= 4,
      "zombie attack/death/growl cue playback must use the SFX scalar")
check("GetRuntimeSfxVolume" in survivor_cpp and survivor_cpp.count("GetRuntimeSfxVolume(this)") >= 2,
      "survivor voice cue playback must use the SFX scalar")
check_all(
    gamemode_cpp,
    [
        "GetRuntimeSfxVolume(this)",
        "GetRuntimeMusicVolume(this)",
        "SpawnSound2D(\n                this, Cue, GetRuntimeSfxVolume(this)",  # 2026-07-11 refresh: radio uses stoppable SpawnSound2D at saved SFX volume
        "AC->VolumeMultiplier = 0.45f * GetRuntimeMusicVolume(this)",
    ],
    "game mode radio and ambient cues must use saved SFX/music values",
)
check_all(
    manifest,
    [
        "MasterVolume",
        "SfxVolume",
        "MusicVolume",
        "SettingsReadouts",
        "ResetAccessibility",
    ],
    "accessibility settings manifest must describe the new settings contract",
)
check("verify_settings_audio_accessibility_slice_pass.py" in full_qa,
      "full QA must run the settings audio accessibility verifier")
check("verify_settings_audio_accessibility_slice_pass.py" in local_ci,
      "local CI must run the settings audio accessibility verifier")
check("Settings audio accessibility slice" in progress,
      "progress log must document the settings audio accessibility slice")
check_all(
    slice_doc,
    [
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "accessibility_settings_manifest",
        "audio_coverage_manifest",
        "MasterVolume",
        "SfxVolume",
        "MusicVolume",
        "Reset Accessibility Defaults",
        "sound classes",
    ],
    "slice doc must map the settings/audio work to the June 25 guidance and note remaining audio-bus work",
)

if errors:
    for error in errors:
        print(f"[verify_settings_audio_accessibility_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_settings_audio_accessibility_slice_pass] PASS: settings audio accessibility slice verified")
