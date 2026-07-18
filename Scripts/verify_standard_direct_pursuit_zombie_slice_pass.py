#!/usr/bin/env python3
"""Static verifier for the standard direct-pursuit zombie slice."""

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


zombie_h = read(SRC / "CodeZombieActor.h")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
ai_cpp = read(SRC / "CodeRescueAIController.cpp")
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/standard_direct_pursuit_zombie_manifest.tsv")
plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "STANDARD_DIRECT_PURSUIT_ZOMBIE_SLICE.md")

apply_profile_fn = function_body(zombie_cpp, "void ACodeZombieActor::ApplyStandardDirectPursuitProfile")
cooldown_fn = function_body(zombie_cpp, "float ACodeZombieActor::GetStandardPursuitAttackCooldown")
summary_fn = function_body(zombie_cpp, "FString ACodeZombieActor::GetStandardPursuitStateSummary")
readability_fn = function_body(zombie_cpp, "void ACodeZombieActor::UpdateStandardPursuitReadability")
tick_fn = function_body(zombie_cpp, "void ACodeZombieActor::Tick")
boomer_fn = function_body(zombie_cpp, "void ACodeZombieActor::OnBoomerDeath")
chase_fn = function_body(ai_cpp, "void ACodeRescueAIController::UpdateChase")
attack_fn = function_body(ai_cpp, "void ACodeRescueAIController::UpdateAttack")
hud_info_fn = function_body(hud_cpp, "FCodeRescueThreatHudInfo GetNearestHudThreat")
hud_refresh_fn = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")

check_all(
    zombie_h,
    [
        "bStandardDirectPursuitEnabled",
        "StandardPursuitAttackCooldown",
        "StandardPursuitReadabilityRange",
        "StandardPursuitClosePressureRange",
        "ApplyStandardDirectPursuitProfile",
        "GetStandardPursuitStateSummary",
        "UpdateStandardPursuitReadability",
        "LastStandardPursuitStateTag",
    ],
    "zombie header must expose the standard pursuit profile and state",
)
check_all(
    apply_profile_fn,
    [
        "bStandardDirectPursuitEnabled = true",
        "StandardDirectPursuitZombie",
        "ZombiePursuitReadableRuntime",
        "FairSurvivalPressure",
        "NoLearningZonePressure",
        "AttackWindupReadable",
        "AttackTelegraphRangeMultiplier",
        "AttackTelegraphLeadSeconds",
        "HitReactionImpulseStrength",
    ],
    "profile application must tag fair pursuit and tune readable combat",
)
check_all(
    cooldown_fn + summary_fn,
    [
        "StandardPursuitAttackCooldown",
        "direct pursuit ready",
        "attack windup",
        "close pursuit",
        "direct pursuit",
        "tracking",
    ],
    "standard pursuit helpers must expose cooldown and state labels",
)
check_all(
    readability_fn,
    [
        "ApplyStandardDirectPursuitProfile()",
        "StandardPursuitState_AttackWindup",
        "StandardPursuitState_Close",
        "StandardPursuitState_DirectChase",
        "StandardPursuitAttackTelegraph",
        "PushThreatCaption(CaptionLabel",
        "LastStandardPursuitCaptionWorldTime",
    ],
    "readability updater must publish state tags and threat captions",
)
check_all(
    tick_fn,
    [
        "StandardPursuitProtectedCodingSpaceHold",
        "StandardPursuitNoLearningZonePressure",
        "holds outside coding zone",
        "const float AttackCooldown = GetStandardPursuitAttackCooldown()",
        "AttackCooldown - AttackTelegraphLeadSeconds",
        "UpdateStandardPursuitReadability",
        "StandardPursuitAttackCommit",
        "Character->ApplyDamage(AttackDamage, this)",
    ],
    "zombie tick must enforce protected learning hold, windup timing, and damage commit",
)
check_all(
    boomer_fn,
    ["Spawn->ApplyStandardDirectPursuitProfile()"],
    "boomer adds must inherit the standard pursuit profile",
)
check_all(
    chase_fn + attack_fn,
    [
        "bStandardDirectPursuitEnabled",
        "StandardPursuitDirectChase",
        "StandardPursuitMoveDirectlyTowardPlayer",
        "MoveDirectlyToward",
        "StandardPursuitAttackHold",
    ],
    "AI controller must tag direct chase and attack hold behavior",
)
if gamemode_cpp.count("ApplyStandardDirectPursuitProfile()") < 5:
    errors.append("game mode must apply the profile across regular city, physics, director, language breach, and horde spawns")
check_all(
    gamemode_cpp,
    [
        "RegularCityZombieFamily",
        "PhysicsLaneZombieFamily",
        "EncounterDirectorZombieFamily",
        "LanguageBreachZombieFamily",
        "BossHordeZombieFamily",
        "StandardDirectPursuitZombie",
        "ZombiePursuitReadableRuntime",
        "FairSurvivalPressure",
    ],
    "game mode must tag common ordinary pressure spawns and marker actors",
)
check_all(
    hud_cpp + hud_info_fn + hud_refresh_fn,
    [
        "bStandardPursuit",
        "PursuitLabel",
        "GetStandardPursuitStateSummary",
        "standard ",
        "PURSUIT",
        "PURSUIT PRESSURE",
        "THREAT COMPASS",
        "Threat %s: %s%s %.0fm %s%s%s",
    ],
    "HUD must surface standard pursuit state in compass, alert, and tactical readout",
)
check_all(
    manifest,
    [
        "Runtime pursuit profile",
        "Fair attack windup",
        "Protected learning hold",
        "Direct movement breadcrumbs",
        "Spawn coverage",
        "HUD and captions",
    ],
    "manifest must document standard pursuit surfaces",
)
check_all(
    plan,
    [
        "standard direct-pursuit zombies",
        "verify_standard_direct_pursuit_zombie_slice_pass.py",
        "verify_may27_safe_learning_city_controls_pass.py",
        "manual standard pursuit combat review",
    ],
    "creative inclusion plan must route the P0 standard zombie row through this verifier",
)
check_all(
    qa,
    [
        "StandardDirectPursuitZombies",
        "regular/director/horde zombie",
        "attack windup",
        "protected coding space",
    ],
    "human QA checklist must include standard pursuit combat review",
)
check_all(
    visual,
    [
        "StandardDirectPursuitZombies",
        "THREAT COMPASS pursuit state",
        "PURSUIT PRESSURE alert",
        "attack-windup glow",
    ],
    "visual regression targets must include standard pursuit screenshots",
)
check_all(
    local_ci + full_qa,
    ["python3 Scripts/verify_standard_direct_pursuit_zombie_slice_pass.py"],
    "local CI and full QA must run the standard pursuit verifier",
)
check_all(
    progress + doc,
    [
        "Standard direct-pursuit zombie slice",
        "ApplyStandardDirectPursuitProfile",
        "StandardDirectPursuitZombie",
        "verify_standard_direct_pursuit_zombie_slice_pass.py",
    ],
    "progress and documentation must summarize the slice",
)

if errors:
    print("[verify_standard_direct_pursuit_zombie_slice_pass] FAIL", file=sys.stderr)
    for error in errors:
        print(f" - {error}", file=sys.stderr)
    sys.exit(1)

print("[verify_standard_direct_pursuit_zombie_slice_pass] OK")
