#!/usr/bin/env python3
"""Static city-layer profile for spawn-heavy GameMode systems."""

from __future__ import annotations

import json
import re
import csv
from datetime import datetime, timezone
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC_FILES = [
    PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.cpp",
    PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueGameModeSpawning.cpp",
]
OUT_DIR = PROJECT_ROOT / "Saved/Profiling"
BUDGET_MANIFEST = PROJECT_ROOT / "Content/CodeRescueData/performance_city_layer_budget.tsv"


LAYER_PATTERNS = {
    "CoreRoute": ("SpawnCampaignCity", "EnsureEntryAccessCorridorClear", "EntryAccess"),
    "ArenaConfinement": ("SpawnGameplayArenaConfinementLayer", "ArenaConfinement", "FallRecovery"),
    "USCityIdentity": ("SpawnUSCitySpecificIdentityLayer", "CitySpecific", "Signature"),
    "CombatActors": ("SpawnZombie", "ACodeZombieActor", "Boss", "Elite"),
    "RescueSquad": ("SpawnRescueSupportSquad", "ACompanionActor", "RescueTeam"),
    "CurriculumSafehouse": ("SpawnProtectedCodingChallengeHub", "CodingChallenge", "Terminal"),
    "HUDAndUI": ("HUD", "Widget", "Subtitles"),
    "AudioAndNarration": ("SpeakRadioBriefing", "RadioBriefing", "Sound"),
    "PhysicsAndProps": ("Pickup", "Barricade", "Physics", "Prop"),
    "StreamingPackage": ("EnsureCampaignCityLoaded", "Unload", "Stream"),
}


def load_budget_rows() -> dict[str, dict[str, str]]:
    if not BUDGET_MANIFEST.exists():
        return {}
    with BUDGET_MANIFEST.open(encoding="utf-8", newline="") as fh:
        return {
            row.get("layer", ""): row
            for row in csv.DictReader(fh, delimiter="\t")
            if row.get("layer")
        }


def main() -> int:
    source = "\n".join(path.read_text(encoding="utf-8", errors="replace") for path in SRC_FILES if path.exists())
    budgets = load_budget_rows()
    profile = {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "source_files": [str(path.relative_to(PROJECT_ROOT)) for path in SRC_FILES],
        "budget_manifest": str(BUDGET_MANIFEST.relative_to(PROJECT_ROOT)),
        "profile_kind": "static_source_proxy",
        "note": "Static hit counts are not frame timing. They identify spawn-heavy ownership areas that need runtime profiling or human review.",
        "layers": {},
    }
    for layer, patterns in LAYER_PATTERNS.items():
        matches = {}
        for pattern in patterns:
            matches[pattern] = len(re.findall(re.escape(pattern), source))
        profile["layers"][layer] = {
            "pattern_hits": matches,
            "total_hits": sum(matches.values()),
            "budget_contract": budgets.get(layer, {}),
            "has_budget_row": layer in budgets,
        }
    profile["spawn_call_counts"] = {
        "SpawnActor": len(re.findall(r"\bSpawnActor\s*<", source)),
        "SpawnBlock": len(re.findall(r"\bSpawnBlock\s*\(", source)),
        "SpawnTexturedBlock": len(re.findall(r"\bSpawnTexturedBlock\s*\(", source)),
        "SpawnGuideText": len(re.findall(r"\bSpawnGuideText\s*\(", source)),
        "SpawnStaticMeshProp": len(re.findall(r"\bSpawnStaticMeshProp\s*\(", source)),
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    latest = OUT_DIR / "city_layer_static_profile_latest.json"
    stamped = OUT_DIR / f"city_layer_static_profile_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
    text = json.dumps(profile, indent=2, sort_keys=True)
    latest.write_text(text + "\n", encoding="utf-8")
    stamped.write_text(text + "\n", encoding="utf-8")
    print(f"[profile_city_layers_static] wrote {latest}")
    for layer, data in profile["layers"].items():
        print(f"[profile_city_layers_static] {layer}: {data['total_hits']} static hits")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
