#!/usr/bin/env python3
"""Static verifier for the city ambient zone audio slice."""

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
manifest = read(DATA / "city_ambient_zone_audio_manifest.tsv")
audio_manifest = read(DATA / "audio_coverage_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
accessibility = read(DATA / "accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "CITY_AMBIENT_ZONE_AUDIO_SLICE.md")

tick_body = function_body(character_cpp, "void ACodeRescueCharacter::Tick")
ambient_body = function_body(character_cpp, "void ACodeRescueCharacter::UpdateCityAmbientZoneAudio")
update_body = function_body(game_instance_cpp, "void UCodeRescueGameInstance::UpdateCityAmbientZone")
summary_body = function_body(game_instance_cpp, "FString UCodeRescueGameInstance::GetCityAmbientZoneSummary")
audio_summary = function_body(game_instance_cpp, "FString UCodeRescueGameInstance::GetAudioMixSummary")

check_all(
    game_instance_h,
    [
        "bCityAmbientZoneDirectorEnabled",
        "CityAmbientZoneLabel",
        "CityAmbientZoneBed",
        "CityAmbientZoneIntensity",
        "UpdateCityAmbientZone",
        "GetCityAmbientZoneSummary",
    ],
    "game instance header must expose city ambient zone state and helpers",
)
check_all(
    character_h,
    [
        "bEnableCityAmbientZoneDirector",
        "CityAmbientZoneUpdateInterval",
        "UpdateCityAmbientZoneAudio",
        "LastCityAmbientZoneWorldTime",
        "LastCityAmbientZoneCaptionWorldTime",
        "LastCityAmbientZoneLabel",
    ],
    "character header must expose ambient-zone tuning, helper, and runtime state",
)
check("UpdateCityAmbientZoneAudio(DeltaSeconds);" in tick_body,
      "tick must update the city ambient zone director")
check_all(
    update_body,
    [
        "bCityAmbientZoneDirectorEnabled",
        "ambient disabled",
        "CityAmbientZoneLabel",
        "CityAmbientZoneBed",
        "CityAmbientZoneIntensity",
        "FMath::Clamp(ZoneIntensity",
    ],
    "game instance ambient-zone update must store enabled/disabled zone state",
)
check_all(
    summary_body + audio_summary,
    [
        "City ambient zone:",
        "CityAmbientZoneBed",
        "CityAmbientZoneIntensity",
        "Ambient %s",
        "CityAmbientZoneLabel",
    ],
    "audio summaries must expose ambient zone state",
)
check_all(
    ambient_body,
    [
        "FindClosestObjectiveIndex",
        "FCodeRescueCampaign::GetMission",
        "FCodeRescueCampaign::GetCityOrigin",
        "FCodeRescueCampaign::GetCitySpanScale",
        "SolvedTerminalIds",
        "RescuedSurvivorNames",
        "IsLocationInsideProtectedLearningZone",
        "UpdateCityAmbientZone",
        "UCodeRescueSubtitlesWidget::Push",
        "[Ambient]:",
        "CityAmbientZoneDirectorRuntime",
        "WorldDevelopmentZoneAmbientCues",
        "Top50Recommendation43SpatialAudio",
        "Top50Recommendation44AudioAccessibility",
        "protected coding lab",
        "entry approach",
        "survivor route open",
        "survivor search pressure",
        "extraction pad ready",
        "transit corridor",
        "civic overrun block",
        "zone_safehouse_low_hum",
        "zone_survivor_beacon_bed",
        "zone_extraction_rotor_wind",
    ],
    "character ambient-zone sampler must classify city spaces, progression state, captions, and QA tags",
)
check_all(
    manifest,
    [
        "zone_classification",
        "zone_ambient_bed_hook",
        "progress_aware_zone_tone",
        "ambient_caption",
        "qa_audit_tags",
        "WorldDevelopmentZoneAmbientCues",
    ],
    "manifest must document ambient-zone signals and QA hooks",
)
check_all(
    audio_manifest,
    [
        "city_ambient_zone_audio",
        "runtime.city_ambient_zone",
        "ambient_zone_state_caption",
        "ZoneAmbientCues",
    ],
    "audio coverage manifest must record the city ambient zone director",
)
check("city ambient zone director" in creative_plan,
      "creative inclusion plan must list city ambient zone director")
check("CityAmbientZoneAudio" in visual_targets,
      "visual regression targets must include city ambient zone audio")
check("CityAmbientZoneAudio" in human_qa,
      "human QA checklist must include city ambient zone audio")
check("CityAmbientZoneAudioAccessibility" in accessibility,
      "accessibility settings manifest must include city ambient zone audio")
check("verify_city_ambient_zone_audio_slice_pass.py" in full_qa,
      "full QA must run the city ambient zone verifier")
check("verify_city_ambient_zone_audio_slice_pass.py" in local_ci,
      "local CI must run the city ambient zone verifier")
check("City ambient zone audio slice" in progress,
      "progress log must document the city ambient zone audio slice")
check_all(
    slice_doc,
    [
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "ZoneAmbientCues",
        "UpdateCityAmbientZoneAudio",
        "UpdateCityAmbientZone",
        "[Ambient]",
        "selected-language",
    ],
    "slice doc must explain PDF guidance, implementation, accessibility, and QA",
)

if errors:
    for error in errors:
        print(f"[verify_city_ambient_zone_audio_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_city_ambient_zone_audio_slice_pass] PASS: city ambient zone audio verified")
