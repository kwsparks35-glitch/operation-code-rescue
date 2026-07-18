#!/usr/bin/env python3
"""Static verifier for the protected learning-zone AI slice."""

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


gamemode_h = read(SRC / "CodeRescueGameMode.h")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
ai_h = read(SRC / "CodeRescueAIController.h")
ai_cpp = read(SRC / "CodeRescueAIController.cpp")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/protected_learning_zone_ai_manifest.tsv")
plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "PROTECTED_LEARNING_ZONE_AI_SLICE.md")

zone_query = function_body(gamemode_cpp, "bool ACodeRescueGameMode::IsLocationInsideProtectedLearningZone")
spawn_terminal = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnTerminal")
ai_update = function_body(ai_cpp, "void ACodeRescueAIController::UpdateState")
ai_protected = function_body(ai_cpp, "bool ACodeRescueAIController::IsPlayerInProtectedLearningZone")
ai_visible = function_body(ai_cpp, "bool ACodeRescueAIController::IsPlayerVisible")
zombie_tick = function_body(zombie_cpp, "void ACodeZombieActor::Tick")
elite_tick = function_body(zombie_cpp, "bool ACodeZombieActor::TickEliteBehavior")
boomer_death = function_body(zombie_cpp, "void ACodeZombieActor::OnBoomerDeath")
apply_damage = function_body(character_cpp, "void ACodeRescueCharacter::ApplyDamage")

check_all(
    gamemode_h,
    [
        "IsLocationInsideProtectedLearningZone",
        "Code Rescue|Safety",
    ],
    "game mode must expose the shared protected-zone query",
)
check_all(
    zone_query,
    [
        "NoZombieLearningZone",
        "ProtectedCodingChallengeZone",
        "ProtectedLearningSpace",
        "SafeTerminalLab",
        "BonusCodingChallengeSafeZone",
        "GetComponentsBoundingBox",
        "ExpandBy(SafeExpansion)",
        "IsInsideOrOn(Location)",
        "MarkerRadius",
    ],
    "protected-zone query must use tagged expanded bounds and marker fallback",
)
check_all(
    spawn_terminal,
    [
        "Terminal->Tags.AddUnique(FName(\"ProtectedCodingChallengeZone\"))",
        "Terminal->Tags.AddUnique(FName(\"NoZombieLearningZone\"))",
        "Terminal->Tags.AddUnique(FName(\"SafeTerminalLab\"))",
        "Terminal->Tags.AddUnique(FName(\"SelectedLanguageOnly\"))",
        "Terminal->Tags.AddUnique(FName(\"LearningWithoutDeathRisk\"))",
    ],
    "spawned terminals must become protected learning anchors",
)
check_all(
    ai_h + ai_cpp,
    [
        "IsPlayerInProtectedLearningZone",
        "#include \"CodeRescueGameMode.h\"",
    ],
    "AI controller must know how to query protected learning zones",
)
check_all(
    ai_update,
    [
        "IsPlayerInProtectedLearningZone()",
        "StopMovement()",
        "CurrentState = EZombieAIState::Patrol",
        "return;",
    ],
    "AI state updates must stop chase/attack while player is protected",
)
check_all(
    ai_protected,
    [
        "ACodeRescueGameMode::IsLocationInsideProtectedLearningZone",
        "PlayerCharacter->GetActorLocation()",
        "300.0f",
    ],
    "AI protected helper must call the shared zone query",
)
check_all(
    ai_visible,
    [
        "IsPlayerInProtectedLearningZone()",
        "return false;",
    ],
    "AI visibility must be suppressed while player is protected",
)
check_all(
    zombie_cpp,
    [
        "#include \"CodeRescueGameMode.h\"",
        "bool IsPawnInsideProtectedLearningZone",
        "ACodeRescueGameMode::IsLocationInsideProtectedLearningZone",
    ],
    "zombie actor must share the protected-zone query",
)
check_all(
    zombie_tick,
    [
        "IsPawnInsideProtectedLearningZone(PlayerPawn)",
        "NoZombieLearningZoneRespected",
        "TimeSinceAttack = 0.0f",
        "DistantTickAccumulator = 0.0f",
        "RetreatDirection",
        "AddMovementInput(RetreatDirection, 0.65f)",
        "StopMovementImmediately()",
        "return;",
    ],
    "zombie tick must retreat/idle and exit before damage while player is protected",
)
check_all(
    elite_tick,
    [
        "IsPawnInsideProtectedLearningZone(PlayerPawn)",
        "NoZombieLearningZoneRespected",
        "return true;",
    ],
    "elite zombie behavior must be blocked in protected zones",
)
check_all(
    boomer_death,
    [
        "IsPawnInsideProtectedLearningZone(PlayerPawn)",
        "NoZombieLearningZoneRespected",
        "C->ApplyDamage(40.0f * DmgScale, this)",
    ],
    "boomer explosion damage must skip protected players",
)
check_all(
    apply_damage,
    [
        "DamageSource->IsA<ACodeZombieActor>()",
        "ACodeRescueGameMode::IsLocationInsideProtectedLearningZone",
        "protected learning zone blocked zombie damage",
        "ProtectedLearningDamageBlocked",
        "return;",
    ],
    "player damage intake must fail-safe zombie damage in protected zones",
)
check_all(
    manifest,
    [
        "Tagged protected bounds",
        "Terminal protection anchors",
        "AI chase exclusion",
        "Zombie attack exclusion",
        "Damage fail-safe",
    ],
    "manifest must describe protected learning-zone AI surfaces",
)
for label, source in {
    "creative inclusion plan": plan,
    "human QA checklist": qa,
    "visual regression targets": visual,
    "full QA command": full_qa,
    "local CI command": local_ci,
    "progress log": progress,
    "slice documentation": doc,
}.items():
    check("verify_protected_learning_zone_ai_slice_pass.py" in source or label in {"human QA checklist", "visual regression targets", "progress log", "slice documentation"}, f"{label} must reference the new protected learning-zone verifier or slice")

check("ProtectedLearningZoneAI" in qa, "human QA checklist must include ProtectedLearningZoneAI")
check("ProtectedLearningZoneAI" in visual, "visual regression targets must include ProtectedLearningZoneAI")
check("Protected learning zone AI slice" in progress, "progress log must mention this slice")
check("verify_protected_learning_zone_ai_slice_pass.py" in doc, "slice doc must list the verifier")

if errors:
    print("[verify_protected_learning_zone_ai_slice_pass] FAIL")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("[verify_protected_learning_zone_ai_slice_pass] PASS: protected learning-zone AI exclusion is implemented and documented")
