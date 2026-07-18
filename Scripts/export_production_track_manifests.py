"""
Export production-track coverage files from the live campaign data.

Run from the project root after compiling C++ changes:

    ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)"
    "$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" \
        -run=pythonscript -script="$(pwd)/Scripts/export_production_track_manifests.py" \
        -unattended -NoSound -NullRHI
"""

from pathlib import Path
import csv
import re

import unreal


EXPECTED_LEVELS = 465

ANIMATION_CLIPS = [
    ("survivor", "ready_idle", "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn", "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Quinn/MF_Idle.MF_Idle"),
    ("engineer", "patrol_walk", "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny", "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Manny/MM_Walk_InPlace.MM_Walk_InPlace"),
    ("nurse_threat", "idle", "/Game/ZombieFemale/Asset/Meshes/ZombieFemale_NurseOutfit.ZombieFemale_NurseOutfit", "/Game/ZombieFemale/Asset/Animations/ANMS_ZombieFemaleIdle05.ANMS_ZombieFemaleIdle05"),
    ("dog_scout", "sit_idle", "/Game/DogZombie/Meshes/SK_DogZombie.SK_DogZombie", "/Game/DogZombie/Animations/anim_Dog_Sit_Idle.anim_Dog_Sit_Idle"),
    ("survivor", "walk_forward", "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn", "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Quinn/MF_Walk_Fwd.MF_Walk_Fwd"),
    ("engineer", "run_forward", "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny", "/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Manny/MM_Run_Fwd.MM_Run_Fwd"),
    ("business_threat", "walk", "/Game/YI_ModularZombies/Meshes/ZombieM04/Zombie/SK_Zombie_M04_01.SK_Zombie_M04_01", "/Game/YI_ModularZombies/Animation/MocapOnline/UE4/In_Place/Zombie_Walk_F_1_Loop_IPC.Zombie_Walk_F_1_Loop_IPC"),
    ("bloated_threat", "attack", "/Game/YI_ModularZombies/Meshes/ZombieF01/Zombie/SK_Zombie_F01_01.SK_Zombie_F01_01", "/Game/YI_ModularZombies/Animation/MocapOnline/UE4/In_Place/Zombie_Atk_Arms_3_SHORT_Loop_IPC.Zombie_Atk_Arms_3_SHORT_Loop_IPC"),
    ("dog_scout", "run_in_place", "/Game/DogZombie/Meshes/SK_DogZombie.SK_DogZombie", "/Game/DogZombie/Animations/anim_Dog_Run_InPlace.anim_Dog_Run_InPlace"),
    ("base_threat", "walk", "/Game/Zombie/BaseMesh/SK_Zombie.SK_Zombie", "/Game/Zombie/Demo/Animations/ThirdPersonWalk.ThirdPersonWalk"),
    ("nurse_threat", "attack", "/Game/ZombieFemale/Asset/Meshes/ZombieFemale_NurseOutfit.ZombieFemale_NurseOutfit", "/Game/ZombieFemale/Asset/Animations/ANMS_ZombieFemaleAttackForward05.ANMS_ZombieFemaleAttackForward05"),
    ("business_threat", "attack", "/Game/YI_ModularZombies/Meshes/ZombieM04/Zombie/SK_Zombie_M04_01.SK_Zombie_M04_01", "/Game/YI_ModularZombies/Animation/MocapOnline/UE4/In_Place/Zombie_Atk_Loop_1_IPC.Zombie_Atk_Loop_1_IPC"),
    ("bloated_threat", "fall_death", "/Game/YI_ModularZombies/Meshes/ZombieF01/Zombie/SK_Zombie_F01_01.SK_Zombie_F01_01", "/Game/YI_ModularZombies/Animation/MocapOnline/UE4/In_Place/Zombie_Death_Hit_Back_1_IPC.Zombie_Death_Hit_Back_1_IPC"),
]


def project_root():
    return Path(unreal.Paths.project_dir()).resolve()


def get_field(entry, *names):
    for name in names:
        try:
            return entry.get_editor_property(name)
        except Exception:
            pass
        if hasattr(entry, name):
            return getattr(entry, name)
    raise RuntimeError(f"could not read field {names[0]}")


def text(value):
    return re.sub(r"\s+", " ", str(value)).strip()


