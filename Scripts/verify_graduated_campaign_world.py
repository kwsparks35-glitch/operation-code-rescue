"""
Verify every generated campaign level has a graduated coding task and a
distinct city/world identity profile.

Run from the project root:

    ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)"
    "$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" \
        -run=pythonscript -script="$(pwd)/Scripts/verify_graduated_campaign_world.py" \
        -unattended -NoSound -NullRHI
"""

import unreal


MIN_EXPECTED_LEVELS = 465

LESSON_TOKEN = {
    "Sum": "sum",
    "Lock": "lock",
    "Reverse": "reverse",
    "Palindrome": "palindrome",
    "FizzBuzz": "fizzbuzz",
    "EvenFilter": "even",
    "LinkedListTraverse": "linked_list",
    "BinarySearch": "binary_search",
}


def fail(message):
    unreal.log_error(f"[cr-graduated-world] {message}")
    raise RuntimeError(message)


def get_field(entry, *names):
    for name in names:
        try:
            return entry.get_editor_property(name)
        except Exception:
            pass
        if hasattr(entry, name):
            return getattr(entry, name)
    fail(f"could not read field {names[0]} from campaign audit entry")


def expected_stage(rank):
    if rank <= 24:
        return "Stage 1 - Foundations"
    if rank <= 84:
        return "Stage 2 - Control Flow"
    if rank <= 174:
        return "Stage 3 - Collections and Strings"
    if rank <= 294:
        return "Stage 4 - Data Structures"
    return "Stage 5 - Algorithmic Search"


def allowed_lessons_for_rank(rank):
    if rank <= 24:
        return {"Sum", "Lock", "Reverse"}
    if rank <= 84:
        return {"Sum", "Lock", "Reverse", "Palindrome", "FizzBuzz"}
    if rank <= 174:
        return {"Reverse", "Palindrome", "FizzBuzz", "EvenFilter"}
    if rank <= 294:
        return {"EvenFilter", "LinkedListTraverse", "Palindrome", "BinarySearch", "FizzBuzz"}
    return {"LinkedListTraverse", "BinarySearch", "EvenFilter", "FizzBuzz", "Palindrome"}


def require_text(rank, label, value, min_len):
    if not value or len(str(value).strip()) < min_len:
        fail(f"level {rank}: {label} is missing or too short")


