#!/usr/bin/env python3
"""verify_menu_movement_v3art_2026_07_06.py

Gate for the 2026-07-06 pass driven by Kenny's playtest report:
  1. Language menu: fits + scrolls at any resolution, buttons first, decor
     never eats clicks, deploy restores game input.
  2. Movement lock: gate failsafe, stuck-movement watchdog, static UI-open
     reset, tutorial/recovery double-fire guard (from part 5).
  3. v3 art: world kit + weapons authored with PBR materials, imported at
     the Interchange double-nested paths, wired into streetscape/city-block/
     mood/weapon code.
"""
from __future__ import annotations
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "Source" / "CodeRescueUnreal"
FAILURES: list[str] = []


def check(cond: bool, msg: str) -> None:
    print(f"[verify_menu_movement_v3art] {'PASS' if cond else 'FAIL'}: {msg}")
    if not cond:
        FAILURES.append(msg)


def has(path: Path, *needles: str) -> bool:
    try:
        t = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return False
    return all(n in t for n in needles)


menu = SRC / "CodeRescueMainMenuWidget.cpp"
menu_h = SRC / "CodeRescueMainMenuWidget.h"
ch = SRC / "CodeRescueCharacter.cpp"
ch_h = SRC / "CodeRescueCharacter.h"
gm = SRC / "CodeRescueGameMode.cpp"
gm_h = SRC / "CodeRescueGameMode.h"

# 1 — language menu
check(has(menu, "FAnchors(0.5f, 0.28f, 0.5f, 0.97f)"),
      "menu 1a: launch panel stretches with the window (no fixed 620px box)")
check(has(menu, "MenuColumnScroll", "MenuScroll->AddChild(Box)"),
      "menu 1b: whole menu column scrolls as final safety")
check(has(menu, "Buttons FIRST", "LaunchKeyboardHint"),
      "menu 1c: track buttons come before the preview text + keyboard hint shown")
check(has(menu, "LowFog->SetVisibility(ESlateVisibility::HitTestInvisible)")
      and has(menu, "FirstSessionRoutePreviewText->SetVisibility(ESlateVisibility::HitTestInvisible)"),
      "menu 1d: decorative layers are hit-test invisible (clicks reach buttons)")
check(has(menu, "RestoreGameInputBeforeTravel", "SetInputMode(FInputModeGameOnly())"),
      "menu 1e: every deploy path restores game input before travel")
check(menu.read_text(encoding="utf-8", errors="replace").count("RestoreGameInputBeforeTravel();") >= 5,
      "menu 1f: restore called from start/resume/new/continue/sandbox paths")
check(has(menu_h, "void RestoreGameInputBeforeTravel();"),
      "menu 1g: helper declared")

# 2 — movement lock defense-in-depth
check(has(ch, "[LaunchGate] RECOVERY REQUIRED", "LanguageGateNoMenuSeconds > 3.0f")
      and has(gm_h, "LaunchLanguageMenu = nullptr"),
      "move 2a: launch widget is strongly owned and a missing selector never auto-commits")
check(has(ch, "UpdateStuckMovementWatchdog", "[MoveWatchdog]"),
      "move 2b: stuck-movement watchdog self-heals a frozen pawn")
check(has(ch_h, "float StuckMovementSeconds", "float LanguageGateNoMenuSeconds"),
      "move 2c: watchdog state members exist")
check(has(ch, "A newly spawned player\n    // pawn is authoritative", "SetUIOpen(false);\n    ApplyRuntimeTuning();"),
      "move 2d: static bUIOpen reset in BeginPlay")
check(has(ch, "Recovery must not fire while the", "UCodeRescueTutorialWidget::IsShowing()"),
      "move 2e: Backspace tutorial-dismiss no longer teleports (part-5 guard)")
check(has(ch, "hand input back to the game", "OpenLevel(GetWorld(), FName(TEXT(\"Entry\")))"),
      "move 2f: pawn-poll deploy path restores input too")

# 3 — v3 art wired
check((ROOT / "Scripts" / "BlenderArt" / "build_world_art_v3.py").exists()
      and (ROOT / "Scripts" / "BlenderArt" / "build_weapons_v3.py").exists(),
      "art 3a: v3 builder scripts exist (reproducible pipeline)")
glbs = list((ROOT / "RawArt" / "CityKitV3").glob("*.glb")) if (ROOT / "RawArt" / "CityKitV3").exists() else []
wglbs = list((ROOT / "RawArt" / "WeaponsV3").glob("*.glb")) if (ROOT / "RawArt" / "WeaponsV3").exists() else []
check(len(glbs) >= 19, f"art 3b: city kit GLBs exported ({len(glbs)}/19)")
check(len(wglbs) >= 5, f"art 3c: weapon GLBs exported ({len(wglbs)}/5)")
city_content = ROOT / "Content" / "CodeRescueArt" / "CityKitV3"
weap_content = ROOT / "Content" / "CodeRescueArt" / "WeaponsV3"
check(city_content.exists() and len(list(city_content.rglob("*.uasset"))) >= 19,
      "art 3d: city kit imported to Content (uassets present)")
check(weap_content.exists() and len(list(weap_content.rglob("*.uasset"))) >= 5,
      "art 3e: weapons imported to Content (uassets present)")
check(has(gm, "SpawnCityBlockV3Layer", "BuildingBrickV3", "StreetlightV3", "[CityBlockV3]"),
      "art 3f: city-block layer spawns buildings + street furniture")
check(has(gm, "SpawnCityMoodLayer", "SetFogDensity", "FilmGrainIntensity", "[CityMood]"),
      "art 3g: mood layer spawns fog + filmic post")
check(has(gm, "/%s/%s/StaticMeshes/%s.%s\"), Sub, *N, *N, *N, *N"),
      "art 3h: Interchange double-nested paths used for v3 kit")
check(has(gm_h, "SpawnCityBlockV3Layer", "SpawnCityMoodLayer"),
      "art 3i: layer functions declared")
check(has(ch, "ResolveWeaponPreviewMesh", "WeaponsV3/PistolV3/PistolV3", "WeaponsV3/RifleV3/RifleV3")
      and has(ch, "SM_Pistol_Compact", "SM_Rifle_Scout"),
      "art 3j: v3 weapon meshes wired with v2 fallback")

print()
if FAILURES:
    print(f"[verify_menu_movement_v3art] {len(FAILURES)} FAILURE(S)")
    sys.exit(1)
print("[verify_menu_movement_v3art] ALL CHECKS PASSED")
