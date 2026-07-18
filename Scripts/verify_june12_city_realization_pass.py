#!/usr/bin/env python3
"""Static verifier for the whole-city U.S. realization pass (doc 45, 2026-06-12).

Locks the contract that every U.S. campaign city is built out city-wide to
approximate its real counterpart across all eight requested domains:
landscape, architecture, sky, roads, sidewalks, homes, vehicles, clothing.

Checks:
- realization params struct + builder exist with all domain fields,
- the four realization layers are declared, implemented, called from
  SpawnCampaignCity, and gated on IsUSMajorCityMission,
- the systemic skyline consumes facade palette / downtown height / sprawl,
- the day/night Tick consumes the per-city sun members,
- the urban street layer consumes road tones, widths, and pattern families,
- the post-process volume consumes the climate grade token,
- decorative civilians consume the city wardrobe palette + accessories,
- named-city sharpening covers the high-signal metros,
- runtime log markers exist for smoke-log verification,
- doc-43 identity contracts remain intact,
- this verifier is registered in Run_Full_QA_Audit.command.
"""

from __future__ import annotations

import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def need(haystack: str, needle: str, where: str) -> None:
    if needle not in haystack:
        errors.append(f"{where}: missing `{needle}`")


cpp = read(SRC / "CodeRescueGameMode.cpp")
spawning_cpp = read(SRC / "CodeRescueGameModeSpawning.cpp")
cpp_with_spawning = cpp + "\n" + spawning_cpp
hdr = read(SRC / "CodeRescueGameMode.h")

# 1. Params struct + builder with all eight domains concretely parameterized.
need(cpp, "struct FCodeRescueUSCityRealizationParams", "GameMode.cpp")
need(cpp, "FCodeRescueUSCityRealizationParams BuildUSCityRealizationParams(", "GameMode.cpp")
for field in ("TerrainToken", "BackdropToken", "VegetationToken", "bWaterEdge",
              "bRiverThrough", "WaterEdgeSide", "DaySunColor", "FogDensity",
              "CloudToken", "GradeToken", "FacadePalette", "DowntownHeightScale",
              "SprawlFalloff", "RoadPatternToken", "AsphaltTone", "SidewalkTone",
              "SidewalkWidthScale", "bBrickHistoricWalks", "HomeArchetypeToken",
              "HomePalette", "VehicleMix", "CurbVehicleCount", "WardrobePalette",
              "WardrobeAccessoryToken"):
    need(cpp, field, "realization params")

# 2. Four layers declared, implemented, called, and US-gated.
for fn in ("ApplyUSCitySkyRealization", "SpawnUSCityLandscapeRealizationLayer",
           "SpawnUSCityResidentialDistrictLayer", "SpawnUSCityVehiclePopulationLayer"):
    need(hdr, f"void {fn}(const struct FCodeRescueCityMission&", "GameMode.h")
    need(cpp, f"void ACodeRescueGameMode::{fn}(", "GameMode.cpp")
    need(cpp, f"{fn}(Mission, CityIndex, Origin, CityLabel);", "SpawnCampaignCity")
spawn_city = cpp.split("void ACodeRescueGameMode::SpawnCampaignCity(")[1].split("\n}\n")[0] \
    if "void ACodeRescueGameMode::SpawnCampaignCity(" in cpp else ""
need(spawn_city, "const bool bUSRealizedCity = IsUSMajorCityMission(Mission);", "SpawnCampaignCity")
need(spawn_city, "if (bUSRealizedCity)", "SpawnCampaignCity")

# 3. Systemic skyline consumes the params (whole-city architecture).
need(spawn_city, "SkylineParams.DowntownHeightScale", "systemic skyline")
need(spawn_city, "SkylineParams.SprawlFalloff", "systemic skyline")
need(spawn_city, "SkylineParams.FacadePalette", "systemic skyline")
need(spawn_city, "MissionFloorColor", "mission floor regional tint")

# 4. Day/night Tick consumes per-city sun members.
tick = cpp.split("void ACodeRescueGameMode::Tick(")[1].split("\n}\n")[0] \
    if "void ACodeRescueGameMode::Tick(" in cpp else ""
for member in ("CityDaySunColor", "CityNightSunColor", "CityDaySunIntensity", "CityNightSunIntensity"):
    need(tick, member, "Tick day/night")
    need(hdr, member, "GameMode.h members")

