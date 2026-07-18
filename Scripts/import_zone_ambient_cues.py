"""Wire imported zone ambient WAVs into BP_CodeRescueGameMode::ZoneAmbientCues.

Run inside the UE editor's Python console. Expects three SoundWaves under
`/Game/CodeRescueAssets/Audio/ZoneAmbient/`:
- Anchorage_Wind   (or any name containing "anchorage" or "wind")
- Seattle_Rain     (or any name containing "seattle" or "rain")
- Tokyo_Urban      (or any name containing "tokyo" or "urban" or "neon")

If you don't have authored cues, you can use any free CC0 ambient WAV from
freesound.org or zapsplat.com. Steps:
  1. Download 3 WAVs (one per zone aesthetic).
  2. Drag into Content/CodeRescueAssets/Audio/ZoneAmbient/ in the editor.
  3. Run this script.

Or, even faster:
  1. Use the Sound Generator skill (UE 5.7 → Audio → Generate White Noise),
     then add a Filter EQ to shape it like wind/rain/urban.
  2. Save out as ZoneAmbient_*.uasset.

Usage (inside UE editor):
    >>> import importlib, import_zone_ambient_cues
    >>> importlib.reload(import_zone_ambient_cues)
    >>> import_zone_ambient_cues.run()
"""

from __future__ import annotations

import unreal


GAMEMODE_BP_PATH = "/Game/CodeRescueAssets/BP_CodeRescueGameMode"
AMBIENT_PATH = "/Game/CodeRescueAssets/Audio/ZoneAmbient"

ZONE_TAGS = [
    ("anchorage", "wind", "snow"),     # zone 0
    ("seattle", "rain", "overcast"),   # zone 1
    ("tokyo", "urban", "neon"),        # zone 2
]


def find_soundwave_for_zone(zone_idx: int) -> unreal.SoundWave | None:
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    assets = asset_registry.get_assets_by_path(AMBIENT_PATH, recursive=True)
    tags = ZONE_TAGS[zone_idx]
    for ad in assets:
        if str(ad.asset_class_path.asset_name) != "SoundWave":
            continue
        name = str(ad.asset_name).lower()
        if any(t in name for t in tags):
            return unreal.load_asset(str(ad.package_name))
    return None


def run() -> None:
    bp = unreal.load_asset(GAMEMODE_BP_PATH)
    if bp is None:
        unreal.log_error(f"[import_zone_ambient_cues] {GAMEMODE_BP_PATH} not found.")
        return

    cdo = unreal.get_default_object(bp.generated_class())
    cues: list = []
    for zone_idx in range(3):
        sw = find_soundwave_for_zone(zone_idx)
        if sw:
            unreal.log(f"[import_zone_ambient_cues] Zone {zone_idx} → {sw.get_name()}")
        else:
            unreal.log_warning(
                f"[import_zone_ambient_cues] Zone {zone_idx} unassigned (no match for {ZONE_TAGS[zone_idx]})"
            )
        cues.append(sw)

    cdo.set_editor_property("ZoneAmbientCues", cues)
    unreal.EditorAssetLibrary.save_loaded_asset(bp)
    unreal.log("[import_zone_ambient_cues] Done.")


if __name__ == "__main__":
    unreal.log_error("Run from the UE editor's Python console, not standalone.")
