#!/usr/bin/env python3
"""Static verifier for the companion gesture readability slice."""

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


def check_all(source: str, tokens: list[str], message: str) -> None:
    missing = [token for token in tokens if token not in source]
    if missing:
        errors.append(f"{message}: missing {', '.join(missing)}")


def check_none(source: str, tokens: list[str], message: str) -> None:
    found = [token for token in tokens if token in source]
    if found:
        errors.append(f"{message}: found {', '.join(found)}")


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


companion_h = read(SRC / "CompanionActor.h")
companion_cpp = read(SRC / "CompanionActor.cpp")
plan = read(DATA / "creative_development_inclusion_plan.tsv")
manifest = read(DATA / "companion_gesture_readability_manifest.tsv")
squad_manifest = read(DATA / "squad_personality_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
qa = read(DATA / "human_qa_signoff_checklist.tsv")
visual = read(DATA / "visual_regression_targets.tsv")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "COMPANION_GESTURE_READABILITY_SLICE.md")

constructor_body = function_body(companion_cpp, "ACompanionActor::ACompanionActor")
begin_play_body = function_body(companion_cpp, "void ACompanionActor::BeginPlay")
tick_body = function_body(companion_cpp, "void ACompanionActor::Tick")
cache_body = function_body(companion_cpp, "void ACompanionActor::CacheCompanionGestureBasePose")
update_body = function_body(companion_cpp, "void ACompanionActor::UpdateCompanionGesture")
support_body = function_body(companion_cpp, "void ACompanionActor::TryFireAtNearbyZombie")
auto_medic_body = function_body(companion_cpp, "void ACompanionActor::TrySupportPlayer")
manual_medic_body = function_body(companion_cpp, "bool ACompanionActor::TryManualMedicPulse")
regroup_body = function_body(companion_cpp, "void ACompanionActor::RegroupNearPlayer")
hold_body = function_body(companion_cpp, "void ACompanionActor::SetHoldPosition")
clear_body = function_body(companion_cpp, "void ACompanionActor::ClearHoldPosition")
configure_body = function_body(companion_cpp, "void ACompanionActor::ConfigureSquadPersonality")
order_bark_body = function_body(companion_cpp, "void ACompanionActor::PushRoleOrderBark")
damage_body = function_body(companion_cpp, "void ACompanionActor::TakeCompanionDamage")
grant_body = function_body(companion_cpp, "void ACompanionActor::TriggerSupportFireGesture")
medic_body = function_body(companion_cpp, "void ACompanionActor::TriggerMedicPulseGesture")
order_body = function_body(companion_cpp, "void ACompanionActor::TriggerOrderGesture")
flinch_body = function_body(companion_cpp, "void ACompanionActor::TriggerDamageGesture")
light_body = function_body(companion_cpp, "void ACompanionActor::RefreshRoleSignalLight")

