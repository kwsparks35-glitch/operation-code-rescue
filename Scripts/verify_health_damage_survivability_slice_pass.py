#!/usr/bin/env python3
"""Static verifier for health gauge and non-instant damage survivability."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"

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
hud_cpp = read(SRC / "CodeRescueHUDWidget.cpp")
death_cpp = read(SRC / "CodeRescueDeathWidget.cpp")
manifest = read(DATA / "health_damage_survivability_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
safe_learning_manifest = read(DATA / "safe_learning_city_controls_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC / "HEALTH_DAMAGE_SURVIVABILITY_SLICE.md")
progress = read(PROJECT_ROOT / "progress.md")

apply_damage_body = function_body(character_cpp, "void ACodeRescueCharacter::ApplyDamage")
hud_refresh_body = function_body(hud_cpp, "void UCodeRescueHUDWidget::RefreshHUD")
death_construct_body = function_body(death_cpp, "void UCodeRescueDeathWidget::NativeConstruct")

check_all(
    character_h,
    [
        "float Health = 250.0f",
        "float MaxHealth = 250.0f",
        "DamageMercyWindowSeconds = 0.85f",
        "MaxEnemyDamagePerHitFraction = 0.16f",
        "bPreventSingleHitEnemyDeaths = true",
        "bAutoUseEmergencyMedkit = true",
        "EmergencyMedkitHealthFraction = 0.18f",
        "EmergencyMedkitCooldownSeconds = 18.0f",
        "GetLastDamageMitigationText",
        "LastDamageMitigationText",
    ],
    "character header must expose survivability settings and mitigation readback",
)

check_all(
    apply_damage_body,
    [
        "bInsideMercyWindow",
        "DamageAmount * 0.25f",
        "bDamageCapped",
        "MaxHealth * MaxEnemyDamagePerHitFraction",
        "bArmorPlateAbsorbed",
        "ArmorDamageReduction",
        "bSingleHitSurvivalLocked",
        "MaxHealth * 0.14f",
        "bEmergencyMedkitReady",
        "bEmergencyMedkitUsed",
        "Emergency medkit deployed",
        "MitigationNotes",
        "mercy window",
        "per-hit cap",
        "armor plate",
        "survival lock",
        "emergency medkit",
        "LastDamageMitigationText",
        "Damage taken from %s",
        "Mission failed: operative down.",
    ],
    "ApplyDamage must implement non-instant survivability and readable mitigation text",
)

check_all(
    hud_refresh_body,
    [
        "TEXT(\"PLAYER HEALTH  %.0f / %.0f   %.0f%%   %s\")",
        "HudVitalStateLabel(HealthPct)",
        "HealthBar->SetPercent(HealthPct)",
        "ATTACKED FROM %s | %.0f dmg | %s%s%s",
        "GetLastDamageMitigationText",
        "MitigationSuffix",
        "MitigationText.ToUpper()",
        "DamageAlertColor",
    ],
    "HUD must show health state and mitigation-aware damage alerts",
)

check_all(
    death_construct_body,
    [
        "RESUME FROM LANGUAGE SAVE",
        "START FRESH LANGUAGE RUN",
        "SAVE THIS LANGUAGE RUN AND QUIT",
        "QUIT TO DESKTOP",
        "BuildDeathLanguageSummary",
        "BuildDeathStatsText",
    ],
    "death screen must preserve replay/save/quit choices",
)

check_all(
    manifest,
    [
        "Health gauge readability",
        "Non-instant enemy damage",
        "Mitigation readability",
        "Emergency recovery",
        "Death replay choices",
        "Damage accessibility",
        "verify_health_damage_survivability_slice_pass.py",
    ],
    "health survivability manifest must describe runtime contracts and validation",
)

check_all(
    creative_plan,
    [
        "health gauge and non-instant zombie damage",
        "verify_health_damage_survivability_slice_pass.py",
        "verify_june01_rescue_survivability_pass.py",
        "manual combat survivability review",
    ],
    "creative plan must route the health/damage row through the new verifier",
)

check_all(
    human_qa,
    [
        "HealthDamageSurvivability",
        "mercy window",
        "per-hit cap",
        "emergency medkit",
        "death save/quit options",
    ],
    "human QA checklist must include health survivability playtest coverage",
)

check_all(
    visual_targets,
    [
        "HealthDamageSurvivability",
        "PLAYER HEALTH readout",
        "ATTACKED FROM alert",
        "mitigation text",
        "death resume/save/quit panel",
    ],
    "visual regression targets must include health survivability capture",
)

check_all(
    safe_learning_manifest,
    [
        "health_gauge",
        "themed health/stamina vitals",
        "enemy damage is capped per hit",
        "A single zombie interaction should not immediately kill a healthy player",
    ],
    "safe learning controls manifest must still document health gauge safety",
)

check_all(
    slice_doc,
    [
        "Health Damage Survivability Slice",
        "LastDamageMitigationText",
        "mercy window",
        "per-hit cap",
        "armor plate",
        "survival lock",
        "emergency medkit",
        "death screen resume/save/quit choices",
    ],
    "slice documentation must explain survivability behavior and validation",
)

check("verify_health_damage_survivability_slice_pass.py" in full_qa,
      "full QA must run the health survivability verifier")
check("verify_health_damage_survivability_slice_pass.py" in local_ci,
      "local CI must run the health survivability verifier")
check("Health damage survivability slice" in progress,
      "progress log must document this slice")

if errors:
    for error in errors:
        print(f"[verify_health_damage_survivability_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_health_damage_survivability_slice_pass] PASS: health damage survivability verified")