# 5. Urban street layer: tones, widths, pattern families.
urban = cpp.split("void ACodeRescueGameMode::SpawnMajorCityUrbanIdentityLayer(")[1].split("\n}\n")[0] \
    if "void ACodeRescueGameMode::SpawnMajorCityUrbanIdentityLayer(" in cpp else ""
for token in ("RoadParams.AsphaltTone", "RoadParams.RoadWidthScale",
              "RoadParams.SidewalkWidthScale", "bBrickHistoricWalks",
              "DiagonalAvenues", "NumberedGrid", "WideArterial", "HillGrid",
              "IrregularHistoric"):
    need(urban, token, "urban street layer")

# 6. PPV grade families.
ppv_spawn = cpp.split("void ACodeRescueGameMode::SpawnPerZonePostProcessVolume(")[1].split("\n}\n")[0] \
    if "void ACodeRescueGameMode::SpawnPerZonePostProcessVolume(" in cpp else ""
ppv_configure = cpp.split("void ACodeRescueGameMode::ConfigurePerZonePostProcessVolume(")[1].split("\n}\n")[0] \
    if "void ACodeRescueGameMode::ConfigurePerZonePostProcessVolume(" in cpp else ""
ppv = ppv_spawn + "\n" + ppv_configure
need(ppv, "ActiveCityRealizationGradeToken", "post-process grade")
need(ppv_spawn, "ConfigurePerZonePostProcessVolume(PPV, CityIndex, GradeToken", "post-process grade")
for grade in ("CoolOvercast", "WarmDesert", "CrispMountain", "HumidGulf",
              "TropicalBright", "GoldenBasin"):
    need(ppv, grade, "post-process grade families")

# 7. Civilian wardrobe hook.
civ = cpp_with_spawning.split("AActor* ACodeRescueGameMode::SpawnDecorativeCivilian(")[1].split("\n}\n")[0] \
    if "AActor* ACodeRescueGameMode::SpawnDecorativeCivilian(" in cpp_with_spawning else ""
need(civ, "ActiveCityWardrobePalette", "civilian wardrobe")
need(civ, "ActiveCityWardrobeAccessory", "civilian wardrobe")
for acc in ("Beanie", "SunHat", "CowboyHat", "Scarf", "ParkaHood", "Backpack",
            "Lanyard", "CityWardrobeJacketFront"):
    need(civ, acc, "civilian wardrobe accessories")

# 8. Named-city sharpening for high-signal metros.
params_body = cpp.split("FCodeRescueUSCityRealizationParams BuildUSCityRealizationParams(")[1] \
    .split("\nACodeRescueGameMode::ACodeRescueGameMode()")[0] \
    if "FCodeRescueUSCityRealizationParams BuildUSCityRealizationParams(" in cpp else ""
for city in ("New York", "Chicago", "Boston", "Washington", "San Francisco",
             "Seattle", "Los Angeles", "New Orleans", "Houston", "Dallas",
             "Salt Lake City", "Las Vegas", "Phoenix", "Detroit",
             "Urban Honolulu", "Anchorage", "San Diego", "Philadelphia"):
    need(params_body, f'TEXT("{city}")', "named-city sharpening")
for archetype in ("BrownstoneRow", "TripleDecker", "VictorianPainted",
                  "CraftsmanBungalow", "AdobeRanch", "BrickTwoFlat",
                  "ShotgunPorch", "SunbeltRanch", "DecoPastelHome",
                  "MountainCabin"):
    need(cpp, archetype, "home archetypes")
for vehicle in ("Taxi", "Pickup", "SUV", "Sedan", "Compact", "EV", "Van",
                "Bus", "Convertible", "PlowTruck"):
    need(cpp, f'TEXT("{vehicle}")', "vehicle fleet tokens")

# 9. Runtime log markers for smoke verification.
need(cpp, "[CodeRescueUSCityRealization]", "runtime log marker")

# 10. Doc-43 identity contracts must remain intact.
need(cpp, "SpawnUSCitySpecificIdentityLayer(Mission, CityIndex, Origin, CityLabel);", "doc-43 identity layer call")
need(cpp, "[CodeRescueUSCityIdentity]", "doc-43 identity log")

# 11. QA audit registration.
audit = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
need(audit, "verify_june12_city_realization_pass.py", "Run_Full_QA_Audit.command")

name = "verify_june12_city_realization_pass"
if errors:
    for e in errors:
        print(f"[{name}] FAIL: {e}")
    sys.exit(1)
print(f"[{name}] PASS: whole-city U.S. realization contract intact "
      "(landscape, architecture, sky, roads, sidewalks, homes, vehicles, clothing)")