check_all(
    companion_h,
    [
        "UPointLightComponent",
        "bEnableCompanionGestureReadability",
        "CompanionIdleGestureScale",
        "SupportFireGestureDuration",
        "MedicPulseGestureDuration",
        "OrderGestureDuration",
        "DamageGestureDuration",
        "RoleSignalLight",
        "CacheCompanionGestureBasePose",
        "UpdateCompanionGesture",
        "TriggerSupportFireGesture",
        "TriggerMedicPulseGesture",
        "TriggerOrderGesture",
        "TriggerDamageGesture",
        "RefreshRoleSignalLight",
    ],
    "companion header must expose gesture readability tuning, component, state, and helpers",
)
check_all(
    constructor_body,
    [
        "CreateDefaultSubobject<UPointLightComponent>",
        "CompanionRoleSignalLight",
        "RoleSignalLight->SetupAttachment(RootComponent)",
        "RoleSignalLightBaseIntensity",
        "RoleSignalLight->SetLightColor(RoleAccentColor)",
    ],
    "constructor must create the role signal light without altering movement setup",
)
check_all(
    begin_play_body,
    ["RefreshRoleSignalLight();", "CacheCompanionGestureBasePose(true)"],
    "begin play must refresh role color and cache visual base poses",
)
check_all(
    tick_body,
    [
        "SupportFireGestureTimer",
        "MedicPulseGestureTimer",
        "OrderGestureTimer",
        "DamageGestureTimer",
        "UpdateCompanionGesture(DeltaSeconds)",
        "TryFireAtNearbyZombie(PlayerPawn)",
        "TrySupportPlayer(PlayerPawn)",
    ],
    "tick must decay gesture timers and update companion readability after gameplay behavior",
)
check_all(
    cache_body,
    [
        "MeshGestureBaseTransform",
        "RoleSignalLightBaseTransform",
        "RoleSignalLightBaseIntensity",
        "CompanionGestureReadabilityComponent",
        "CompanionFormationLocomotionPoseTarget",
        "CompanionOrderGestureSlotRuntime",
        "CompanionRoleSignalLightRuntime",
        "CompanionGestureReadabilityRuntime",
        "CompanionIdleFollowHoldPose",
        "CharacterAnimationDeepDive",
    ],
    "cache must store mesh/light base transforms and audit tags",
)
check_all(
    update_body,
    [
        "GetVelocity().Size2D()",
        "MoveAlpha",
        "SupportPulse",
        "MedicPulse",
        "OrderPulse",
        "DamagePulse",
        "HoldPose",
        "MeshComp->SetRelativeTransform",
        "RoleSignalLight->SetRelativeTransform",
        "RoleSignalLight->SetIntensity",
    ],
    "gesture update must drive idle/follow/hold, support, medic, order, damage, and light states",
)
check_none(
    update_body,
    ["SetActorLocation", "SetActorRotation", "AddMovementInput", "GetCapsuleComponent"],
    "gesture update must not own root movement or capsule collision",
)
check_all(
    support_body,
    ["ApplyRescueDamage", "TriggerSupportFireGesture();", "GetSupportFireBark"],
    "support fire must trigger a visible recoil cue after valid combat contribution",
)
check_all(
    auto_medic_body + manual_medic_body,
    ["TriggerMedicPulseGesture();", "Medic pulse applied", "Manual medic pulse applied"],
    "automatic and manual medic support must trigger visible medic-pulse cues",
)
check_all(
    regroup_body + hold_body + clear_body + order_bark_body,
    ["TriggerOrderGesture();", "GetOrderResponseBark"],
    "regroup, hold, follow, and order barks must trigger visible acknowledgment cues",
)
check_all(
    damage_body,
    ["TriggerDamageGesture();", "Companion %s has fallen"],
    "damage handling must trigger a visual flinch before death cleanup",
)
check_all(
    configure_body + light_body,
    [
        "RoleAccentColor = InRoleAccentColor",
        "CompanionRole_",
        "RefreshRoleSignalLight();",
        "RoleSignalLight->SetLightColor(RoleAccentColor)",
        "CompanionRoleSignalLightRuntime",
    ],
    "squad personality must drive the in-world role signal color and tags",
)
check_all(
    grant_body + medic_body + order_body + flinch_body,
    [
        "CompanionSupportFirePose",
        "CompanionMedicPulsePose",
        "CompanionOrderAcknowledgedPose",
        "CompanionDamageFlinchPose",
        "CompanionGestureReadabilityRuntime",
    ],
    "trigger helpers must tag all companion gesture states",
)
check_all(
    plan,
    ["rescue squad companion gesture readability", "verify_companion_gesture_readability_slice_pass.py"],
    "creative inclusion plan must include the companion gesture slice",
)
check_all(
    manifest + squad_manifest,
    [
        "Companion idle follow hold pose",
        "Role signal light",
        "Support-fire pose",
        "Medic-pulse pose",
        "Order acknowledgment pose",
        "Damage flinch pose",
        "CompanionGestureReadabilityRuntime",
    ],
    "manifests must document companion gesture readability surfaces",
)
check_all(
    onboarding + qa + visual,
    [
        "CompanionGestureReadability",
        "role signal light",
        "support-fire recoil",
        "medic pulse",
        "order acknowledgment",
    ],
    "onboarding, human QA, and visual review must cover companion gesture cues",
)
check_all(
    local_ci + full_qa,
    ["python3 Scripts/verify_companion_gesture_readability_slice_pass.py"],
    "local CI and full QA must run the companion gesture readability verifier",
)
check_all(
    progress + doc,
    [
        "Companion gesture readability slice",
        "CHARACTER_ANIMATION_DEEPDIVE",
        "CompanionGestureReadabilityRuntime",
        "verify_companion_gesture_readability_slice_pass.py",
    ],
    "progress and documentation must summarize the slice",
)

if errors:
    print("[verify_companion_gesture_readability_slice_pass] FAIL", file=sys.stderr)
    for error in errors:
        print(f" - {error}", file=sys.stderr)
    sys.exit(1)

print("[verify_companion_gesture_readability_slice_pass] OK")
