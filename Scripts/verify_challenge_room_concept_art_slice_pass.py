#!/usr/bin/env python3
"""Static verifier for the challenge room concept art slice."""

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


gamemode_h = read(SRC / "CodeRescueGameMode.h")
gamemode_cpp = read(SRC / "CodeRescueGameMode.cpp")
manifest = read(DATA / "challenge_room_concept_art_manifest.tsv")
gallery_manifest = read(DATA / "curriculum_first_review_gallery_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
curriculum_manifest = read(DATA / "curriculum_feedback_manifest.tsv")
onboarding = read(DATA / "first_ten_minutes_onboarding.tsv")
visual_manifest = read(DATA / "visual_regression_targets.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
accessibility_manifest = read(DATA / "accessibility_settings_manifest.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
progress = read(PROJECT_ROOT / "progress.md")
slice_doc = read(DOC_DIR / "CHALLENGE_ROOM_CONCEPT_ART_SLICE.md")
gallery_doc = read(DOC_DIR / "CURRICULUM_FIRST_REVIEW_GALLERY_SLICE.md")
render_script = read(PROJECT_ROOT / "Scripts/render_curriculum_first_review_gallery.py")

spawn_city_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnCampaignCity")
challenge_body = function_body(gamemode_cpp, "void ACodeRescueGameMode::SpawnChallengeRoomConceptArtLayer")

check_all(
    gamemode_h,
    ["SpawnChallengeRoomConceptArtLayer"],
    "game mode header must declare the challenge room concept art layer",
)

safehouse_idx = spawn_city_body.find("SpawnProtectedCodingChallengeHub(Mission, CityIndex, Origin, CityLabel)")
challenge_idx = spawn_city_body.find("SpawnChallengeRoomConceptArtLayer(Mission, CityIndex, Origin, CityLabel)")
check(safehouse_idx >= 0, "campaign city spawn must still create the protected coding hub")
check(challenge_idx > safehouse_idx >= 0, "challenge concept rooms must spawn after the protected coding hub")

check_all(
    challenge_body,
    [
        "VARIABLES LAB",
        "LOOP CONTROL ROOM",
        "ARRAY INDEX HALL",
        "FUNCTION RELAY ROOM",
        "DEBUGGER TEST BAY",
        "VariablesLab",
        "LoopControlRoom",
        "ArrayIndexHall",
        "FunctionRelayRoom",
        "DebuggerTestBay",
    ],
    "challenge room layer must implement all five concept-room archetypes",
)
check_all(
    challenge_body,
    [
        "ChallengeRoomConceptArt",
        "ChallengeConceptRoomReady",
        "CodeConceptPhysicalSpace",
        "TextFirstLearningCue",
        "ProtectedLearningSpace",
        "SelectedLanguageOnly",
        "LearningWithoutDeathRisk",
        "NoAccessBlocker",
        "WorldDevelopmentDeepDive",
        "Top50Recommendations",
        "ChallengeRoomConceptLight",
        "OpenFrontConceptRoom",
        "LessonKindConceptArtifact",
        "CurriculumFirstReviewGallery",
        "VisibleHiddenTestGallery",
        "ValidatorArchetypeProof",
        "IntrinsicIntegrationReview",
        "OperationReview20260630",
        "ThreeDReviewCandidate",
        "VisibleTestProofPylon",
        "HiddenTestProofPylon",
        "CommonMistakeMarker",
        "MentorCharacterProxy",
        "SurvivorCharacterProxy",
        "APointLight",
        "SpawnTexturedBlock",
        "SpawnBlock",
        "SpawnGuideText",
        "[CodeRescueChallengeRoomConceptArt]",
        "[CodeRescueCurriculumFirstGallery]",
    ],
    "challenge room layer must be tagged, lit, text-first, nonblocking, reviewable, and logged",
)
check_all(
    challenge_body,
    [
        "Mission.CurriculumFocus",
        "Mission.VisibleTestBrief",
        "Mission.HiddenTestBrief",
        "Mission.HintText",
        "Mission.LearningSupportText",
        "Mission.VisualDebuggerPlan",
        "Mission.ProgressionPlan",
        "Mission.LessonKind",
        "Mission.NovelGameplayDetail",
        "Mission.TerminalTitle",
    ],
    "challenge rooms must be driven by mission curriculum data",
)
check_all(
    challenge_body,
    [
        "Lock Truth Gate",
        "Reverse Signal Arrows",
        "Palindrome Mirror Walk",
        "FizzBuzz Beacon Grid",
        "Even Filter Sorting Lanes",
        "Linked List Rescue Chain",
        "Binary Search Shrinking Arena",
        "SUM POWER CELL BANK",
        "SUM RETURN",
        "LOCK BOOLEAN",
        "REVERSE STRING",
        "PALINDROME",
        "FIZZBUZZ",
        "EVEN FILTER",
        "LINKED LIST",
        "BINARY SEARCH",
    ],
    "challenge room layer must include lesson-specific artifacts and all validator gallery stations",
)

check_all(
    manifest,
    [
        "VariablesLab",
        "LoopControlRoom",
        "ArrayIndexHall",
        "FunctionRelayRoom",
        "DebuggerTestBay",
        "LessonKindConceptArtifact",
        "CurriculumFirstReviewGallery",
        "SpawnChallengeRoomConceptArtLayer",
    ],
    "challenge room manifest must document rooms, lesson artifact, gallery, and spawn owner",
)
check_all(
    gallery_manifest,
    [
        "CurriculumFirstReviewGallery",
        "VisibleTestProofPylon",
        "HiddenTestProofPylon",
        "CommonMistakeMarker",
        "MentorCharacterProxy",
        "SurvivorCharacterProxy",
        "ThreeDReviewRender",
        "Saved/VisualReview/curriculum_first_review_gallery_render.png",
    ],
    "curriculum-first gallery manifest must document structures, proxies, and render evidence",
)
check_all(
    creative_plan,
    [
        "challenge room concept art",
        "verify_challenge_room_concept_art_slice_pass.py plus verify_curriculum_validator_shapes.py plus packaged render smoke plus visual review",
    ],
    "creative plan must route challenge room concept art through the new verifier",
)
check_all(
    curriculum_manifest,
    [
        "ChallengeRoomConceptArt",
        "eight visible/hidden validator stations",
        "selected-language concept spaces before terminal validation",
    ],
    "curriculum feedback manifest must document challenge-room and gallery learning context",
)
check_all(
    onboarding,
    [
        "Inspect challenge concept rooms",
        "curriculum-first review gallery",
        "eight visible/hidden validator stations",
    ],
    "first-ten-minutes onboarding must include challenge-room and gallery inspection",
)
check_all(
    visual_manifest,
    [
        "ChallengeRoomConceptArt",
        "CurriculumFirstReviewGallery",
        "curriculum_first_review_gallery_render.png",
    ],
    "visual regression targets must include challenge room and generated gallery render review targets",
)
check_all(
    human_qa,
    [
        "ChallengeRoomConceptArt",
        "CurriculumFirstReviewGalleryRender",
        "mentor proxy",
        "survivor proxy",
    ],
    "human QA checklist must include challenge-room walkthrough and render review",
)
check_all(
    accessibility_manifest,
    [
        "ChallengeRoomConceptArtAccessibility",
        "text-first concept labels",
        "nonblocking concept rooms",
        "visible/hidden/mistake labels",
        "human-scale proxies",
    ],
    "accessibility manifest must document challenge-room and gallery cues without color dependency",
)
check("verify_challenge_room_concept_art_slice_pass.py" in full_qa,
      "full QA must run the challenge room concept art verifier")
check("verify_challenge_room_concept_art_slice_pass.py" in local_ci,
      "local CI must run the challenge room concept art verifier")
check("claude_oversight_watchdog.py" in local_ci,
      "local CI must delegate verifier execution to the oversight watchdog")
check_all(
    progress,
    [
        "Challenge room concept art slice",
        "curriculum-first gallery slice",
        "SpawnChallengeRoomConceptArtLayer",
        "VARIABLES LAB",
        "DEBUGGER TEST BAY",
        "CurriculumFirstReviewGallery",
        "curriculum_first_review_gallery_render.png",
    ],
    "progress log must record the challenge room concept art and review gallery slices",
)
check_all(
    slice_doc,
    [
        "Challenge Room Concept Art Slice",
        "SpawnChallengeRoomConceptArtLayer",
        "Variables Lab",
        "Loop Control Room",
        "Array Index Hall",
        "Function Relay Room",
        "Debugger Test Bay",
        "curriculum-first review gallery",
        "render_curriculum_first_review_gallery.py",
        "Validation",
    ],
    "slice documentation must explain implementation, gallery extension, and validation",
)
check_all(
    gallery_doc,
    [
        "Operation_Code_Rescue_Review_2026-06-30.pdf",
        "Curriculum First Review Gallery Slice",
        "visible-test pylon",
        "hidden-test pylon",
        "mentor character proxy",
        "survivor character proxy",
        "Saved/VisualReview/curriculum_first_review_gallery_render.png",
        "claude_oversight_watchdog.py",
    ],
    "gallery documentation must map the work to the independent review and render deliverable",
)
check_all(
    render_script,
    [
        "curriculum_first_review_gallery_render.png",
        "SUM RETURN",
        "LOCK BOOLEAN",
        "REVERSE STRING",
        "PALINDROME",
        "FIZZBUZZ",
        "EVEN FILTER",
        "LINKED LIST",
        "BINARY SEARCH",
        "mentor",
        "survivor",
    ],
    "render script must produce the 3D owner-review gallery with all stations and character proxies",
)

if errors:
    for error in errors:
        print(f"[verify_challenge_room_concept_art_slice_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_challenge_room_concept_art_slice_pass] PASS: challenge room concept art slice verified")
