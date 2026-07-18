"""Import the sinister Maple narration WAVs and wire them into the GameMode.

Run inside the Unreal Editor's Python console (Window → Developer → Python
Console), after Generate_Maple_Sinister_Narrations.command has produced WAVs
in Content/CodeRescueAssets/Audio/RadioSamples/:

    >>> exec(open(r"<project>/Scripts/import_and_wire_maple_narrations.py").read())

What it does:
  1. Bulk-imports any missing `*_radio_briefing.wav` files in the RadioSamples
     folder as SoundWave assets. Existing assets are skipped by default so
     commandlet reruns can wire cues without re-triggering audio import. Set
     `CODE_RESCUE_FORCE_MAPLE_IMPORT=1` to replace existing SoundWaves.
  2. Re-runs the existing Scripts/wire_radio_cues.py assignment, which maps
     SoundWaves to ACodeRescueGameMode::CityRadioBriefingCues by city slug.
     Missions without a WAV (the male-voiced cities) keep their current
     behavior — the runtime `say` fallback.

Idempotent: safe to re-run after generating more WAVs.
"""

from __future__ import annotations

import os
import sys

import unreal

PROJECT_DIR = unreal.SystemLibrary.get_project_directory()
SCRIPTS_DIR = os.path.join(PROJECT_DIR, "Scripts")
WAV_DIR = os.path.join(PROJECT_DIR, "Content", "CodeRescueAssets", "Audio", "RadioSamples")
DEST_PATH = "/Game/CodeRescueAssets/Audio/RadioSamples"


def import_wavs() -> int:
    force_import = os.environ.get("CODE_RESCUE_FORCE_MAPLE_IMPORT", "").lower() in {
        "1",
        "true",
        "yes",
    }
    wavs = sorted(
        os.path.join(WAV_DIR, f)
        for f in os.listdir(WAV_DIR)
        if f.lower().endswith("_radio_briefing.wav")
    )
    if not wavs:
        unreal.log_error(
            "[maple_narrations] no *_radio_briefing.wav files found — run "
            "Generate_Maple_Sinister_Narrations.command first."
        )
        return 0

    tasks = []
    for wav in wavs:
        asset_name = os.path.splitext(os.path.basename(wav))[0]
        asset_path = f"{DEST_PATH}/{asset_name}"
        if not force_import and unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            continue
        task = unreal.AssetImportTask()
        task.filename = wav
        task.destination_path = DEST_PATH
        task.automated = True
        task.replace_existing = True
        task.save = True
        tasks.append(task)

    if not tasks:
        unreal.log(f"[maple_narrations] all {len(wavs)} WAVs already have SoundWave assets")
        return len(wavs)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
    unreal.log(
        f"[maple_narrations] imported/updated {len(tasks)} of {len(wavs)} SoundWave assets"
    )
    return len(wavs)


def run() -> None:
    imported = import_wavs()
    if imported == 0:
        return
    if SCRIPTS_DIR not in sys.path:
        sys.path.insert(0, SCRIPTS_DIR)
    import importlib

    import wire_radio_cues

    importlib.reload(wire_radio_cues)
    wire_radio_cues.run()
    unreal.log(
        "[maple_narrations] done — imported female-voiced cities now play the "
        "sinister Maple cues; any missing cues keep the existing fallback, and "
        "male-voiced cities are unchanged."
    )


run()