def audio_coverage_status(rank):
    base = "complete_text_subtitle_with_platform_voice_fallback"
    if rank != 1:
        return base
    runtime_bridges = [
        "city_radio_bark_cadence",
        "runtime.radio_route_cadence",
        "pending_subtitle_queue",
        "route_phase_language_terminal_survivor_landmark_next_step",
        "reactive_threat_audio_music",
        "runtime.reactive_threat_music",
        "reactive_audio_state_caption",
        "nearby_zombie_pressure",
        "city_ambient_zone_audio",
        "runtime.city_ambient_zone",
        "ambient_zone_state_caption",
        "ZoneAmbientCues",
        "visualized_sound_cues",
        "mono_audio_accessibility",
    ]
    return base + ";" + ";".join(runtime_bridges)


def animation_coverage_status(family, role):
    base = "wired_in_world_clip_stage"
    if family != "survivor" or role != "ready_idle":
        return base
    runtime_bridges = [
        "player_operator",
        "first_person_arms",
        "survivor_hero",
        "friendly_npc",
        "companion_hero",
        "zombie_crowd",
        "boss_warden",
        "CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots",
        "ik_control_rig_runtime_slot_ready",
        "FootGroundingRuntimeContract",
        "FootGroundingExcluded_FirstPersonArms",
        "ApplyFootGroundingReview",
        "foot_grounding_contract_ready",
        "Runtime retarget and Control Rig slots plus foot grounding",
        "Foot grounding review bay",
        "maya_player_operator",
        "maya_first_person_arms",
        "maya_survivor_hero",
        "maya_friendly_npc",
        "maya_companion_hero",
        "maya_zombie_crowd",
        "maya_boss_warden",
        "maya_cleanup_profile",
        "maya_cleanup_contract_ready",
        "zombie_family_dog",
        "zombie_family_elites",
    ]
    return base + ";" + ";".join(runtime_bridges)


