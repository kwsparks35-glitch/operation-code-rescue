#!/usr/bin/env python3
"""Static and artifact gate for the July 9 production presentation pass."""

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "Source" / "CodeRescueUnreal"
FAILURES = []


def check(condition, label):
    print("[verify_production_presentation] {}: {}".format(
        "PASS" if condition else "FAIL", label))
    if not condition:
        FAILURES.append(label)


def text(path):
    return path.read_text(encoding="utf-8", errors="replace")


character = text(SRC / "CodeRescueCharacter.cpp")
character_h = text(SRC / "CodeRescueCharacter.h")
game_mode = text(SRC / "CodeRescueGameMode.cpp")
game_mode_h = text(SRC / "CodeRescueGameMode.h")
survivor = text(SRC / "SurvivorActor.cpp")
npc = text(SRC / "FriendlyNPCActor.cpp")
poll = character[character.index("void ACodeRescueCharacter::PollDirectKeys"):]

# Launch gate
check("FInputModeGameAndUI" in game_mode and "bShowMouseCursor = true" in game_mode,
      "launch menu supports pointer and keyboard selection")
check(poll.index("const bool bLanguageGateActive") < poll.index("const bool bCameraPollAllowed"),
      "language gate runs before camera and weapon actions")
check("!LaunchMenu || !LaunchMenu->IsInViewport()" not in poll,
      "visible programmatic widget cannot trigger the false auto-deploy")
check("LaunchLanguageMenu = nullptr" in game_mode_h and
      "bHasSelectedLaunchLanguageThisSession = true" not in poll[:poll.index("// Hard input gate")],
      "start selector is strongly owned and never silently commits")
check("DoesLanguageSaveExist(Chosen)" in poll and "ResumeLanguageRun(Chosen)" in poll,
      "selected-language resume remains available from the start screen")

# Curated production world and camera
check("CodeRescueDevelopmentShowcase" in game_mode and
      "ShouldSpawnDevelopmentShowcaseLayers" in game_mode_h,
      "prototype review layers are explicit opt-in")
check("[ProductionWorld]" in game_mode and "development_showcases=%d" in game_mode,
      "curated production profile emits an auditable runtime contract")
check("ApplyProductionPresentationCleanup" in game_mode and
      "ProductionPresentationHidden" in game_mode,
      "bounds-based arrival and world-label cleanup is wired")
check("Production Glass Coding Concourse Canopy" in game_mode and  # 2026-07-11 refresh: pavilion renamed
      "bDevelopmentShowcase ? Spec.SpawnOffset : ProductionOffset" in game_mode,
      "primitive safehouse and briefing-wall presentation are replaced in production")
check("UpdateCameraOcclusion" in character_h and
      "FMath::LineBoxIntersection" in character and
      "UpdateCameraOcclusion(DeltaSeconds)" in character,
      "third-person camera independently handles visible no-collision walls")
check("RoadIntersectionV3" in game_mode and "Cross Street Sidewalk" in game_mode,
      "connected cross street and pedestrian routes are spawned")
check("FMath::Abs(i) <= 1" in game_mode,
      "central building row leaves the intersection and camera sight line open")

# Character and asset presentation
check("CodeRescueUsePrototypeCharacters" in character and
      "authored Manny six-state locomotion rig" in character,  # 2026-07-11 refresh: presentation log wording
      "player defaults to the complete production locomotion rig")
check("CodeRescueUsePrototypeCharacters" in survivor and
      "production Quinn" in survivor,
      "survivor defaults to the complete production rig")
check("CodeRescueUsePrototypeCharacters" in npc and
      "production mannequin" in npc,
      "friendly NPC roles default to complete animation rigs")

world_builder = text(ROOT / "Scripts" / "BlenderArt" / "build_world_art_v3.py")
weapon_builder = text(ROOT / "Scripts" / "BlenderArt" / "build_weapons_v3.py")
check("CR_ProductionEdgeRadius" in world_builder and "tapered_box" in world_builder,
      "city/vehicle pipeline includes production bevels and sloped bodywork")
check("door seam" in world_builder and "tactile warning strip" in world_builder,
      "vehicle and sidewalk scale details are authored")
check("CR_ProductionEdgeRadius" in weapon_builder and "compact optic" in weapon_builder,
      "weapon pipeline includes production edge and hardware detail")

city_glbs = list((ROOT / "RawArt" / "CityKitV3").glob("*.glb"))
weapon_glbs = list((ROOT / "RawArt" / "WeaponsV3").glob("*.glb"))
check(len(city_glbs) >= 19 and all(path.stat().st_size > 1000 for path in city_glbs),
      "19 production city GLBs were regenerated")
check(len(weapon_glbs) >= 5 and all(path.stat().st_size > 1000 for path in weapon_glbs),
      "five production weapon GLBs were regenerated")
check(len(list((ROOT / "Content" / "CodeRescueArt" / "CityKitV3").rglob("*V3.uasset"))) >= 19,
      "production city meshes are imported into Unreal Content")
check(len(list((ROOT / "Content" / "CodeRescueArt" / "WeaponsV3").rglob("*V3.uasset"))) >= 5,
      "production weapon meshes are imported into Unreal Content")

render_dir = ROOT / "Documentation" / "improvement_pass_2026-07-09" / "Renders"
for name in ("production_city_assets.png", "production_weapon_assets.png"):
    path = render_dir / name
    check(path.exists() and path.stat().st_size > 10000,
          "review render exists: " + name)

if FAILURES:
    print("\n[verify_production_presentation] {} FAILURE(S)".format(len(FAILURES)))
    sys.exit(1)
print("\n[verify_production_presentation] ALL CHECKS PASSED")
