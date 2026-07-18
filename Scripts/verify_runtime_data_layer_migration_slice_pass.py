#!/usr/bin/env python3
"""Static verifier for the runtime Data Layer migration slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOC_DIR = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"
SOURCE_DOC_DIR = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-25"

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


gamemode_h = read(SRC / "CodeRescueGameMode.h")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(DATA / "runtime_data_layer_migration_manifest.tsv")
world_contract = read(DATA / "world_promotion_validation_contract.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
performance_budget = read(DATA / "performance_city_layer_budget.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "RUNTIME_DATA_LAYER_MIGRATION_SLICE.md")
world_doc = read(SOURCE_DOC_DIR / "WORLD_DEVELOPMENT_DEEPDIVE.md")
top50_doc = read(SOURCE_DOC_DIR / "TOP_50_RECOMMENDATIONS_2026-06-25.md")

spawn_city_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnCampaignCity")
register_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::RegisterStreamedActor")
helper_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::ApplyRuntimeDataLayerTags")
runtime_layer_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnRuntimeDataLayerMigrationLayer")
weather_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnWeatherLightingIdentityLayer")
terminal_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnTerminal")
safehouse_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnProtectedCodingChallengeHub")
director_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnEncounterDirectorLayer")
solved_route_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::RevealSolvedTerminalRescueRoute")

check_all(
    gamemode_h,
    [
        "SpawnRuntimeDataLayerMigrationLayer",
        "ApplyRuntimeDataLayerTags",
    ],
    "game mode header must declare the Data Layer migration helpers",
)
check_all(
    helper_body,
    [
        "RuntimeDataLayerStandIn",
        "WorldPartitionDataLayerMigration",
        "Top50Recommendation31",
        "DataLayerReadyFallback",
        "LayerTags",
    ],
    "runtime Data Layer helper must add review and future-migration tags",
)
check_all(
    register_body,
    [
        "ActiveCampaignCityStreamedActor",
        "RuntimeWorldPartitionStreamCell",
        "CurrentCppWorldPartitionFallback",
        "OneFilePerActorMigrationReady",
        "WorldPartitionReady",
    ],
    "streamed actor registration must tag the current C++ World Partition fallback",
)
check_all(
    runtime_layer_body,
    [
        "RuntimeDataLayerMigrationLayer",
        "WorldPartitionDataLayerBridge",
        "DataLayerStateTimeModeReview",
        "OFPAActorLayerAuditReady",
        "STREAMING CELL",
        "SAFEHOUSE MODE",
        "COMBAT MODE",
        "OBJECTIVE STATE",
        "WEATHER / TIME",
        "WORLD PARTITION NAVIGATION",
        "RuntimeDataLayer_State_SafeBeat",
        "RuntimeDataLayer_State_Overrun",
        "RuntimeDataLayer_State_TerminalLocked",
        "RuntimeDataLayer_State_RescueRouteOpen",
        "RuntimeDataLayer_Time_DayNightCycle",
        "RuntimeDataLayer_Weather_",
        "RuntimeDataLayer_Grade_",
        "[CodeRescueRuntimeDataLayers]",
    ],
    "runtime layer must create visible state/time/mode cards and log review state",
)
creative_idx = spawn_city_body.find("SpawnCreativeRecommendationImplementationLayer(Mission, CityIndex, Origin, CityLabel)")
data_layer_idx = spawn_city_body.find("SpawnRuntimeDataLayerMigrationLayer(Mission, CityIndex, Origin, CityLabel)")
check(creative_idx >= 0, "campaign city must still spawn the creative recommendation layer")
check(data_layer_idx > creative_idx >= 0, "runtime Data Layer station should spawn after creative recommendation evidence")
check_all(
    weather_body,
    [
        "RuntimeDataLayer_Time_DayNightCycle",
        "RuntimeDataLayer_Mode_WeatherLighting",
        "RuntimeDataLayer_Weather_",
        "RuntimeDataLayer_Grade_",
        "ApplyRuntimeDataLayerTags",
    ],
    "weather layer must carry time/weather/grade Data Layer tags",
)
check_all(
    terminal_body,
    [
        "RuntimeDataLayer_State_SafeBeat",
        "RuntimeDataLayer_Mode_CodingSafehouse",
        "RuntimeDataLayer_Mode_SelectedLanguageOnly",
    ],
    "terminal actor must be tagged as a safe selected-language Data Layer member",
)
check_all(
    safehouse_body,
    [
        "RuntimeDataLayer_State_SafeBeat",
        "RuntimeDataLayer_Mode_CodingSafehouse",
        "RuntimeDataLayer_Mode_SelectedLanguageOnly",
    ],
    "protected safehouse hub must tag its actors as safe learning Data Layer stand-ins",
)
check_all(
    director_body,
    [
        "RuntimeDataLayer_Mode_Combat",
        "RuntimeDataLayer_State_Overrun",
        "RuntimeDataLayer_State_RescueRouteOpen",
        "RuntimeDataLayer_State_TerminalLocked",
    ],
    "encounter director must expose combat and objective-state Data Layer tags",
)
check_all(
    solved_route_body,
    [
        "RuntimeDataLayer_State_RescueRouteOpen",
        "RuntimeDataLayer_Mode_RescueTraversal",
        "RuntimeDataLayer_State_Prerecovery",
    ],
    "solved terminal world response must expose the rescue-open Data Layer transition",
)
check_all(
    manifest,
    [
        "RuntimeWorldPartitionFallback",
        "SafehouseModeLayer",
        "CombatModeLayer",
        "ObjectiveStateLayer",
        "WeatherTimeLayer",
        "SpawnRuntimeDataLayerMigrationLayer",
        "verify_runtime_data_layer_migration_slice_pass.py",
    ],
    "runtime Data Layer manifest must document all review cards",
)
check_all(
    world_contract,
    [
        "Data Layer migration",
        "verify_runtime_data_layer_migration_slice_pass.py",
        "RuntimeDataLayer_State_SafeBeat",
        "RuntimeDataLayer_State_Overrun",
        "RuntimeDataLayer_Time_DayNightCycle",
    ],
    "world promotion contract must include the runtime Data Layer slice",
)
check_all(
    creative_plan,
    [
        "World Partition + Data Layer migration",
        "verify_runtime_data_layer_migration_slice_pass.py plus verify_world_promotion_validation_contract_pass.py",
    ],
    "creative plan must route World Partition/Data Layer migration through this verifier",
)
check_all(
    visual_manifest,
    [
        "RuntimeDataLayerMigration",
        "streaming, safehouse, combat, objective, and weather/time Data Layer cards",
    ],
    "visual regression targets must include the runtime Data Layer station",
)
check_all(
    human_qa,
    [
        "RuntimeDataLayerMigration",
        "streaming cell, safehouse mode, combat mode, objective state, and weather/time cards",
    ],
    "human QA checklist must include the runtime Data Layer walkthrough",
)
check_all(
    performance_budget,
    [
        "RuntimeDataLayerMigration",
        "World Partition/Data Layer fallback station",
    ],
    "performance budget must include the lightweight runtime Data Layer station",
)
check("verify_runtime_data_layer_migration_slice_pass.py" in full_qa,
      "full QA must run the runtime Data Layer migration verifier")
check("verify_runtime_data_layer_migration_slice_pass.py" in local_ci,
      "local CI must run the runtime Data Layer migration verifier")
check_all(
    slice_doc,
    [
        "Runtime Data Layer Migration Slice",
        "SpawnRuntimeDataLayerMigrationLayer",
        "ApplyRuntimeDataLayerTags",
        "World Partition/Data Layer",
        "state, time, and mode",
        "Validation",
    ],
    "slice documentation must explain implementation and validation",
)
check_all(
    progress,
    [
        "Runtime Data Layer migration slice",
        "SpawnRuntimeDataLayerMigrationLayer",
        "RuntimeDataLayer_State_SafeBeat",
        "RuntimeDataLayer_Time_DayNightCycle",
    ],
    "progress log must record this slice",
)
check_all(
    world_doc,
    [
        "Data Layers for state, time, and mode",
        "World Partition",
        "One File Per Actor",
        "Pristine/Overrun",
        "Time-of-day / weather",
        "Game mode",
        "bSandboxMode",
    ],
    "world deep dive must contain the Data Layer guidance this slice implements",
)
check_all(
    top50_doc,
    [
        "World Partition + streaming",
        "World Partition/Data Layers/Level Instances",
    ],
    "Top 50 recommendations must contain the World Partition/Data Layer target",
)

if errors:
    for error in errors:
        print(f"[verify_runtime_data_layer_migration_slice_pass] FAIL: {error}", file=sys.stderr)
    raise SystemExit(1)

print("[verify_runtime_data_layer_migration_slice_pass] PASS: runtime Data Layer migration verified")
