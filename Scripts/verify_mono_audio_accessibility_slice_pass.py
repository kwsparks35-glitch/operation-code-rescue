#!/usr/bin/env python3
"""Static verifier for the mono audio accessibility slice."""

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
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
zombie_h = read(SRC / "CodeZombieActor.h")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
survivor_cpp = read(SRC / "SurvivorActor.cpp")
game_mode_h = read(SRC / "CodeRescueGameMode.h")
game_mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(DATA / "mono_audio_accessibility_manifest.tsv")
accessibility = read(DATA / "accessibility_settings_manifest.tsv")
audio_manifest = read(DATA / "audio_coverage_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "MONO_AUDIO_ACCESSIBILITY_SLICE.md")

accessibility_summary = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetAccessibilitySummary")
mono_summary = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetMonoAudioSummary")
audio_summary = function_body(gi_cpp, "FString UCodeRescueGameInstance::GetAudioMixSummary")
apply_mix = function_body(gi_cpp, "void UCodeRescueGameInstance::ApplyAudioMixSettings")
save_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::SavePersistentRunInternal")
load_body = function_body(gi_cpp, "bool UCodeRescueGameInstance::LoadPersistentRun")
settings_construct = function_body(settings_cpp, "void UCodeRescueSettingsWidget::NativeConstruct")
mono_changed = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnMonoAudioChanged")
visual_changed = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnVisualizeSoundCuesChanged")
refresh_readouts = function_body(settings_cpp, "void UCodeRescueSettingsWidget::RefreshReadouts")
reset_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnResetAccessibilityClicked")
sync_body = function_body(settings_cpp, "void UCodeRescueSettingsWidget::SyncCachedControls")
apply_settings = function_body(settings_cpp, "void UCodeRescueSettingsWidget::OnApplyClicked")
zombie_mono = function_body(zombie_cpp, "void ACodeZombieActor::ApplyMonoAudioAccessibility")
zombie_growl = function_body(zombie_cpp, "void ACodeZombieActor::ScheduleNextGrowl")
ambient_spawn = function_body(game_mode_cpp, "void ACodeRescueGameMode::SpawnAmbientSoundForCity")
mono_refresh = function_body(game_mode_cpp, "int32 ACodeRescueGameMode::RefreshMonoAudioSpatialization")

check("bool bMonoAudio = false" in save_h, "save game must persist bMonoAudio")
check_all(
    gi_h,
    [
        "bool bMonoAudio = false",
        "GetMonoAudioSummary",
        "GetAccessibilitySummary",
        "GetAudioMixSummary",
    ],
    "game instance header must expose mono state and summaries",
)
check_all(
    accessibility_summary + mono_summary + audio_summary,
    [
        "Mono %s",
        "Mono audio: on",
        "positional cues centered",
        "visual sound cues locked on",
        "stereo spatial cues active",
    ],
    "game instance summaries must report mono audio state",
)
check_all(
    apply_mix,
    [
        "if (bMonoAudio)",
        "bVisualizeSoundCues = true",
        "SetTransientPrimaryVolume",
    ],
    "audio mix application must preserve volume behavior and lock visual cues when mono is on",
)
check_all(
    save_body + load_body,
    [
        "Save->bMonoAudio = bMonoAudio",
        "bMonoAudio = Save->bMonoAudio",
        "if (bMonoAudio)",
        "bVisualizeSoundCues = true",
    ],
    "persistent save/load must carry mono audio and restore visual cue lock",
)
check_all(
    settings_h,
    [
        "OnMonoAudioChanged",
        "MonoAudioCheck",
        "bCachedMonoAudio",
    ],
    "settings header must expose mono controls",
)
check_all(
    settings_construct + mono_changed + visual_changed + refresh_readouts + reset_body + sync_body + apply_settings,
    [
        "GI->bMonoAudio",
        "AddRow(TEXT(\"Mono Audio\"), MonoAudioCheck)",
        "bCachedMonoAudio",
        "bCachedVisualizeSoundCues = true",
        "Mono %s",
        "GI->bMonoAudio = bCachedMonoAudio",
        "if (bCachedMonoAudio)",
        "GI->bVisualizeSoundCues = true",
        "GI->bVisualizeSoundCues = bCachedVisualizeSoundCues",
        "RefreshMonoAudioSpatialization",
        "GetMonoAudioSummary",
        "SavePersistentRun",
    ],
    "settings widget must preview, apply, save, and live-refresh mono audio",
)
check("GetMonoSafeSoundLocation" in character_cpp and character_cpp.count("GetMonoSafeSoundLocation(this,") >= 7,
      "player weapon/melee cues must route through mono-safe sound locations")
check("GetMonoSafeSoundLocation" in survivor_cpp and survivor_cpp.count("GetMonoSafeSoundLocation(this,") >= 2,
      "survivor bark/rescue VO must route through mono-safe sound locations")
check_all(
    zombie_h + zombie_cpp + zombie_growl + zombie_mono,
    [
        "ApplyMonoAudioAccessibility",
        "GrowlAudio->bAllowSpatialization = !bMonoAudioEnabled",
        "MonoAudioCenteredThreatCue",
        "ApplyMonoAudioAccessibility(IsRuntimeMonoAudioEnabled(this))",
        "GetMonoSafeSoundLocation",
    ],
    "zombie attack/death/growl cues must support mono audio",
)
check(zombie_cpp.count("GetMonoSafeSoundLocation(this, GetActorLocation())") >= 3,
      "zombie attack/death cues must use mono-safe actor locations")
check_all(
    game_mode_h + ambient_spawn + mono_refresh,
    [
        "RefreshMonoAudioSpatialization",
        "AC->bAllowSpatialization = !IsRuntimeMonoAudioEnabled(this)",
        "MonoAudioRefreshableAmbientCue",
        "TActorIterator<AAmbientSound>",
        "TActorIterator<ACodeZombieActor>",
        "ApplyMonoAudioAccessibility(bMonoAudioEnabled)",
        "[CodeRescueMonoAudio]",
    ],
    "game mode must spawn and refresh mono-aware ambient/growl components",
)
check_all(
    manifest,
    [
        "mono_audio_toggle",
        "mono_visual_cue_lock",
        "centered_project_sfx",
        "active_spatial_refresh",
        "mono_review_summary",
    ],
    "mono audio manifest must describe all runtime surfaces",
)
check("MonoAudioAccessibility" in accessibility, "accessibility manifest must include MonoAudioAccessibility")
check("mono_audio_accessibility" in audio_manifest, "audio coverage manifest must include mono runtime row")
check("mono audio accessibility" in creative_plan.lower(), "creative inclusion plan must include mono audio accessibility row")
check("MonoAudioAccessibility" in visual_targets, "visual regression targets must include mono audio review")
check("MonoAudioAccessibility" in human_qa, "human QA checklist must include mono audio signoff")
check("verify_mono_audio_accessibility_slice_pass.py" in full_qa, "full QA must run mono audio verifier")
check("verify_mono_audio_accessibility_slice_pass.py" in local_ci, "local CI readiness must run mono audio verifier")
check_all(
    slice_doc,
    [
        "Mono Audio Accessibility Slice",
        "bMonoAudio",
        "GetMonoSafeSoundLocation",
        "RefreshMonoAudioSpatialization",
        "Scope Note",
        "verify_mono_audio_accessibility_slice_pass.py",
    ],
    "slice documentation must describe implementation, scope, and validation",
)
check("Mono audio accessibility slice" in progress, "progress log must include mono audio accessibility slice")

if errors:
    print("Mono audio accessibility slice verification failed:")
    for err in errors:
        print(f" - {err}")
    sys.exit(1)

print("Mono audio accessibility slice verification passed.")
