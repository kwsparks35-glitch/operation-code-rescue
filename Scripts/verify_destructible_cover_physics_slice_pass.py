#!/usr/bin/env python3
"""Static verifier for the destructible-cover physics slice."""

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


barricade_h = read(SRC / "BarricadeActor.h")
barricade_cpp = read(SRC / "BarricadeActor.cpp")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
throwable_cpp = read(SRC / "ThrowableActor.cpp")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "DESTRUCTIBLE_COVER_PHYSICS_SLICE.md")

yard_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnPhysicsTraversalYard")

check_all(
    barricade_h + barricade_cpp,
    [
        "TakeBarricadeDamage",
        "TakeDamage",
        "OnBarricadeHit",
        "BreakApart",
        "SpawnDebrisChunk",
        "DebrisCount",
        "DebrisLifetime",
        "DebrisImpulseStrength",
        "DestructibleCover",
        "ChaosDestructionFallback",
        "ChaosReadableDestruction",
        "BarricadeCracked",
        "BarricadeCritical",
        "BarricadeDebris",
        "SetSimulatePhysics(true)",
        "AddImpulseAtLocation",
        "AddAngularImpulseInDegrees",
    ],
    "barricades must expose readable health, impact damage, and simulated debris",
)
check_all(
    character_cpp,
    [
        "TActorIterator<ABarricadeActor>",
        "Barricade->TakeBarricadeDamage(42.0f",
        "Barricade->TakeBarricadeDamage(Damage, ImpactPoint",
        "Cast<ABarricadeActor>(Hit.GetActor())",
        "Barricade->TakeBarricadeDamage(EffectiveDamage",
        "bAnyHostileHit = true",
    ],
    "player melee, area effects, and firearm traces must damage destructible cover",
)
check_all(
    throwable_cpp,
    [
        "#include \"BarricadeActor.h\"",
        "Cast<ABarricadeActor>(OtherActor)",
        "ThrowableDamagedDestructibleCover",
        "TActorIterator<ABarricadeActor>",
        "BarricadesDamaged",
        "destructible barricades",
    ],
    "throwables must damage breakable cover on impact and utility pulse",
)
check_all(
    zombie_cpp,
    [
        "#include \"BarricadeActor.h\"",
        "FindBlockingBarricadeBetween",
        "CodeRescueZombieBarricadeTrace",
        "BlockingBarricade->TakeBarricadeDamage",
        "ZombieAttackedDestructibleCover",
    ],
    "zombies must attack barricades that block line of sight to the player",
)
check_all(
    yard_body,
    [
        "SpawnDestructibleBarricade",
        "SpawnActorDeferred<ABarricadeActor>",
        "DESTRUCTIBLE COVER DRILL",
        "PhysicsLaneBreakableBarricade",
        "PhysicsAmbushBreakableBarricadeA",
        "PhysicsAmbushBreakableBarricadeB",
        "PhysicsAmbushBreakableBarricadeC",
        "ThrowableBreakableCover",
        "SurfaceWood",
    ],
    "physics-lane encounter must include authored breakable barricade training",
)
check("verify_destructible_cover_physics_slice_pass.py" in full_qa,
      "full QA must run the destructible-cover verifier")
check("verify_destructible_cover_physics_slice_pass.py" in local_ci,
      "local CI must run the destructible-cover verifier")
check("Destructible cover physics slice" in progress,
      "progress log must document the destructible-cover slice")
check("GAME_PHYSICS_DEEPDIVE" in slice_doc and "WORLD_DEVELOPMENT_DEEPDIVE" in slice_doc,
      "slice doc must map destructible cover to physics and world-development guidance")
check("CHARACTER_ANIMATION_DEEPDIVE" in slice_doc,
      "slice doc must mention readable zombie attack/death integration")

if errors:
    for error in errors:
        print(f"[verify_destructible_cover_physics_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_destructible_cover_physics_slice_pass] PASS: destructible-cover physics slice verified")
