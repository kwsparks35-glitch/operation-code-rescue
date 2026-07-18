"""
Verify the production-track completion pass: full campaign radio text coverage,
localization source export, art/visual/performance manifests, authored
animation clip assets, and source hooks that make the coverage visible in game.

Run from the project root:

    ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)"
    "$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" \
        -run=pythonscript -script="$(pwd)/Scripts/verify_production_track_completion.py" \
        -unattended -NoSound -NullRHI
"""

from pathlib import Path
import csv

import unreal


EXPECTED_LEVELS = 465
EXPECTED_LOCALIZED_FIELDS = {
    "terminal_title",
    "mission_brief",
    "radio_briefing",
    "hint",
    "visible",
    "hidden",
}


def fail(message):
    unreal.log_error(f"[cr-production-verify] {message}")
    raise RuntimeError(message)


def project_root():
    return Path(unreal.Paths.project_dir()).resolve()


def read_tsv(relative_path):
    path = project_root() / relative_path
    if not path.exists():
        fail(f"missing file {relative_path}")
    with path.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle, delimiter="\t")
        rows = list(reader)
    if not rows:
        fail(f"{relative_path} has no data rows")
    unreal.log(f"[cr-production-verify] OK read {relative_path} rows={len(rows)}")
    return rows


def require_tokens(relative_path, tokens):
    path = project_root() / relative_path
    if not path.exists():
        fail(f"missing file {relative_path}")
    text = path.read_text(encoding="utf-8")
    missing = [token for token in tokens if token not in text]
    if missing:
        fail(f"{relative_path} missing tokens: {', '.join(missing)}")
    unreal.log(f"[cr-production-verify] OK source tokens {relative_path}")


def verify_campaign():
    entries = unreal.CodeRescueCurriculumLibrary.get_campaign_audit_entries()
    if len(entries) < EXPECTED_LEVELS:
        fail(f"expected at least {EXPECTED_LEVELS} campaign entries, got {len(entries)}")
    ranks = set()
    for entry in entries:
        rank = int(entry.get_editor_property("rank"))
        ranks.add(rank)
        radio = str(entry.get_editor_property("radio_briefing"))
        voice = str(entry.get_editor_property("radio_voice_name"))
        if len(radio.strip()) < 100:
            fail(f"rank {rank} radio briefing too short")
        if not voice.strip():
            fail(f"rank {rank} missing radio voice")
    expected = set(range(1, len(entries) + 1))
    if ranks != expected:
        fail("campaign ranks are not contiguous")
    unreal.log(f"[cr-production-verify] OK campaign radio fields levels={len(entries)}")
    return len(entries)


def verify_complete_rows(label, rows, count):
    if len(rows) != count:
        fail(f"{label} expected {count} rows, got {len(rows)}")
    ranks = {int(row["rank"]) for row in rows}
    if ranks != set(range(1, count + 1)):
        fail(f"{label} ranks are not contiguous")
    unreal.log(f"[cr-production-verify] OK complete rows {label}")


def verify_radio(rows, count):
    verify_complete_rows("radio_briefings.tsv", rows, count)
    for row in rows:
        if len(row["briefing"]) < 100:
            fail(f"radio row {row['rank']} briefing too short")
        if not row["voice"]:
            fail(f"radio row {row['rank']} missing voice")
    unreal.log("[cr-production-verify] OK radio briefing text/voice coverage")


def verify_localization(rows, count):
    keys = {row["key"] for row in rows}
    if len(rows) != count * len(EXPECTED_LOCALIZED_FIELDS):
        fail(f"localization_source.tsv expected {count * len(EXPECTED_LOCALIZED_FIELDS)} rows, got {len(rows)}")
    for row in rows:
        if row["namespace"] != "CodeRescueCampaign":
            fail(f"unexpected localization namespace {row['namespace']}")
        if len(row["source_text"]) < 8:
            fail(f"localization key {row['key']} source text too short")
    for rank in range(1, count + 1):
        rank_prefix = f"campaign.{rank:03d}."
        rank_keys = {key for key in keys if key.startswith(rank_prefix)}
        found_fields = {key.rsplit(".", 1)[-1] for key in rank_keys}
        missing = EXPECTED_LOCALIZED_FIELDS - found_fields
        if missing:
            fail(f"rank {rank} localization missing fields: {', '.join(sorted(missing))}")
    unreal.log("[cr-production-verify] OK localization source coverage")


