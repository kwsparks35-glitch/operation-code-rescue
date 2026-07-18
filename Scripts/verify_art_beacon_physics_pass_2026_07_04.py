#!/usr/bin/env python3
"""verify_art_beacon_physics_pass_2026_07_04.py

Gate for the 2026-07-04 pass: v2 Blender art (characters w/ morphs, weapons, vehicles,
nature, streets, sky), beacon-marker word-competition fix, world solidity (collision +
ground snap + audit command), character wiring (player body, FP weapons, facial
expressions), and the night-sky layer.
"""
from __future__ import annotations
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "Source" / "CodeRescueUnreal"
RAW = ROOT / "RawArt"
FAILURES: list[str] = []


def check(cond: bool, msg: str) -> None:
    tag = "PASS" if cond else "FAIL"
    print(f"[verify_art_beacon_physics_pass_2026_07_04] {tag}: {msg}")
    if not cond:
        FAILURES.append(msg)


def has(path: Path, *needles: str) -> bool:
    try:
        text = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return False
    return all(n in text for n in needles)


# --- 1. RawArt assets on disk -------------------------------------------------
for name in ("SurvivorKenny", "SurvivorMaya", "ZombieShamblerV2", "ZombieBruteV2"):
    p = RAW / "Characters" / f"{name}.fbx"
    check(p.exists() and p.stat().st_size > 100_000, f"character FBX exported: {name}")

GLBS = {
    "Weapons": ["SM_Rifle_Scout", "SM_Pistol_Compact", "SM_Shotgun_Breacher", "SM_Machete_Field", "SM_Wrench_Heavy"],
    "Vehicles": ["SM_Sedan_Wreck", "SM_Van_Delivery", "SM_Police_Cruiser"],
    "Nature": ["SM_Tree_Oak_8m", "SM_Tree_Dead_6m", "SM_Bush_Round"],
    "CityKit": ["SM_Road_Straight_12m", "SM_Crosswalk_8m", "SM_Sidewalk_6m", "SM_StreetSign_Stop", "SM_TrafficLight"],
    "Sky": ["SM_SkyDome_Stars", "SM_Moon"],
}
for sub, names in GLBS.items():
    for n in names:
        p = RAW / sub / f"{n}.glb"
        check(p.exists() and p.stat().st_size > 1_000, f"world GLB exported: {sub}/{n}")

check(has(ROOT / "Scripts" / "BlenderArt" / "build_characters_v2.py",
          "shape_key_add", "KEY_MOUTH", "def rig_and_animate", "validate_parts"),
      "characters_v2 pipeline carries shape keys, rig, and the placement validator")
check(has(ROOT / "Scripts" / "BlenderArt" / "build_world_art_v2.py",
          "def skydome", "def vehicle", "def tree_oak", "export_glb"),
      "world-art pipeline builds sky/vehicles/nature and exports GLB")

# --- 2. Beacon word-competition fix -------------------------------------------
beacon_h = SRC / "CodeRescueBeaconMarkerActor.h"
beacon_c = SRC / "CodeRescueBeaconMarkerActor.cpp"
check(has(beacon_h, "class CODERESCUEUNREAL_API ACodeRescueBeaconMarkerActor : public ACodeRescueMessageMarkerActor",
          "ConfigureBeacon"),
      "beacon actor subclasses the message marker (keeps read-on-demand contract)")
check(has(beacon_c, "BeaconBeam", "BeaconGlyph", "ApplyTintedMaterial"),
      "beacon actor builds a beam + glyph with emissive tint")
gm = SRC / "CodeRescueGameMode.cpp"
check(has(gm, "const bool bMultiWord = FlatText.Contains(TEXT(\" \"))",
          "ACodeRescueBeaconMarkerActor* Marker"),
      "SpawnGuideText routes every non-essential multi-word label to a beacon")
check(has(gm, "IsEssentialGuideText(UpperText) || !bMultiWord"),
      "single words and control prompts stay as world text")

# --- 3. World physics ----------------------------------------------------------
sp = SRC / "CodeRescueGameModeSpawning.cpp"
check(has(sp, "EnsureComplexAsSimpleCollision", "CTF_UseComplexAsSimple"),
      "imported kit meshes get complex-as-simple blocking collision")
