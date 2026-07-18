#!/usr/bin/env python3
"""Static verifier for the rescue-loop helipad extraction-ready slice."""

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


helipad_h = read(SRC / "HelipadActor.h")
helipad_cpp = read(SRC / "HelipadActor.cpp")
survivor_cpp = read(SRC / "SurvivorActor.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "HELIPAD_EXTRACTION_READY_SLICE.md")

constructor_body = function_body(helipad_cpp, "AHelipadActor::AHelipadActor")
tick_body = function_body(helipad_cpp, "void AHelipadActor::Tick")
ready_body = function_body(helipad_cpp, "void AHelipadActor::SetExtractionReady")
visual_body = function_body(helipad_cpp, "void AHelipadActor::ApplyExtractionVisualState")
rescue_body = function_body(survivor_cpp, "bool ASurvivorActor::Rescue")
spawn_helipad_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnHelipadForCity")

check_all(
    helipad_h,
    [
        "virtual void Tick",
        "bExtractionReady",
        "ExtractionSurvivorName",
        "ExtractionAccentColor",
        "bReducedMotion",
        "SetExtractionReady",
        "ExtractionColumn",
        "ExtractionSweepA",
        "ExtractionSweepB",
        "ExtractionBeacon",
    ],
    "helipad header must expose extraction-ready state and visual components",
)
check_all(
    constructor_body,
    [
        "PrimaryActorTick.bCanEverTick = true",
        "ExtractionReadyColumn",
        "ExtractionReadySweepA",
        "ExtractionReadySweepB",
        "ExtractionReadyBeacon",
        "/Engine/BasicShapes/Cylinder.Cylinder",
        "/Engine/BasicShapes/Cube.Cube",
        "/Engine/BasicShapes/Sphere.Sphere",
        "ConfigureExtractionComponent",
        "WorldDevelopmentDeepDive",
    ],
    "helipad constructor must build a dormant extraction-ready visual rig",
)
check_all(
    tick_body,
    [
        "if (!bExtractionReady)",
        "ExtractionPulseTime += DeltaSeconds",
        "bReducedMotion ? 0.22f : 1.0f",
        "SetRelativeScale3D",
        "SetRelativeRotation",
        "SetRelativeLocation",
        "SetIntensity",
    ],
    "helipad tick must animate extraction-ready visuals with reduced-motion damping",
)
check_all(
    ready_body,
    [
        "bExtractionReady = true",
        "ExtractionSurvivorName = SurvivorName",
        "ExtractionAccentColor = AccentColor",
        "bReducedMotion = bInReducedMotion",
        "ExtractionReadyHelipad",
        "RescueLoopClosure",
        "Top50Recommendations",
        "ReleaseDossier",
        "ApplyExtractionVisualState",
    ],
    "SetExtractionReady must store state, add audit tags, and apply visuals",
)
check_all(
    visual_body,
    [
        "SetVisibility(bVisible, true)",
        "GlowLight->SetLightColor",
        "ApplyComponentTint",
        "ExtractionColumn",
        "ExtractionSweepA",
        "ExtractionSweepB",
        "ExtractionBeacon",
    ],
    "helipad visual state must show/hide/tint the extraction rig",
)
check("HelipadActor.h" in survivor_cpp,
      "survivor rescue flow must include helipad activation")
check_all(
    rescue_body,
    [
        "UGameplayStatics::GetAllActorsOfClass(World, AHelipadActor::StaticClass(), HelipadActors)",
        "Helipad->CityIndex == CityIndex",
        "Helipad->SetExtractionReady(SurvivorName, PresentationAccent, GI && GI->bReducedMotion)",
        "GI->SavePersistentRun()",
    ],
    "live survivor rescue must activate the current city helipad without removing save behavior",
)
check_all(
    spawn_helipad_body,
    [
        "const FCodeRescueCityMission* Mission = FCodeRescueCampaign::GetMission(CityIndex)",
        "GI->RescuedSurvivorNames.Contains(Mission->SurvivorName)",
        "Helipad->SetExtractionReady(Mission->SurvivorName, ExtractionAccent, GI->bReducedMotion)",
        "RegisterStreamedActor(Helipad)",
    ],
    "helipad spawn must restore extraction-ready state from saved survivor progress",
)
check("verify_helipad_extraction_ready_slice_pass.py" in full_qa,
      "full QA must run the helipad extraction-ready verifier")
check("verify_helipad_extraction_ready_slice_pass.py" in local_ci,
      "local CI must run the helipad extraction-ready verifier")
check("Helipad extraction-ready slice" in progress,
      "progress log must document the helipad extraction-ready slice")
check_all(
    slice_doc,
    [
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "survivor rescue",
        "helipad",
        "extraction-ready",
        "reduced motion",
        "save",
    ],
    "slice doc must map helipad extraction readiness to the June 25 guidance",
)

if errors:
    for error in errors:
        print(f"[verify_helipad_extraction_ready_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_helipad_extraction_ready_slice_pass] PASS: helipad extraction-ready rescue loop slice verified")