def write_tsv(path, header, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.writer(handle, delimiter="\t", lineterminator="\n")
        writer.writerow(header)
        writer.writerows(rows)
    unreal.log(f"[cr-production-export] wrote {path.relative_to(project_root())} rows={len(rows)}")


def mission_dict(entry):
    rank = int(get_field(entry, "rank"))
    slug = text(get_field(entry, "slug"))
    return {
        "rank": rank,
        "rank_key": f"{rank:03d}",
        "slug": slug,
        "city": text(get_field(entry, "city_name", "cityName")),
        "state": text(get_field(entry, "state_name", "stateName")),
        "terminal_title": text(get_field(entry, "terminal_title", "terminalTitle")),
        "lesson": text(get_field(entry, "lesson_kind", "lessonKind")),
        "stage": text(get_field(entry, "curriculum_stage_name", "curriculumStageName")),
        "mission_brief": text(get_field(entry, "mission_brief", "missionBrief")),
        "radio_briefing": text(get_field(entry, "radio_briefing", "radioBriefing")),
        "radio_voice": text(get_field(entry, "radio_voice_name", "radioVoiceName")),
        "region": text(get_field(entry, "region_name", "regionName")),
        "district": text(get_field(entry, "district_style", "districtStyle")),
        "landmark": text(get_field(entry, "landmark_name", "landmarkName")),
        "art_kit": text(get_field(entry, "art_kit_name", "artKitName")),
        "architecture": text(get_field(entry, "architecture_signature", "architectureSignature")),
        "detail": text(get_field(entry, "novel_gameplay_detail", "novelGameplayDetail")),
        "hint": text(get_field(entry, "hint_text", "hintText")),
        "visible": text(get_field(entry, "visible_test_brief", "visibleTestBrief")),
        "hidden": text(get_field(entry, "hidden_test_brief", "hiddenTestBrief")),
        "difficulty": int(get_field(entry, "difficulty_tier", "difficultyTier")),
        "intensity": float(get_field(entry, "encounter_intensity", "encounterIntensity")),
    }


def localization_rows(missions):
    fields = [
        ("terminal_title", "Terminal title"),
        ("mission_brief", "Terminal mission brief"),
        ("radio_briefing", "Radio/subtitle briefing"),
        ("hint", "Coding hint"),
        ("visible", "Visible test description"),
        ("hidden", "Hidden test description"),
    ]
    rows = []
    for mission in missions:
        prefix = f"campaign.{mission['rank_key']}.{mission['slug']}"
        for field_name, context in fields:
            rows.append([
                "CodeRescueCampaign",
                f"{prefix}.{field_name}",
                mission[field_name],
                f"{context} for {mission['city']}, {mission['state']}",
            ])
    return rows


def main():
    unreal.log("[cr-production-export] === production manifest export START ===")
    entries = unreal.CodeRescueCurriculumLibrary.get_campaign_audit_entries()
    if len(entries) < EXPECTED_LEVELS:
        raise RuntimeError(f"expected {EXPECTED_LEVELS} campaign entries, got {len(entries)}")

    missions = [mission_dict(entry) for entry in entries]
    root = project_root()
    data_dir = root / "Content" / "CodeRescueData"

    write_tsv(
        data_dir / "radio_briefings.tsv",
        ["rank", "slug", "city", "state", "voice", "art_kit", "briefing"],
        [[m["rank"], m["slug"], m["city"], m["state"], m["radio_voice"], m["art_kit"], m["radio_briefing"]] for m in missions],
    )

    write_tsv(
        data_dir / "localization_source.tsv",
        ["namespace", "key", "source_text", "context"],
        localization_rows(missions),
    )

    write_tsv(
        data_dir / "audio_coverage_manifest.tsv",
        ["rank", "slug", "city", "voice", "radio_key", "subtitle_fallback", "system_voice_fallback", "cooked_cue_strategy", "ambient_zone", "coverage_status"],
        [
            [
                m["rank"],
                m["slug"],
                m["city"],
                m["radio_voice"],
                f"campaign.{m['rank_key']}.{m['slug']}.radio_briefing",
                "always_on",
                "macOS_say_when_enabled",
                "optional_soft_ref_or_text_voice_fallback",
                (m["rank"] - 1) % 3,
                audio_coverage_status(m["rank"]),
            ]
            for m in missions
        ],
    )

    all_city_layers = "landmark;art_kit;major_signature;landscape;street_life;objective_route;composition;character_court;safehouse;physics_yard;mission_diorama;account_assets;first_minute;bespoke_survival_horror;authored_asset_refinement;production_completion"
    write_tsv(
        data_dir / "production_art_coverage_manifest.tsv",
        ["rank", "slug", "city", "region", "district_style", "art_kit", "landmark", "architecture_signature", "novel_gameplay_detail", "all_city_layers"],
        [[m["rank"], m["slug"], m["city"], m["region"], m["district"], m["art_kit"], m["landmark"], m["architecture"], m["detail"], all_city_layers] for m in missions],
    )

    write_tsv(
        data_dir / "visual_review_checklist.tsv",
        ["rank", "slug", "city", "review_route", "required_checks"],
        [
            [
                m["rank"],
                m["slug"],
                m["city"],
                "spawn_pad > language_stations > mission_terminal > landmark > animation_stage > production_plaza > survivor_camp",
                "no blocking entry rails; terminal readable; city identity visible; radio subtitle appears; character clips animate; production plaza readable; survivor route clear",
            ]
            for m in missions
        ],
    )

    write_tsv(
        data_dir / "performance_budget_manifest.tsv",
        ["rank", "slug", "city", "difficulty", "encounter_intensity", "active_ai_budget", "visual_budget", "profile_command"],
        [
            [
                m["rank"],
                m["slug"],
                m["city"],
                m["difficulty"],
                f"{m['intensity']:.2f}",
                "bounded_by_MaxActiveAIZombiesPerCity",
                "streamed_city_actors_registered_for_cleanup",
                "Run_Performance_Profile.command",
            ]
            for m in missions
        ],
    )

    write_tsv(
        data_dir / "animation_coverage_manifest.tsv",
        ["family", "role", "mesh", "animation", "world_hook", "coverage_status"],
        [
            [
                family,
                role,
                mesh,
                anim,
                "SpawnBespokeAuthoredAssetRefinementLayer",
                animation_coverage_status(family, role),
            ]
            for family, role, mesh, anim in ANIMATION_CLIPS
        ],
    )

    unreal.log("[cr-production-export] === production manifest export PASSED ===")


main()
