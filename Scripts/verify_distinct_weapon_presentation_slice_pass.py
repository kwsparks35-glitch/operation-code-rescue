#!/usr/bin/env python3
"""Static verifier for the distinct weapon presentation slice."""

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


character_h = read(SRC / "CodeRescueCharacter.h")
character_cpp = read(SRC / "CodeRescueCharacter.cpp")
manifest = read(DATA / "distinct_weapon_presentation_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
access_manifest = read(DATA / "accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "DISTINCT_WEAPON_PRESENTATION_SLICE.md")

constructor_body = function_body(character_cpp, "ACodeRescueCharacter::ACodeRescueCharacter")
profile_body = function_body(character_cpp, "static FCodeRescueWeaponPresentationProfile GetWeaponPresentationProfile")
sync_body = function_body(character_cpp, "void ACodeRescueCharacter::SyncActiveWeaponStateFromLoadout")
tick_body = function_body(character_cpp, "void ACodeRescueCharacter::Tick")
camera_body = function_body(character_cpp, "void ACodeRescueCharacter::ApplyCameraPerspective")
update_body = function_body(character_cpp, "void ACodeRescueCharacter::UpdateFirstPersonWeaponPresentation")
fire_body = function_body(character_cpp, "void ACodeRescueCharacter::Fire")
reload_body = function_body(character_cpp, "void ACodeRescueCharacter::Reload")
reload_complete_body = function_body(character_cpp, "void ACodeRescueCharacter::OnReloadComplete")

weapon_types = [
    "Pistol",
    "Shotgun",
    "Rifle",
    "Grenade",
    "CombatKnife",
    "HeavyHandgun",
    "BurstHandgun",
    "TacticalShotgun",
    "AutoShotgun",
    "SMG",
    "PrecisionRifle",
    "SemiAutoRifle",
    "Magnum",
    "BoltLauncher",
    "RocketLauncher",
    "IncendiaryGrenade",
    "FlashGrenade",
]
profile_tags = [
    "WeaponProfile_BalancedHandgun",
    "WeaponProfile_PumpShotgun",
    "WeaponProfile_AssaultRifle",
    "WeaponProfile_FragGrenade",
    "WeaponProfile_CombatKnife",
    "WeaponProfile_HeavyHandgun",
    "WeaponProfile_BurstHandgun",
    "WeaponProfile_TacticalShotgun",
    "WeaponProfile_AutoShotgun",
    "WeaponProfile_SMG",
    "WeaponProfile_PrecisionRifle",
    "WeaponProfile_SemiAutoRifle",
    "WeaponProfile_Magnum",
    "WeaponProfile_BoltLauncher",
    "WeaponProfile_RocketLauncher",
    "WeaponProfile_IncendiaryGrenade",
    "WeaponProfile_FlashGrenade",
]

check_all(
    character_h,
    [
        "class UStaticMeshComponent",
        "UStaticMeshComponent* FirstPersonWeaponSilhouette",
        "UpdateFirstPersonWeaponPresentation",
        "FirstPersonWeaponPresentationTime",
        "LastWeaponPresentationFireWorldTime",
        "LastWeaponPresentationReloadWorldTime",
        "LastPresentedWeapon",
        "bWeaponPresentationProfileInitialized",
    ],
    "character header must declare the first-person weapon presentation surface and timing state",
)
check_all(
    character_cpp,
    [
        "#include \"Components/StaticMeshComponent.h\"",
        "#include \"Engine/StaticMesh.h\"",
        "#include \"Materials/MaterialInstanceDynamic.h\"",
        "#include \"UObject/ConstructorHelpers.h\"",
        "struct FCodeRescueWeaponPresentationProfile",
    ],
    "character implementation must include static mesh and material support for the fallback model",
)
check_all(
    constructor_body,
    [
        "CreateDefaultSubobject<UStaticMeshComponent>(TEXT(\"FirstPersonWeaponSilhouette\"))",
        "SetupAttachment(FirstPersonCamera)",
        "SetCollisionEnabled(ECollisionEnabled::NoCollision)",
        "SetOnlyOwnerSee(true)",
        "SetVisibility(false, true)",
        "/Engine/BasicShapes/Cube.Cube",
        "DistinctWeaponSilhouette",
        "WeaponPresentationModelFallback",
        "WeaponAnimationMontageFutureHook",
        "DistinctWeaponPresentationRuntime",
        "FirstPersonWeaponSilhouetteReady",
    ],
    "constructor must create a package-safe owner-only weapon silhouette and audit tags",
)
check_all(
    profile_body,
    [f"EWeaponType::{weapon_type}" for weapon_type in weapon_types] + profile_tags,
    "weapon presentation profile table must cover every EWeaponType with named profiles",
)
check_all(
    sync_body,
    [
        "bWeaponPresentationProfileInitialized = false",
        "UpdateFirstPersonWeaponPresentation(0.0f)",
    ],
    "weapon state sync must invalidate/apply the visible profile on weapon changes",
)
check("UpdateFirstPersonWeaponPresentation(DeltaSeconds);" in tick_body,
      "tick must update weapon presentation motion")
check_all(
    camera_body,
    [
        "FirstPersonWeaponSilhouette->SetVisibility(bFirstPerson",
        "FirstPersonWeaponSilhouette->GetStaticMesh()",
        "UpdateFirstPersonWeaponPresentation(0.0f)",
    ],
    "camera perspective changes must show weapon only in first person",
)
check_all(
    update_body,
    [
        "CameraPerspective == 0",
        "GetWeaponPresentationProfile(ActiveWeapon)",
        "CreateAndSetMaterialInstanceDynamic(0)",
        "SetVectorParameterValue(TEXT(\"Color\")",
        "SetVectorParameterValue(TEXT(\"BaseColor\")",
        "SetVectorParameterValue(TEXT(\"EmissiveColor\")",
        "bReducedMotion",
        "MotionScale",
        "FireAlpha",
        "ReloadAlpha",
        "SetRelativeLocation",
        "SetRelativeRotation",
        "DistinctWeaponPresentationUpdated",
        "WeaponFireReloadMotionCue",
    ],
    "presentation update must apply profile tint, first-person visibility, recoil, reload, and reduced-motion motion",
)
check_all(
    fire_body,
    [
        "LastWeaponPresentationFireWorldTime = World->GetTimeSeconds()",
        "LastWeaponPresentationFireWorldTime = NowSeconds",
        "UpdateFirstPersonWeaponPresentation(0.0f)",
        "DistinctWeaponPresentationFireCue",
        "DistinctWeaponPresentationMeleeFallbackCue",
    ],
    "fire path must stamp weapon presentation fire and melee fallback cues",
)
check_all(
    reload_body,
    [
        "bIsReloading = true",
        "LastWeaponPresentationReloadWorldTime",
        "UpdateFirstPersonWeaponPresentation(0.0f)",
        "DistinctWeaponPresentationReloadCue",
    ],
    "reload path must stamp the reload presentation cue",
)
check("UpdateFirstPersonWeaponPresentation(0.0f)" in reload_complete_body,
      "reload completion must refresh presentation state")
check_all(
    manifest,
    weapon_types + profile_tags + [
        "visual_silhouette",
        "motion_cues",
        "accessibility_contract",
        "authoring_hook",
    ],
    "manifest must document every weapon presentation profile and future art hook",
)
check(
    "distinct weapon models and animations" in creative_plan
    and "verify_distinct_weapon_presentation_slice_pass.py plus verify_may27_tactical_arsenal_mcp_runtime.py plus verify_player_first_person_animation_slice_pass.py plus packaged smoke plus manual visual review" in creative_plan,
    "creative development plan must route distinct weapon presentation through the new verifier",
)
check_all(
    visual_targets,
    [
        "DistinctWeaponPresentation",
        "first-person silhouettes",
        "fire/reload motion",
        "17 weapon profiles",
    ],
    "visual regression targets must include distinct weapon presentation review",
)
check_all(
    human_qa,
    [
        "DistinctWeaponPresentation",
        "Cycle all weapon slots in first-person",
        "distinct stance, silhouette, recoil, reload, and melee fallback",
    ],
    "human QA checklist must include weapon presentation cycling and combat cue review",
)
check_all(
    access_manifest,
    [
        "DistinctWeaponPresentationAccessibility",
        "FirstPersonWeaponSilhouette + weapon HUD text",
        "reduced-motion fire/reload cue scaling",
    ],
    "accessibility manifest must document reduced-motion and text-backed weapon presentation",
)
check("verify_distinct_weapon_presentation_slice_pass.py" in full_qa,
      "full QA must run the distinct weapon presentation verifier")
check("verify_distinct_weapon_presentation_slice_pass.py" in local_ci,
      "local CI must run the distinct weapon presentation verifier")
check("Distinct weapon presentation slice" in progress,
      "progress log must document the distinct weapon presentation slice")
check_all(
    slice_doc,
    [
        "CHARACTER_ANIMATION_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "FirstPersonWeaponSilhouette",
        "17 weapon presentation profiles",
        "fire",
        "reload",
        "Reduced Motion",
        "authored meshes",
    ],
    "slice doc must map the work to animation/readability guidance and future authored assets",
)

if errors:
    for error in errors:
        print(f"[verify_distinct_weapon_presentation_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_distinct_weapon_presentation_slice_pass] PASS: distinct weapon presentation slice verified")
