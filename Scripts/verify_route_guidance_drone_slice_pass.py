#!/usr/bin/env python3
"""Static verifier for the animated rescue-route guidance drone slice."""

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


drone_h = read(SRC / "RescueRouteGuidanceDroneActor.h")
drone_cpp = read(SRC / "RescueRouteGuidanceDroneActor.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "ROUTE_GUIDANCE_DRONE_SLICE.md")

constructor_body = function_body(drone_cpp, "ARescueRouteGuidanceDroneActor::ARescueRouteGuidanceDroneActor")
configure_body = function_body(drone_cpp, "void ARescueRouteGuidanceDroneActor::ConfigureDrone")
tick_body = function_body(drone_cpp, "void ARescueRouteGuidanceDroneActor::Tick")
reveal_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::RevealSolvedTerminalRescueRoute")

check_all(
    drone_h,
    [
        "ARescueRouteGuidanceDroneActor",
        "RouteStartWorld",
        "RouteEndWorld",
        "GuidanceTint",
        "PatrolSpeed",
        "HoverHeight",
        "BobAmplitude",
        "Phase",
        "bReducedMotion",
        "ConfigureDrone",
    ],
    "guidance drone header must expose route and motion tuning",
)
check_all(
    constructor_body,
    [
        "PrimaryActorTick.bCanEverTick = true",
        "RouteGuidanceDroneBody",
        "RouteGuidanceDroneNose",
        "RouteGuidanceDroneRotorArmA",
        "RouteGuidanceDroneRotorArmB",
        "RouteGuidanceDroneSignalPanel",
        "RouteGuidanceDroneLight",
        "/Engine/BasicShapes/Cube.Cube",
        "/Engine/BasicShapes/Sphere.Sphere",
        "RescueRouteGuidanceDrone",
        "AnimatedWayfinding",
        "CodingToRescueWorldResponse",
        "TerminalSolvedRouteVisible",
        "CharacterAnimationDeepDive",
        "WorldDevelopmentDeepDive",
        "Top50Recommendations",
    ],
    "guidance drone must create tagged animated route-wayfinding rig",
)
check_all(
    configure_body,
    [
        "RouteStartWorld = InRouteStartWorld",
        "RouteEndWorld = InRouteEndWorld",
        "GuidanceTint = InTint",
        "Phase = InPhase",
        "bReducedMotion = bInReducedMotion",
        "BobAmplitude = bReducedMotion ? 8.0f : 28.0f",
        "PatrolSpeed = bReducedMotion ? 0.18f : 0.52f",
        "ApplyTint",
    ],
    "guidance drone must support runtime route, tint, phase, and reduced-motion configuration",
)
check_all(
    tick_body,
    [
        "MotionTime += DeltaSeconds",
        "FMath::Lerp(RouteStartWorld, RouteEndWorld, Alpha)",
        "SetActorLocation",
        "SetActorRotation",
        "SetRelativeRotation",
        "SetIntensity",
        "bReducedMotion ?",
    ],
    "guidance drone tick must patrol the route, orient itself, animate rotors, and pulse light",
)
check("RescueRouteGuidanceDroneActor.h" in gamemode_cpp,
      "game mode must include the route guidance drone actor")
check_all(
    reveal_body,
    [
        "SpawnRouteGuidanceDrone",
        "ARescueRouteGuidanceDroneActor",
        "ConfigureDrone",
        "GI && GI->bReducedMotion",
        "SolvedRouteGuidanceDrone",
        "RescueRouteGuidanceDroneLayer",
        "AnimatedWayfinding",
        "RegisterStreamedActor(Drone)",
        "TagSolvedRoute(Drone)",
        "SpawnRouteGuidanceDrone(RoutePoints[i - 1], RoutePoints[i], i)",
    ],
    "solved-route reveal must spawn tagged, streamed guidance drones on every route segment",
)
check("verify_route_guidance_drone_slice_pass.py" in full_qa,
      "full QA must run the route guidance drone verifier")
check("verify_route_guidance_drone_slice_pass.py" in local_ci,
      "local CI must run the route guidance drone verifier")
check("Animated route guidance drone slice" in progress,
      "progress log must document the animated route guidance drone slice")
check_all(
    slice_doc,
    [
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "CHARACTER_ANIMATION_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "coding terminal",
        "solved rescue route",
        "animated wayfinding",
        "reduced motion",
        "save",
    ],
    "slice doc must map route guidance drones to the June 25 guidance",
)

if errors:
    for error in errors:
        print(f"[verify_route_guidance_drone_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_route_guidance_drone_slice_pass] PASS: animated rescue-route guidance drone slice verified")
