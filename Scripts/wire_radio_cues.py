"""Auto-wire radio briefing SoundWaves into BP_CodeRescueGameMode.

Run inside the Unreal Editor's Python console (Window → Developer → Python
Console), NOT from a system shell. Scans
`/Game/CodeRescueAssets/Audio/RadioSamples/` for SoundWave assets, matches
each to the city's slug from `radio_briefings.tsv`, and assigns to the
`CityRadioBriefingCues` array on the default `BP_CodeRescueGameMode`.

Why a separate script: matching 342 imports by slug substring is a 30-second
job that's easy to get wrong by hand.

Usage (inside UE editor):
    >>> import importlib, wire_radio_cues
    >>> importlib.reload(wire_radio_cues)
    >>> wire_radio_cues.run()

The script writes a summary at the end:
    [wire_radio_cues] 287 / 342 cities matched (86.2%)
    [wire_radio_cues] missing slugs: ['x_unmatched_city', ...]

Missing slugs almost always mean the WAV wasn't imported yet. Re-run after
importing the missing files and the assignment is idempotent.
"""

from __future__ import annotations

import csv
import os
import unreal


# ---- Configuration ---------------------------------------------------------
RADIO_SAMPLES_DIR = "/Game/CodeRescueAssets/Audio/RadioSamples"
GAMEMODE_BP_PATH = "/Game/CodeRescueAssets/Blueprints/BP_CodeRescueGameMode"
TSV_PATH = (
    os.path.join(
        unreal.SystemLibrary.get_project_directory(),
        "Content",
        "CodeRescueData",
        "radio_briefings.tsv",
    )
)


def load_briefing_rows() -> list[dict]:
    """Read radio_briefings.tsv. Returns rows ordered by city rank (1..342)."""
    with open(TSV_PATH, newline="", encoding="utf-8") as fh:
        reader = csv.DictReader(fh, delimiter="\t")
        return list(reader)


def find_soundwave_for_slug(slug: str) -> unreal.SoundWave | None:
    """Look for a SoundWave whose asset name contains the slug."""
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    assets = asset_registry.get_assets_by_path(RADIO_SAMPLES_DIR, recursive=True)
    target = slug.lower()
    for ad in assets:
        name = str(ad.asset_name).lower()
        if target in name and ad.asset_class_path.asset_name == "SoundWave":
            return unreal.load_asset(str(ad.package_name))
    return None


def run() -> None:
    """Main entry point."""
    rows = load_briefing_rows()
    rows.sort(key=lambda r: int(r["rank"]))
    n_cities = len(rows)

    bp = unreal.load_asset(GAMEMODE_BP_PATH)
    if bp is None:
        unreal.log_error(
            f"[wire_radio_cues] Could not load {GAMEMODE_BP_PATH}. "
            "Create the BP first via right-click → Blueprint Class → "
            "ACodeRescueGameMode."
        )
        return

    cdo = unreal.get_default_object(bp.generated_class())
    cue_array: list = []
    matched = 0
    missing: list[str] = []

    for row in rows:
        slug = row["slug"]
        sw = find_soundwave_for_slug(slug)
        if sw is None:
            missing.append(slug)
            cue_array.append(None)
            continue
        cue_array.append(sw)
        matched += 1

    cdo.set_editor_property("CityRadioBriefingCues", cue_array)
    unreal.EditorAssetLibrary.save_loaded_asset(bp)

    pct = (matched / n_cities) * 100.0 if n_cities else 0.0
    unreal.log(f"[wire_radio_cues] {matched} / {n_cities} cities matched ({pct:.1f}%)")
    if missing:
        sample = missing[:10]
        unreal.log(
            f"[wire_radio_cues] missing slugs (showing up to 10 of {len(missing)}): {sample}"
        )


if __name__ == "__main__":
    run()
