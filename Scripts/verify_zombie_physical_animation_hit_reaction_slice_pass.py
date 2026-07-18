#!/usr/bin/env python3
"""Static verifier for zombie physical-animation hit reactions."""

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


zombie_h = read(SRC / "CodeZombieActor.h")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
manifest = read(DATA / "zombie_physical_animation_hit_reaction_manifest.tsv")
enemy_manifest = read(DATA / "enemy_readability_manifest.tsv")
physics_contract = read(DATA / "physics_promotion_validation_contract.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "ZOMBIE_PHYSICAL_ANIMATION_HIT_REACTION_SLICE.md")

constructor_body = function_body(zombie_cpp, "ACodeZombieActor::ACodeZombieActor")
begin_play_body = function_body(zombie_cpp, "void ACodeZombieActor::BeginPlay")
professional_body = function_body(zombie_cpp, "void ACodeZombieActor::ApplyProfessionalVisuals")
bind_body = function_body(zombie_cpp, "void ACodeZombieActor::BindPhysicalHitReactionComponent")
root_body = function_body(zombie_cpp, "FName ACodeZombieActor::ResolvePhysicalHitReactionRootBone")
impact_body = function_body(zombie_cpp, "FName ACodeZombieActor::ResolvePhysicalHitReactionImpactBone")
trigger_body = function_body(zombie_cpp, "bool ACodeZombieActor::TriggerPhysicalAnimationHitReaction")
update_body = function_body(zombie_cpp, "void ACodeZombieActor::UpdatePhysicalAnimationHitReaction")
reset_body = function_body(zombie_cpp, "void ACodeZombieActor::ResetPhysicalAnimationHitReaction")
hit_body = function_body(zombie_cpp, "void ACodeZombieActor::ApplyHitReadabilityImpulse")
disable_body = function_body(zombie_cpp, "void ACodeZombieActor::DisableGameplayCollisionForDeath")
tick_body = function_body(zombie_cpp, "void ACodeZombieActor::Tick")
damage_body = function_body(zombie_cpp, "void ACodeZombieActor::ApplyRescueDamage")

check_all(
    zombie_h,
    [
        "UPhysicalAnimationComponent",
        "bEnablePhysicalHitReaction",
        "PhysicalHitReactionRootBone",
        "PhysicalHitReactionBlendWeight",
        "PhysicalHitReactionDuration",
        "PhysicalHitReactionImpulseStrength",
        "PhysicalHitReactionComponent",
        "PhysicalHitReactionTimer",
        "bPhysicalHitReactionActive",
        "BindPhysicalHitReactionComponent",
        "ResolvePhysicalHitReactionRootBone",
        "ResolvePhysicalHitReactionImpactBone",
        "TriggerPhysicalAnimationHitReaction",
        "UpdatePhysicalAnimationHitReaction",
        "ResetPhysicalAnimationHitReaction",
    ],
    "zombie header must expose physical-animation hit reaction tuning and helpers",
)
check("PhysicsEngine/PhysicalAnimationComponent.h" in zombie_cpp,
      "zombie source must include the physical animation component API")
check_all(
    constructor_body,
    [
        "CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT(\"ZombiePhysicalHitReaction\"))",
        "ZombiePhysicalAnimationHitReactionComponent",
        "GamePhysicsDeepDive",
        "CharacterAnimationDeepDive",
    ],
    "constructor must create and tag the physical hit-reaction component",
)
check("BindPhysicalHitReactionComponent();" in begin_play_body,
      "BeginPlay must bind the physical-animation component after visual setup")
check("BindPhysicalHitReactionComponent();" in professional_body,
      "professional visual promotion must rebind physical-animation settings")
check_all(
    bind_body,
    [
        "GetPhysicsAsset()",
        "SetSkeletalMeshComponent",
        "SetStrengthMultiplyer(0.0f)",
        "ZombiePhysicalAnimationHitReactionReady",
        "PhysicalAnimationComponentRuntime",
        "ZombiePhysicalAnimationHitReactionFallbackOnly",
    ],
    "binding must bind the skeletal mesh without applying drive settings before hit time",
)
check_all(
    root_body,
    [
        "PhysicalHitReactionRootBone",
        "GetPhysicsAsset()",
        "FindBodyIndex",
        "spine_01",
        "spine",
        "pelvis",
        "root",
        "SkeletalBodySetups",
        "NAME_None",
    ],
    "root resolver must support common names and require a PhysicsAsset body root",
)
check_all(
    impact_body,
    [
        "GetPhysicsAsset()",
        "FindBodyIndex",
        "EHitZone::Head",
        "EHitZone::Limb",
        "head",
        "neck_01",
        "upperarm_l",
        "thigh_l",
        "ResolvePhysicalHitReactionRootBone",
    ],
    "impact resolver must prefer head and limb bones before root fallback",
)
check_all(
    trigger_body,
    [
        "bEnablePhysicalHitReaction",
        "GetPhysicsAsset()",
        "BindPhysicalHitReactionComponent",
        "ResolvePhysicalHitReactionRootBone",
        "ResolvePhysicalHitReactionImpactBone",
        "SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics)",
        "FPhysicalAnimationData",
        "OrientationStrength",
        "AngularVelocityStrength",
        "MaxAngularForce",
        "ApplyPhysicalAnimationSettingsBelow",
        "SetAllBodiesBelowSimulatePhysics",
        "SetAllBodiesBelowPhysicsBlendWeight",
        "SetStrengthMultiplyer",
        "AddImpulseAtLocation",
        "PhysicalHitReactionTimer",
        "bPhysicalHitReactionActive = true",
        "ZombiePhysicalAnimationHitReaction",
        "ZombiePhysicalAnimationHitReactionFallbackOnly",
        "HitReactionPhysicsBlend",
    ],
    "trigger must enable short-lived physical blend and apply hit-zone impulse",
)
check_all(
    update_body,
    [
        "bPhysicalHitReactionActive",
        "PhysicalHitReactionTimer",
        "BlendWeight",
        "SetStrengthMultiplyer",
        "SetAllBodiesBelowPhysicsBlendWeight",
        "ResetPhysicalAnimationHitReaction",
    ],
    "tick update must fade physical-animation strength and reset after duration",
)
check_all(
    reset_body,
    [
        "SetStrengthMultiplyer(0.0f)",
        "RootBone != NAME_None",
        "SetAllBodiesBelowPhysicsBlendWeight",
        "SetAllBodiesBelowSimulatePhysics",
        "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
        "bPhysicalHitReactionActive = false",
        "ZombiePhysicalAnimationHitReactionSettled",
    ],
    "reset must disable temporary simulation, restore no-collision, and clear active state",
)
check("TriggerPhysicalAnimationHitReaction(HitZone, FinalDamage)" in hit_body,
      "nonfatal hit impulse path must attempt physical-animation hit reaction")
check("PhysicalHitReactionFallback" in hit_body,
      "fallback hit-reaction tag must be preserved when physical animation cannot run")
check("ResetPhysicalAnimationHitReaction();" in disable_body,
      "death collision handoff must reset physical hit reactions before ragdoll")
check("UpdatePhysicalAnimationHitReaction(DeltaSeconds);" in tick_body,
      "tick must update the physical-animation blend")
check_all(
    damage_body,
    [
        "ApplyHitReadabilityImpulse(HitZone, FinalDamage)",
        "TriggerHitReactionMotionCue(HitZone, FinalDamage)",
        "HitReactMontage",
        "TryActivateDeathRagdoll(HitZone, FinalDamage)",
        "ActivatePrimitiveDeathPhysics(HitZone, FinalDamage)",
    ],
    "damage path must preserve flinch, montage, ragdoll, and primitive corpse behavior",
)
check("verify_zombie_physical_animation_hit_reaction_slice_pass.py" in full_qa,
      "full QA must run the zombie physical-animation hit-reaction verifier")
check("verify_zombie_physical_animation_hit_reaction_slice_pass.py" in local_ci,
      "local CI must run the zombie physical-animation hit-reaction verifier")
for label, source in [
    ("manifest", manifest),
    ("enemy readability manifest", enemy_manifest),
    ("physics promotion contract", physics_contract),
    ("human QA", human_qa),
    ("visual targets", visual_targets),
    ("creative plan", creative_plan),
]:
    check("ZombiePhysicalAnimationHitReaction" in source,
          f"{label} must document zombie physical-animation hit reactions")
check("Zombie physical-animation hit reaction slice" in progress,
      "progress log must document this slice")
check_all(
    slice_doc,
    [
        "GAME_PHYSICS_DEEPDIVE",
        "CHARACTER_ANIMATION_DEEPDIVE",
        "UPhysicalAnimationComponent",
        "PhysicsAsset",
        "SetStrengthMultiplyer",
        "SetAllBodiesBelowPhysicsBlendWeight",
        "PhysicalHitReactionFallback",
        "death ragdoll",
    ],
    "slice doc must map implementation and boundaries to the June 25 guidance",
)

if errors:
    for error in errors:
        print(f"[verify_zombie_physical_animation_hit_reaction_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_zombie_physical_animation_hit_reaction_slice_pass] PASS: zombie physical-animation hit reaction slice verified")
