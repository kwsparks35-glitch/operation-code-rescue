#!/usr/bin/env python3
"""Static verifier for the boss phase telegraph/readability slice."""

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


boss_h = read(SRC / "BossZombieActor.h")
boss_cpp = read(SRC / "BossZombieActor.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "BOSS_PHASE_TELEGRAPH_SLICE.md")

constructor_body = function_body(boss_cpp, "ABossZombieActor::ABossZombieActor")
begin_play_body = function_body(boss_cpp, "void ABossZombieActor::BeginPlay")
enter_phase_body = function_body(boss_cpp, "void ABossZombieActor::EnterPhase")
tick_body = function_body(boss_cpp, "void ABossZombieActor::Tick")
start_body = function_body(boss_cpp, "void ABossZombieActor::StartPhaseTelegraph")
update_body = function_body(boss_cpp, "void ABossZombieActor::UpdatePhaseTelegraph")
visibility_body = function_body(boss_cpp, "void ABossZombieActor::ApplyPhaseTelegraphVisibility")
configure_body = function_body(boss_cpp, "void ABossZombieActor::ConfigurePhaseTelegraphComponent")
tint_body = function_body(boss_cpp, "void ABossZombieActor::ApplyPhaseTelegraphTint")

check_all(
    boss_h,
    [
        "PhaseTelegraphDuration",
        "PhaseTelegraphRadius",
        "OnBossPhaseTelegraphStarted",
        "PhaseTelegraphRing",
        "PhaseTelegraphCore",
        "PhaseTelegraphSweep",
        "PhaseTelegraphAddBeaconA",
        "PhaseTelegraphAddBeaconB",
        "PhaseTelegraphLight",
        "bPhaseTelegraphReducedMotion",
        "StartPhaseTelegraph",
        "UpdatePhaseTelegraph",
        "ApplyPhaseTelegraphVisibility",
    ],
    "boss header must expose phase telegraph tuning, components, and authored-event hook",
)
check_all(
    constructor_body,
    [
        "PrimaryActorTick.bCanEverTick = true",
        "BossPhaseTelegraphRing",
        "BossPhaseTelegraphCore",
        "BossPhaseTelegraphSweep",
        "BossPhaseTelegraphAddBeaconA",
        "BossPhaseTelegraphLight",
        "/Engine/BasicShapes/Cylinder.Cylinder",
        "/Engine/BasicShapes/Cube.Cube",
        "/Engine/BasicShapes/Sphere.Sphere",
        "ConfigurePhaseTelegraphComponent",
        "BossPhaseTelegraph",
        "EnemyTelegraphReadability",
        "CharacterAnimationDeepDive",
        "Top50Recommendations",
    ],
    "constructor must build a cook-safe telegraph rig and audit tags",
)
check_all(
    begin_play_body,
    [
        "GetGameInstance<UCodeRescueGameInstance>",
        "GI->bReducedMotion",
        "bPhaseTelegraphReducedMotion",
        "ApplyPhaseTelegraphVisibility(false)",
    ],
    "BeginPlay must cache reduced-motion preference and start hidden",
)
check_all(
    enter_phase_body,
    [
        "StartPhaseTelegraph(Phase)",
        "MoveSpeed * 1.5f",
        "MoveSpeed * 1.2f",
        "BOSS PHASE 2 - sprint + regen",
        "BOSS PHASE 3 - spawning adds",
    ],
    "EnterPhase must start readable telegraphs while preserving phase behavior",
)
check_all(
    tick_body,
    [
        "ApplyPhaseTelegraphVisibility(false)",
        "Frac <= 0.33f",
        "Frac <= 0.66f",
        "UpdatePhaseTelegraph(DeltaSeconds)",
        "CountLivingAdds()",
        "SpawnActor<ACodeZombieActor>",
    ],
    "Tick must update/read phase telegraphs without losing existing boss logic",
)
check_all(
    start_body,
    [
        "PhaseTelegraphActivePhase = Phase",
        "PhaseTelegraphTimeRemaining",
        "FLinearColor(1.0f, 0.04f, 0.14f)",
        "FLinearColor(1.0f, 0.48f, 0.02f)",
        "ApplyPhaseTelegraphTint",
        "SetLightColor",
        "ApplyPhaseTelegraphVisibility(true)",
        "BossPhase3SpawnAddsTelegraph",
        "BossPhase2RegenSprintTelegraph",
        "OnBossPhaseTelegraphStarted(Phase)",
    ],
    "StartPhaseTelegraph must color and tag phase 2/3 warnings with future Blueprint hook",
)
check_all(
    update_body,
    [
        "bPhaseTelegraphReducedMotion ? 0.24f : 1.0f",
        "PhaseTelegraphRadius / 560.0f",
        "SetRelativeScale3D",
        "SetRelativeLocation",
        "SetRelativeRotation",
        "SetVisibility(PhaseTelegraphActivePhase >= 3",
        "SetIntensity",
    ],
    "UpdatePhaseTelegraph must animate rings, sweeps, spawn beacons, light, and reduced motion",
)
check_all(
    visibility_body,
    [
        "PhaseTelegraphRing",
        "PhaseTelegraphCore",
        "PhaseTelegraphSweep",
        "PhaseTelegraphAddBeaconA",
        "PhaseTelegraphAddBeaconB",
        "SetIntensity(bVisible ? 8000.0f : 0.0f)",
    ],
    "visibility helper must hide/show all phase telegraph primitives and light",
)
check_all(
    configure_body,
    [
        "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
        "SetGenerateOverlapEvents(false)",
        "SetMobility(EComponentMobility::Movable)",
        "SetVisibility(false, true)",
        "BossPhaseTelegraph",
        "EnemyTelegraphReadability",
    ],
    "component setup must be nonblocking and tagged for review",
)
check_all(
    tint_body,
    [
        "CreateAndSetMaterialInstanceDynamic",
        "Color",
        "BaseColor",
        "EmissiveColor",
    ],
    "tint helper must create runtime-readable emissive warning colors",
)
check("verify_boss_phase_telegraph_slice_pass.py" in full_qa,
      "full QA must run the boss phase telegraph verifier")
check("verify_boss_phase_telegraph_slice_pass.py" in local_ci,
      "local CI must run the boss phase telegraph verifier")
check("Boss phase telegraph slice" in progress,
      "progress log must document the boss phase telegraph slice")
check_all(
    slice_doc,
    [
        "CHARACTER_ANIMATION_DEEPDIVE",
        "GAME_PHYSICS_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "phase 2",
        "phase 3",
        "reduced motion",
        "telegraph",
        "Blueprint",
    ],
    "slice doc must map the boss phase telegraph work to the June 25 documents",
)

if errors:
    for error in errors:
        print(f"[verify_boss_phase_telegraph_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_boss_phase_telegraph_slice_pass] PASS: boss phase telegraph slice verified")
