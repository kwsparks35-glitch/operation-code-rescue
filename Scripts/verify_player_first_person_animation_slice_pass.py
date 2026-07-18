#!/usr/bin/env python3
"""Static verifier for the player first-person animation/readability slice."""

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


character_h = read(SRC / "CodeRescueCharacter.h")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "PLAYER_FIRST_PERSON_ANIMATION_SLICE.md")

constructor_body = function_body(character_cpp, "ACodeRescueCharacter::ACodeRescueCharacter")
begin_play_body = function_body(character_cpp, "void ACodeRescueCharacter::BeginPlay")
tick_body = function_body(character_cpp, "void ACodeRescueCharacter::Tick")
camera_body = function_body(character_cpp, "void ACodeRescueCharacter::ApplyCameraPerspective")
arms_body = function_body(character_cpp, "void ACodeRescueCharacter::UpdateFirstPersonArms")

check_all(
    character_h,
    [
        "USkeletalMeshComponent* FirstPersonArmsMesh",
        "UpdateFirstPersonArms",
        "FirstPersonArmsSwayTime",
    ],
    "character header must declare first-person arms component and update state",
)
check_all(
    constructor_body,
    [
        "CreateDefaultSubobject<USkeletalMeshComponent>(TEXT(\"FirstPersonArmsMesh\"))",
        "SetupAttachment(FirstPersonCamera)",
        "SetOnlyOwnerSee(true)",
        "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
        "SetVisibility(false, true)",
    ],
    "constructor must create safe owner-only first-person arms mesh",
)
# 2026-07-16 migration: the Manny full-body "arms" hung its head inside the
# first-person lens (it only looked fine while the import-scale bug culled it
# invisible). First person is weapon-only until a dedicated arms rig ships —
# BeginPlay must PARK the component, not wire the mannequin.
check_all(
    begin_play_body,
    [
        "FirstPersonArmsMesh->SetSkeletalMesh(nullptr)",
        "FirstPersonArmsMesh->SetVisibility(false, true)",
        "weapon-only",
        "PlayerFirstPersonArmsMesh",
        "CharacterAnimationDeepDive",
        "FirstPersonAnimationPrototype",
    ],
    "begin play must park the first-person arms (weapon-only FP) with audit tags",
)
check("UpdateFirstPersonArms(DeltaSeconds);" in tick_body,
      "tick must update first-person arms motion")
# 2026-07-16 pin refresh: the presentation-layer passes (aim mesh, hero mesh)
# extended the body-visibility condition; pin the current authoritative line.
check_all(
    camera_body,
    [
        "BodyMesh->SetVisibility(!bFirstPerson && !bAimPresentationConfigured && !bHeroPresentationConfigured, false)",
        "FirstPersonArmsMesh->SetVisibility(bFirstPerson",
        "GetSkeletalMeshAsset()",
    ],
    "camera perspective changes must hide driver bodies and gate arms to first person",
)
check_all(
    arms_body,
    [
        "CameraPerspective != 0",
        "WalkSpeed",
        "FirstPersonArmsSwayTime",
        "BoundTurnValue",
        "BoundLookUpValue",
        "SetRelativeLocation",
        "SetRelativeRotation",
        "FirstPersonArmsProceduralSway",
    ],
    "first-person arms update must provide movement/look procedural readability",
)
check("verify_player_first_person_animation_slice_pass.py" in full_qa,
      "full QA must run the player first-person animation verifier")
check("verify_player_first_person_animation_slice_pass.py" in local_ci,
      "local CI must run the player first-person animation verifier")
check("Player first-person animation slice" in progress,
      "progress log must document the player first-person animation slice")
check_all(
    slice_doc,
    [
        "CHARACTER_ANIMATION_DEEPDIVE",
        "first-person arms",
        "SKM_Manny",
        "ABP_Manny",
        "owner-only",
        "procedural sway",
    ],
    "slice doc must map the first-person arms work to the character animation guidance",
)

if errors:
    for error in errors:
        print(f"[verify_player_first_person_animation_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_player_first_person_animation_slice_pass] PASS: player first-person animation slice verified")
