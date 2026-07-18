#!/usr/bin/env python3
"""Static verifier for the reactive threat audio and music slice."""

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


character_h = read(SRC / "CodeRescueCharacter.h")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
game_instance_h = read(SRC / "CodeRescueGameInstance.h")
game_instance_cpp = read(SRC / "CodeRescueGameInstance.cpp")
manifest = read(DATA / "reactive_threat_audio_music_manifest.tsv")
audio_manifest = read(DATA / "audio_coverage_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
accessibility = read(DATA / "accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "REACTIVE_THREAT_AUDIO_MUSIC_SLICE.md")

tick_body = function_body(character_cpp, "void ACodeRescueCharacter::Tick")
reactive_body = function_body(character_cpp, "void ACodeRescueCharacter::UpdateReactiveThreatAudio")
scalar_body = function_body(game_instance_cpp, "float UCodeRescueGameInstance::GetReactiveThreatMusicScalar")
refresh_body = function_body(game_instance_cpp, "void UCodeRescueGameInstance::RefreshReactiveThreatMusicVolume")
update_body = function_body(game_instance_cpp, "void UCodeRescueGameInstance::UpdateReactiveThreatMusic")
play_music_body = function_body(game_instance_cpp, "void UCodeRescueGameInstance::PlayMusic")
apply_mix_body = function_body(game_instance_cpp, "void UCodeRescueGameInstance::ApplyAudioMixSettings")
summary_body = function_body(game_instance_cpp, "FString UCodeRescueGameInstance::GetAudioMixSummary")

check_all(
    game_instance_h,
    [
        "bReactiveThreatMusicEnabled",
        "ReactiveThreatMusicIntensity",
        "ReactiveThreatMusicState",
        "GetReactiveThreatMusicScalar",
        "UpdateReactiveThreatMusic",
        "GetReactiveThreatMusicSummary",
        "RefreshReactiveThreatMusicVolume",
    ],
    "game instance header must expose reactive music settings, state, helpers, and summary",
)
check_all(
    character_h,
    [
        "bEnableReactiveThreatAudio",
        "ReactiveThreatAudioRange",
        "ReactiveThreatAudioCriticalRange",
        "ReactiveThreatAudioUpdateInterval",
        "UpdateReactiveThreatAudio",
        "LastReactiveThreatAudioWorldTime",
        "LastReactiveThreatAudioCaptionWorldTime",
        "ReactiveThreatAudioSmoothedIntensity",
        "LastReactiveThreatAudioState",
    ],
    "character header must expose reactive threat audio tuning, helper, and runtime state",
)
check("UpdateReactiveThreatAudio(DeltaSeconds);" in tick_body,
      "tick must update the reactive threat audio sampler")
check_all(
    scalar_body,
    [
        "bReactiveThreatMusicEnabled",
        "FMath::Lerp(0.82f, 1.22f",
        "ReactiveThreatMusicIntensity",
    ],
    "reactive music scalar must be toggleable and restrained",
)
check_all(
    refresh_body,
    [
        "MusicComponent->SetVolumeMultiplier",
        "GetMusicVolumeScalar() * GetReactiveThreatMusicScalar()",
    ],
    "music refresh must layer the reactive scalar on top of saved music volume",
)
check_all(
    update_body,
    [
        "ReactiveThreatMusicIntensity",
        "ReactiveThreatMusicState",
        "RefreshReactiveThreatMusicVolume",
    ],
    "game instance update must store state and refresh music volume",
)
check("GetReactiveThreatMusicScalar" in play_music_body,
      "PlayMusic must initialize the existing music component with reactive scalar")
check("RefreshReactiveThreatMusicVolume" in apply_mix_body,
      "ApplyAudioMixSettings must preserve reactive music volume after settings changes")
check_all(
    summary_body,
    [
        "Threat music",
        "ReactiveThreatMusicState",
        "ReactiveThreatMusicIntensity",
    ],
    "audio summary must expose reactive threat music state for settings and QA",
)
check_all(
    reactive_body,
    [
        "TActorIterator<ACodeZombieActor>",
        "Zombie->Health <= 0.0f",
        "ReactiveThreatAudioRange",
        "ReactiveThreatAudioCriticalRange",
        "IsLocationInsideProtectedLearningZone",
        "FMath::FInterpTo",
        "UpdateReactiveThreatMusic",
        "UCodeRescueSubtitlesWidget::Push",
        "[Audio]: threat mix",
        "ReactiveThreatAudioRuntime",
        "ReactiveThreatMusicDirector",
        "Top50Recommendation43ReactiveAudio",
        "WorldDevelopmentAudioGuidance",
        "calm",
        "watch",
        "pursuit",
        "critical",
        "safehouse muted",
    ],
    "character sampler must score living zombies, damp safehouses, push captions, and tag QA state",
)
check_all(
    manifest,
    [
        "threat_pressure_scan",
        "safehouse_music_damp",
        "music_volume_scalar",
        "audio_state_caption",
        "qa_audit_tags",
        "Top50Recommendation43ReactiveAudio",
    ],
    "manifest must document reactive audio signals and QA tags",
)
check_all(
    audio_manifest,
    [
        "reactive_threat_audio_music",
        "runtime.reactive_threat_music",
        "reactive_audio_state_caption",
        "nearby_zombie_pressure",
    ],
    "audio coverage manifest must record the runtime reactive music bridge",
)
check("reactive threat music and captions" in creative_plan,
      "creative inclusion plan must list reactive threat audio")
check("ReactiveThreatAudioMusic" in visual_targets,
      "visual regression targets must include reactive threat audio manual review")
check("ReactiveThreatAudioMusic" in human_qa,
      "human QA checklist must include reactive threat audio")
check("ReactiveThreatAudioMusicAccessibility" in accessibility,
      "accessibility settings manifest must include reactive threat audio")
check("verify_reactive_threat_audio_music_slice_pass.py" in full_qa,
      "full QA must run the reactive threat audio verifier")
check("verify_reactive_threat_audio_music_slice_pass.py" in local_ci,
      "local CI must run the reactive threat audio verifier")
check("Reactive threat audio music slice" in progress,
      "progress log must document the reactive threat audio music slice")
check_all(
    slice_doc,
    [
        "TOP_50_RECOMMENDATIONS",
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "UpdateReactiveThreatAudio",
        "UpdateReactiveThreatMusic",
        "MusicVolume",
        "[Audio]",
        "protected learning",
    ],
    "slice doc must explain PDF guidance, implementation, accessibility, and QA",
)

if errors:
    for error in errors:
        print(f"[verify_reactive_threat_audio_music_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_reactive_threat_audio_music_slice_pass] PASS: reactive threat audio music verified")
