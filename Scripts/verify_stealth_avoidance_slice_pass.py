#!/usr/bin/env python3
"""Static verifier for the stealth/avoidance gameplay slice."""

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


def check_all(source: str, tokens: list[str], message: str) -> None:
    missing = [token for token in tokens if token not in source]
    if missing:
        errors.append(f"{message}: missing {', '.join(repr(token) for token in missing)}")


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


character_h = read(SRC / "CodeRescueCharacter.h")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
ai_h = read(SRC / "CodeRescueAIController.h")
ai_cpp = read(SRC / "CodeRescueAIController.cpp")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/stealth_avoidance_manifest.tsv")
plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "STEALTH_AVOIDANCE_SLICE.md")

update_noise_fn = function_body(character_cpp, "void ACodeRescueCharacter::UpdateStealthNoise")
report_noise_fn = function_body(character_cpp, "void ACodeRescueCharacter::ReportStealthNoise")
noise_radius_fn = function_body(character_cpp, "float ACodeRescueCharacter::GetStealthNoiseRadius")
noise_summary_fn = function_body(character_cpp, "FString ACodeRescueCharacter::GetStealthStateSummary")
patrol_fn = function_body(ai_cpp, "void ACodeRescueAIController::UpdatePatrol")
investigate_fn = function_body(ai_cpp, "void ACodeRescueAIController::UpdateInvestigate")
chase_fn = function_body(ai_cpp, "void ACodeRescueAIController::UpdateChase")
sight_fn = function_body(ai_cpp, "bool ACodeRescueAIController::CanDetectPlayerBySight")
noise_detect_fn = function_body(ai_cpp, "bool ACodeRescueAIController::CanDetectPlayerByNoise")
range_fn = function_body(ai_cpp, "float ACodeRescueAIController::GetSightDetectionRange")
visible_fn = function_body(ai_cpp, "bool ACodeRescueAIController::IsPlayerVisible")
hud_refresh_fn = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")

check_all(
    character_h,
    [
        "StealthNoiseLevel",
        "StealthNoiseDecayRate",
        "QuietMovementNoiseRadius",
        "SprintNoiseRadius",
        "WeaponNoiseRadius",
        "UtilityNoiseRadius",
        "ReportStealthNoise",
        "GetStealthNoiseRadius",
        "GetStealthStateSummary",
        "UpdateStealthNoise",
        "LastStealthNoiseReason",
    ],
    "character header must expose stealth noise state and helpers",
)
check_all(
    update_noise_fn + report_noise_fn + noise_radius_fn + noise_summary_fn,
    [
        "StealthAvoidanceNoiseEmitter",
        "NoiseDetectionGameplay",
        "StealthFlashlightExposure",
        "StealthQuietRouteAvailable",
        "sprinting",
        "moving",
        "EXPOSED: flashlight",
        "NOISY:",
        "AUDIBLE:",
        "QUIET: avoid sightlines",
    ],
    "character implementation must maintain readable stealth state",
)
check_all(
    character_cpp,
    [
        "UpdateStealthNoise(DeltaSeconds)",
        "Top50Recommendation40StealthAvoidance",
        "ReportStealthNoise(0.42f, UtilityNoiseRadius, TEXT(\"throwable\"))",
        "ReportStealthNoise(0.38f, UtilityNoiseRadius, TEXT(\"flashlight exposure\"))",
        "ReportStealthNoise(0.36f, UtilityNoiseRadius, TEXT(\"radio scan\"))",
        "ReportStealthNoise(0.58f, UtilityNoiseRadius, TEXT(\"melee swing\"))",
        "TEXT(\"weapon fire\")",
        "ReportStealthNoise(0.22f, UtilityNoiseRadius * 0.65f, TEXT(\"reload\"))",
        "Shift sprint/noise",
    ],
    "player actions must feed the stealth noise system",
)
check_all(
    ai_h + ai_cpp,
    [
        "LastKnownPlayerLocation",
        "LostSightGraceSeconds",
        "CanDetectPlayerBySight",
        "CanDetectPlayerByNoise",
        "GetSightDetectionRange",
        "StealthNoiseDetected",
        "StealthSightlineDetected",
        "StealthInvestigateNoise",
        "StealthInvestigateLastKnownLocation",
        "StealthAvoidanceLostContact",
        "StealthAvoidanceNoiseListener",
    ],
    "AI controller must expose sight/noise detection and last-known investigation",
)
check_all(
    patrol_fn,
    [
        "CanDetectPlayerBySight(DistanceToPlayer)",
        "CanDetectPlayerByNoise(DistanceToPlayer)",
        "CurrentState = bSightDetected ? EZombieAIState::Chase : EZombieAIState::Investigate",
    ],
    "patrol must use sight/noise instead of proximity-only chase",
)
if "DistanceToPlayer < ZombieCharacter->ActivationRange" in patrol_fn:
    errors.append("patrol still contains old proximity-only activation chase")
check_all(
    investigate_fn + chase_fn,
    [
        "TryMoveToLocationWithFallback(LastKnownPlayerLocation",
        "LostSightGraceSeconds",
        "StealthAvoidanceLostContact",
        "ResolveEncounterMoveTarget(TargetLocation)",
    ],
    "investigate/chase must use last-known location and lose contact when stealth succeeds",
)
check_all(
    visible_fn + range_fn + sight_fn + noise_detect_fn,
    [
        "PlayerCharacter->bFieldFlashlightActive",
        "PlayerCharacter->GetStealthNoiseRadius()",
        "IsPlayerInProtectedLearningZone()",
        "CodeRescueCollision::AISightTrace",
    ],
    "detection helpers must respect flashlight, noise radius, protected zones, and sight trace",
)
check_all(
    zombie_cpp,
    [
        "Top50Recommendation40StealthAvoidance",
        "StealthAvoidanceParticipant",
    ],
    "zombies must be tagged as stealth/avoidance participants",
)
check_all(
    hud_refresh_fn,
    [
        "Stealth %s %.0f%%",
        "GetStealthStateSummary",
        "GetStealthNoiseLevel",
    ],
    "HUD tactical readout must show stealth state",
)
check_all(
    manifest,
    [
        "Player noise state",
        "Sightline detection",
        "Noise investigation",
        "HUD stealth readout",
        "Protected coding boundary",
    ],
    "manifest must document stealth surfaces",
)
check_all(
    plan,
    [
        "stealth and avoidance",
        "verify_stealth_avoidance_slice_pass.py",
        "manual StealthAvoidance review",
    ],
    "creative plan must include the stealth/avoidance row",
)
check_all(
    qa + visual,
    [
        "StealthAvoidance",
        "QUIET",
        "NOISY",
        "EXPOSED",
    ],
    "QA and visual targets must cover stealth states",
)
check_all(
    local_ci + full_qa,
    ["python3 Scripts/verify_stealth_avoidance_slice_pass.py"],
    "local CI and full QA must run the stealth verifier",
)
check_all(
    progress + doc,
    [
        "Stealth avoidance slice",
        "Top 50 recommendation 40",
        "verify_stealth_avoidance_slice_pass.py",
    ],
    "progress log and documentation must describe the slice",
)

if errors:
    for error in errors:
        print(f"[verify_stealth_avoidance_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_stealth_avoidance_slice_pass] PASS: stealth avoidance slice verified")
