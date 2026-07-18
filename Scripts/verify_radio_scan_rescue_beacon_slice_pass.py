#!/usr/bin/env python3
"""Static verifier for the radio scan and rescue beacon effects slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
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
radio_manifest = read(DATA / "radio_scan_rescue_beacon_manifest.tsv")
curriculum_manifest = read(DATA / "curriculum_feedback_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
accessibility_manifest = read(DATA / "accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "RADIO_SCAN_RESCUE_BEACON_SLICE.md")

constructor_body = function_body(actor_cpp, "AObjectiveFocusBeaconActor::AObjectiveFocusBeaconActor")
tick_body = function_body(actor_cpp, "void AObjectiveFocusBeaconActor::Tick")
configure_body = function_body(actor_cpp, "void AObjectiveFocusBeaconActor::ConfigureObjectiveBeacon")
phase_label_body = function_body(actor_cpp, "FString AObjectiveFocusBeaconActor::BuildPhaseLabel")
radio_label_body = function_body(actor_cpp, "FString AObjectiveFocusBeaconActor::BuildRadioScanLine")
visuals_body = function_body(actor_cpp, "void AObjectiveFocusBeaconActor::RefreshPhaseVisuals")
visibility_body = function_body(actor_cpp, "void AObjectiveFocusBeaconActor::SetBeaconVisible")
component_body = function_body(actor_cpp, "void AObjectiveFocusBeaconActor::ConfigureBeaconComponent")
purpose_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnPurposeClarityLayer")

check_all(
    actor_h,
    [
        "TerminalTitle",
        "MissionConcept",
        "LandmarkName",
        "RadioScanRing",
        "RadioSweepArm",
        "RescueBeaconHalo",
        "RadioPingA",
        "RadioPingB",
        "RadioScanLabel",
        "BuildRadioScanLine",
    ],
    "objective beacon header must expose radio-scan copy and procedural effect components",
)
check_all(
    constructor_body,
    [
        "ObjectiveFocusRadioScanRing",
        "ObjectiveFocusRadioSweepArm",
        "ObjectiveFocusRescueBeaconHalo",
        "ObjectiveFocusRadioPingA",
        "ObjectiveFocusRadioPingB",
        "ObjectiveFocusRadioScanLabel",
        "RadioScanRescueBeaconEffects",
        "RescueBeaconEffects",
        "ConfigureBeaconComponent",
    ],
    "constructor must build cook-safe radio-scan and rescue-beacon primitives",
)
check_all(
    tick_body,
    [
        "RadioScanRing",
        "RadioSweepArm",
        "RescueBeaconHalo",
        "RadioPingA",
        "RadioPingB",
        "RadioScanLabel",
        "ObjectivePhase == ObjectivePhaseExtraction ? 164.0f : 118.0f",
        "bReducedMotion ? 0.25f : 1.0f",
        "SetVisibility(ObjectivePhase >= ObjectivePhaseSurvivor",
        "SetVisibility(ObjectivePhase >= ObjectivePhaseExtraction",
        "SetWorldRotation",
    ],
    "tick must animate scan ring, sweep arm, rescue halo, pings, and text with reduced-motion support",
)
check_all(
    configure_body,
    [
        "TerminalTitle = InTerminalTitle",
        "MissionConcept = InMissionConcept",
        "LandmarkName = InLandmarkName",
    ],
    "ConfigureObjectiveBeacon must store mission copy for readable radio labels",
)
check_all(
    phase_label_body,
    [
        "RADIO SCAN: %s TERMINAL",
        "SURVIVOR PING: RESCUE SURVIVOR",
        "RESCUE BEACON: EXTRACTION READY",
    ],
    "objective label must name the radio-scan phase, survivor ping, and rescue beacon",
)
check_all(
    radio_label_body,
    [
        "GI->GetLanguageName()",
        "TerminalTitle.IsEmpty() ? TerminalId : TerminalTitle",
        "MissionConcept.IsEmpty()",
        "LandmarkName.IsEmpty() ? CityName : LandmarkName",
        "RADIO SCAN",
        "SURVIVOR PING",
        "RESCUE BEACON",
        "Extraction live",
    ],
    "radio scan label must use selected language, terminal title, concept, landmark, and phase copy",
)
check_all(
    visuals_body,
    [
        "ApplyComponentTint(RadioScanRing",
        "ApplyComponentTint(RadioSweepArm",
        "ApplyComponentTint(RescueBeaconHalo",
        "ApplyComponentTint(RadioPingA",
        "ApplyComponentTint(RadioPingB",
        "RadioScanLabel->SetText",
        "BuildRadioScanLine(ObjectivePhase)",
        "RadioScanRescueBeaconActive",
        "RadioScanTerminalPhase",
        "SurvivorPingPhase",
        "RescueBeaconExtractionPhase",
    ],
    "phase visuals must tint radio effects, update labels, and tag active phase state",
)
check_all(
    visibility_body,
    [
        "RadioScanRing",
        "RadioSweepArm",
        "RescueBeaconHalo",
        "RadioPingA",
        "RadioPingB",
        "RadioScanLabel",
    ],
    "visibility helper must hide/show the new effect primitives and label",
)
check_all(
    component_body,
    [
        "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
        "RadioScanRescueBeaconEffects",
    ],
    "new beacon primitives must remain nonblocking and tagged",
)
check_all(
    purpose_body,
    [
        "Mission.TerminalTitle",
        "Mission.CurriculumFocus",
        "Mission.LandmarkName",
        "RadioScanRescueBeaconEffects",
        "RegisterStreamedActor(ObjectiveBeacon)",
    ],
    "game mode must feed mission copy into the effect layer and register the beacon",
)
check_all(
    radio_manifest,
    [
        "TerminalRadioScan",
        "SurvivorPing",
        "ExtractionRescueBeacon",
        "RadioScanRing",
        "RadioSweepArm",
        "RescueBeaconHalo",
        "bReducedMotion",
        "selected language",
    ],
    "radio scan manifest must document phases, components, labels, and accessibility",
)
check_all(
    curriculum_manifest,
    [
        "RadioScanRescueBeacon",
        "AObjectiveFocusBeaconActor radio scan + rescue beacon",
        "selected-language terminal route, survivor ping, and extraction beacon",
    ],
    "curriculum feedback manifest must document radio-scan world feedback",
)
check_all(
    onboarding,
    [
        "radio scan beacon points to the active terminal",
        "rescue beacon changes from SURVIVOR PING to RESCUE BEACON",
    ],
    "onboarding must explain the radio scan and rescue beacon progression",
)
check_all(
    visual_manifest,
    [
        "RadioScanRescueBeacon",
        "RADIO SCAN, SURVIVOR PING, and RESCUE BEACON labels",
    ],
    "visual regression targets must include radio-scan/rescue-beacon review",
)
check("RadioScanRescueBeacon" in human_qa and "reduced motion still shows text-first phase changes" in human_qa,
      "human QA checklist must include radio-scan/rescue-beacon accessibility check")
check_all(
    creative_plan,
    [
        "radio scan and rescue beacon effects",
        "verify_radio_scan_rescue_beacon_slice_pass.py plus render smoke plus visual capture",
    ],
    "creative plan must mark the VFX slice as implemented and verifiable",
)
check_all(
    accessibility_manifest,
    [
        "RadioScanBeaconAccessibility",
        "bReducedMotion",
        "text-first RADIO SCAN, SURVIVOR PING, and RESCUE BEACON labels",
    ],
    "accessibility manifest must record reduced-motion/text-first beacon behavior",
)
check("verify_radio_scan_rescue_beacon_slice_pass.py" in full_qa,
      "full QA must run the radio scan/rescue beacon verifier")
check("verify_radio_scan_rescue_beacon_slice_pass.py" in local_ci,
      "local CI must run the radio scan/rescue beacon verifier")
check("Radio scan rescue beacon slice" in progress,
      "progress log must document the radio scan rescue beacon slice")
check_all(
    slice_doc,
    [
        "Radio Scan Rescue Beacon Slice",
        "AObjectiveFocusBeaconActor",
        "RadioScanRing",
        "RadioSweepArm",
        "RescueBeaconHalo",
        "BuildRadioScanLine",
        "CHARACTER_ANIMATION_DEEPDIVE",
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "Validation",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, source guidance, validation, and QA",
)

if errors:
    print("Radio scan rescue beacon slice verification FAILED:")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("Radio scan rescue beacon slice verification passed.")
