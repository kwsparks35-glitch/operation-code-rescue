#!/usr/bin/env python3
"""Static verifier for the weather and lighting identity slice."""

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


gamemode_h = read(SRC / "CodeRescueGameMode.h")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(DATA / "weather_lighting_identity_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
accessibility_manifest = read(DATA / "accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "WEATHER_LIGHTING_IDENTITY_SLICE.md")

spawn_city_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnCampaignCity")
weather_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnWeatherLightingIdentityLayer")

check_all(
    gamemode_h,
    [
        "SpawnWeatherLightingIdentityLayer",
    ],
    "game mode header must declare the weather lighting identity layer",
)

weather_idx = spawn_city_body.find("SpawnWeatherForCity(CityIndex, Origin)")
identity_idx = spawn_city_body.find("SpawnWeatherLightingIdentityLayer(Mission, CityIndex, Origin, CityLabel)")
check(weather_idx >= 0, "campaign city spawn must still call SpawnWeatherForCity")
check(identity_idx > weather_idx >= 0, "weather lighting identity must spawn after the base weather hook")

check_all(
    weather_body,
    [
        "CLEAR SKY CONTRAST",
        "OVERCAST SHELTER LIGHTING",
        "MARINE FOG ROUTE LIGHTING",
        "WARM HAZE HEAT MIRAGE",
        "HUMID STORM GLOW",
        "COLD SNOW SKY",
        "Climate.CloudToken",
        "Climate.GradeToken",
        "Climate.FogDensity",
    ],
    "weather layer must expose every climate family label and use existing climate tokens",
)
check_all(
    weather_body,
    [
        "ENTRY WEATHER SHELTER",
        "SAFEHOUSE LIGHT POOL",
        "RESCUE ROUTE SKY CUE",
        "WeatherLightingIdentity",
        "WeatherLightingIdentityReady",
        "DistrictWeatherCue",
        "NonBlockingWeatherCue",
        "WeatherLightingSignalLight",
        "WorldDevelopmentDeepDive",
        "Top50Recommendations",
        "APointLight",
        "SpawnBlock",
        "SpawnGuideText",
        "[CodeRescueWeatherLightingIdentity]",
    ],
    "weather layer must create tagged district cues, lights, labels, and a runtime marker",
)

check_all(
    manifest,
    [
        "WeatherLightingIdentity",
        "SpawnWeatherLightingIdentityLayer",
        "EntryWeatherShelter",
        "SafehouseLightPool",
        "RescueRouteSkyCue",
        "NonBlockingWeatherCue",
        "WeatherLightingSignalLight",
    ],
    "weather lighting manifest must document district cues and review tags",
)
check_all(
    creative_plan,
    [
        "weather and lighting identity",
        "verify_weather_lighting_identity_slice_pass.py plus packaged render smoke plus visual review",
    ],
    "creative plan must move weather/lighting identity from package-only to verified implementation",
)
check_all(
    visual_manifest,
    [
        "WeatherLightingIdentity",
        "entry weather shelter, safehouse light pool, and rescue route sky cue",
    ],
    "visual regression targets must include the weather lighting review target",
)
check_all(
    human_qa,
    [
        "WeatherLightingIdentity",
        "entry weather shelter",
        "safehouse light pool",
        "rescue route sky cue",
    ],
    "human QA checklist must include the weather lighting walkthrough",
)
check_all(
    accessibility_manifest,
    [
        "WeatherLightingIdentityAccessibility",
        "text-first climate labels",
        "nonblocking weather cues",
    ],
    "accessibility manifest must document weather/light cues without color dependency",
)
check_all(
    full_qa + local_ci,
    [
        "verify_weather_lighting_identity_slice_pass.py",
    ],
    "QA scripts must run the weather lighting identity verifier",
)
check_all(
    progress,
    [
        "Weather and lighting identity slice",
        "SpawnWeatherLightingIdentityLayer",
        "OVERCAST SHELTER LIGHTING",
        "MARINE FOG ROUTE LIGHTING",
    ],
    "progress log must record the weather lighting identity slice",
)
check_all(
    slice_doc,
    [
        "Weather and lighting identity slice",
        "SpawnWeatherLightingIdentityLayer",
        "Entry Weather Shelter",
        "Safehouse Light Pool",
        "Rescue Route Sky Cue",
        "Validation",
    ],
    "slice documentation must explain implementation and validation",
)

if errors:
    for error in errors:
        print(f"[verify-weather-lighting-identity] BLOCKER: {error}", file=sys.stderr)
    raise SystemExit(1)

print("[verify-weather-lighting-identity] PASS")
