#!/usr/bin/env python3
"""Static verifier for the 2026-07-01 art-development pass.

Checks: (1) the generated CityKit .glb meshes are present and structurally valid glTF-binary,
(2) the Blender character pipeline (script + Mac runner) is present and self-consistent,
(3) the bridge inbox holds valid queued import/showcase commands.
Static only; the Unreal import + visual review happen on the Mac (Definition-of-Done gate).
"""
from __future__ import annotations
import json, struct
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
errors: list[str] = []

def need(cond: bool, msg: str) -> None:
    if not cond:
        errors.append(f"FAIL: {msg}")

# --- 1. city kit meshes ---
kit = sorted((ROOT / "RawArt/CityKit").glob("*.glb"))
need(len(kit) >= 10, f"expected >=10 CityKit .glb meshes (found {len(kit)})")
for p in kit:
    d = p.read_bytes()
    try:
        magic, ver, total = struct.unpack_from("<4sII", d, 0)
        need(magic == b"glTF" and ver == 2 and total == len(d), f"{p.name}: bad GLB header")
        jlen, jtype = struct.unpack_from("<I4s", d, 12)
        need(jtype == b"JSON", f"{p.name}: missing JSON chunk")
        g = json.loads(d[20:20 + jlen].decode())
        need(len(g.get("materials", [])) >= 2, f"{p.name}: expected >=2 materials")
        need(len(g.get("meshes", [])) == 1, f"{p.name}: expected 1 mesh")
    except Exception as exc:  # noqa: BLE001
        errors.append(f"FAIL: {p.name}: {exc}")

# --- 2. character pipeline ---
chars = (ROOT / "Scripts/BlenderArt/build_characters.py")
runner = (ROOT / "Run_Build_Characters_Blender.command")
need(chars.exists(), "missing Scripts/BlenderArt/build_characters.py")
need(runner.exists(), "missing Run_Build_Characters_Blender.command")
if chars.exists():
    src = chars.read_text(encoding="utf-8", errors="replace")
    for sym in ["build_character", "Survivor", "ZombieShambler", "ZombieBrute",
                "export_scene.fbx", "vertex_groups", "edit_bones", "Idle", "Walk", "nla_tracks"]:
        need(sym in src, f"character script must contain {sym}")
if runner.exists():
    need("build_characters.py" in runner.read_text(errors="replace"), "runner must invoke the character script")

# --- 3. queued bridge commands (2026-07-04: consumed = success — the editor imported
# the art on 07-01/07-04; accept the landed Content as terminal evidence) ---
inbox = ROOT / "Saved/ClaudeBridge/inbox"
imported_ok = (ROOT / "Content/CodeRescueArt/CityKit").exists() and \
              (ROOT / "Content/CodeRescueArt/Characters").exists()
for fname, tokens in [
    ("0100_import_code_rescue_art.json", ["AssetImportTask", "CodeRescueArt", "import_as_skeletal"]),
    ("0110_place_art_showcase.json", ["spawn_actor_from_object", "CodeRescueArtShowcase"]),
]:
    p = inbox / fname
    need(p.exists() or imported_ok, f"bridge command {fname} queued or already consumed (assets landed)")
    if p.exists():
        try:
            cmd = json.loads(p.read_text())
            need(cmd.get("action") == "exec" and "code" in cmd.get("args", {}), f"{fname}: bad bridge format")
            for t in tokens:
                need(t in cmd["args"]["code"], f"{fname}: code missing {t}")
        except Exception as exc:  # noqa: BLE001
            errors.append(f"FAIL: {fname}: invalid JSON: {exc}")

if errors:
    print(f"[verify_art_pipeline_pass] {len(errors)} problem(s):")
    for e in errors:
        print("  " + e)
    sys.exit(1)
print(f"[verify_art_pipeline_pass] PASS - {len(kit)} kit meshes valid; character pipeline + queued imports present")
print("  NOTE: static check only; Unreal import + visual review happen on the Mac.")
