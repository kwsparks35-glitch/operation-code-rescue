#!/usr/bin/env python3
"""Static verifier for the gameplay collision channel contract slice."""

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


header = read(SRC / "CodeRescueCollisionChannels.h")
config = read(PROJECT_ROOT / "Config/DefaultEngine.ini")
character = read(SRC / "CodeRescueCharacter.cpp")
ai_controller = read(SRC / "CodeRescueAIController.cpp")
zombie = read(SRC / "CodeZombieActor.cpp")
barricade = read(SRC / "BarricadeActor.cpp")
pickup = read(SRC / "PickupActor.cpp")
case_file = read(SRC / "CaseFilePickupActor.cpp")
terminal = read(SRC / "CodingTerminalActor.cpp")
survivor = read(SRC / "SurvivorActor.cpp")
friendly = read(SRC / "FriendlyNPCActor.cpp")
helipad = read(SRC / "HelipadActor.cpp")
jeep = read(SRC / "JeepActor.cpp")
manifest = read(DATA / "collision_channel_gameplay_contract_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
physics_contract = read(DATA / "physics_promotion_validation_contract.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
doc = read(DOC_DIR / "COLLISION_CHANNEL_GAMEPLAY_CONTRACT_SLICE.md")

check_all(
    header,
    [
        "namespace CodeRescueCollision",
        "PlayerPawnObject = ECC_GameTraceChannel1",
        "ZombiePawnObject = ECC_GameTraceChannel2",
        "CoverObject = ECC_GameTraceChannel3",
        "PickupObject = ECC_GameTraceChannel4",
        "WeaponTrace = ECC_GameTraceChannel5",
        "AISightTrace = ECC_GameTraceChannel6",
        "InteractionTrace = ECC_GameTraceChannel7",
    ],
    "collision helper must centralize the configured channel mapping",
)
check_all(
    config,
    [
        'Name="PlayerPawn"',
        'Name="ZombiePawn"',
        'Name="CoverObject"',
        'Name="PickupObject"',
        'Name="WeaponTrace"',
        'Name="AISightTrace"',
        'Name="InteractionTrace"',
    ],
    "engine config must define project object and trace channels",
)

trace_forward = function_body(character, "bool ACodeRescueCharacter::TraceForward")
fire_body = function_body(character, "void ACodeRescueCharacter::Fire")
ai_visible = function_body(ai_controller, "bool ACodeRescueAIController::IsPlayerVisible")
zombie_barricade = function_body(zombie, "ABarricadeActor* FindBlockingBarricadeBetween")
zombie_elite = function_body(zombie, "bool ACodeZombieActor::TickEliteBehavior")

check_all(
    character,
    [
        '#include "CodeRescueCollisionChannels.h"',
        "PlayerPawnObject",
        "CollisionChannel_PlayerPawnObject",
    ],
    "player character must include collision helper and mark the player object channel",
)
check("CodeRescueCollision::InteractionTrace" in trace_forward and "ECC_Visibility" not in trace_forward,
      "player interaction trace must use InteractionTrace instead of ECC_Visibility")
check("CodeRescueCollision::WeaponTrace" in fire_body,
      "player weapon fire must use WeaponTrace")
check("CodeRescueCollision::AISightTrace" in ai_visible and "ECC_Visibility" not in ai_visible,
      "AI visibility must use AISightTrace instead of ECC_Visibility")
check("CodeRescueCollision::AISightTrace" in zombie_barricade,
      "zombie barricade sight check must use AISightTrace")
check("CodeRescueCollision::WeaponTrace" in zombie_elite,
      "elite spitter acid hitscan must use WeaponTrace")

check_all(
    zombie,
    [
        "ZombiePawnObject",
        "CollisionChannel_ZombiePawnObject",
        "CodeRescueCollision::WeaponTrace",
        "CodeRescueCollision::AISightTrace",
    ],
    "zombie capsule and combat traces must use the project collision contract",
)
check_all(
    barricade,
    [
        "CoverObject",
        "CollisionChannel_CoverObject",
        "CodeRescueCollision::WeaponTrace",
        "CodeRescueCollision::AISightTrace",
    ],
    "barricades must be typed as cover and block combat/sight traces",
)
for label, source in {
    "pickup": pickup,
    "case file": case_file,
}.items():
    check_all(
        source,
        [
            "PickupObject",
            "CodeRescueCollision::InteractionTrace",
            "CollisionChannel_PickupObject",
            "CollisionChannel_InteractionTraceTarget",
        ],
        f"{label} trigger must use PickupObject and InteractionTrace",
    )
for label, source in {
    "terminal": terminal,
    "survivor": survivor,
    "friendly NPC": friendly,
    "helipad": helipad,
    "jeep": jeep,
}.items():
    check_all(
        source,
        [
            "CodeRescueCollision::InteractionTrace",
            "CollisionChannel_InteractionTraceTarget",
        ],
        f"{label} must explicitly block the interaction trace",
    )

check_all(
    manifest,
    [
        "Weapon traces",
        "AI sight traces",
        "Interaction targeting",
        "Object typing",
        "verify_collision_channel_gameplay_contract_slice_pass.py",
    ],
    "manifest must describe channel coverage",
)
check("collision channel gameplay contract" in creative_plan,
      "creative inclusion plan must include the collision channel gameplay contract")
check("CollisionChannelGameplayContract" in visual_targets,
      "visual regression targets must include collision channel gameplay contract")
check("CollisionChannelGameplayContract" in human_qa,
      "human QA checklist must include collision channel gameplay contract")
check("Collision channel gameplay contract" in physics_contract,
      "physics promotion contract must include collision channel gameplay contract")
check("verify_collision_channel_gameplay_contract_slice_pass.py" in full_qa,
      "full QA must run the collision channel verifier")
check("verify_collision_channel_gameplay_contract_slice_pass.py" in local_ci,
      "local CI must run the collision channel verifier")
check("Collision channel gameplay contract slice" in progress,
      "progress log must document the collision channel slice")
check("GAME_PHYSICS_DEEPDIVE" in doc and "recommendation 21" in doc,
      "slice doc must map the work to the physics guidance and Top 50 recommendation")

if errors:
    for error in errors:
        print(f"[verify_collision_channel_gameplay_contract_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_collision_channel_gameplay_contract_slice_pass] PASS: gameplay collision channel contract verified")
