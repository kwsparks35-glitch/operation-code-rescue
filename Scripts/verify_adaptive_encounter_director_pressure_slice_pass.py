#!/usr/bin/env python3
"""Static verifier for adaptive encounter director pressure."""

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


game_mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
director_body = function_body(game_mode_cpp, "void ACodeRescueGameMode::SpawnEncounterDirectorLayer")
manifest = read(DATA / "encounter_director_adaptive_pressure_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC / "ADAPTIVE_ENCOUNTER_DIRECTOR_PRESSURE_SLICE.md")
progress = read(PROJECT_ROOT / "progress.md")

check_all(
    director_body,
    [
        "bTerminalSolvedForDirector",
        "SolvedTerminalIds.Contains(Mission.TerminalId)",
        "bResumeHasResources",
        "LastPlayerHealth",
        "LastPlayerAmmo",
        "LastPlayerMedkits",
        "LastPlayerArmorPlates",
        "DirectorObjectiveState",
        "DirectorResourceState",
        "GetDifficultyDisplayName()",
    ],
    "director must read objective, difficulty, and resumed player resource state",
)

check_all(
    director_body,
    [
        "EncounterDirectorAdaptivePressure",
        "ObjectiveStateAwareEncounter",
        "EncounterDirectorRouteOpenPressure",
        "EncounterDirectorRouteLockedPressure",
        "EncounterDirectorResourceRelief",
        "EncounterDirectorStandardResources",
        "DIRECTOR STATE",
    ],
    "director actors must expose adaptive pressure tags and readable board text",
)

check_all(
    director_body,
    [
        "EncounterDirectorReliefMedkit",
        "EncounterDirectorReliefAmmo",
        "EncounterDirectorReliefStim",
        "EPickupKind::Medkit",
        "EPickupKind::Stim",
        "if (bDirectorReliefEnabled)",
    ],
    "low-resource resumes must receive explicit relief pickups",
)

check_all(
    director_body,
    [
        "ObjectivePressureScale",
        "ResourceReliefPressureScale",
        "AdaptiveEncounterScale",
        "AdaptiveSpeedScale",
        "AdaptiveActivationScale",
        "Zombie->Health",
        "Zombie->AttackDamage",
        "Zombie->MoveSpeed",
        "Zombie->ActivationRange",
        "adaptive_pressure",
    ],
    "directed zombies must use adaptive health, damage, speed, activation, and log evidence",
)

check_all(
    manifest,
    [
        "Objective-aware pressure",
        "Low-resource relief cache",
        "Adaptive enemy tuning",
        "Readable review surface",
        "Runtime evidence",
        "verify_adaptive_encounter_director_pressure_slice_pass.py",
    ],
    "adaptive encounter manifest must describe contracts and validation",
)

check_all(
    creative_plan,
    [
        "encounter director",
        "verify_adaptive_encounter_director_pressure_slice_pass.py",
        "verify_encounter_director_ai_slice_pass.py",
        "manual encounter pacing review",
    ],
    "creative plan must route the encounter director row through the adaptive verifier",
)

check_all(
    human_qa,
    [
        "EncounterDirectorAdaptivePressure",
        "DIRECTOR STATE board",
        "low-health or low-ammo save",
        "route-open pressure",
    ],
    "human QA checklist must include adaptive director playtest coverage",
)

check_all(
    visual_targets,
    [
        "EncounterDirectorAdaptivePressure",
        "DIRECTED ENCOUNTER deck",
        "DIRECTOR STATE board",
        "relief pickups",
    ],
    "visual regression targets must include adaptive director pressure",
)

check_all(
    slice_doc,
    [
        "Adaptive Encounter Director Pressure Slice",
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "GAME_PHYSICS_DEEPDIVE",
        "CHARACTER_ANIMATION_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "AdaptiveEncounterScale",
        "EncounterDirectorReliefMedkit",
    ],
    "slice documentation must map adaptive director behavior to source guidance",
)

check("verify_adaptive_encounter_director_pressure_slice_pass.py" in full_qa,
      "full QA must run the adaptive encounter director verifier")
check("verify_adaptive_encounter_director_pressure_slice_pass.py" in local_ci,
      "local CI must run the adaptive encounter director verifier")
check("Adaptive encounter director pressure slice" in progress,
      "progress log must document this slice")

if errors:
    for error in errors:
        print(f"[verify_adaptive_encounter_director_pressure_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_adaptive_encounter_director_pressure_slice_pass] PASS: adaptive encounter director pressure verified")
