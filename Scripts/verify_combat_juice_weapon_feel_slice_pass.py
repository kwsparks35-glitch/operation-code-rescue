#!/usr/bin/env python3
"""Static verifier for the combat juice and weapon feel slice."""

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
manifest = read(DATA / "combat_juice_weapon_feel_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
accessibility = read(DATA / "accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "COMBAT_JUICE_WEAPON_FEEL_SLICE.md")

tick_body = function_body(character_cpp, "void ACodeRescueCharacter::Tick")
presentation_body = function_body(character_cpp, "void ACodeRescueCharacter::UpdateFirstPersonWeaponPresentation")
fire_cue_body = function_body(character_cpp, "void ACodeRescueCharacter::TriggerCombatJuiceFireCue")
hit_cue_body = function_body(character_cpp, "void ACodeRescueCharacter::TriggerCombatJuiceHitConfirm")
reload_cue_body = function_body(character_cpp, "void ACodeRescueCharacter::TriggerCombatJuiceReloadStageCue")
damage_cue_body = function_body(character_cpp, "void ACodeRescueCharacter::TriggerCombatJuiceDamageCue")
update_body = function_body(character_cpp, "void ACodeRescueCharacter::UpdateCombatJuice")
fire_body = function_body(character_cpp, "void ACodeRescueCharacter::Fire")
melee_body = function_body(character_cpp, "void ACodeRescueCharacter::MeleeAttack")
area_body = function_body(character_cpp, "int32 ACodeRescueCharacter::ApplyAreaWeaponEffect")
damage_body = function_body(character_cpp, "void ACodeRescueCharacter::ApplyDamage")
reload_body = function_body(character_cpp, "void ACodeRescueCharacter::Reload")
reload_complete_body = function_body(character_cpp, "void ACodeRescueCharacter::OnReloadComplete")

check_all(
    character_h,
    [
        "bEnableCombatJuice",
        "CombatJuiceFireKickPitch",
        "CombatJuiceFireKickYaw",
        "CombatJuiceHitStopSeconds",
        "CombatJuiceHitConfirmKick",
        "CombatJuiceDamageKick",
        "CombatJuiceReloadSettleKick",
        "GetCombatJuiceMotionScale",
        "TriggerCombatJuiceFireCue",
        "TriggerCombatJuiceHitConfirm",
        "TriggerCombatJuiceReloadStageCue",
        "TriggerCombatJuiceDamageCue",
        "UpdateCombatJuice",
        "LastCombatJuiceFireWorldTime",
        "LastCombatJuiceHitConfirmWorldTime",
        "LastCombatJuiceReloadStageWorldTime",
        "LastCombatJuiceDamageWorldTime",
        "LastCombatJuiceHitStopDuration",
        "LastCombatJuiceHitStopScale",
        "bLastCombatJuiceHeadshot",
    ],
    "character header must expose combat juice tuning, helpers, and runtime state",
)
check("UpdateCombatJuice(DeltaSeconds);" in tick_body,
      "tick must keep combat juice windows active for runtime QA tags")
check_all(
    presentation_body,
    [
        "HitStopSpan",
        "HitStopAge",
        "HitStopAlpha",
        "HitStopOffset",
        "bLastCombatJuiceHeadshot",
        "CombatJuiceHitStopStyleCue",
        "LastCombatJuiceHitConfirmWorldTime",
    ],
    "first-person weapon presentation must use hit-confirm state for hit-stop-style weight",
)
check_all(
    fire_cue_body,
    [
        "GetCombatJuiceMotionScale",
        "AddControllerPitchInput",
        "AddControllerYawInput",
        "CombatJuiceWeaponFeelRuntime",
        "CombatJuiceFireCameraKick",
        "Top50Recommendation27CombatJuice",
        "Top50Recommendation38WeaponFeel",
    ],
    "fire cue must provide reduced-motion-aware camera kick and audit tags",
)
check_all(
    hit_cue_body,
    [
        "LastCombatJuiceHitConfirmWorldTime",
        "LastCombatJuiceHitStopDuration",
        "LastCombatJuiceHitStopScale",
        "CombatJuiceHeadshotCrunch",
        "CombatJuiceHitConfirmCue",
        "CombatJuiceHitStopStyleCue",
        "AddControllerPitchInput",
    ],
    "hit-confirm cue must record hit-stop-style state, headshot state, and camera feedback",
)
check_all(
    reload_cue_body,
    [
        "LastCombatJuiceReloadStageWorldTime",
        "CombatJuiceReloadStageCue",
        "CombatJuiceReloadCompleteCue",
        "CombatJuiceReloadStartCue",
        "CombatJuiceReloadStageFull",
        "CombatJuiceReloadStagePartial",
    ],
    "reload cue must tag start/complete reload stages",
)
check_all(
    damage_cue_body,
    [
        "LastCombatJuiceDamageWorldTime",
        "DamageSource",
        "DamageFeedbackAccessibleCameraKick",
        "CombatJuiceDamageCameraCue",
        "AddControllerYawInput",
    ],
    "damage cue must integrate direction-aware camera feedback",
)
check_all(
    update_body,
    [
        "CombatJuiceFireWindowActive",
        "CombatJuiceHitStopWindowActive",
        "CombatJuiceDamageWindowActive",
    ],
    "combat juice update must maintain runtime QA tags",
)
check_all(
    fire_body,
    [
        "TriggerCombatJuiceFireCue(WDef, true)",
        "TriggerCombatJuiceFireCue(WDef, false)",
        "TriggerCombatJuiceHitConfirm(ImpactPoint, HitZone, true)",
        "TriggerCombatJuiceHitConfirm(Hit.ImpactPoint, EHitZone::Torso, true)",
        "TriggerCombatJuiceHitConfirm(ImpactPoint, EHitZone::Torso, true)",  # 2026-07-11 refresh: assisted cone replaced by auto target lock; third real call form pinned
    ],
    "fire path must trigger fire kick and confirmed-hit feedback across weapon paths",
)
check("TriggerCombatJuiceHitConfirm(Start + Forward * 120.0f, EHitZone::Torso, true)" in melee_body,
      "melee fallback must trigger confirmed-hit combat juice")
check("TriggerCombatJuiceHitConfirm(ImpactPoint, EHitZone::Torso, true)" in area_body,
      "area weapons must trigger confirmed-hit combat juice")
check("TriggerCombatJuiceDamageCue(EffectiveDamage, DamageSource)" in damage_body,
      "player damage must trigger combat juice damage feedback")
check("TriggerCombatJuiceReloadStageCue(0.0f, false)" in reload_body,
      "reload start must trigger combat juice reload stage feedback")
check("TriggerCombatJuiceFireCue(WeaponDef, true)" in reload_body,
      "empty-reserve reload must trigger a dry combat juice cue")
check("TriggerCombatJuiceReloadStageCue(1.0f, true)" in reload_complete_body,
      "reload completion must trigger combat juice reload stage feedback")
check_all(
    manifest,
    [
        "fire_camera_kick",
        "hit_confirm_hitstop",
        "reload_stage_feedback",
        "damage_direction_kick",
        "weapon_hit_hold",
        "qa_tags",
        "GetCombatJuiceMotionScale",
    ],
    "manifest must document each combat juice signal and reduced-motion contract",
)
check(
    "combat juice and weapon feel" in creative_plan
    and "verify_combat_juice_weapon_feel_slice_pass.py plus verify_distinct_weapon_presentation_slice_pass.py plus verify_player_first_person_animation_slice_pass.py plus verify_damage_feedback_accessibility_slice_pass.py plus packaged smoke plus manual CombatJuiceWeaponFeel review" in creative_plan,
    "creative development plan must route combat juice through the new verifier and adjacent checks",
)
check_all(
    visual_targets,
    [
        "CombatJuiceWeaponFeel",
        "fire camera kick",
        "hit-stop-style weapon hold",
        "reload settle",
        "damage direction kick",
    ],
    "visual regression targets must include combat juice review",
)
check_all(
    human_qa,
    [
        "CombatJuiceWeaponFeel",
        "Fire multiple weapon families",
        "body hit, headshot, barricade hit, area hit, reload, dry fire, melee fallback, and incoming damage",
    ],
    "human QA checklist must include combat juice playtest coverage",
)
check_all(
    accessibility,
    [
        "CombatJuiceWeaponFeelAccessibility",
        "bReducedMotion + weapon HUD text + DamageFeedbackWidget",
        "Scales fire kick, hit-confirm hold, reload settle, and damage direction kick",
    ],
    "accessibility manifest must document reduced-motion combat juice behavior",
)
check("verify_combat_juice_weapon_feel_slice_pass.py" in full_qa,
      "full QA must run the combat juice verifier")
check("verify_combat_juice_weapon_feel_slice_pass.py" in local_ci,
      "local CI must run the combat juice verifier")
check("Combat juice weapon feel slice" in progress,
      "progress log must document the combat juice weapon feel slice")
check_all(
    slice_doc,
    [
        "TOP_50_RECOMMENDATIONS",
        "CHARACTER_ANIMATION_DEEPDIVE",
        "TriggerCombatJuiceFireCue",
        "TriggerCombatJuiceHitConfirm",
        "TriggerCombatJuiceReloadStageCue",
        "TriggerCombatJuiceDamageCue",
        "Reduced Motion",
        "authored weapon montages",
    ],
    "slice doc must map combat juice work to recommendations and future authored assets",
)

if errors:
    for error in errors:
        print(f"[verify_combat_juice_weapon_feel_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_combat_juice_weapon_feel_slice_pass] PASS: combat juice weapon feel slice verified")
