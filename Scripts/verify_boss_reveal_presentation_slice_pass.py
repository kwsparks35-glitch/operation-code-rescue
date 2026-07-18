#!/usr/bin/env python3
"""Static verifier for the boss reveal presentation slice."""

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


actor_h = read(SRC / "BossRevealPresentationActor.h")
actor_cpp = read(SRC / "BossRevealPresentationActor.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "BOSS_REVEAL_PRESENTATION_SLICE.md")

constructor_body = function_body(actor_cpp, "ABossRevealPresentationActor::ABossRevealPresentationActor")
tick_body = function_body(actor_cpp, "void ABossRevealPresentationActor::Tick")
configure_body = function_body(actor_cpp, "void ABossRevealPresentationActor::ConfigureReveal")
begin_reveal_body = function_body(actor_cpp, "void ABossRevealPresentationActor::BeginReveal")
spawn_boss_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnBossForCity")

check_all(
    actor_h,
    [
        "ABossRevealPresentationActor",
        "TriggerRadius",
        "DurationSeconds",
        "bReducedMotion",
        "OptionalSequencerRevealAsset",
        "ConfigureReveal",
        "OnBossRevealStarted",
        "BossActor",
        "ArenaRing",
        "ThreatGateA",
        "ThreatGateB",
        "BossCrown",
        "WarningLight",
    ],
    "boss reveal header must expose presentation state and future cinematic hooks",
)
check_all(
    constructor_body,
    [
        "PrimaryActorTick.bCanEverTick = true",
        "BossRevealArenaRing",
        "BossRevealThreatGateA",
        "BossRevealThreatGateB",
        "BossRevealSweepA",
        "BossRevealCrown",
        "/Engine/BasicShapes/Cylinder.Cylinder",
        "/Engine/BasicShapes/Cube.Cube",
        "/Engine/BasicShapes/Sphere.Sphere",
        "ConfigureRevealComponent",
        "SequencerReadyFallback",
        "ControlRigReadyFallback",
        "CharacterAnimationDeepDive",
    ],
    "boss reveal constructor must build a cook-safe cinematic fallback rig",
)
check_all(
    tick_body,
    [
        "UGameplayStatics::GetPlayerPawn",
        "FVector::DistSquared",
        "TriggerRadius",
        "BeginReveal",
        "bReducedMotion ? 0.24f : 1.0f",
        "SetRelativeScale3D",
        "SetRelativeLocation",
        "SetRelativeRotation",
        "SetIntensity",
        "Destroy()",
    ],
    "boss reveal tick must trigger by proximity, animate, respect reduced motion, and self-clean",
)
check_all(
    configure_body,
    [
        "BossActor = InBossActor",
        "CityIndex = InCityIndex",
        "CityName = InCityName",
        "BossTitle = InBossTitle",
        "WarningColor = InWarningColor",
        "bReducedMotion = bInReducedMotion",
        "ApplyComponentTint",
    ],
    "ConfigureReveal must store boss context and apply mission colors",
)
check_all(
    begin_reveal_body,
    [
        "bRevealStarted = true",
        "ApplyRevealVisualState(true)",
        "BossRevealTriggered",
        "OnBossRevealStarted(BossActor)",
        "AddOnScreenDebugMessage",
    ],
    "BeginReveal must start the beat, expose Blueprint hook, and add audit tags",
)
check("BossRevealPresentationActor.h" in gamemode_cpp,
      "game mode must include the boss reveal actor")
check_all(
    spawn_boss_body,
    [
        "SpawnActor<ABossRevealPresentationActor>",
        "ABossRevealPresentationActor::StaticClass()",
        "Reveal->ConfigureReveal",
        "GI && GI->bReducedMotion",
        "BossRevealPresentationLayer",
        "CharacterAnimationDeepDive",
        "Top50Recommendations",
        "RegisterStreamedActor(Reveal)",
    ],
    "SpawnBossForCity must create and register the boss reveal presentation layer",
)
check("verify_boss_reveal_presentation_slice_pass.py" in full_qa,
      "full QA must run the boss reveal presentation verifier")
check("verify_boss_reveal_presentation_slice_pass.py" in local_ci,
      "local CI must run the boss reveal presentation verifier")
check("Boss reveal presentation slice" in progress,
      "progress log must document the boss reveal presentation slice")
check_all(
    slice_doc,
    [
        "CHARACTER_ANIMATION_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "boss reveal",
        "Sequencer",
        "Control Rig",
        "reduced motion",
        "proximity",
    ],
    "slice doc must map boss reveal presentation to the June 25 guidance",
)

if errors:
    for error in errors:
        print(f"[verify_boss_reveal_presentation_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_boss_reveal_presentation_slice_pass] PASS: boss reveal presentation slice verified")
