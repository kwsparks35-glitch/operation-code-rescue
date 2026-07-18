#!/usr/bin/env python3
"""Static verifier for zombie death physics and hit readability."""

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


zombie_h = read(SRC / "CodeZombieActor.h")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "ZOMBIE_DEATH_PHYSICS_SLICE.md")

damage_body = function_body(zombie_cpp, "void ACodeZombieActor::ApplyRescueDamage")
hit_body = function_body(zombie_cpp, "void ACodeZombieActor::ApplyHitReadabilityImpulse")
disable_body = function_body(zombie_cpp, "void ACodeZombieActor::DisableGameplayCollisionForDeath")
ragdoll_body = function_body(zombie_cpp, "bool ACodeZombieActor::TryActivateDeathRagdoll")
primitive_body = function_body(zombie_cpp, "bool ACodeZombieActor::ActivatePrimitiveDeathPhysics")
impulse_body = function_body(zombie_cpp, "FVector ACodeZombieActor::ComputeDeathPhysicsImpulse")
release_body = function_body(zombie_cpp, "void ACodeZombieActor::ReleaseRagdollBudget")
end_play_body = function_body(zombie_cpp, "void ACodeZombieActor::EndPlay")

check_all(
    zombie_h,
    [
        "HitReactionImpulseStrength",
        "bEnableDeathRagdoll",
        "bEnablePrimitiveCorpsePhysics",
        "RagdollImpulseStrength",
        "PrimitiveCorpseImpulseStrength",
        "RagdollCorpseLifetime",
        "ApplyHitReadabilityImpulse",
        "DisableGameplayCollisionForDeath",
        "TryActivateDeathRagdoll",
        "ActivatePrimitiveDeathPhysics",
        "ComputeDeathPhysicsImpulse",
    ],
    "zombie header must expose death-physics tuning and helper APIs",
)
check_all(
    impulse_body,
    [
        "EHitZone::Head",
        "EHitZone::Limb",
        "FMath::Clamp",
        "-GetActorForwardVector()",
    ],
    "death impulse must vary by hit zone and damage",
)
check_all(
    hit_body,
    [
        "LaunchCharacter",
        "ZombieHitPhysicsReadability",
        "PhysicalHitReactionFallback",
        "Glow->SetLightColor",
    ],
    "nonfatal hits must provide readable physical/visual feedback",
)
check_all(
    disable_body,
    [
        "StopMovementImmediately",
        "DisableMovement",
        "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
        "SetGenerateOverlapEvents(false)",
    ],
    "death physics must disable gameplay collision and movement first",
)
check_all(
    ragdoll_body,
    [
        "CodeRescueMaxActiveRagdollCorpses",
        "GetPhysicsAsset()",
        "SetCollisionProfileName(TEXT(\"Ragdoll\"))",
        "SetAllBodiesSimulatePhysics(true)",
        "SetSimulatePhysics(true)",
        "WakeAllRigidBodies",
        "AddImpulse",
        "ZombieDeathRagdoll",
        "GamePhysicsDeepDive",
        "CharacterAnimationDeepDive",
        "DeathPhysicsReadable",
    ],
    "skeletal zombies must activate a capped ragdoll path when a physics asset exists",
)
check_all(
    primitive_body,
    [
        "SetCollisionProfileName(TEXT(\"PhysicsActor\"))",
        "SetSimulatePhysics(true)",
        "WakeAllRigidBodies",
        "AddImpulse",
        "AddAngularImpulseInDegrees",
        "ZombiePrimitiveCorpsePhysics",
        "DeathPhysicsReadable",
    ],
    "primitive fallback zombies must detach into simulated corpse parts",
)
check_all(
    damage_body,
    [
        "ApplyHitReadabilityImpulse(HitZone, FinalDamage)",
        "TryActivateDeathRagdoll(HitZone, FinalDamage)",
        "ActivatePrimitiveDeathPhysics(HitZone, FinalDamage)",
        "ZombieDeathPhysicsActive",
        "ZombieDeathMontageFallback",
        "MarkZombieNeutralized",
        "SavePersistentRun",
        "VisualMarkerActor->Destroy",
        "OnBoomerDeath",
        "DeathMontage",
    ],
    "damage/death pipeline must preserve save markers, boomer behavior, montage fallback, and new physics",
)
check("!bActivatedDeathPhysics && DeathMontage" in damage_body,
      "death montage must remain a fallback instead of competing with active ragdoll physics")
check("GCodeRescueActiveRagdollCorpses" in release_body and "bCountedActiveRagdoll = false" in release_body
      and "ReleaseRagdollBudget();" in end_play_body,
      "ragdoll cap counter must be released exactly once during fade or actor teardown")
check("verify_zombie_death_physics_slice_pass.py" in full_qa,
      "full QA must run the zombie death physics verifier")
check("verify_zombie_death_physics_slice_pass.py" in local_ci,
      "local CI must run the zombie death physics verifier")
check("Zombie death physics and hit readability" in progress,
      "progress log must document the zombie death physics slice")
check("GAME_PHYSICS_DEEPDIVE" in slice_doc and "CHARACTER_ANIMATION_DEEPDIVE" in slice_doc,
      "slice doc must map the work to the physics and animation deep dives")
check("WORLD_DEVELOPMENT_DEEPDIVE" in slice_doc,
      "slice doc must explain the world/playability reason for readable combat feedback")

if errors:
    for error in errors:
        print(f"[verify_zombie_death_physics_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_zombie_death_physics_slice_pass] PASS: zombie death physics/readability slice verified")
