#!/usr/bin/env python3
"""Static verifier for the Sequencer intro and boss reveal blocking slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-30"
SOURCE_DOC = PROJECT_ROOT / "Documentation/improvement_pass_2026-06-25"

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


game_mode_cpp = read(SRC / "CodeRescueGameMode.cpp")
boss_reveal_cpp = read(SRC / "BossRevealPresentationActor.cpp")
manifest = read(DATA / "cinematic_sequence_blocking_manifest.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
visual_targets = read(DATA / "visual_regression_targets.tsv")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC / "SEQUENCER_INTRO_BOSS_REVEAL_SLICE.md")
boss_reveal_doc = read(DOC / "BOSS_REVEAL_PRESENTATION_SLICE.md")
top50 = read(SOURCE_DOC / "TOP_50_RECOMMENDATIONS_2026-06-25.md")
character_deep_dive = read(SOURCE_DOC / "CHARACTER_ANIMATION_DEEPDIVE.md")
progress = read(PROJECT_ROOT / "progress.md")
self_source = read(PROJECT_ROOT / "Scripts/verify_sequencer_intro_boss_reveal_slice_pass.py")


check_all(
    game_mode_cpp,
    [
        "SEQUENCER BLOCKING REEL",
        "FSequencerBeatSpec",
        "SequencerIntroBossRevealBlocking",
        "CinematicCameraBlockingReady",
        "SequencerReadyFallback",
        "ControlRigReadyFallback",
        "SequencerIntroBeat",
        "SequencerLanguageBeat",
        "SequencerTerminalBeat",
        "SequencerSurvivorBeat",
        "SequencerExtractionBeat",
        "SequencerBossRevealBeat",
        "SequencerCameraRail",
        "SequencerCameraTripod",
        "SequencerCameraLens",
        "SequencerBeatLabel",
        "SequencerBeatConnector",
        "INTRO",
        "LANGUAGE",
        "TERMINAL",
        "RESCUE",
        "EXTRACTION",
        "BOSS",
    ],
    "game mode must expose a complete Sequencer blocking reel",
)

check_all(
    boss_reveal_cpp,
    [
        "BossRevealPresentation",
        "SequencerIntroBossRevealBlocking",
        "SequencerBossRevealBeat",
        "CinematicCameraBlockingReady",
        "SequencerReadyFallback",
        "ControlRigReadyFallback",
        "BossRevealTriggered",
    ],
    "boss reveal actor must participate in the cinematic blocking contract",
)

check_all(
    manifest,
    [
        "Intro",
        "Language Selection",
        "Terminal Solve",
        "Survivor Rescue",
        "Extraction",
        "Boss Reveal",
        "SequencerIntroBossRevealBlocking",
        "CinematicCameraBlockingReady",
        "SequencerBossRevealBeat",
        "verify_sequencer_intro_boss_reveal_slice_pass.py",
        "sequence_blocking_ready",
    ],
    "cinematic manifest must cover every story beat and validation route",
)

check_all(
    creative_plan,
    [
        "Sequencer intros and boss reveals",
        "verify_sequencer_intro_boss_reveal_slice_pass.py",
        "verify_boss_reveal_presentation_slice_pass.py",
        "manual cinematic review",
    ],
    "creative plan must route the P2 cinematic row through the new verifier",
)

check_all(
    human_qa,
    [
        "SequencerIntroBossReveal",
        "intro, language, terminal, survivor, extraction, and boss reveal camera markers",
        "future Sequencer/Control Rig assets",
    ],
    "human QA checklist must include cinematic blocking review",
)

check_all(
    visual_targets,
    [
        "SequencerIntroBossReveal",
        "camera rails/tripods/lenses",
        "public-demo-ready camera plan",
    ],
    "visual regression targets must include cinematic blocking review",
)

check("verify_sequencer_intro_boss_reveal_slice_pass.py" in full_qa,
      "full QA must run the Sequencer intro/boss reveal verifier")
check("verify_sequencer_intro_boss_reveal_slice_pass.py" in local_ci,
      "local CI must run the Sequencer intro/boss reveal verifier")

check_all(
    slice_doc,
    [
        "Sequencer Intro Boss Reveal Slice",
        "SpawnCinematicStreetLifeLayer",
        "SEQUENCER BLOCKING REEL",
        "SequencerIntroBossRevealBlocking",
        "CinematicCameraBlockingReady",
        "ABossRevealPresentationActor",
        "Validation",
        "Boundaries",
    ],
    "slice documentation must explain implementation, validation, and boundaries",
)

check_all(
    boss_reveal_doc,
    [
        "boss reveal",
        "Sequencer",
        "Control Rig",
        "reduced motion",
    ],
    "boss reveal documentation must remain aligned with the cinematic slice",
)

check_all(
    top50,
    [
        "Sequencer beats",
        "Control-Rig cinematics",
        "boss reveal",
        "extraction",
    ],
    "top-50 source guidance must contain the cinematic recommendation",
)

check_all(
    character_deep_dive,
    [
        "Sequencer",
        "Control Rig",
        "boss reveal",
    ],
    "character deep dive must contain the Sequencer/Control Rig guidance",
)

check_all(
    progress,
    [
        "Sequencer intro boss reveal slice",
        "SequencerIntroBossRevealBlocking",
        "verify_sequencer_intro_boss_reveal_slice_pass.py",
    ],
    "progress log must record the Sequencer intro/boss reveal slice",
)

check("verify_sequencer_intro_boss_reveal_slice_pass.py" in self_source,
      "static verifier should identify itself")

if errors:
    print("[verify_sequencer_intro_boss_reveal_slice_pass] FAIL")
    for err in errors:
        print(f" - {err}")
    sys.exit(1)

print("[verify_sequencer_intro_boss_reveal_slice_pass] PASS: Sequencer intro/boss reveal blocking verified")
