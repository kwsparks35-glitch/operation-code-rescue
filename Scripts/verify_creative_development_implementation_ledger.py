#!/usr/bin/env python3
"""Verify the creative-development implementation ledger against the plan."""

from __future__ import annotations

import csv
import re
import sys
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
DATA_DIR = PROJECT_ROOT / "Content" / "CodeRescueData"
DOC_DIR = PROJECT_ROOT / "Documentation" / "improvement_pass_2026-06-30"
SCRIPT_DIR = PROJECT_ROOT / "Scripts"

PLAN_PATH = DATA_DIR / "creative_development_inclusion_plan.tsv"
LEDGER_PATH = DOC_DIR / "CREATIVE_DEVELOPMENT_IMPLEMENTATION_LEDGER.md"

PRIMARY_DOCS = {
    "protected coding safehouse and annexes": "PROTECTED_LEARNING_ZONE_AI_SLICE.md",
    "selected language terminal flow": "SELECTED_LANGUAGE_TERMINAL_FLOW_SLICE.md",
    "playable rescue operator": "PLAYABLE_OPERATOR_IDENTITY_SLICE.md",
    "survivor archetype roster": "SURVIVOR_ARCHETYPE_ROSTER_SLICE.md",
    "friendly safehouse NPCs": "FRIENDLY_SAFEHOUSE_NPC_SERVICE_SLICE.md",
    "rescue squad companion gesture readability": "COMPANION_GESTURE_READABILITY_SLICE.md",
    "standard direct-pursuit zombies": "STANDARD_DIRECT_PURSUIT_ZOMBIE_SLICE.md",
    "zombie safe-zone exclusion": "PROTECTED_LEARNING_ZONE_AI_SLICE.md",
    "zombie physical-animation hit reactions": "ZOMBIE_PHYSICAL_ANIMATION_HIT_REACTION_SLICE.md",
    "city street grid and storefront shell": "CITY_STREET_GRID_STOREFRONT_SHELL_SLICE.md",
    "human-scale doors windows stairs and cover": "HUMAN_SCALE_ROUTE_ACCESS_SLICE.md",
    "health gauge and non-instant zombie damage": "HEALTH_DAMAGE_SURVIVABILITY_SLICE.md",
    "weapon quick slots and visible armory": "WEAPON_QUICK_SLOT_ARMORY_SLICE.md",
    "death replay save-and-quit flow": "DEATH_REPLAY_SAVE_QUIT_FLOW_SLICE.md",
    "MetaHuman face and groom pass": "MAC_HAIR_CARD_COMPATIBILITY_SLICE.md",
    "IK retargeting and Control Rig slots": "RETARGET_CONTROL_RIG_SLOTS_SLICE.md",
    "zombie family variants": "ZOMBIE_FAMILY_VARIANTS_SLICE.md",
    "elite warden and mini-bosses": "ELITE_WARDEN_MINIBOSS_SLICE.md",
    "interior mission spaces": "INTERIOR_MISSION_SPACES_SLICE.md",
    "major city regional kits": "REGIONAL_CITY_KIT_IDENTITY_SLICE.md",
    "weather and lighting identity": "WEATHER_LIGHTING_IDENTITY_SLICE.md",
    "World Partition + Data Layer migration": "RUNTIME_DATA_LAYER_MIGRATION_SLICE.md",
    "challenge room concept art": "CHALLENGE_ROOM_CONCEPT_ART_SLICE.md",
    "terminal post-solve after-action debrief": "TERMINAL_POST_SOLVE_DEBRIEF_SLICE.md",
    "survivor intel dossier UI": "SURVIVOR_INTEL_DOSSIER_SLICE.md",
    "distinct weapon models and animations": "DISTINCT_WEAPON_PRESENTATION_SLICE.md",
    "combat juice and weapon feel": "COMBAT_JUICE_WEAPON_FEEL_SLICE.md",
    "tactical gear pickups": "TACTICAL_GEAR_PICKUPS_SLICE.md",
    "encounter director": "ADAPTIVE_ENCOUNTER_DIRECTOR_PRESSURE_SLICE.md",
    "stealth and avoidance": "STEALTH_AVOIDANCE_SLICE.md",
    "collision channel gameplay contract": "COLLISION_CHANNEL_GAMEPLAY_CONTRACT_SLICE.md",
    "determinism and fixed timestep physics contract": "FIXED_TIMESTEP_PHYSICS_CONTRACT_SLICE.md",
    "interactive barricades and cover": "DESTRUCTIBLE_COVER_PHYSICS_SLICE.md",
    "city radio and survivor barks": "CITY_RADIO_BARK_CADENCE_SLICE.md",
    "reactive threat music and captions": "REACTIVE_THREAT_AUDIO_MUSIC_SLICE.md",
    "city ambient zone director": "CITY_AMBIENT_ZONE_AUDIO_SLICE.md",
    "visualized sound cues": "VISUALIZED_SOUND_CUES_ACCESSIBILITY_SLICE.md",
    "mono audio accessibility": "MONO_AUDIO_ACCESSIBILITY_SLICE.md",
    "inventory map and journal polish": "INVENTORY_MAP_JOURNAL_POLISH_SLICE.md",
    "objective route toast clarity": "OBJECTIVE_ROUTE_TOAST_CLARITY_SLICE.md",
    "Houdini modular city output": "HOUDINI_MODULAR_CITY_OUTPUT_SLICE.md",
    "Maya character cleanup": "MAYA_CHARACTER_CLEANUP_SLICE.md",
    "expanded extraction set pieces": "EXPANDED_EXTRACTION_SET_PIECES_SLICE.md",
    "collectible case files": "CASE_FILE_COLLECTIBLES_SLICE.md",
    "environmental storytelling": "ENVIRONMENTAL_STORYTELLING_SLICE.md",
    "world bible and lore": "WORLD_BIBLE_LORE_SLICE.md",
    "Sequencer intros and boss reveals": "SEQUENCER_INTRO_BOSS_REVEAL_SLICE.md",
    "radio scan and rescue beacon effects": "RADIO_SCAN_RESCUE_BEACON_SLICE.md",
    "expanded accessibility options": "EXPANDED_ACCESSIBILITY_OPTIONS_SLICE.md",
    "LOD texture and shader budget pass": "MAC_ASSET_IMPORT_BUDGET_GATE_SLICE.md",
    "signed external demo preparation": "EXTERNAL_DEMO_PREFLIGHT_SLICE.md",
}


