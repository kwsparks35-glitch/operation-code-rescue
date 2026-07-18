#!/usr/bin/env python3
"""Static verifier for the state-aware objective focus beacon slice."""

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


actor_h = read(SRC / "ObjectiveFocusBeaconActor.h")
actor_cpp = read(SRC / "ObjectiveFocusBeaconActor.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "OBJECTIVE_FOCUS_BEACON_SLICE.md")

constructor_body = function_body(actor_cpp, "AObjectiveFocusBeaconActor::AObjectiveFocusBeaconActor")
tick_body = function_body(actor_cpp, "void AObjectiveFocusBeaconActor::Tick")
configure_body = function_body(actor_cpp, "void AObjectiveFocusBeaconActor::ConfigureObjectiveBeacon")
resolve_body = function_body(actor_cpp, "int32 AObjectiveFocusBeaconActor::ResolveObjectivePhase")
target_body = function_body(actor_cpp, "FVector AObjectiveFocusBeaconActor::ResolvePhaseTargetLocation")
label_body = function_body(actor_cpp, "FString AObjectiveFocusBeaconActor::BuildPhaseLabel")
visuals_body = function_body(actor_cpp, "void AObjectiveFocusBeaconActor::RefreshPhaseVisuals")
visibility_body = function_body(actor_cpp, "void AObjectiveFocusBeaconActor::SetBeaconVisible")
component_body = function_body(actor_cpp, "void AObjectiveFocusBeaconActor::ConfigureBeaconComponent")
purpose_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnPurposeClarityLayer")

check_all(
    actor_h,
    [
        "AObjectiveFocusBeaconActor",
        "CityIndex",
        "TerminalId",
        "SurvivorName",
        "EntryLocation",
        "TerminalLocation",
        "SurvivorLocation",
        "ExtractionLocation",
        "TerminalColor",
        "SurvivorColor",
        "ExtractionColor",
        "bReducedMotion",
        "ConfigureObjectiveBeacon",
        "OnObjectiveBeaconPhaseChanged",
        "BaseRing",
        "BeaconColumn",
        "DirectionArrow",
        "PulseCore",
        "ObjectiveLabel",
    ],
    "objective beacon header must expose target state, colors, components, and Blueprint hook",
)
check_all(
    constructor_body,
    [
        "PrimaryActorTick.bCanEverTick = true",
        "ObjectiveFocusBeaconBaseRing",
        "ObjectiveFocusBeaconColumn",
        "ObjectiveFocusBeaconDirectionArrow",
        "ObjectiveFocusBeaconPulseCore",
        "ObjectiveFocusBeaconLabel",
        "SetHorizontalAlignment(EHTA_Center)",
        "/Engine/BasicShapes/Cylinder.Cylinder",
        "/Engine/BasicShapes/Cube.Cube",
        "/Engine/BasicShapes/Sphere.Sphere",
        "ConfigureBeaconComponent",
        "ObjectiveFocusBeacon",
        "ObjectiveClarity",
        "WorldDevelopmentDeepDive",
        "Top50Recommendations",
    ],
    "constructor must build a cook-safe beacon rig and audit tags",
)
check_all(
    tick_body,
    [
        "ResolveObjectivePhase()",
        "SetBeaconVisible(false)",
        "RefreshPhaseVisuals(ObjectivePhase)",
        "OnObjectiveBeaconPhaseChanged(ObjectivePhase)",
        "FMath::VInterpTo",
        "bReducedMotion ? 0.25f : 1.0f",
        "SetRelativeScale3D",
        "SetRelativeRotation",
        "SetVisibility(ObjectivePhase >= ObjectivePhaseSurvivor",
        "SetVisibility(ObjectivePhase >= ObjectivePhaseExtraction",
        "SetIntensity",
        "UGameplayStatics::GetPlayerPawn",
        "SetWorldRotation",
    ],
    "tick must resolve objective state, animate, face player text, and support reduced motion",
)
check_all(
    configure_body,
    [
        "CityIndex = InCityIndex",
        "CityName = InCityName",
        "TerminalId = InTerminalId",
        "SurvivorName = InSurvivorName",
        "EntryLocation = InEntryLocation",
        "TerminalLocation = InTerminalLocation",
        "SurvivorLocation = InSurvivorLocation",
        "ExtractionLocation = InExtractionLocation",
        "bReducedMotion = bInReducedMotion",
        "SetActorLocation(TerminalLocation)",
        "SetBeaconVisible(false)",
    ],
    "ConfigureObjectiveBeacon must store target state and start hidden",
)
check_all(
    resolve_body,
    [
        "GetFirstIncompleteCityIndex",
        "SolvedTerminalIds.Contains(TerminalId)",
        "RescuedSurvivorNames.Contains(SurvivorName)",
        "ObjectivePhaseTerminal",
        "ObjectivePhaseSurvivor",
        "ObjectivePhaseExtraction",
        "FVector::DistSquared2D",
        "ExtractionLocation",
    ],
    "ResolveObjectivePhase must react to saved terminal/survivor progress and extraction proximity",
)
check_all(
    target_body,
    [
        "TerminalLocation",
        "SurvivorLocation",
        "ExtractionLocation",
        "EntryLocation",
    ],
    "phase targets must map terminal, survivor, extraction, and entry fallback locations",
)
check_all(
    label_body,
    [
        "GI->GetLanguageName()",
        "CURRENT OBJECTIVE",
        "TERMINAL",
        "RESCUE SURVIVOR",
        "EXTRACTION READY",
    ],
    "label builder must surface active language and objective phase",
)
check_all(
    visuals_body,
    [
        "TerminalColor",
        "SurvivorColor",
        "ExtractionColor",
        "ApplyComponentTint",
        "ObjectiveLabel->SetText",
        "ObjectiveLabel->SetTextRenderColor",
        "ObjectiveBeaconTerminalPhase",
        "ObjectiveBeaconSurvivorPhase",
        "ObjectiveBeaconExtractionPhase",
    ],
    "RefreshPhaseVisuals must tint visuals, update text, and tag phase state",
)
check_all(
    visibility_body,
    [
        "BaseRing",
        "BeaconColumn",
        "DirectionArrow",
        "PulseCore",
        "ObjectiveLabel",
        "SetIntensity(bVisible ? 9200.0f : 0.0f)",
    ],
    "visibility helper must hide/show all primitives, label, and light",
)
check_all(
    component_body,
    [
        "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
        "SetGenerateOverlapEvents(false)",
        "SetMobility(EComponentMobility::Movable)",
        "ObjectiveFocusBeacon",
        "ObjectiveClarity",
    ],
    "beacon components must be nonblocking and tagged",
)
check("ObjectiveFocusBeaconActor.h" in gamemode_cpp,
      "game mode must include the objective focus beacon actor")
check_all(
    purpose_body,
    [
        "SpawnActor<AObjectiveFocusBeaconActor>",
        "AObjectiveFocusBeaconActor::StaticClass()",
        "ConfigureObjectiveBeacon",
        "Mission.TerminalId",
        "Mission.SurvivorName",
        "GI && GI->bReducedMotion",
        "StateAwareObjectiveBeacon",
        "ObjectiveClarityRuntimeLayer",
        "RegisterStreamedActor(ObjectiveBeacon)",
    ],
    "SpawnPurposeClarityLayer must create, configure, tag, and register the objective beacon",
)
check("verify_objective_focus_beacon_slice_pass.py" in full_qa,
      "full QA must run the objective focus beacon verifier")
check("verify_objective_focus_beacon_slice_pass.py" in local_ci,
      "local CI must run the objective focus beacon verifier")
check("Objective focus beacon slice" in progress,
      "progress log must document the objective focus beacon slice")
check_all(
    slice_doc,
    [
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "objective clarity",
        "terminal",
        "survivor",
        "extraction",
        "reduced motion",
        "language",
    ],
    "slice doc must map objective beacon work to the June 25 documents",
)

if errors:
    for error in errors:
        print(f"[verify_objective_focus_beacon_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_objective_focus_beacon_slice_pass] PASS: objective focus beacon slice verified")
