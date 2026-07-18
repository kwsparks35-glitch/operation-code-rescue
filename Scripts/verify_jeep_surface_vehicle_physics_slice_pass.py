#!/usr/bin/env python3
"""Static verifier for the surface-aware jeep vehicle physics slice."""

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


jeep_h = read(SRC / "JeepActor.h")
jeep_cpp = read(SRC / "JeepActor.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "JEEP_SURFACE_VEHICLE_PHYSICS_SLICE.md")

spawn_jeep_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnJeepForCity")

check_all(
    jeep_h + jeep_cpp,
    [
        "SurfaceProbeInterval",
        "SurfaceProbeDistance",
        "HighTractionLateralDamping",
        "LowTractionLateralDamping",
        "CurrentGroundSurface",
        "CurrentTraction",
        "CurrentSpeedScale",
        "CurrentTurnScale",
        "UpdateGroundSurface",
        "ResolveGroundSurface",
        "ApplySurfaceTuning",
        "GetSurfaceTraction",
        "GetSurfaceSpeedScale",
        "GetSurfaceTurnScale",
        "GetSurfaceCueColor",
        "SurfaceCueLight",
        "CodeRescueJeepGroundSurfaceProbe",
        "UGameplayStatics::GetSurfaceType",
        "SurfaceConcrete",
        "SurfaceMetal",
        "SurfaceWood",
        "SurfaceGlass",
        "SurfaceFlesh",
        "SurfaceDirt",
        "JeepSurfaceTractionActive",
        "ChaosVehicleReadyFallback",
        "SurfaceAwareVehicle",
        "VehiclePhysicsFallback",
    ],
    "jeep must provide surface-aware fallback vehicle physics and Chaos readiness tags",
)
check_all(
    jeep_cpp,
    [
        "Movement->MaxSpeed = MaxJeepSpeed * CurrentSpeedScale",
        "Movement->Acceleration = BaseAcceleration",
        "Movement->Deceleration = BaseDeceleration",
        "Movement->Velocity -= Right * LateralSpeed * DampingAlpha",
        "AddMovementInput(GetActorForwardVector(), Forward * CurrentTraction)",
        "TurnRateDegPerSec * CurrentTurnScale",
        "surface-aware traction active",
    ],
    "jeep movement must tune speed, acceleration, braking, drift, and turn rate by surface",
)
check_all(
    spawn_jeep_body,
    [
        "VehicleSurfaceTractionTraining",
        "SurfaceAwareVehicle",
        "ChaosVehicleReadyFallback",
        "Jeep Surface Traction Pad",
        "SurfaceConcrete",
        "surface-aware traction active",
    ],
    "spawned jeep must include authored traction-training context",
)
check("ChaosVehiclesPlugin" in slice_doc,
      "slice doc must call out the full Chaos Vehicles plugin path")
check("GAME_PHYSICS_DEEPDIVE" in slice_doc,
      "slice doc must map this work to the physics deep dive")
check("Jeep surface-aware vehicle physics slice" in progress,
      "progress log must document the jeep vehicle physics slice")
check("verify_jeep_surface_vehicle_physics_slice_pass.py" in full_qa,
      "full QA must run the jeep vehicle verifier")
check("verify_jeep_surface_vehicle_physics_slice_pass.py" in local_ci,
      "local CI must run the jeep vehicle verifier")

if errors:
    for error in errors:
        print(f"[verify_jeep_surface_vehicle_physics_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_jeep_surface_vehicle_physics_slice_pass] PASS: jeep surface-aware vehicle physics slice verified")
