#!/usr/bin/env python3
"""verify_gameplay_fixes_2026_07_07.py

Gate for Kenny's 2026-07-07 four-issue report. Every fix below was verified
LIVE in the editor -game build before this gate was written:

  1. Weapons: polled wheel/bracket cycling (+debounce), third-person weapon on
     the hand_R bone ("[HeldWeapon] body weapon 'PistolV3' attached via
     hand_R"), dual-channel swap feedback.
  2. 15-degree tilt: the CAMERA was rolled (world verticals leaned with the
     body). Roll is now refused at the boom and stripped per frame.
  3. T-stuck: teleport/recovery/spawn destinations depenetrate via
     FindTeleportSpot + ring search ("[Teleport] destination blocked —
     relocated"); one-shot spawn footing check ("[SpawnClear]"); watchdog
     rebuilt (raw keys, 1.25s, escalating rescue).
  4. Camera obstruction: boom probe always on; top-down inside the street canyon;
     proximity body-fade (no more back-of-head screens); CityBlockV3
     absolute-Y bug fixed (buildings spawned ON the street/pad); the rebuilt
     point-star field is permitted while legacy solid domes stay hidden; fog
     is thinned for readability.
"""
from __future__ import annotations
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "Source" / "CodeRescueUnreal"
FAILURES: list[str] = []


def check(cond: bool, msg: str) -> None:
    print(f"[verify_gameplay_fixes_07_07] {'PASS' if cond else 'FAIL'}: {msg}")
    if not cond:
        FAILURES.append(msg)


def has(path: Path, *needles: str) -> bool:
    try:
        t = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return False
    return all(n in t for n in needles)


ch = SRC / "CodeRescueCharacter.cpp"
ch_h = SRC / "CodeRescueCharacter.h"
gm = SRC / "CodeRescueGameMode.cpp"

# 1 — weapons
check(has(ch, "ThirdPersonWeaponMesh = CreateDefaultSubobject"),
      "1a: third-person held-weapon component exists")
check(has(ch, "hand_R", "[HeldWeapon]"),
      "1b: weapon snaps to the rig's right hand with forensic logging")
check(has(ch, "MouseScrollDown) || PC->WasInputKeyJustPressed(EKeys::RightBracket)"),
      "1c: wheel/bracket cycling is POLLED (packaged-reliable)")
check(has(ch, "WeaponCycleCooldown", "double-stepping"),
      "1d: cycle debounce dedupes bound+polled delivery")
check(has(ch, "Equipped: %s  (1-0 slots"),
      "1e: swap announces through the subtitle channel")
check(has(ch, "ThirdPersonWeaponMesh->SetVisibility(!bFirstPerson"),
      "1f: body weapon visible exactly when not in first person")

# 2 — camera roll (the '15 degree' report)
check(has(ch, "bInheritRoll = false"),
      "2a: camera boom refuses roll")
check(has(ch, "strip camera ROLL every frame", "FRotator(ClampedPitch, ControlNow.Yaw, 0.0f)"),
      "2b: residual control-rotation roll stripped per frame")
check(has(ch, "FMath::Clamp(NormPitch, -28.0f, 38.0f)"),
      "2c: boom-camera pitch clamped (no ground-diving cameras)")

# 3 — teleport/spawn placement
check(has(ch, "bool ACodeRescueCharacter::AdjustTeleportDestination", "FindTeleportSpot"),
      "3a: teleport destinations depenetrate via the engine")
check(has(ch, "[Teleport] destination blocked"),
      "3b: relocations are logged for forensics")
check(ch.read_text(encoding="utf-8", errors="replace").count("AdjustTeleportDestination(") >= 6,
      "3c: helper wired into step/city/pad teleports, recovery, spawn, watchdog")
check(has(ch, "EnsureSpawnClearance", "[SpawnClear]"),
      "3d: one-shot spawn footing check")
check(has(ch, "bRawKeysHeld", "StuckRescueEscalation", "1.25f"),
      "3e: watchdog reads raw keys, triggers fast, escalates")

# 4 — camera obstruction
check(has(ch, "The probe is now ALWAYS on", "CameraBoom->bDoCollisionTest = true;"),
      "4a: boom probe always on (fixed cams no longer clip inside walls)")
# 2026-07-16 pin migration: pass 5 pulled top-down in further (1150 -> 820,
# Kenny: the player was an unreadable speck). 820 sits even deeper inside the
# canyon, so the 2026-07-07 intent (never ride above the roofline) holds.
check(has(ch, "TargetArmLength = 820.0f", "street canyon"),
      "4b: top-down stays INSIDE the street canyon (2026-07-07 rev: Kenny — "
      "the camera must align with the player's environment, not roofs)")
check(has(ch, "UpdateCameraProximityFade", "bCameraProximityHidden"),
      "4c: body hides when the boom collapses onto the pawn")
check(has(gm, "Start.Y + (BuildingY + DepthJitter) * SideSign"),
      "4d: city-block layer placements are relative to the street (Y bug fixed)")
check(has(gm, "SafePointStarField", "!bSafePointField || !bShowStars") and
      has(gm, "PointStarFieldV5"),
      "4e: only the rebuilt non-occluding point-star field may become visible")
check(has(gm, "SetFogDensity(0.011f)", "SetStartDistance(1600.0f)"),
      "4f: fog thinned for mid-range readability")

print()
if FAILURES:
    print(f"[verify_gameplay_fixes_07_07] {len(FAILURES)} FAILURE(S)")
    sys.exit(1)
print("[verify_gameplay_fixes_07_07] ALL CHECKS PASSED")
