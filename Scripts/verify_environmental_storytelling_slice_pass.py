#!/usr/bin/env python3
"""Static verifier for the environmental storytelling slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOC_DIR = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"
SOURCE_DOC_DIR = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-25"

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
manifest = read(DATA / "environmental_storytelling_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
performance_budget = read(DATA / "performance_city_layer_budget.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "ENVIRONMENTAL_STORYTELLING_SLICE.md")
world_doc = read(SOURCE_DOC_DIR / "WORLD_DEVELOPMENT_DEEPDIVE.md")
top50_doc = read(SOURCE_DOC_DIR / "TOP_50_RECOMMENDATIONS_2026-06-25.md")

spawn_city_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnCampaignCity")
story_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnEnvironmentalStorytellingLayer")

check_all(
    gamemode_h,
    [
        "SpawnEnvironmentalStorytellingLayer",
        "SpawnCollectibleCaseFilesForCity",
    ],
    "game mode header must declare the environmental storytelling layer near narrative case files",
)

case_idx = spawn_city_body.find("SpawnCollectibleCaseFilesForCity(Mission, CityIndex, Origin, CityLabel)")
story_idx = spawn_city_body.find("SpawnEnvironmentalStorytellingLayer(Mission, CityIndex, Origin, CityLabel)")
creative_idx = spawn_city_body.find("SpawnCreativeRecommendationImplementationLayer(Mission, CityIndex, Origin, CityLabel)")
check(case_idx >= 0, "campaign city must still spawn collectible case files")
check(story_idx > case_idx >= 0, "environmental storytelling should spawn after case files")
check(creative_idx > story_idx >= 0, "environmental storytelling should spawn before the broad creative recommendation showcase")

check_all(
    story_body,
    [
        "EnvironmentalStorytellingLayer",
        "CodingRescuesPeoplePremise",
        "WorldBiblePillar",
        "AutomationFailureScene",
        "TechnologyRuleReadable",
        "NonBlockingWorldStoryCue",
        "Top50Recommendation35",
        "WorldDevelopmentDeepDive",
        "RuntimeDataLayer_Mode_Storytelling",
        "RuntimeDataLayer_State_Prerecovery",
        "RuntimeDataLayer_Mode_CodingSafehouse",
        "RuntimeDataLayer_Mode_RescueTraversal",
        "ApplyRuntimeDataLayerTags",
        "OBJECTIVE AUTOMATION FAILURE",
        "SAFEHOUSE ENGINEER NETWORK",
        "SURVIVOR STAKE",
        "EXTRACTION CODE CAUSE/EFFECT",
        "CITY CHAPTER NAVIGATION",
        "Mission.MissionBrief",
        "Mission.TerminalTitle",
        "Mission.RadioBriefing",
        "Mission.SurvivorName",
        "Mission.CharacterStoryPlan",
        "Mission.LandmarkName",
        "Mission.RegionName",
        "Mission.DistrictStyle",
        "Mission.NovelGameplayDetail",
        "[CodeRescueEnvironmentalStorytelling]",
    ],
    "story layer must build mission-aware, tagged, readable environmental storytelling beats",
)
check_all(
    story_body,
    [
        "SpawnTexturedBlock",
        "SpawnBlock",
        "SpawnGuideText",
        "APointLight",
        "SetActorEnableCollision(false)",
        "false)",
    ],
    "story layer must create visible nonblocking props, text, and lights",
)
check_all(
    manifest,
    [
        "AutomationFailure",
        "EngineerNetwork",
        "SurvivorStake",
        "CodeCauseEffect",
        "CityChapter",
        "SpawnEnvironmentalStorytellingLayer",
        "RuntimeDataLayer_Mode_Storytelling",
        "verify_environmental_storytelling_slice_pass.py",
    ],
    "environmental storytelling manifest must document the five story beats",
)
check_all(
    creative_plan,
    [
        "environmental storytelling",
        "verify_environmental_storytelling_slice_pass.py plus verify_case_file_collectibles_slice_pass.py",
    ],
    "creative plan must route environmental storytelling through the new verifier",
)
check_all(
    visual_manifest,
    [
        "EnvironmentalStorytelling",
        "automation failure, safehouse engineer network, survivor stake, code cause/effect, and city chapter",
    ],
    "visual manifest must include the environmental storytelling target",
)
check(
    "EnvironmentalStorytelling" in human_qa
    and "selected-language save" in human_qa
    and "nonblocking" in human_qa,
    "human QA must include an environmental storytelling playtest row",
)
check_all(
    performance_budget,
    [
        "EnvironmentalStorytelling",
        "five nonblocking story cards",
    ],
    "performance budget must include the environmental storytelling layer",
)
check_all(
    slice_doc,
    [
        "SpawnEnvironmentalStorytellingLayer",
        "automation failure",
        "safehouse engineer network",
        "survivor stake",
        "code cause/effect",
        "Honesty Boundary",
        "verify_environmental_storytelling_slice_pass.py",
    ],
    "slice doc must describe implementation, scope boundary, and validation",
)
check_all(
    world_doc,
    [
        'A believable premise for "why coding rescues people"',
        "survival-horror dread",
        "Technology rules",
        "Per-city lore as data",
        "show, not tell",
        "environmental storytelling",
    ],
    "source world deep-dive must still contain the narrative guidance used by this slice",
)
check_all(
    top50_doc,
    [
        "Environmental storytelling",
        '"coding rescues people"',
        "World bible & lore",
    ],
    "source Top 50 recommendations must still contain the storytelling items used by this slice",
)
check("verify_environmental_storytelling_slice_pass.py" in full_qa,
      "full QA audit must run the environmental storytelling verifier")
check("verify_environmental_storytelling_slice_pass.py" in local_ci,
      "local CI readiness must run the environmental storytelling verifier")
check_all(
    progress,
    [
        "Environmental storytelling slice",
        "SpawnEnvironmentalStorytellingLayer",
        "verify_environmental_storytelling_slice_pass.py",
    ],
    "progress log must record the environmental storytelling implementation",
)

if errors:
    for error in errors:
        print(f"ERROR: {error}")
    sys.exit(1)

print("Environmental storytelling slice verification passed.")
