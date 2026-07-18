#!/usr/bin/env python3
"""Static verifier for the world bible and lore slice."""

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
manifest = read(DATA / "world_bible_lore_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
performance_budget = read(DATA / "performance_city_layer_budget.tsv")
character_manifest = read(DATA / "novel_character_world_design_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "WORLD_BIBLE_LORE_SLICE.md")
world_doc = read(SOURCE_DOC_DIR / "WORLD_DEVELOPMENT_DEEPDIVE.md")
top50_doc = read(SOURCE_DOC_DIR / "TOP_50_RECOMMENDATIONS_2026-06-25.md")

spawn_city_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnCampaignCity")
lore_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnWorldBibleLoreLayer")

check_all(
    gamemode_h,
    [
        "SpawnEnvironmentalStorytellingLayer",
        "SpawnWorldBibleLoreLayer",
    ],
    "game mode header must declare the world bible lore layer beside environmental storytelling",
)

story_idx = spawn_city_body.find("SpawnEnvironmentalStorytellingLayer(Mission, CityIndex, Origin, CityLabel)")
lore_idx = spawn_city_body.find("SpawnWorldBibleLoreLayer(Mission, CityIndex, Origin, CityLabel)")
creative_idx = spawn_city_body.find("SpawnCreativeRecommendationImplementationLayer(Mission, CityIndex, Origin, CityLabel)")
check(story_idx >= 0, "campaign city must still spawn environmental storytelling")
check(lore_idx > story_idx >= 0, "world bible lore should spawn after environmental storytelling")
check(creative_idx > lore_idx >= 0, "world bible lore should spawn before the broad creative recommendation showcase")

check_all(
    lore_body,
    [
        "WorldBibleLoreLayer",
        "CanonicalLoreContract",
        "WorldBibleAndLoreGuidance",
        "CodingAsEmpowermentPillar",
        "SurvivingEngineersNetwork",
        "AutomationAntagonistForce",
        "InfectedPressureForce",
        "TechnologyRulesReadable",
        "PerCityLoreData",
        "Top50Recommendation36",
        "WorldDevelopmentDeepDive",
        "RuntimeDataLayer_Mode_Storytelling",
        "RuntimeDataLayer_Mode_RescueTraversal",
        "ApplyRuntimeDataLayerTags",
        "OBJECTIVE WORLD BIBLE PREMISE",
        "NAVIGATION PILLARS",
        "SAFEHOUSE FACTIONS / FORCES",
        "EXTRACTION TECH RULES",
        "SURVIVOR CITY CHAPTER",
        "Mission.MissionBrief",
        "Mission.RadioBriefing",
        "Mission.CharacterStoryPlan",
        "Mission.SurvivorName",
        "[CodeRescueWorldBibleLore]",
    ],
    "lore layer must build tagged, mission-aware world bible cards",
)
check_all(
    lore_body,
    [
        "SpawnTexturedBlock",
        "SpawnBlock",
        "SpawnGuideText",
        "APointLight",
        "SetActorEnableCollision(false)",
    ],
    "lore layer must create visible collisionless props, text, and lights",
)
check_all(
    manifest,
    [
        "Premise",
        "Pillars",
        "FactionsForces",
        "TechnologyRules",
        "PerCityLoreData",
        "SpawnWorldBibleLoreLayer",
        "verify_world_bible_lore_slice_pass.py",
    ],
    "world bible lore manifest must document canonical lore contract cards",
)
check_all(
    creative_plan,
    [
        "world bible and lore",
        "verify_world_bible_lore_slice_pass.py plus verify_environmental_storytelling_slice_pass.py",
    ],
    "creative plan must route world bible and lore through the new verifier",
)
check_all(
    visual_manifest,
    [
        "WorldBibleLore",
        "premise, pillars, factions/forces, technology rules, and per-city lore data",
    ],
    "visual manifest must include world bible lore target",
)
check(
    "WorldBibleLore" in human_qa
    and "selected-language save" in human_qa
    and "nonblocking" in human_qa,
    "human QA must include a world bible lore playtest row",
)
check_all(
    performance_budget,
    [
        "WorldBibleLore",
        "five lore cards",
    ],
    "performance budget must include world bible lore layer",
)
check_all(
    character_manifest,
    [
        "The Signal Concord",
        "The Orphaned Automata",
        "CanonicalLoreContract",
        "SpawnWorldBibleLoreLayer",
    ],
    "character/world design manifest must record lore factions and forces",
)
check_all(
    slice_doc,
    [
        "SpawnWorldBibleLoreLayer",
        "premise",
        "factions/forces",
        "technology rules",
        "per-city lore data",
        "Honesty Boundary",
        "verify_world_bible_lore_slice_pass.py",
    ],
    "slice doc must describe implementation, scope boundary, and validation",
)
check_all(
    world_doc,
    [
        'A believable premise for "why coding rescues people"',
        "Pillars:",
        "Factions / forces",
        "Technology rules",
        "Per-city lore as data",
    ],
    "source world deep-dive must still contain the world-bible guidance used by this slice",
)
check_all(
    top50_doc,
    [
        "World bible & lore",
        "premise, factions",
        "why coding",
        "465 cities",
    ],
    "source Top 50 recommendations must still contain the world-bible item used by this slice",
)
check("verify_world_bible_lore_slice_pass.py" in full_qa,
      "full QA audit must run the world bible lore verifier")
check("verify_world_bible_lore_slice_pass.py" in local_ci,
      "local CI readiness must run the world bible lore verifier")
check_all(
    progress,
    [
        "World bible lore slice",
        "SpawnWorldBibleLoreLayer",
        "verify_world_bible_lore_slice_pass.py",
    ],
    "progress log must record the world bible lore implementation",
)

if errors:
    for error in errors:
        print(f"ERROR: {error}")
    sys.exit(1)

print("World bible lore slice verification passed.")
