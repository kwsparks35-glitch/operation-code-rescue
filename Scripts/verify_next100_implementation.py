"""
Verify the Next 100 implementation pass across campaign data, terminal coaching,
world-development source hooks, and documentation.

Run from the project root:

    ENGINE_ROOT="$(Scripts/find_unreal_mac.sh)"
    "$ENGINE_ROOT/Binaries/Mac/UnrealEditor-Cmd" "$(pwd)/CodeRescueUnreal.uproject" \
        -run=pythonscript -script="$(pwd)/Scripts/verify_next100_implementation.py" \
        -unattended -NoSound -NullRHI
"""

from pathlib import Path

import unreal


MIN_EXPECTED_LEVELS = 465

REQUIRED_FIELDS = {
    "language_track_text": ("track", "launch track"),
    "learning_support_text": (
        "Why this matters",
        "Predict before coding",
        "worked example",
        "Code-trace mini-game",
        "Mistake glossary",
        "Hidden-test replay",
    ),
    "visual_debugger_plan": ("Visual debugger",),
    "progression_plan": ("Progression plan", "spaced review", "side objectives"),
    "character_story_plan": ("Character plan", "mentor", "debrief"),
    "gameplay_flow_plan": ("Flow plan", "practice-only", "reward choice", "save-slot"),
    "accessibility_polish_plan": ("Accessibility and polish plan", "colorblind", "controller", "audio"),
    "qa_verification_plan": ("QA plan", "runtime spawn audit", "packaged-build", "weekly release checklist"),
}

MISSION_BRIEF_TOKENS = (
    "Language track:",
    "Learning support:",
    "Visual debugger:",
    "Progression plan:",
    "Character plan:",
    "Flow plan:",
    "Accessibility and polish plan:",
    "QA plan:",
)

SOURCE_EXPECTATIONS = {
    "Source/CodeRescueUnreal/CodeTerminalWidget.cpp": (
        "GetTerminalLearningCoach",
        "Why it matters:",
        "Predict before coding:",
        "Worked example:",
        "Visual debugger:",
        "Mistake glossary:",
        "Hidden-test debrief:",
    ),
    "Source/CodeRescueUnreal/CodeRescueGameMode.h": (
        "SpawnNext100DevelopmentLayer",
    ),
    "Source/CodeRescueUnreal/CodeRescueGameMode.cpp": (
        "SpawnNext100DevelopmentLayer",
        "Next100Implementation",
        "CURRICULUM 1-20",
        "WORLD 21-40",
        "CHARACTERS 41-60",
        "FLOW 61-80",
        "UI ACCESS AUDIO 81-95",
        "QA 96-100",
        "ACCESSIBILITY CONSOLE",
        "ALL-LEVEL QA",
    ),
    "Source/CodeRescueUnreal/CodeRescueCampaign.cpp": (
        "LanguageTrackFor",
        "LearningSupportFor",
        "ProgressionPlanFor",
        "CharacterStoryPlanFor",
        "GameplayFlowPlanFor",
        "AccessibilityPolishPlanFor",
        "QAVerificationPlanFor",
    ),
}

DOC_EXPECTATIONS = {
    "Documentation/improvement_pass_2026-05-24/22_NEXT_100_IMPLEMENTATION_PASS.md": (
        "Items 1-100",
        "Status: complete",
        "SpawnNext100DevelopmentLayer",
        "verify_next100_implementation.py",
    ),
    "Documentation/improvement_pass_2026-05-24/21_NEXT_100_DEVELOPMENT_ROADMAP.md": (
        "Status: implemented",
        "Completion ledger",
    ),
}


def fail(message):
    unreal.log_error(f"[cr-next100] {message}")
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


def require_tokens(label, text, tokens):
    value = str(text)
    missing = [token for token in tokens if token not in value]
    if missing:
        fail(f"{label} is missing tokens: {', '.join(missing)}")


def project_root():
    return Path(unreal.Paths.project_dir()).resolve()


def verify_campaign_data():
    entries = unreal.CodeRescueCurriculumLibrary.get_campaign_audit_entries()
    if len(entries) < MIN_EXPECTED_LEVELS:
        fail(f"expected at least {MIN_EXPECTED_LEVELS} levels, got {len(entries)}")

    lessons_seen = set()
    stages_seen = set()
    languages_seen = set()
    ranks_seen = set()

    for entry in entries:
        rank = int(get_field(entry, "rank"))
        lesson = str(get_field(entry, "lesson_kind", "lessonKind"))
        stage = str(get_field(entry, "curriculum_stage_name", "curriculumStageName"))
        brief = str(get_field(entry, "mission_brief", "missionBrief"))

        if rank in ranks_seen:
            fail(f"duplicate rank {rank}")
        ranks_seen.add(rank)
        lessons_seen.add(lesson)
        stages_seen.add(stage)

        require_tokens(f"level {rank} mission brief", brief, MISSION_BRIEF_TOKENS)
        for field_name, tokens in REQUIRED_FIELDS.items():
            camel_name = "".join([field_name.split("_")[0]] + [part.capitalize() for part in field_name.split("_")[1:]])
            value = str(get_field(entry, field_name, camel_name))
            if len(value.strip()) < 80:
                fail(f"level {rank} {field_name} is too short")
            require_tokens(f"level {rank} {field_name}", value, tokens)
            if field_name == "language_track_text":
                for language in ("Java", "C", "Python", "MATLAB"):
                    if language in value:
                        languages_seen.add(language)

    if ranks_seen != set(range(1, len(entries) + 1)):
        missing = sorted(set(range(1, len(entries) + 1)) - ranks_seen)[:10]
        fail(f"campaign ranks are not contiguous; first missing: {missing}")
    if len(lessons_seen) < 8:
        fail(f"expected 8 lesson kinds, got {len(lessons_seen)}")
    if len(stages_seen) < 5:
        fail(f"expected 5 stages, got {len(stages_seen)}")
    if languages_seen != {"Java", "C", "Python", "MATLAB"}:
        fail(f"expected all four recommended language tracks, got {sorted(languages_seen)}")

    unreal.log(
        f"[cr-next100] OK campaign data levels={len(entries)} "
        f"lessons={len(lessons_seen)} stages={len(stages_seen)} languages={len(languages_seen)}"
    )


def verify_files(expectations):
    root = project_root()
    for relative_path, tokens in expectations.items():
        path = root / relative_path
        if not path.exists():
            fail(f"missing file {relative_path}")
        text = path.read_text(encoding="utf-8")
        require_tokens(relative_path, text, tokens)
        unreal.log(f"[cr-next100] OK file tokens {relative_path}")


def main():
    unreal.log("[cr-next100] === Next 100 implementation verification START ===")
    verify_campaign_data()
    verify_files(SOURCE_EXPECTATIONS)
    verify_files(DOC_EXPECTATIONS)
    unreal.log("[cr-next100] === Next 100 implementation verification PASSED ===")


main()
