#!/usr/bin/env python3
"""Static verifier for the rescue extraction presentation slice."""

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


presentation_h = read(SRC / "RescueExtractionPresentationActor.h")
presentation_cpp = read(SRC / "RescueExtractionPresentationActor.cpp")
survivor_cpp = read(SRC / "SurvivorActor.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "RESCUE_EXTRACTION_PRESENTATION_SLICE.md")

constructor_body = function_body(presentation_cpp, "ARescueExtractionPresentationActor::ARescueExtractionPresentationActor")
begin_body = function_body(presentation_cpp, "void ARescueExtractionPresentationActor::BeginPlay")
configure_body = function_body(presentation_cpp, "void ARescueExtractionPresentationActor::ConfigurePresentation")
tick_body = function_body(presentation_cpp, "void ARescueExtractionPresentationActor::Tick")
rescue_body = function_body(survivor_cpp, "bool ASurvivorActor::Rescue")

check_all(
    presentation_h,
    [
        "ARescueExtractionPresentationActor",
        "PresentedSurvivorName",
        "PresentedCityIndex",
        "AccentColor",
        "DurationSeconds",
        "bReducedMotion",
        "OptionalSequencerBeatAsset",
        "ConfigurePresentation",
        "OnRescuePresentationStarted",
    ],
    "presentation actor header must expose survivor, city, accessibility, and Sequencer hook fields",
)
check_all(
    constructor_body,
    [
        "PrimaryActorTick.bCanEverTick = true",
        "ExtractionLandingDisc",
        "ExtractionRescueBeam",
        "ExtractionSweepArmA",
        "ExtractionSweepArmB",
        "ExtractionLiftMarker",
        "ExtractionOrbitBeaconA",
        "ExtractionOrbitBeaconB",
        "ExtractionOrbitBeaconC",
        "ExtractionKeyLight",
        "ExtractionFillLight",
        "/Engine/BasicShapes/Cylinder.Cylinder",
        "/Engine/BasicShapes/Cube.Cube",
        "/Engine/BasicShapes/Sphere.Sphere",
        "RescueExtractionPresentation",
        "SequencerReadyFallback",
        "ControlRigReadyFallback",
        "CharacterAnimationDeepDive",
        "WorldDevelopmentDeepDive",
        "Top50Recommendations",
        "ReleaseDossier",
    ],
    "presentation actor must create a tagged runtime extraction rig",
)
check_all(
    begin_body,
    [
        "ApplyVisualTints",
        "OnRescuePresentationStarted",
        "[CodeRescueRescuePresentation]",
        "reduced motion",
    ],
    "presentation actor must start with tinting, Blueprint hook, and audit log",
)
check_all(
    configure_body,
    [
        "PresentedSurvivorName = InSurvivorName",
        "PresentedCityIndex = InCityIndex",
        "AccentColor = InAccentColor",
        "bReducedMotion = bInReducedMotion",
        "ApplyVisualTints",
    ],
    "presentation actor must support runtime survivor/city/color/accessibility configuration",
)
check_all(
    tick_body,
    [
        "ElapsedSeconds += DeltaSeconds",
        "FMath::Sin",
        "bReducedMotion ? 0.22f : 1.0f",
        "SetRelativeRotation",
        "SetRelativeLocation",
        "SetRelativeScale3D",
        "SetIntensity",
        "Destroy()",
    ],
    "presentation actor tick must animate, respect reduced motion, light the beat, and self-clean",
)
check("RescueExtractionPresentationActor.h" in survivor_cpp,
      "survivor rescue flow must include the presentation actor")
check("CodeRescueCampaign.h" in survivor_cpp,
      "survivor rescue flow must resolve city accent colors")
check_all(
    rescue_body,
    [
        "FCodeRescueCampaign::GetMission(CityIndex)",
        "SpawnActor<ARescueExtractionPresentationActor>",
        "ConfigurePresentation(SurvivorName, CityIndex, PresentationAccent, GI && GI->bReducedMotion)",
        "ESpawnActorCollisionHandlingMethod::AlwaysSpawn",
        "SetActorHiddenInGame(true)",
        "GI->SavePersistentRun()",
    ],
    "survivor rescue must spawn the extraction beat without removing persistence/hide behavior",
)
check("verify_rescue_extraction_presentation_slice_pass.py" in full_qa,
      "full QA must run the rescue extraction presentation verifier")
check("verify_rescue_extraction_presentation_slice_pass.py" in local_ci,
      "local CI must run the rescue extraction presentation verifier")
check("Rescue extraction presentation slice" in progress,
      "progress log must document the rescue extraction presentation slice")
check_all(
    slice_doc,
    [
        "CHARACTER_ANIMATION_DEEPDIVE",
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "Sequencer",
        "Control Rig",
        "reduced motion",
        "survivor rescue",
        "save",
    ],
    "slice doc must map rescue extraction presentation to the June 25 guidance",
)

if errors:
    for error in errors:
        print(f"[verify_rescue_extraction_presentation_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_rescue_extraction_presentation_slice_pass] PASS: rescue extraction presentation slice verified")
