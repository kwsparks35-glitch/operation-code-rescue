#!/usr/bin/env python3
"""
Static verifier for the June 12 U.S. city landscape and architecture pass.

This locks the user-facing contract:
- the first 342 campaign rows are recognized as U.S. major-city missions,
- every U.S. mission receives the city-specific identity layer,
- the layer includes landscape, architecture, sky, roads, sidewalks, homes,
  vehicles, and local clothing cues,
- signature silhouettes make high-signal cities readable beyond text labels,
- district micro-scenes add neighborhood, transit, waterfront, civic,
  industrial, entertainment, campus, mountain, desert, suburban, and clothing
  accessory cues,
- named high-signal cities keep explicit overrides,
- the new set dressing stays non-blocking so access points remain open.
"""

from __future__ import annotations

import re
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"


def read(path: Path) -> str:
    if not path.exists():
        raise RuntimeError(f"missing {path}")
    return path.read_text(encoding="utf-8")


def require(path: Path, tokens: list[str]) -> None:
    content = read(path)
    missing = [token for token in tokens if token not in content]
    if missing:
        raise RuntimeError(f"{path} missing tokens: {', '.join(missing)}")


def extract_function_body(content: str, signature: str) -> str:
    start = content.find(signature)
    if start < 0:
        raise RuntimeError(f"missing function signature: {signature}")
    brace = content.find("{", start)
    if brace < 0:
        raise RuntimeError(f"missing function body: {signature}")
    depth = 0
    for index in range(brace, len(content)):
        char = content[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return content[brace + 1:index]
    raise RuntimeError(f"unterminated function body: {signature}")


def verify_us_city_table(campaign_cpp: Path) -> None:
    content = read(campaign_cpp)
    rows = re.findall(
        r"\{\s*(\d+),\s*TEXT\(\"([^\"]+)\"\),\s*TEXT\(\"([^\"]+)\"\)\s*\}",
        content,
    )
    if len(rows) < 342:
        raise RuntimeError(f"expected at least 342 campaign rows, found {len(rows)}")

    us_rows = [(int(rank), city, state) for rank, city, state in rows if int(rank) <= 342]
    if len(us_rows) != 342:
        raise RuntimeError(f"expected 342 U.S. campaign rows, found {len(us_rows)}")

    bad = [(rank, city, state) for rank, city, state in us_rows if len(state) != 2]
    if bad:
        sample = ", ".join(f"{rank}:{city}:{state}" for rank, city, state in bad[:8])
        raise RuntimeError(f"U.S. rows should use two-letter state codes: {sample}")

    first_global = next(((int(rank), city, state) for rank, city, state in rows if int(rank) == 343), None)
    if first_global != (343, "Tokyo", "JP"):
        raise RuntimeError(f"expected row 343 to remain first global row Tokyo/JP, found {first_global}")


def main() -> int:
    mode_h = SRC / "CodeRescueGameMode.h"
    mode_cpp = SRC / "CodeRescueGameMode.cpp"
    campaign_cpp = SRC / "CodeRescueCampaign.cpp"
    audit = PROJECT_ROOT / "Run_Full_QA_Audit.command"

    verify_us_city_table(campaign_cpp)

    require(mode_h, [
        "SpawnUSCitySpecificIdentityLayer",
    ])
    require(mode_cpp, [
        "struct FCodeRescueUSCityVisualProfile",
        "BuildUSCityVisualProfile",
        "IsUSMajorCityMission",
        "Mission.Rank <= FCodeRescueCampaign::USCityMissionCount",
        "SpawnUSCitySpecificIdentityLayer",
        "SpawnUSCitySpecificIdentityLayer(Mission, CityIndex, Origin, CityLabel);",
        "[CodeRescueUSCityIdentity]",
    ])

    require(mode_cpp, [
        "LandscapeCue",
        "ArchitectureCue",
        "SkyCue",
        "RoadCue",
        "SidewalkCue",
        "HomeCue",
        "VehicleCue",
        "ClothingCue",
        "SignatureCue",
        "SignatureShapeToken",
        "DistrictCue",
        "US City Specific Terrain Plate",
        "US City Specific Sky Mood Band",
        "US City Specific Road Pattern",
        "US City Specific North Sidewalk",
        "US City Specific Architecture Facade",
        "US City Specific Home Row",
        "Vehicle Cue",
        "US City Specific Local Clothing Civilian",
        "SIGNATURE SILHOUETTE",
        "US City Signature",
        "signature='%s'",
        "districts='%s'",
    ])

    require(mode_cpp, [
        "USCitySpecificIdentity",
        "CitySpecificLandscape",
        "CitySpecificArchitecture",
        "CitySpecificSky",
        "CitySpecificRoads",
        "CitySpecificSidewalks",
        "CitySpecificHomes",
        "CitySpecificVehicles",
        "CitySpecificClothing",
        "CitySpecificDistricts",
        "NoAccessBlocker",
    ])

    require(mode_cpp, [
        "City == TEXT(\"New York\")",
        "City == TEXT(\"Los Angeles\")",
        "City == TEXT(\"Chicago\")",
        "City == TEXT(\"Houston\")",
        "City == TEXT(\"Philadelphia\")",
        "City == TEXT(\"San Antonio\")",
        "City == TEXT(\"San Diego\")",
        "City == TEXT(\"Dallas\")",
        "City == TEXT(\"San Jose\")",
        "City == TEXT(\"Austin\")",
        "City == TEXT(\"San Francisco\")",
        "City == TEXT(\"Seattle\")",
        "City == TEXT(\"Denver\")",
        "City == TEXT(\"Washington\")",
        "City == TEXT(\"Las Vegas\")",
        "City == TEXT(\"Boston\")",
        "City == TEXT(\"Detroit\")",
        "City == TEXT(\"Nashville-Davidson\")",
        "City == TEXT(\"Miami\")",
        "City == TEXT(\"New Orleans\")",
        "City == TEXT(\"Urban Honolulu\")",
        "City == TEXT(\"Anchorage\")",
        "City == TEXT(\"Salt Lake City\")",
    ])

    require(mode_cpp, [
        "HarborStatue",
        "HillsideLetters",
        "SuspensionBridge",
        "ObservationNeedle",
        "CivicObelisk",
        "NeonMarquee",
        "RiverBridge",
        "BayouEnergy",
        "MountainPeakTower",
        "TropicalDeco",
        "DesertSun",
        "TechCampus",
        "HistoricBell",
        "MusicNote",
        "IndustrialMotor",
        "MissionArch",
        "HarborNaval",
        "VolcanicSurf",
        "SnowInlet",
        "MountainGridSpire",
        "CampusQuad",
        "StockyardGate",
        "HarborBeacon",
        "BalconyStreetcar",
        "FreewayCrown",
        "EvergreenWaterTower",
    ])

    require(mode_cpp, [
        "DISTRICT CUES",
        "US City District Waterfront Boardwalk",
        "US City District Transit Shelter Roof",
        "US City District Historic Rowfront",
        "US City District Civic Bollard",
        "US City District Warehouse Dock",
        "US City District Freight Container",
        "US City District Venue Marquee",
        "US City District Campus Lab Hall",
        "US City District Trailhead Kiosk",
        "US City District Desert Shade Canopy",
        "US City District Townhome Front",
        "US City District Clothing Accessory",
        "DISTRICTS\\n%s",
    ])

    spawn_body = extract_function_body(
        read(mode_cpp),
        "void ACodeRescueGameMode::SpawnUSCitySpecificIdentityLayer",
    )
    blocking_spawn_patterns = [
        "CityLabel + TEXT(\" US City Specific",
        "CityLabel + TEXT(\" US City Freeway",
        "CityLabel + TEXT(\" US City Transit",
        "CityLabel + TEXT(\" US City Palm",
        "CityLabel + TEXT(\" US City Desert",
        "CityLabel + TEXT(\" US City Mountain",
        "CityLabel + TEXT(\" US City Industrial",
        "CityLabel + TEXT(\" US City Entertainment",
        "CityLabel + TEXT(\" US City Signature",
        "CityLabel + TEXT(\" US City District",
    ]
    for marker in blocking_spawn_patterns:
        nearby = [line for line in spawn_body.splitlines() if marker in line]
        if not nearby:
            raise RuntimeError(f"missing expected spawned marker: {marker}")
    if re.search(r"CityLabel \+ TEXT\(\" US City[^\n]+,\s*true\)\)?;", spawn_body):
        raise RuntimeError("U.S. city identity layer should not add blocking set dressing")

    mode_content = read(mode_cpp)
    call = "SpawnUSCitySpecificIdentityLayer(Mission, CityIndex, Origin, CityLabel);"
    major = "SpawnMajorCityUrbanIdentityLayer(Mission, CityIndex, Origin, CityLabel);"
    world = "SpawnWorldMajorCitySignatureLayer(Mission, CityIndex, Origin, CityLabel);"
    if not (mode_content.index(major) < mode_content.index(call) < mode_content.index(world)):
        raise RuntimeError("U.S. city identity layer should run after the street-grid layer and before the world-atlas layer")

    require(audit, [
        "verify_june12_us_city_identity_pass.py",
    ])

    print("[verify-june12-us-city-identity-pass] PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
