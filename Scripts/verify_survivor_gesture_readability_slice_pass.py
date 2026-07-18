#!/usr/bin/env python3
"""Static verifier for the survivor gesture readability slice."""

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


survivor_h = read(SRC / "SurvivorActor.h")
survivor_cpp = read(SRC / "SurvivorActor.cpp")
plan = read(PROJECT_ROOT / "Content/CodeRescueData/creative_development_inclusion_plan.tsv")
manifest = read(PROJECT_ROOT / "Content/CodeRescueData/survivor_gesture_readability_manifest.tsv")
survivor_manifest = read(PROJECT_ROOT / "Content/CodeRescueData/survivor_archetype_roster_manifest.tsv")
qa = read(PROJECT_ROOT / "Content/CodeRescueData/human_qa_signoff_checklist.tsv")
visual = read(PROJECT_ROOT / "Content/CodeRescueData/visual_regression_targets.tsv")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "SURVIVOR_GESTURE_READABILITY_SLICE.md")

constructor_body = function_body(survivor_cpp, "ASurvivorActor::ASurvivorActor")
begin_play_body = function_body(survivor_cpp, "void ASurvivorActor::BeginPlay")
tick_body = function_body(survivor_cpp, "void ASurvivorActor::Tick")
cache_body = function_body(survivor_cpp, "void ASurvivorActor::CacheGestureBasePose")
update_body = function_body(survivor_cpp, "void ASurvivorActor::UpdateSurvivorGesture")
locked_body = function_body(survivor_cpp, "void ASurvivorActor::TriggerLockedGesture")
rescue_gesture_body = function_body(survivor_cpp, "void ASurvivorActor::TriggerRescueGesture")
fade_body = function_body(survivor_cpp, "void ASurvivorActor::ScheduleRescueFadeOut")
rescue_body = function_body(survivor_cpp, "bool ASurvivorActor::Rescue")

check_all(
    survivor_h,
    [
        "virtual void Tick",
        "bEnableSurvivorGestureReadability",
        "SurvivorIdleGestureScale",
        "RescueGestureDuration",
        "LockedGestureDuration",
        "CacheGestureBasePose",
        "UpdateSurvivorGesture",
        "TriggerLockedGesture",
        "TriggerRescueGesture",
        "ScheduleRescueFadeOut",
        "RescueFadeOutTimer",
    ],
    "survivor header must expose gesture readability tuning, state, and helpers",
)
check_all(
    constructor_body,
    ["PrimaryActorTick.bCanEverTick = true"],
    "survivor constructor must enable lightweight gesture ticking",
)
check_all(
    begin_play_body,
    ["CacheGestureBasePose(true)", "ScheduleNextIdleBark"],
    "begin play must cache visual base poses before idle/rescue gesture updates",
)
check_all(
    tick_body,
    ["UpdateSurvivorGesture(DeltaSeconds)"],
    "tick must update survivor gesture readability",
)
check_all(
    cache_body,
    [
        "SkeletalGestureBaseLocation",
        "HeadGestureBaseLocation",
        "LightGestureBaseLocation",
        "LightGestureBaseIntensity",
        "SurvivorGestureReadabilityComponent",
        "SurvivorGestureReadabilityRuntime",
        "CharacterAnimationDeepDive",
    ],
    "cache must store visual component bases and audit tags",
)
check_all(
    update_body,
    [
        "SurvivorIdleLifePose",
        "SurvivorLockedGesturePose",
        "SurvivorRescueGesturePose",
        "SetRelativeLocation",
        "SetRelativeRotation",
        "SetRelativeScale3D",
        "RescueLight->SetIntensity",
        "bRescued ? 0.0f",
    ],
    "gesture update must drive idle, locked, rescue, and light-pulse states",
)
check_all(
    locked_body + rescue_gesture_body,
    [
        "LockedGestureTimer",
        "RescueGestureTimer",
        "SurvivorLockedGesturePose",
        "SurvivorRescueGesturePose",
        "SelectedLanguageSurvivorHandoff",
    ],
    "trigger helpers must mark locked and rescued gesture states",
)
check_all(
    fade_body,
    [
        "RescueFadeOutTimer",
        "RescueGestureDuration",
        "SetActorHiddenInGame(true)",
        "SetActorTickEnabled(false)",
    ],
    "fade-out helper must delay hiding only long enough for rescue readability",
)
check_all(
    rescue_body,
    [
        "TriggerLockedGesture();",
        "TriggerRescueGesture();",
        "SetActorEnableCollision(false);",
        "ScheduleRescueFadeOut();",
        "SavePersistentRun",
        "BuildExtractionDispatchLine",
    ],
    "rescue flow must show gestures while preserving collision disable, save, and dispatch behavior",
)
check_all(
    plan,
    [
        "survivor archetype roster",
        "verify_survivor_gesture_readability_slice_pass.py",
    ],
    "creative inclusion plan must route the survivor row through this verifier",
)
check_all(
    manifest + survivor_manifest,
    [
        "Survivor idle life pose",
        "Locked route refusal",
        "Rescue confirmation gesture",
        "Safe fade-out",
        "SurvivorGestureReadabilityRuntime",
    ],
    "manifests must document survivor gesture readability surfaces",
)
check_all(
    qa,
    [
        "SurvivorGestureReadability",
        "idle motion",
        "locked refusal",
        "rescue gesture",
        "delayed fade-out",
    ],
    "human QA checklist must include survivor gesture review",
)
check_all(
    visual,
    [
        "SurvivorGestureReadability",
        "idle life pose",
        "locked refusal pose",
        "rescue gesture pose",
        "rescue light pulse",
    ],
    "visual regression targets must include survivor gesture screenshots",
)
check_all(
    local_ci + full_qa,
    ["python3 Scripts/verify_survivor_gesture_readability_slice_pass.py"],
    "local CI and full QA must run the survivor gesture readability verifier",
)
check_all(
    progress + doc,
    [
        "Survivor gesture readability slice",
        "CHARACTER_ANIMATION_DEEPDIVE",
        "SurvivorGestureReadabilityRuntime",
        "verify_survivor_gesture_readability_slice_pass.py",
    ],
    "progress and documentation must summarize the slice",
)

if errors:
    print("[verify_survivor_gesture_readability_slice_pass] FAIL", file=sys.stderr)
    for error in errors:
        print(f" - {error}", file=sys.stderr)
    sys.exit(1)

print("[verify_survivor_gesture_readability_slice_pass] OK")
