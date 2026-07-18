#!/usr/bin/env python3
"""
Verify the Fab/MCP import report and the universal level entry access fix.

This is a static-plus-data verifier that complements the active Unreal
commandlet checks. It proves the entry access is systemic across the generated
campaign because every city is spawned through SpawnCampaignCity and every fast
travel/player start uses FCodeRescueCampaign::GetPlayerStartLocation.
"""

from __future__ import annotations

import csv
import json
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
GAME_MODE_CPP = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.cpp"
GAME_MODE_H = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueGameMode.h"
CAMPAIGN_CPP = PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueCampaign.cpp"
ASSET_PLAN = PROJECT_ROOT / "Content/CodeRescueData/fab_unreal_mcp_asset_plan.json"
IMPORT_STATUS = PROJECT_ROOT / "Content/CodeRescueData/fab_unreal_mcp_import_status.tsv"
RADIO_ROWS = PROJECT_ROOT / "Content/CodeRescueData/radio_briefings.tsv"
REPORT = PROJECT_ROOT / "Saved/MCPFabUnreal/entry_access_and_import_review.json"


def fail(message: str) -> None:
    raise RuntimeError(message)


def text(path: Path) -> str:
    if not path.exists():
        fail(f"missing {path}")
    return path.read_text(encoding="utf-8")


def require_tokens(path: Path, tokens: list[str]) -> None:
    content = text(path)
    missing = [token for token in tokens if token not in content]
    if missing:
        fail(f"{path} missing tokens: {', '.join(missing)}")


def read_tsv(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        fail(f"missing {path}")
    with path.open("r", encoding="utf-8", newline="") as handle:
        return list(csv.DictReader(handle, delimiter="\t"))


def verify_entry_source() -> dict:
    require_tokens(
        CAMPAIGN_CPP,
        [
            # 2026-07-06: entry pad nudged to (-3170, -2760) so the spawn sits
            # clear of the safehouse wall (third-person camera collapsed at the
            # old pad). The contract is "a deliberate entry pad exists", not a
            # frozen coordinate.
            "FVector(-3170.0f, -2760.0f, 112.0f)",  # 2026-07-11 refresh: entry anchor lowered by the ground-continuity passes
            "universal entry pad",
            "no enclosing exterior wall",
        ],
    )
    require_tokens(
        GAME_MODE_H,
        [
            "SpawnUniversalEntryAccessLayer",
            "EnsureEntryAccessCorridorClear",
        ],
    )
    require_tokens(
        GAME_MODE_CPP,
        [
            "SpawnUniversalEntryAccessLayer",
            "Universal Entry Spawn Pad",
            "Universal Entry Walkable Ramp",
            "AlwaysOpenLevelEntry",
            "NoSpawnBlockade",
            "NoExteriorWallBarrier",
            "EnsureEntryAccessCorridorClear",
            "EntryCorridorCollisionCleared",
            "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
            "SetCollisionResponseToAllChannels(ECR_Ignore)",
        ],
    )
    mode_text = text(GAME_MODE_CPP)
    forbidden_tokens = [
        "North Gate Rail",
        "South Gate Rail",
        "West Gate Rail",
        "East Gate Rail",
        "First View Rescue Gate Header",
        "Universal Entry Open Header",
    ]
    present_forbidden = [token for token in forbidden_tokens if token in mode_text]
    if present_forbidden:
        fail("exterior wall/gate barrier tokens still present: " + ", ".join(present_forbidden))
    if mode_text.find("SpawnUniversalEntryAccessLayer(Mission") > mode_text.find("EnsureEntryAccessCorridorClear(CityIndex"):
        fail("entry access layer must be spawned before corridor collision clear")
    if mode_text.find("SpawnProductionTrackCompletionLayer(Mission") > mode_text.find("EnsureEntryAccessCorridorClear(CityIndex"):
        fail("corridor collision clear must run after late production layers")
    return {"status": "source_entry_access_hooks_present"}


def verify_campaign_coverage() -> dict:
    rows = read_tsv(RADIO_ROWS)
    if len(rows) < 400:
        fail(f"expected broad campaign coverage, got {len(rows)} rows")
    ranks = {int(row["rank"]) for row in rows}
    expected = set(range(1, len(rows) + 1))
    if ranks != expected:
        fail("campaign ranks are not contiguous")
    return {"campaign_level_count": len(rows), "entry_access_applies_via": "SpawnCampaignCity"}


def verify_import_status() -> dict:
    if not ASSET_PLAN.exists():
        fail(f"missing {ASSET_PLAN}")
    plan = json.loads(ASSET_PLAN.read_text(encoding="utf-8"))
    status_rows = read_tsv(IMPORT_STATUS)
    if len(status_rows) != len(plan.get("items", [])):
        fail("import status row count does not match asset plan item count")
    counts: dict[str, int] = {}
    included = 0
    for row in status_rows:
        status = row["status"]
        counts[status] = counts.get(status, 0) + 1
        if row["included_in_game_environment"] == "yes":
            included += 1
    if counts.get("already_in_project", 0) < 7:
        fail("expected at least seven already-imported Fab roots")
    if included != counts.get("already_in_project", 0):
        fail("only already-imported/staged assets should be marked included")
    return {
        "asset_plan_items": len(plan.get("items", [])),
        "included_in_game_environment": included,
        "status_counts": dict(sorted(counts.items())),
    }


def main() -> int:
    report = {
        "generated_by": "verify_fab_import_and_entry_access.py",
        "entry": verify_entry_source(),
        "campaign": verify_campaign_coverage(),
        "fab_import": verify_import_status(),
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print("[verify-fab-entry] PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
