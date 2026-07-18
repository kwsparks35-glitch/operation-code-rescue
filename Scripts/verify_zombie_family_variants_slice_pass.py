#!/usr/bin/env python3
"""Static verifier for the zombie family variants runtime slice."""

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


mode_h = read(SRC / "CodeRescueGameMode.h")
mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
boss_cpp = read(SRC / "BossZombieActor.cpp")
manifest = read(DATA / "zombie_family_variants_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
enemy_manifest = read(DATA / "enemy_readability_manifest.tsv")
animation_manifest = read(DATA / "animation_coverage_manifest.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "ZOMBIE_FAMILY_VARIANTS_SLICE.md")
self_source = read(PROJECT_ROOT / "Scripts/verify_zombie_family_variants_slice_pass.py")

fixed_helper = function_body(mode_cpp, "void ACodeRescueGameMode::ApplyZombieFamilyVariant")
city_helper = function_body(mode_cpp, "void ACodeRescueGameMode::ApplyCityZombieFamilyVariant")
horde_body = function_body(mode_cpp, "void ACodeRescueGameMode::TriggerBossHorde")

check_all(
    mode_h,
    [
        "ApplyZombieFamilyVariant",
        "ApplyCityZombieFamilyVariant",
        "SelectZombieVariantRow",
    ],
    "GameMode header must expose shared zombie family helpers",
)

check_all(
    mode_cpp,
    [
        "GetZombieFamilyVariantAuditTag",
        "GetZombieFamilyVariantMarkerLabel",
        "GetZombieFamilyVariantMarkerColor",
        "ZombieFamily_DogZombie",
        "ZombieFamily_UrbanZombie",
        "ZombieFamily_BusinessSuit",
        "ZombieFamily_Bloated",
        "ZombieFamily_Nurse",
        "ZombieFamily_EliteSpitter",
        "ZombieFamily_EliteCharger",
        "ZombieFamily_EliteBoomer",
        "ZombieFamily_Default",
    ],
    "GameMode must define labels, colors, and audit tags for every zombie family",
)

check_all(
    fixed_helper,
    [
        "FindZombieVariantRow",
        "InitializeFromVariant",
        "ZombieFamilyVariantRuntime",
        "CityZombieFamilyVariant",
        "CharacterAnimationDeepDive",
        "Top50Recommendations",
        "GetZombieFamilyVariantAuditTag",
        "RecordZombieVariant",
        "bPersistAssignment",
    ],
    "fixed variant helper must initialize, tag, and optionally persist assignments",
)

check_all(
    city_helper,
    [
        "SelectZombieVariantRow",
        "InitializeFromVariant",
        "CityWeightedZombieFamily",
        "ZombieFamilyVariantRuntime",
        "CityZombieFamilyVariant",
        "GetZombieFamilyVariantAuditTag",
        "RecordZombieVariant",
        "bPersistAssignment",
    ],
    "city weighted helper must initialize, tag, and optionally persist assignments",
)

for context_tag in [
    "PhysicsLaneZombieFamily",
    "EncounterDirectorZombieFamily",
    "RegularCityZombieFamily",
    "BossZombieFamily",
    "EliteWardenMiniBossFamily",
    "DogDenZombieFamily",
    "LanguageBreachZombieFamily",
    "BossHordeZombieFamily",
]:
    check(context_tag in mode_cpp, f"missing runtime context tag {context_tag}")

check_all(
    horde_body,
    [
        "ApplyCityZombieFamilyVariant",
        "BossHordeZombieFamily",
        "false",
        "GetZombieFamilyVariantMarkerLabel",
        "GetZombieFamilyVariantMarkerColor",
        "CityZombieFamilyVariant",
    ],
    "terminal-solve horde must use weighted variants without save persistence",
)

check_all(
    zombie_cpp,
    [
        "EliteBoomerSpawnFamily",
        "ZombieFamilyVariantRuntime",
        "CityZombieFamilyVariant",
        "ZombieFamily_Default",
    ],
    "boomer death adds must carry family-readability tags",
)

check_all(
    boss_cpp,
    [
        "BossPhaseAddFamily",
        "ZombieFamilyVariantRuntime",
        "CityZombieFamilyVariant",
        "ZombieFamily_Default",
    ],
    "boss phase adds must carry family-readability tags",
)

check_all(
    manifest,
    [
        "RegularCityWave",
        "PhysicsLaneAmbush",
        "EncounterDirector",
        "BossWarden",
        "EliteWardenMiniBosses",
        "DogDenSetPiece",
        "LanguageBreachPatrol",
        "TerminalSolveHorde",
        "BoomerDeathAdds",
        "BossPhaseAdds",
        "transient only",
        "persist assigned variant",
    ],
    "zombie family manifest must cover saved and transient spawn contexts",
)

check("verify_zombie_family_variants_slice_pass.py" in creative_plan, "creative plan must include the new verifier")
check("verify_character_promotion_validation_unreal.py" in creative_plan, "creative plan must preserve promoted DataTable validation")
check("ZombieFamilyVariants" in enemy_manifest, "enemy readability manifest must include ZombieFamilyVariants")
check("zombie_family_dog" in animation_manifest and "zombie_family_elites" in animation_manifest, "animation manifest must cover family variant rows")
check("ZombieFamilyVariants" in human_qa, "human QA checklist must include ZombieFamilyVariants")
check("ZombieFamilyVariants" in visual_targets, "visual regression targets must include ZombieFamilyVariants")
check("verify_zombie_family_variants_slice_pass.py" in full_qa, "full QA audit must run the zombie family verifier")
check("verify_zombie_family_variants_slice_pass.py" in local_ci, "local CI readiness must run the zombie family verifier")
check("Zombie family variants slice" in progress, "progress log must include the zombie family variants slice")
check("ZOMBIE_FAMILY_VARIANTS_SLICE.md" in self_source, "verifier must check the slice documentation file")
check_all(
    slice_doc,
    [
        "ApplyZombieFamilyVariant",
        "ApplyCityZombieFamilyVariant",
        "zombie_family_variants_manifest.tsv",
        "save-backed",
        "transient",
        "Future Art Pass",
    ],
    "slice documentation must describe runtime implementation, persistence, and future art pass",
)

if errors:
    for error in errors:
        print(f"[zombie-family-variants] ERROR: {error}")
    sys.exit(1)

print("[zombie-family-variants] OK")