check(has(sp, "cr.AuditWorldSolidity", "floaters"),
      "cr.AuditWorldSolidity console command exists (reports + fixes floaters)")
check(has(gm, "float ACodeRescueGameMode::GroundZAt", "LineTraceSingleByChannel"),
      "GroundZAt trace helper implemented")
check(has(gm, "Kit Lamp\"), true", "Kit Kiosk\"), true", "Kit Rubble\"), true"),
      "city-kit props now spawn with collision enabled")

# --- 4. Streetscape + night sky -------------------------------------------------
check(has(gm, "void ACodeRescueGameMode::SpawnStreetscapeLayer", "SM_Road_Straight_12m",
          "SM_Sedan_Wreck", "SM_Tree_Oak_8m", "[Streetscape]"),
      "streetscape layer spawns roads, vehicles, and trees with logging")
check(has(gm, "void ACodeRescueGameMode::SpawnNightSkyLayer", "SM_SkyDome_Stars", "SM_Moon"),
      "night-sky layer spawns the star dome and moon")
check(has(gm, "void ACodeRescueGameMode::UpdateNightSkyVisibility", "SetActorHiddenInGame"),
      "night sky follows the player and toggles by time of day")

# --- 5. Character wiring ---------------------------------------------------------
ch = SRC / "CodeRescueCharacter.cpp"
check(has(ch, "/Game/CodeRescueArt/CharactersV2/SurvivorKenny.SurvivorKenny", "bUsingV2Body = true"),
      "player body prefers the authored SurvivorKenny with mannequin fallback")
check(has(ch, "void ACodeRescueCharacter::RefreshFirstPersonWeapon", "SM_Rifle_Scout", "SM_Pistol_Compact"),
      "first-person weapon model follows the active weapon")
check(has(ch, "void ACodeRescueCharacter::UpdateV2BodyLocomotion", "PlayAnimation"),
      "v2 body switches idle/walk/run by ground speed")
check(has(ch, "FacialExpression->SetExpression(FName(TEXT(\"Grimace\"))"),
      "damage triggers a facial grimace")
fx_h = SRC / "CodeRescueFacialExpressionComponent.h"
fx_c = SRC / "CodeRescueFacialExpressionComponent.cpp"
check(has(fx_h, "SetExpression", "TriggerOnActor") and has(fx_c, "SetMorphTarget", "Blink"),
      "facial expression component drives morph targets with auto-blink")
check(has(SRC / "SurvivorActor.cpp", "SurvivorMaya.SurvivorMaya"),
      "rescue survivors prefer the authored SurvivorMaya")
check(has(SRC / "FriendlyNPCActor.cpp", "CharactersV2/SurvivorMaya", "CharactersV2/SurvivorKenny"),
      "friendly NPCs prefer authored v2 humans by role")
check(has(SRC / "CodeTerminalWidget.cpp", "FName(TEXT(\"Smile\"))"),
      "terminal solve success triggers a smile")
check(has(gm, "FName(TEXT(\"Alarm\"))"),
      "horde trigger raises an alarmed expression")

# --- 6. Import job queued OR already consumed successfully -----------------------
job = ROOT / "Saved" / "ClaudeBridge" / "inbox" / "0200_import_art_pass_v2.json"
done = ROOT / "Saved" / "ClaudeBridge" / "outbox" / "0200_import_art_pass_v2.json"
imported_dir = ROOT / "Content" / "CodeRescueArt" / "CharactersV2"
check((job.exists() and has(job, "CharactersV2", "import_morph_targets"))
      or (done.exists() and has(done, "imported_count"))
      or imported_dir.exists(),
      "bridge import job queued, or consumed with assets landed (2026-07-04: consumed OK)")

print()
if FAILURES:
    print(f"[verify_art_beacon_physics_pass_2026_07_04] {len(FAILURES)} FAILURE(S)")
    sys.exit(1)
print("[verify_art_beacon_physics_pass_2026_07_04] ALL CHECKS PASSED")
