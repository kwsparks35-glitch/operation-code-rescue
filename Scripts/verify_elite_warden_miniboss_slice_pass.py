#!/usr/bin/env python3
"""Static verifier for the elite warden mini-boss staging slice."""

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


gamemode_h = read(SRC / "CodeRescueGameMode.h")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(DATA / "elite_warden_miniboss_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
enemy_manifest = read(DATA / "enemy_readability_manifest.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
accessibility_manifest = read(DATA / "accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "ELITE_WARDEN_MINIBOSS_SLICE.md")

spawn_city_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnCampaignCity")
staging_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnEliteWardenMiniBossStagingLayer")

check("SpawnEliteWardenMiniBossStagingLayer" in gamemode_h,
      "game mode header must declare elite warden mini-boss staging layer")
boss_idx = spawn_city_body.find("SpawnBossForCity(CityIndex, Origin, CityLabel, Mission)")
staging_idx = spawn_city_body.find("SpawnEliteWardenMiniBossStagingLayer(Mission, CityIndex, Origin, CityLabel)")
check(boss_idx >= 0, "campaign city spawn must still call SpawnBossForCity")
check(staging_idx > boss_idx >= 0, "elite warden staging must spawn after the boss actor")

check_all(
    staging_body,
    [
        "SolvedTerminalIds.Contains(Mission.TerminalId)",
        "NeutralizedZombieIds.Contains(BossId)",
        "ACTIVE AFTER TERMINAL INTEL",
        "DORMANT UNTIL TERMINAL INTEL",
        "WARDEN DEFEATED",
        "ELITE WARDEN RUNWAY",
        "INTEL LOCK GATE",
        "MINI-BOSS SENTINEL LANE",
        "WARDEN PHASE GATE",
    ],
    "staging layer must be driven by terminal-intel and boss-defeat save state",
)
check_all(
    staging_body,
    [
        "EliteWardenMiniBossStaging",
        "EliteWardenPressureGate",
        "MiniBossAfterIntelMilestone",
        "TextFirstEnemyReadability",
        "NoAccessBlocker",
        "CharacterAnimationDeepDive",
        "Top50Recommendations",
        "EliteWardenSignalLight",
        "APointLight",
        "SpawnTexturedBlock",
        "SpawnBlock",
        "SpawnGuideText",
        "[CodeRescueEliteWardenMiniBoss]",
    ],
    "staging layer must be tagged, lit, text-first, nonblocking, and logged",
)
check_all(
    staging_body,
    [
        "CHARGER MINI-BOSS",
        "SPITTER MINI-BOSS",
        "BOOMER MINI-BOSS",
        "EZombieVariant::EliteCharger",
        "EZombieVariant::EliteSpitter",
        "EZombieVariant::EliteBoomer",
        "ECodeRescueZombieEncounterRole::Pressure",
        "ECodeRescueZombieEncounterRole::Sentinel",
        "ECodeRescueZombieEncounterRole::Anchor",
        "ConfigureEncounterDirective",
        "ApplyZombieFamilyVariant",
        "EliteWardenMiniBossFamily",
        "CodeRescueBossZombieIdBase + 100000 + CityIndex * 10 + i",
        "WardenRunwaySentinel",
        "UCodeRescueSubtitlesWidget::Push",
    ],
    "staging layer must spawn post-intel elite mini-bosses with variants, roles, save IDs, and dispatch feedback",
)

check_all(
    manifest,
    [
        "WardenIntelLockGate",
        "WardenMiniBossSentinelLane",
        "WardenPhaseGate",
        "EliteWardenChargerMiniBoss",
        "EliteWardenSpitterMiniBoss",
        "EliteWardenBoomerMiniBoss",
        "SpawnEliteWardenMiniBossStagingLayer",
    ],
    "elite warden mini-boss manifest must document gates and sentinels",
)
check_all(
    creative_plan,
    [
        "elite warden and mini-bosses",
        "verify_elite_warden_miniboss_slice_pass.py plus verify_boss_reveal_presentation_slice_pass.py plus verify_boss_phase_telegraph_slice_pass.py plus packaged smoke plus manual playtest",
    ],
    "creative plan must route elite warden mini-bosses through the new verifier",
)
check_all(
    enemy_manifest,
    [
        "EliteWardenMiniBoss",
        "SpawnEliteWardenMiniBossStagingLayer",
        "terminal-intel gated charger, spitter, and boomer sentinels",
    ],
    "enemy readability manifest must include elite warden mini-boss readability",
)
check_all(
    visual_manifest,
    [
        "EliteWardenMiniBosses",
        "intel lock gate, mini-boss sentinel lane, warden phase gate",
    ],
    "visual regression targets must include elite warden mini-boss staging",
)
check_all(
    human_qa,
    [
        "EliteWardenMiniBosses",
        "charger, spitter, and boomer mini-boss sentinels",
    ],
    "human QA checklist must include elite warden mini-boss playtest",
)
check_all(
    accessibility_manifest,
    [
        "EliteWardenMiniBossAccessibility",
        "text-first enemy labels",
        "nonblocking pressure gates",
    ],
    "accessibility manifest must document elite warden readability",
)
check("verify_elite_warden_miniboss_slice_pass.py" in full_qa,
      "full QA must run the elite warden mini-boss verifier")
check("verify_elite_warden_miniboss_slice_pass.py" in local_ci,
      "local CI must run the elite warden mini-boss verifier")
check_all(
    progress,
    [
        "Elite warden mini-boss staging slice",
        "SpawnEliteWardenMiniBossStagingLayer",
        "INTEL LOCK GATE",
        "CHARGER MINI-BOSS",
        "SPITTER MINI-BOSS",
        "BOOMER MINI-BOSS",
    ],
    "progress log must record the elite warden mini-boss staging slice",
)
check_all(
    slice_doc,
    [
        "Elite Warden Mini-Boss Slice",
        "SpawnEliteWardenMiniBossStagingLayer",
        "Intel Lock Gate",
        "Mini-Boss Sentinel Lane",
        "Warden Phase Gate",
        "Validation",
    ],
    "slice documentation must explain implementation and validation",
)

if errors:
    for error in errors:
        print(f"[verify_elite_warden_miniboss_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_elite_warden_miniboss_slice_pass] PASS: elite warden mini-boss slice verified")
