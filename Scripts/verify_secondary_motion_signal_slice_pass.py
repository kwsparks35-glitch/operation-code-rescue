#!/usr/bin/env python3
"""Static verifier for the procedural secondary-motion signal slice."""

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


signal_h = read(SRC / "SecondaryMotionSignalActor.h")
signal_cpp = read(SRC / "SecondaryMotionSignalActor.cpp")
gamemode_h = read(SRC / "CodeRescueGameMode.h")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "SECONDARY_MOTION_SIGNAL_SLICE.md")

constructor_body = function_body(signal_cpp, "ASecondaryMotionSignalActor::ASecondaryMotionSignalActor")
tick_body = function_body(signal_cpp, "void ASecondaryMotionSignalActor::Tick")
configure_body = function_body(signal_cpp, "void ASecondaryMotionSignalActor::ConfigureSignal")
layer_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnSecondaryMotionSignalLayer")

check_all(
    signal_h,
    [
        "ASecondaryMotionSignalActor",
        "WindAmplitudeDegrees",
        "WindSpeed",
        "FlutterPhase",
        "SignalTint",
        "ConfigureSignal",
    ],
    "secondary-motion actor header must expose tuning and configuration",
)
check_all(
    constructor_body,
    [
        "PrimaryActorTick.bCanEverTick = true",
        "SignalMast",
        "SignalCrossbar",
        "SignalBannerA",
        "SignalBannerB",
        "SignalCable",
        "/Engine/BasicShapes/Cube.Cube",
        "SetMobility(EComponentMobility::Movable)",
        "SecondaryMotionSignal",
        "ProceduralClothFallback",
        "ChaosClothReadyFallback",
        "CharacterAnimationDeepDive",
        "WorldDevelopmentDeepDive",
    ],
    "secondary-motion actor must create a tagged procedural cloth-ready signal rig",
)
check_all(
    tick_body,
    [
        "MotionTime += DeltaSeconds",
        "FMath::Sin",
        "WindSpeed",
        "WindAmplitudeDegrees",
        "SetRelativeRotation",
        "BannerA",
        "BannerB",
        "Cable",
    ],
    "secondary-motion actor tick must animate banner/cable motion",
)
check_all(
    configure_body,
    [
        "SignalTint = InTint",
        "FlutterPhase = InPhase",
        "FMath::Clamp",
        "ApplyTint",
    ],
    "secondary-motion actor must support runtime tint and wind tuning",
)
check("SecondaryMotionSignalActor.h" in gamemode_cpp,
      "game mode must include the secondary-motion actor")
check("SpawnSecondaryMotionSignalLayer" in gamemode_h,
      "game mode header must declare the secondary-motion layer")
check("SpawnSecondaryMotionSignalLayer(Mission, CityIndex, Origin, CityLabel, Survivor)" in gamemode_cpp,
      "campaign city spawn must call the secondary-motion layer")
check_all(
    layer_body,
    [
        "ASecondaryMotionSignalActor",
        "ConfigureSignal",
        "SecondaryMotionSignalLayer",
        "ChaosClothReadyFallback",
        "ProceduralClothFallback",
        "CharacterAnimationDeepDive",
        "WorldDevelopmentDeepDive",
        "SecondaryMotionSafehouseSignal",
        "SecondaryMotionHelipadSignal",
        "SecondaryMotionRouteSignal",
        "SecondaryMotionSurvivorCampSignal",
        "RegisterStreamedActor",
        "Survivor->AddHelperActor",
        "[CodeRescueSecondaryMotion]",
    ],
    "secondary-motion layer must place tagged streamed signals at rescue landmarks",
)
check("verify_secondary_motion_signal_slice_pass.py" in full_qa,
      "full QA must run the secondary-motion signal verifier")
check("verify_secondary_motion_signal_slice_pass.py" in local_ci,
      "local CI must run the secondary-motion signal verifier")
check("Procedural secondary-motion signal slice" in progress,
      "progress log must document the procedural secondary-motion signal slice")
check_all(
    slice_doc,
    [
        "CHARACTER_ANIMATION_DEEPDIVE",
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "secondary motion",
        "Chaos Cloth",
        "safehouse",
        "helipad",
        "survivor camp",
        "route",
    ],
    "slice doc must map secondary motion to the June 25 character/world guidance",
)

if errors:
    for error in errors:
        print(f"[verify_secondary_motion_signal_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_secondary_motion_signal_slice_pass] PASS: procedural secondary-motion signal slice verified")
