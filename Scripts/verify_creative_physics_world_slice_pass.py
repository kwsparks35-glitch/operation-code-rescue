#!/usr/bin/env python3
"""Static verifier for the June 30 creative physics/world vertical slice."""

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


throwable_h = read(SRC / "ThrowableActor.h")
throwable_cpp = read(SRC / "ThrowableActor.cpp")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
config = read(PROJECT_ROOT / "Config/DefaultEngine.ini")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "CREATIVE_PHYSICS_WORLD_VERTICAL_SLICE.md")

throw_body = function_body(character_cpp, "void ACodeRescueCharacter::ThrowActive")
launch_body = function_body(throwable_cpp, "void AThrowableActor::LaunchThrowable")
pulse_body = function_body(throwable_cpp, "void AThrowableActor::FireUtilityPulse")
zombie_pulse_body = function_body(throwable_cpp, "void AThrowableActor::ApplyZombieUtilityPulse")
yard_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnPhysicsTraversalYard")
safehouse_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnEnterableCivicSafehouse")

check_all(
    throwable_h,
    [
        "ThrowImpulseStrength",
        "ThrowUpwardImpulse",
        "UtilityPulseDelay",
        "UtilityPulseRadius",
        "UtilityPulseImpulseStrength",
        "FlarePulseDamage",
        "SmokePulseDamage",
        "LaunchThrowable",
        "FireUtilityPulse",
        "ApplyZombieUtilityPulse",
    ],
    "throwable header must expose tuned physics and pulse properties",
)
check_all(
    launch_body,
    [
        "SetSimulatePhysics(true)",
        "SetPhysicsLinearVelocity",
        "AddImpulse",
        "AddAngularImpulseInDegrees",
        "[CodeRescuePhysicsVerticalSlice]",
    ],
    "throwable launch must apply real physics impulse and log the slice",
)
check_all(
    pulse_body,
    [
        "OverlapMultiByObjectType",
        "ECC_PhysicsBody",
        "ECC_WorldDynamic",
        "AddRadialImpulse",
        "ERadialImpulseFalloff::RIF_Linear",
        "ApplyZombieUtilityPulse",
        "CodeRescuePhysicsVerticalSlice",
    ],
    "throwable pulse must affect nearby physics bodies and enemies",
)
check_all(
    zombie_pulse_body,
    [
        "TActorIterator<ACodeZombieActor>",
        "ApplyRescueDamage",
        "LaunchCharacter",
        "UtilityPulseRadius",
    ],
    "throwable pulse must connect to zombie damage/stagger feedback",
)

check("SpawnActorDeferred<AThrowableActor>" in throw_body, "throw path must use deferred spawn")
check("T->Kind = KindToThrow;" in throw_body, "throw path must set kind before FinishSpawningActor")
check("UGameplayStatics::FinishSpawningActor" in throw_body, "throw path must finish deferred spawn")
check("T->LaunchThrowable(Forward);" in throw_body, "throw path must call the named launch API")
check(
    throw_body.find("T->Kind = KindToThrow;") < throw_body.find("UGameplayStatics::FinishSpawningActor"),
    "throwable kind must be assigned before BeginPlay runs",
)
check(
    throw_body.find("*CountPtr -= 1;") > throw_body.find("if (T)"),
    "throwable inventory should be consumed only after a successful spawn",
)

check_all(
    yard_body,
    [
        "EnableTrainingPhysics",
        "SetCollisionProfileName(TEXT(\"PhysicsActor\"))",
        "SetSimulatePhysics(true)",
        "Throwable Physics Target",
        "ThrowablePhysicsTarget",
        "PhysicsDeepDiveC23",
        "RadialImpulseTrainingProp",
        "THROWABLE PHYSICS LANE",
        "X SLOT utility pulses",
    ],
    "physics traversal yard must include tagged throwable training props",
)
check_all(
    safehouse_body,
    [
        "Enterable Safehouse Utility Bench",
        "Enterable Safehouse Flare Training Prop",
        "Enterable Safehouse Smoke Training Prop",
        "Enterable Safehouse Stim Training Prop",
        "UTILITY BENCH",
        "X SLOT: flare lures, smoke staggers, stim restores",
    ],
    "safehouse must teach the utility loop in-world",
)

check_all(
    config,
    [
        "bSubstepping=True",
        "MaxSubstepDeltaTime=0.016667",
        "MaxSubsteps=6",
        'Name="PlayerPawn"',
        'Name="ZombiePawn"',
        'Name="CoverObject"',
        'Name="PickupObject"',
        'Name="WeaponTrace"',
        'Name="AISightTrace"',
        'Name="InteractionTrace"',
    ],
    "engine config must include the physics/collision foundation",
)
check("verify_creative_physics_world_slice_pass.py" in full_qa, "full QA must run this verifier")
check("verify_creative_physics_world_slice_pass.py" in local_ci, "local CI must run this verifier")
check("Creative throwable physics/world vertical slice" in progress, "progress log must document this pass")
check("GAME_PHYSICS_DEEPDIVE C21-C23" in slice_doc, "slice doc must map work to the physics deep-dive")
check("WORLD_DEVELOPMENT_DEEPDIVE" in slice_doc, "slice doc must map work to the world deep-dive")

if errors:
    for error in errors:
        print(f"[verify_creative_physics_world_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_creative_physics_world_slice_pass] PASS: creative physics/world vertical slice verified")
