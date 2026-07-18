#!/usr/bin/env python3
"""Static verifier for the friendly NPC gesture readability slice."""

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


npc_h = read(SRC / "FriendlyNPCActor.h")
npc_cpp = read(SRC / "FriendlyNPCActor.cpp")
plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/friendly_npc_gesture_readability_manifest.tsv")
service_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/friendly_safehouse_npc_service_manifest.tsv")
qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "FRIENDLY_NPC_GESTURE_READABILITY_SLICE.md")

constructor_body = function_body(npc_cpp, "AFriendlyNPCActor::AFriendlyNPCActor")
begin_play_body = function_body(npc_cpp, "void AFriendlyNPCActor::BeginPlay")
tick_body = function_body(npc_cpp, "void AFriendlyNPCActor::Tick")
cache_body = function_body(npc_cpp, "void AFriendlyNPCActor::CacheServiceGestureBasePose")
update_body = function_body(npc_cpp, "void AFriendlyNPCActor::UpdateServiceGesture")
grant_body = function_body(npc_cpp, "void AFriendlyNPCActor::TriggerServiceGrantGesture")
denied_body = function_body(npc_cpp, "void AFriendlyNPCActor::TriggerServiceDeniedGesture")
interact_body = function_body(npc_cpp, "bool AFriendlyNPCActor::Interact")
reset_body = function_body(npc_cpp, "void AFriendlyNPCActor::ResetDailyPerk")

check_all(
    npc_h,
    [
        "virtual void Tick",
        "bEnableServiceGestureReadability",
        "ServiceIdleGestureScale",
        "ServiceGrantGestureDuration",
        "ServiceDeniedGestureDuration",
        "CacheServiceGestureBasePose",
        "UpdateServiceGesture",
        "TriggerServiceGrantGesture",
        "TriggerServiceDeniedGesture",
        "bServiceGestureBasePoseCached",
        "ServiceGrantGestureTimer",
        "ServiceDeniedGestureTimer",
        "RoleLightGestureBaseIntensity",
    ],
    "friendly NPC header must expose gesture tuning, state, and helpers",
)
check_all(
    constructor_body,
    ["PrimaryActorTick.bCanEverTick = true"],
    "friendly NPC constructor must enable lightweight gesture ticking",
)
check_all(
    begin_play_body,
    ["ApplyRoleVisualIdentity();", "CacheServiceGestureBasePose(true)", "ApplySavedServiceState()"],
    "begin play must cache role visual bases after role identity is applied",
)
check_all(
    tick_body,
    ["UpdateServiceGesture(DeltaSeconds)"],
    "tick must update safehouse support gesture readability",
)
check_all(
    cache_body,
    [
        "SkeletalGestureBaseTransform",
        "HeadGestureBaseTransform",
        "RoleBadgeGestureBaseTransform",
        "RolePropGestureBaseTransform",
        "RoleIconAGestureBaseTransform",
        "RoleIconBGestureBaseTransform",
        "RoleLightGestureBaseTransform",
        "RoleLightGestureBaseIntensity",
        "FriendlyNPCGestureReadabilityComponent",
        "SafehouseNPCServiceLightPulse",
        "FriendlyNPCGestureReadabilityRuntime",
        "SafehouseNPCIdleServicePose",
        "CharacterAnimationDeepDive",
    ],
    "cache must store visual component bases and audit tags",
)
check_all(
    update_body,
    [
        "ServiceGrantGestureTimer",
        "ServiceDeniedGestureTimer",
        "CooldownDim",
        "SkeletalBody->SetRelativeTransform",
        "Head->SetRelativeTransform",
        "RoleProp->SetRelativeTransform",
        "RoleBadge->SetRelativeTransform",
        "RoleIconA->SetRelativeTransform",
        "RoleIconB->SetRelativeTransform",
        "RoleLight->SetIntensity",
        "bPerkUsedThisDay ? 0.72f : 1.0f",
    ],
    "gesture update must drive idle, success, denial, cooldown, and light states",
)
check_none(
    update_body,
    ["\n        Body->SetRelativeTransform", "\n        Root->SetRelativeTransform"],
    "gesture update must not move root or collision body",
)
check_all(
    grant_body + denied_body,
    [
        "SafehouseNPCServiceGrantPose",
        "SelectedLanguageSupportSaveGesture",
        "SafehouseNPCServiceDeniedPose",
        "FriendlyNPCGestureReadabilityRuntime",
    ],
    "gesture triggers must mark granted and denied service states",
)
check_all(
    interact_body,
    [
        "TriggerServiceDeniedGesture();",
        "TriggerServiceGrantGesture();",
        "MarkFriendlyNPCServiceUsed(GetServiceId())",
        "Support saved to %s profile",
        "Bring me five scrap",
        "You're not hurt",
        "day-night shift",
    ],
    "interaction flow must show gestures while preserving selected-language service persistence",
)
check_all(
    reset_body,
    ["bPerkUsedThisDay = false", "SafehouseNPCDailyRefreshReady"],
    "day-night reset must refresh state and tag the visual service loop",
)
check_all(
    plan,
    [
        "friendly safehouse NPCs",
        "verify_friendly_safehouse_npc_service_slice_pass.py",
        "verify_friendly_npc_gesture_readability_slice_pass.py",
    ],
    "creative inclusion plan must route the friendly NPC row through this verifier",
)
check_all(
    manifest + service_manifest,
    [
        "Safehouse support idle pose",
        "Service grant acknowledgment",
        "Cooldown and precondition refusal",
        "Visual-only transform cache",
        "Selected-language save gesture",
        "FriendlyNPCGestureReadabilityRuntime",
    ],
    "manifests must document friendly NPC gesture readability surfaces",
)
check_all(
    qa,
    [
        "FriendlyNPCGestureReadability",
        "idle support motion",
        "success acknowledgment",
        "cooldown refusal",
        "precondition refusal",
    ],
    "human QA checklist must include friendly NPC gesture review",
)
check_all(
    visual,
    [
        "FriendlyNPCGestureReadability",
        "idle service pose",
        "service grant pose",
        "service denied pose",
        "role light pulse",
    ],
    "visual regression targets must include friendly NPC gesture screenshots",
)
check_all(
    local_ci + full_qa,
    ["python3 Scripts/verify_friendly_npc_gesture_readability_slice_pass.py"],
    "local CI and full QA must run the friendly NPC gesture readability verifier",
)
check_all(
    progress + doc,
    [
        "Friendly NPC gesture readability slice",
        "CHARACTER_ANIMATION_DEEPDIVE",
        "FriendlyNPCGestureReadabilityRuntime",
        "verify_friendly_npc_gesture_readability_slice_pass.py",
    ],
    "progress and documentation must summarize the slice",
)

if errors:
    print("[verify_friendly_npc_gesture_readability_slice_pass] FAIL", file=sys.stderr)
    for error in errors:
        print(f" - {error}", file=sys.stderr)
    sys.exit(1)

print("[verify_friendly_npc_gesture_readability_slice_pass] OK")