def fail(message: str) -> None:
    print(f"[verify_creative_development_implementation_ledger] FAIL: {message}")
    sys.exit(1)


def read_text(path: Path) -> str:
    if not path.exists():
        fail(f"missing required file: {path.relative_to(PROJECT_ROOT)}")
    return path.read_text(encoding="utf-8")


def main() -> None:
    plan_text = read_text(PLAN_PATH)
    ledger = read_text(LEDGER_PATH)
    rows = list(csv.DictReader(plan_text.splitlines(), delimiter="\t"))

    if len(rows) != 51:
        fail(f"expected 51 creative inclusion rows, found {len(rows)}")

    plan_inclusions = {row["inclusion"] for row in rows}
    mapped_inclusions = set(PRIMARY_DOCS)
    missing_mappings = sorted(plan_inclusions - mapped_inclusions)
    extra_mappings = sorted(mapped_inclusions - plan_inclusions)
    if missing_mappings:
        fail("ledger doc mapping is missing: " + ", ".join(missing_mappings))
    if extra_mappings:
        fail("ledger doc mapping has unknown inclusions: " + ", ".join(extra_mappings))

    for source_pdf in (
        "CHARACTER_ANIMATION_DEEPDIVE.pdf",
        "GAME_PHYSICS_DEEPDIVE.pdf",
        "OPERATION_CODE_RESCUE_RELEASE_DOSSIER.pdf",
        "TOP_50_RECOMMENDATIONS.pdf",
        "WORLD_DEVELOPMENT_DEEPDIVE.pdf",
    ):
        if source_pdf not in ledger:
            fail(f"ledger must cite source PDF {source_pdf}")

    all_verifiers: set[str] = set()
    for row in rows:
        inclusion = row["inclusion"]
        if inclusion not in ledger:
            fail(f"ledger missing inclusion row: {inclusion}")

        doc_name = PRIMARY_DOCS[inclusion]
        if doc_name not in ledger:
            fail(f"ledger row for {inclusion} missing primary doc {doc_name}")
        if not (DOC_DIR / doc_name).exists():
            fail(f"primary doc missing for {inclusion}: {doc_name}")

        verifier_names = re.findall(r"(verify_[A-Za-z0-9_]+\.py)", row.get("validation", ""))
        if not verifier_names:
            fail(f"plan row has no verifier references: {inclusion}")
        for verifier in verifier_names:
            all_verifiers.add(verifier)
            if not (SCRIPT_DIR / verifier).exists():
                fail(f"missing verifier script referenced by {inclusion}: {verifier}")

    if len(all_verifiers) != 110:
        fail(f"expected 110 unique verifier scripts from the plan, found {len(all_verifiers)}")

    for token in (
        "51 inclusion rows",
        "155 named verifier references",
        "110 unique verifier scripts",
        "STEALTH_AVOIDANCE_SLICE.md",
        "CHALLENGE_REPLAY_JOURNAL_SLICE.md",
        "verify_challenge_replay_journal_slice_pass.py",
        "LANGUAGE_PROFILE_RECAP_SLICE.md",
        "verify_language_profile_recap_slice_pass.py",
        "SURVIVOR_INTEL_ARCHIVE_SLICE.md",
        "verify_survivor_intel_archive_slice_pass.py",
        "FAIL_SAFE_OBJECTIVE_BOARD_SLICE.md",
        "verify_fail_safe_objective_board_slice_pass.py",
        "TERMINAL_PRACTICE_RUN_SLICE.md",
        "verify_terminal_practice_run_slice_pass.py",
        "TERMINAL_REWARD_CHOICE_KIOSK_SLICE.md",
        "verify_terminal_reward_choice_kiosk_slice_pass.py",
        "FIRST_SESSION_ROUTE_PREVIEW_SLICE.md",
        "verify_first_session_route_preview_slice_pass.py",
        "FIELD_CHECKLIST_HUD_SLICE.md",
        "verify_field_checklist_hud_slice_pass.py",
        "packaged render smoke",
        "human_qa_signoff_checklist.tsv",
        "visual_regression_targets.tsv",
    ):
        if token not in ledger:
            fail(f"ledger missing audit token: {token}")

    print("[verify_creative_development_implementation_ledger] PASS: creative-development ledger verified")


if __name__ == "__main__":
    main()