def verify_audio(rows, count):
    verify_complete_rows("audio_coverage_manifest.tsv", rows, count)
    for row in rows:
        if row["subtitle_fallback"] != "always_on":
            fail(f"rank {row['rank']} subtitle fallback is not always_on")
        if "fallback" not in row["coverage_status"]:
            fail(f"rank {row['rank']} audio coverage status does not document fallback")
    unreal.log("[cr-production-verify] OK audio coverage manifest")


def verify_art(rows, count):
    verify_complete_rows("production_art_coverage_manifest.tsv", rows, count)
    art_kits = {row["art_kit"] for row in rows}
    if len(art_kits) < 10:
        fail(f"expected at least 10 art kits, got {len(art_kits)}")
    for row in rows:
        for column in ("district_style", "landmark", "architecture_signature", "novel_gameplay_detail"):
            if len(row[column]) < 12:
                fail(f"rank {row['rank']} art column {column} too short")
        if "production_completion" not in row["all_city_layers"]:
            fail(f"rank {row['rank']} art coverage missing production_completion layer")
    unreal.log(f"[cr-production-verify] OK art coverage art_kits={len(art_kits)}")


def verify_visual(rows, count):
    verify_complete_rows("visual_review_checklist.tsv", rows, count)
    for row in rows:
        if "animation_stage" not in row["review_route"] or "production_plaza" not in row["review_route"]:
            fail(f"rank {row['rank']} visual review route incomplete")
    unreal.log("[cr-production-verify] OK visual review checklist")


def verify_performance(rows, count):
    verify_complete_rows("performance_budget_manifest.tsv", rows, count)
    for row in rows:
        if row["profile_command"] != "Run_Performance_Profile.command":
            fail(f"rank {row['rank']} performance profile command mismatch")
        if "MaxActiveAIZombiesPerCity" not in row["active_ai_budget"]:
            fail(f"rank {row['rank']} missing active AI budget note")
    unreal.log("[cr-production-verify] OK performance budget manifest")


def verify_animation(rows):
    if len(rows) < 13:
        fail(f"animation_coverage_manifest.tsv expected at least 13 rows, got {len(rows)}")
    roles = {row["role"] for row in rows}
    for required in ("ready_idle", "patrol_walk", "attack", "fall_death", "run_in_place"):
        if required not in roles:
            fail(f"animation manifest missing role {required}")
    for row in rows:
        for column in ("mesh", "animation"):
            path = row[column]
            if not unreal.load_asset(path):
                fail(f"could not load {column} asset {path}")
    unreal.log("[cr-production-verify] OK animation manifest assets load")


def verify_scripts_and_docs():
    required_files = [
        "Run_Performance_Profile.command",
        "Run_Visual_Review_Capture.command",
        "Package_Mac_App.command",
        "Smoke_Test_Packaged_App.command",
        "Documentation/improvement_pass_2026-05-24/27_PRODUCTION_TRACK_COMPLETION_PASS.md",
    ]
    for relative_path in required_files:
        if not (project_root() / relative_path).exists():
            fail(f"missing required file {relative_path}")
    unreal.log("[cr-production-verify] OK production scripts/docs present")


def main():
    unreal.log("[cr-production-verify] === production track verification START ===")
    count = verify_campaign()
    verify_radio(read_tsv("Content/CodeRescueData/radio_briefings.tsv"), count)
    verify_localization(read_tsv("Content/CodeRescueData/localization_source.tsv"), count)
    verify_audio(read_tsv("Content/CodeRescueData/audio_coverage_manifest.tsv"), count)
    verify_art(read_tsv("Content/CodeRescueData/production_art_coverage_manifest.tsv"), count)
    verify_visual(read_tsv("Content/CodeRescueData/visual_review_checklist.tsv"), count)
    verify_performance(read_tsv("Content/CodeRescueData/performance_budget_manifest.tsv"), count)
    verify_animation(read_tsv("Content/CodeRescueData/animation_coverage_manifest.tsv"))
    require_tokens(
        "Source/CodeRescueUnreal/CodeRescueGameMode.cpp",
        [
            "UCodeRescueSubtitlesWidget::Push(Mission.RadioBriefing",
            "SpawnProductionTrackCompletionLayer",
            "ProductionTrackCompletion",
            "13 authored loop clips",
        ],
    )
    require_tokens(
        "Source/CodeRescueUnreal/CodeRescueCurriculumLibrary.h",
        ["RadioBriefing", "RadioVoiceName"],
    )
    verify_scripts_and_docs()
    unreal.log("[cr-production-verify] === production track verification PASSED ===")


main()
