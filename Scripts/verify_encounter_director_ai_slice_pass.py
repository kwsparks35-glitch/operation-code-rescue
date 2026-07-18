#!/usr/bin/env python3
"""Static verifier for the authored encounter director AI slice."""

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


zombie_h = read(SRC / "CodeZombieActor.h")
zombie_cpp = read(SRC / "CodeZombieActor.cpp")
ai_cpp = read(SRC / "CodeRescueAIController.cpp")
gamemode_h = read(SRC / "CodeRescueGameMode.h")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "ENCOUNTER_DIRECTOR_AI_SLICE.md")

configure_body = function_body(zombie_cpp, "void ACodeZombieActor::ConfigureEncounterDirective")
target_body = function_body(zombie_cpp, "FVector ACodeZombieActor::ResolveEncounterMoveTarget")
director_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnEncounterDirectorLayer")

check_all(
    zombie_h,
    [
        "enum class ECodeRescueZombieEncounterRole",
        "Anchor",
        "Flanker",
        "Pressure",
        "Sentinel",
        "bHasEncounterDirective",
        "EncounterAnchorLocation",
        "EncounterLeashRadius",
        "ConfigureEncounterDirective",
        "ResolveEncounterMoveTarget",
        "HasEncounterDirective",
    ],
    "zombie header must expose encounter-director roles and tuning",
)
check_all(
    configure_body,
    [
        "AIDirectedEncounter",
        "EncounterDirectedZombie",
        "WorldDevelopmentDeepDive",
        "CharacterAnimationDeepDive",
        "EncounterRole_Anchor",
        "EncounterRole_Flanker",
        "EncounterRole_Pressure",
        "EncounterRole_Sentinel",
    ],
    "encounter directive configuration must tag role and source-doc intent",
)
check_all(
    target_body,
    [
        "ClampToLeash",
        "EncounterAnchorLocation",
        "EncounterFlankOffset",
        "ECodeRescueZombieEncounterRole::Anchor",
        "ECodeRescueZombieEncounterRole::Flanker",
        "ECodeRescueZombieEncounterRole::Pressure",
        "ECodeRescueZombieEncounterRole::Sentinel",
    ],
    "encounter role target resolver must implement anchor/flank/pressure/sentinel movement",
)
check_all(
    ai_cpp,
    [
        "HasEncounterDirective()",
        "ResolveEncounterMoveTarget(PlayerCharacter->GetActorLocation())",
        "TryMoveToLocationWithFallback",
        "MoveDirectlyToward(ZombieCharacter->ResolveEncounterMoveTarget",
    ],
    "AI controller must honor encounter directives during patrol/chase/attack",
)
check("SpawnEncounterDirectorLayer" in gamemode_h,
      "game mode header must declare the encounter director layer")
check("SpawnEncounterDirectorLayer(Mission, CityIndex, Origin, CityLabel, Survivor)" in gamemode_cpp,
      "campaign city spawn must call the encounter director layer")
check_all(
    director_body,
    [
        "EncounterDirectorLayer",
        "AIDirectedEncounter",
        "AuthoredEncounterDirector",
        "DIRECTED ENCOUNTER",
        "anchor | flank | pressure | sentinel",
        "SpawnDirectorBarricade",
        "EncounterDirectorCover",
        "SpawnDirectorPickup",
        "EncounterDirectorReward",
        "if (bSandboxMode)",
        "NeutralizedZombieIds.Contains(ZombieId)",
        "CodeRescueHordeZombieIdBase + CityIndex * 1000 + 800 + i",
        "FDirectedZombieSpec",
        "ECodeRescueZombieEncounterRole::Anchor",
        "ECodeRescueZombieEncounterRole::Flanker",
        "ECodeRescueZombieEncounterRole::Pressure",
        "ECodeRescueZombieEncounterRole::Sentinel",
        "ApplyZombieFamilyVariant",
        "EncounterDirectorZombieFamily",
        "ConfigureEncounterDirective",
        "EncounterDirectorZombie",
        "UCodeRescueSubtitlesWidget::Push",
        "[CodeRescueEncounterDirector]",
    ],
    "encounter director layer must place readable world beats, supplies, and save-aware directed zombies",
)
check("verify_encounter_director_ai_slice_pass.py" in full_qa,
      "full QA must run the encounter director verifier")
check("verify_encounter_director_ai_slice_pass.py" in local_ci,
      "local CI must run the encounter director verifier")
check("Encounter director AI slice" in progress,
      "progress log must document the encounter director AI slice")
check_all(
    slice_doc,
    [
        "GAME_PHYSICS_DEEPDIVE",
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "CHARACTER_ANIMATION_DEEPDIVE",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "TOP_50_RECOMMENDATIONS",
        "anchor",
        "flanker",
        "pressure",
        "sentinel",
    ],
    "slice doc must map the implementation to the June 25 documents",
)

if errors:
    for error in errors:
        print(f"[verify_encounter_director_ai_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_encounter_director_ai_slice_pass] PASS: authored encounter director AI verified")
