"""Scaffold the missing maps that earlier passes' GameModes need.

Run inside the UE editor's Python console (Window → Developer → Python
Console). Creates:
  - /Game/Maps/MainMenu  — GameModeOverride = AMainMenuGameMode (item 38)
  - /Game/Maps/Sandbox   — GameModeOverride = ASandboxGameMode  (item 33)

Both maps start as the engine's default empty template; the GameMode
override is what makes them functional.

Usage:
    >>> import importlib, scaffold_maps
    >>> importlib.reload(scaffold_maps)
    >>> scaffold_maps.run()

After running:
  1. Open Project Settings → Maps & Modes.
  2. Set "Game Default Map" to /Game/Maps/MainMenu.
  3. Save All.
  4. Verify the splash launches by hitting PIE.
"""

from __future__ import annotations

import unreal


def make_level(package_path: str, gamemode_class_path: str) -> None:
    if unreal.EditorAssetLibrary.does_asset_exist(package_path):
        unreal.log(f"[scaffold_maps] {package_path} already exists, skipping.")
        return

    # UE 5.7 path: EditorLevelLibrary moved/renamed for some operations,
    # but new_level still works.
    ok = unreal.EditorLevelLibrary.new_level(package_path)
    if not ok:
        unreal.log_error(f"[scaffold_maps] Could not create level {package_path}.")
        return

    # Resolve the WorldSettings of the just-created world and set the GM.
    world = unreal.EditorLevelLibrary.get_editor_world()
    ws = world.get_world_settings() if world else None
    if not ws:
        unreal.log_warning(f"[scaffold_maps] Created level {package_path} but no WorldSettings.")
        return

    gm_class = unreal.load_class(None, gamemode_class_path)
    if gm_class is None:
        unreal.log_warning(
            f"[scaffold_maps] GameMode class {gamemode_class_path} not loadable. "
            "Did the C++ module compile?"
        )
        return
    ws.set_editor_property("default_game_mode", gm_class)
    unreal.EditorLevelLibrary.save_current_level()
    unreal.log(f"[scaffold_maps] Created {package_path} with GameMode {gamemode_class_path}.")


def run() -> None:
    make_level(
        "/Game/Maps/MainMenu",
        "/Script/CodeRescueUnreal.MainMenuGameMode",
    )
    make_level(
        "/Game/Maps/Sandbox",
        "/Script/CodeRescueUnreal.SandboxGameMode",
    )

    # Configure the project's default map.
    settings = unreal.get_default_object(unreal.GameMapsSettings.static_class())
    settings.set_editor_property(
        "game_default_map",
        unreal.SoftObjectPath("/Game/Maps/MainMenu.MainMenu"),
    )
    settings.set_editor_property(
        "editor_startup_map",
        unreal.SoftObjectPath("/Game/Maps/MainMenu.MainMenu"),
    )
    unreal.log("[scaffold_maps] Set GameDefaultMap = /Game/Maps/MainMenu")
    unreal.log("[scaffold_maps] Done. Save Project Settings to persist.")


if __name__ == "__main__":
    unreal.log_error("Run from the UE editor's Python console, not standalone.")
