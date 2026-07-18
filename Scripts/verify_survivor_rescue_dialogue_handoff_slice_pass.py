#!/usr/bin/env python3
"""Static verifier for the survivor rescue dialogue and extraction handoff slice."""

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


survivor_cpp = read(SRC / "SurvivorActor.cpp")
curriculum_manifest = read(DATA / "curriculum_feedback_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
character_manifest = read(DATA / "novel_character_world_design_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "SURVIVOR_RESCUE_DIALOGUE_HANDOFF_SLICE.md")

rescue_body = function_body(survivor_cpp, "bool ASurvivorActor::Rescue")
locked_body = function_body(survivor_cpp, "FString BuildSurvivorLockedRouteLine")
rescue_line_body = function_body(survivor_cpp, "FString BuildSurvivorRescueLine")
dispatch_body = function_body(survivor_cpp, "FString BuildExtractionDispatchLine")
companion_body = function_body(survivor_cpp, "FString BuildCompanionHandoffLine")

check_all(
    survivor_cpp,
    [
        "GetMissionCityLabel",
        "GetMissionConceptLabel",
        "GetMissionLandmarkLabel",
        "GetMissionTerminalLabel",
        "GetMissionPayoffLabel",
        "GetSelectedLanguageLabel",
        "BuildStoryTail",
        "BuildSurvivorLockedRouteLine",
        "BuildSurvivorRescueLine",
        "BuildExtractionDispatchLine",
        "BuildCompanionHandoffLine",
    ],
    "survivor implementation must define mission-aware dialogue helpers",
)
check_all(
    locked_body,
    [
        "Route is still locked",
        "GetMissionTerminalLabel",
        "GetMissionConceptLabel",
        "GetSelectedLanguageLabel",
        "GetMissionLandmarkLabel",
        "BuildStoryTail",
    ],
    "locked survivor line must name the terminal, concept, language, landmark, and story",
)
check_all(
    rescue_line_body,
    [
        "Your %s fix held",
        "GetSelectedLanguageLabel",
        "GetMissionConceptLabel",
        "GetMissionLandmarkLabel",
        "GetMissionPayoffLabel",
        "BuildStoryTail",
    ],
    "successful rescue line must connect language solve, concept, landmark, world payoff, and survivor story",
)
check_all(
    dispatch_body,
    [
        "[Dispatch]:",
        "Helipad route is live",
        "save updated",
        "journal dossier marked RESCUED",
        "GetMissionCityLabel",
        "GetSelectedLanguageLabel",
    ],
    "dispatch handoff must call out extraction, save continuity, dossier state, city, and language",
)
check_all(
    companion_body,
    [
        "I'll cover the %s route",
        "Beacon first, then next city",
        "GetMissionConceptLabel",
    ],
    "first companion handoff must stay contextual instead of generic",
)
check_all(
    rescue_body,
    [
        "const FCodeRescueCityMission* Mission = FCodeRescueCampaign::GetMission(CityIndex)",
        "BuildSurvivorLockedRouteLine(",
        "ArchetypeFieldNeed",
        "BuildSurvivorRescueLine(",
        "ArchetypeRescueSkill",
        "ArchetypeDossierHook",
        "Presentation->ConfigurePresentation(SurvivorName, CityIndex, PresentationAccent, GI && GI->bReducedMotion)",
        "Helipad->SetExtractionReady(SurvivorName, PresentationAccent, GI && GI->bReducedMotion)",
        "GI->MarkSurvivorRescued(SurvivorName)",
        "GI->SavePersistentRun()",
        "BuildExtractionDispatchLine(SurvivorName, ArchetypeTitle, Mission, GI)",
        "BuildCompanionHandoffLine(SurvivorName, ArchetypeTitle, Mission)",
        "SetActorHiddenInGame(true)",
    ],
    "rescue flow must add dialogue while preserving presentation, helipad activation, save, companion, and hide behavior",
)
check_all(
    curriculum_manifest,
    [
        "SurvivorRescueHandoff",
        "BuildSurvivorRescueLine + BuildExtractionDispatchLine",
        "language solve, city landmark, world payoff, helipad route, save continuity, and dossier state",
    ],
    "curriculum feedback manifest must document survivor rescue handoff coverage",
)
check_all(
    onboarding,
    [
        "post-rescue line names the active language fix",
        "dispatch confirms helipad route, save update, and RESCUED dossier",
    ],
    "first-ten-minutes onboarding must include rescue dialogue expectations",
)
check_all(
    visual_manifest,
    [
        "SurvivorRescueHandoff",
        "Active-language rescue subtitle, dispatch handoff, and extraction beacon",
    ],
    "visual regression targets must include the rescue handoff review surface",
)
check("survivor rescue subtitle names route, language, helipad, save, and dossier state" in human_qa,
      "human QA checklist must ask reviewers to inspect rescue handoff subtitles")
check_all(
    creative_plan,
    [
        "city radio and survivor barks",
        "verify_survivor_rescue_dialogue_handoff_slice_pass.py",
    ],
    "creative development inclusion plan must track survivor bark/handoff implementation",
)
check_all(
    character_manifest,
    [
        "frontline survivor rescue identity",
        "survivor rescue dialogue handoff",
    ],
    "character/world manifest must record the survivor identity handoff support",
)
check("verify_survivor_rescue_dialogue_handoff_slice_pass.py" in full_qa,
      "full QA must run the survivor rescue dialogue handoff verifier")
check("verify_survivor_rescue_dialogue_handoff_slice_pass.py" in local_ci,
      "local CI must run the survivor rescue dialogue handoff verifier")
check("Survivor rescue dialogue handoff slice" in progress,
      "progress log must document the survivor rescue dialogue handoff slice")
check_all(
    slice_doc,
    [
        "Survivor Rescue Dialogue Handoff Slice",
        "BuildSurvivorLockedRouteLine",
        "BuildSurvivorRescueLine",
        "BuildExtractionDispatchLine",
        "BuildCompanionHandoffLine",
        "CHARACTER_ANIMATION_DEEPDIVE",
        "WORLD_DEVELOPMENT_DEEPDIVE",
        "TOP_50_RECOMMENDATIONS",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER",
        "Validation",
        "Human QA Notes",
    ],
    "slice doc must explain implementation, source guidance, validation, and QA",
)

if errors:
    for error in errors:
        print(f"[verify_survivor_rescue_dialogue_handoff_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_survivor_rescue_dialogue_handoff_slice_pass] PASS: survivor rescue dialogue handoff slice verified")
