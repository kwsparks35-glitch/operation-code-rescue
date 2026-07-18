#!/usr/bin/env python3
"""Static verifier for the authored physics-lane combat encounter."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DOC_DIR = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def check(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def check_all(source: str, tokens: list[str], message: str) -> None:
    missing = [token for token in tokens if token not in source]
    if missing:
        errors.append(f"{message}: missing {', '.join(missing)}")


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        errors.append(f"missing function {signature}")
        return ""
    brace = source.find("{", start)
    if brace < 0:
        errors.append(f"missing body for {signature}")
        return ""
    depth = 0
    for idx in range(brace, len(source)):
        char = source[idx]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                _cr_body = source[brace : idx + 1]  # 2026-07-04 BuildWidgetTreeNow migration
                if "::NativeConstruct" in signature and "BuildWidgetTreeNow();" in _cr_body:
                    return function_body(source, signature.replace("::NativeConstruct", "::BuildWidgetTreeNow"))
                return _cr_body
    errors.append(f"unterminated function {signature}")
    return ""


gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "PHYSICS_LANE_COMBAT_ENCOUNTER_SLICE.md")

yard_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnPhysicsTraversalYard")

check_all(
    yard_body,
    [
        "TagPhysicsEncounter",
        "PhysicsLaneCombatEncounter",
        "AuthoredCombatEncounter",
        "UsesThrowablePhysicsLane",
        "PHYSICS AMBUSH DRILL",
        "use X slot throwables, impact props, and cover to clear the lane",
    ],
    "physics yard must expose an authored combat encounter pocket",
)
check_all(
    yard_body,
    [
        "PhysicsLaneCombatProp",
        "ThrowableImpactCoverProp",
        "SurfaceImpactCombatTraining",
        "SurfaceMetal",
        "SurfaceWood",
        "SurfaceConcrete",
        "EnableTrainingPhysics",
        "SetSimulatePhysics(true)",
    ],
    "encounter must include throwable-reactive physics props and readable cover",
)
check_all(
    yard_body,
    [
        "SpawnEncounterPickup",
        "PhysicsLaneCombatReward",
        "PhysicsAmbushSmokeCache",
        "PhysicsAmbushFlareCache",
        "PhysicsAmbushAmmoCache",
        "EPickupKind::Smoke",
        "EPickupKind::Flare",
        "EPickupKind::Ammo",
    ],
    "encounter must seed utility rewards that teach the intended loop",
)
check_all(
    yard_body,
    [
        "if (!bSandboxMode)",
        "NeutralizedZombieIds.Contains(ZombieId)",
        "CodeRescueHordeZombieIdBase + CityIndex * 1000 + 650 + i",
        "PhysicsLaneCombatZombie",
        "ZombieDeathPhysicsReadabilityTarget",
        "ApplyZombieFamilyVariant",
        "PhysicsLaneZombieFamily",
        "VisualMarkerActor",
    ],
    "encounter zombies must be sandbox-gated, save-aware, variant-aware, and visibly marked",
)
check("verify_physics_lane_combat_encounter_slice_pass.py" in full_qa,
      "full QA must run the physics-lane encounter verifier")
check("verify_physics_lane_combat_encounter_slice_pass.py" in local_ci,
      "local CI must run the physics-lane encounter verifier")
check("Authored physics-lane combat encounter" in progress,
      "progress log must document the physics-lane encounter slice")
check("GAME_PHYSICS_DEEPDIVE" in slice_doc and "WORLD_DEVELOPMENT_DEEPDIVE" in slice_doc,
      "slice doc must map the encounter to the physics and world deep dives")
check("CHARACTER_ANIMATION_DEEPDIVE" in slice_doc,
      "slice doc must mention zombie readability/animation-death integration")

if errors:
    for error in errors:
        print(f"[verify_physics_lane_combat_encounter_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_physics_lane_combat_encounter_slice_pass] PASS: authored physics-lane combat encounter verified")