def main():
    unreal.log("[cr-graduated-world] === graduated campaign/world verification START ===")

    entries = unreal.CodeRescueCurriculumLibrary.get_campaign_audit_entries()
    if len(entries) < MIN_EXPECTED_LEVELS:
        fail(f"expected at least {MIN_EXPECTED_LEVELS} levels, got {len(entries)}")

    seen_ranks = set()
    seen_slugs = set()
    seen_terminal_ids = set()
    seen_identity_profiles = set()
    lessons_seen = set()
    stages_seen = set()
    art_kits_seen = set()

    for entry in entries:
        rank = int(get_field(entry, "rank"))
        city = str(get_field(entry, "city_name", "cityName"))
        state = str(get_field(entry, "state_name", "stateName"))
        slug = str(get_field(entry, "slug"))
        terminal_id = str(get_field(entry, "terminal_id", "terminalId"))
        terminal_title = str(get_field(entry, "terminal_title", "terminalTitle"))
        lesson = str(get_field(entry, "lesson_kind", "lessonKind"))
        stage = str(get_field(entry, "curriculum_stage_name", "curriculumStageName"))
        focus = str(get_field(entry, "curriculum_focus", "curriculumFocus"))
        brief = str(get_field(entry, "mission_brief", "missionBrief"))
        region = str(get_field(entry, "region_name", "regionName"))
        district = str(get_field(entry, "district_style", "districtStyle"))
        landmark = str(get_field(entry, "landmark_name", "landmarkName"))
        art_kit = str(get_field(entry, "art_kit_name", "artKitName"))
        architecture = str(get_field(entry, "architecture_signature", "architectureSignature"))
        detail = str(get_field(entry, "novel_gameplay_detail", "novelGameplayDetail"))
        hint = str(get_field(entry, "hint_text", "hintText"))
        visible = str(get_field(entry, "visible_test_brief", "visibleTestBrief"))
        hidden = str(get_field(entry, "hidden_test_brief", "hiddenTestBrief"))
        difficulty = int(get_field(entry, "difficulty_tier", "difficultyTier"))
        intensity = float(get_field(entry, "encounter_intensity", "encounterIntensity"))

        if rank in seen_ranks:
            fail(f"duplicate rank {rank}")
        seen_ranks.add(rank)

        if slug in seen_slugs:
            fail(f"level {rank}: duplicate slug {slug}")
        seen_slugs.add(slug)

        if terminal_id in seen_terminal_ids:
            fail(f"level {rank}: duplicate terminal id {terminal_id}")
        seen_terminal_ids.add(terminal_id)

        if stage != expected_stage(rank):
            fail(f"level {rank}: expected {expected_stage(rank)}, got {stage}")
        if lesson not in allowed_lessons_for_rank(rank):
            fail(f"level {rank}: lesson {lesson} is not allowed for this graduation stage")
        if lesson not in LESSON_TOKEN:
            fail(f"level {rank}: unknown lesson kind {lesson}")
        if LESSON_TOKEN[lesson] not in terminal_id:
            fail(f"level {rank}: terminal id {terminal_id} does not contain token {LESSON_TOKEN[lesson]}")

        require_text(rank, "city", city, 2)
        require_text(rank, "state", state, 2)
        require_text(rank, "terminal title", terminal_title, 12)
        require_text(rank, "curriculum focus", focus, 80)
        require_text(rank, "mission brief", brief, 180)
        require_text(rank, "region", region, 8)
        require_text(rank, "district style", district, 12)
        require_text(rank, "landmark", landmark, 12)
        require_text(rank, "art kit", art_kit, 8)
        require_text(rank, "architecture signature", architecture, 40)
        require_text(rank, "novel gameplay detail", detail, 45)
        require_text(rank, "hint", hint, 25)
        require_text(rank, "visible test", visible, 25)
        require_text(rank, "hidden test", hidden, 25)

        if "Architecture:" not in brief or "Play detail:" not in brief:
            fail(f"level {rank}: mission brief does not include architecture/play-detail documentation")
        if city not in terminal_title:
            fail(f"level {rank}: terminal title does not name the city")
        if difficulty < 1 or difficulty > 5:
            fail(f"level {rank}: difficulty tier out of range: {difficulty}")
        if intensity <= 0.0:
            fail(f"level {rank}: encounter intensity must be positive")

        identity_key = f"{rank}|{city}|{state}|{landmark}|{architecture}|{detail}"
        if identity_key in seen_identity_profiles:
            fail(f"level {rank}: duplicate city identity profile")
        seen_identity_profiles.add(identity_key)

        lessons_seen.add(lesson)
        stages_seen.add(stage)
        art_kits_seen.add(art_kit)

    expected_ranks = set(range(1, len(entries) + 1))
    if seen_ranks != expected_ranks:
        missing = sorted(expected_ranks - seen_ranks)[:10]
        fail(f"campaign ranks are not contiguous; first missing ranks: {missing}")

    missing_lessons = set(LESSON_TOKEN.keys()) - lessons_seen
    if missing_lessons:
        fail("missing lesson kinds across campaign: " + ", ".join(sorted(missing_lessons)))
    if len(stages_seen) < 5:
        fail(f"expected all 5 curriculum stages, got {len(stages_seen)}")
    if len(art_kits_seen) < 10:
        fail(f"expected at least 10 art kits, got {len(art_kits_seen)}")

    unreal.log(
        f"[cr-graduated-world] OK levels={len(entries)} lessons={len(lessons_seen)} "
        f"stages={len(stages_seen)} art_kits={len(art_kits_seen)}"
    )
    unreal.log("[cr-graduated-world] === graduated campaign/world verification PASSED ===")


main()
