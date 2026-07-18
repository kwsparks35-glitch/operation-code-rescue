#!/usr/bin/env python3
"""Static verifier for the 2026-07-01 Learning Vertical slice (pedagogy implementation).

Checks the data pack, the additive C++ learning library, the intrinsic world-effect actor, and
the docs. Static only (no Unreal): a Mac compile + playtest is the Definition-of-Done gate.
"""
from __future__ import annotations
import json
from collections import Counter
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "Source/CodeRescueUnreal"
DOC = ROOT / "Documentation/improvement_pass_2026-07-01"
errors: list[str] = []


def read(p: Path) -> str:
    if not p.exists():
        errors.append(f"FAIL: missing {p.relative_to(ROOT)}")
        return ""
    return p.read_text(encoding="utf-8", errors="replace")


def need(cond: bool, msg: str) -> None:
    if not cond:
        errors.append(f"FAIL: {msg}")


# --- 1. curriculum data pack ---
db_path = ROOT / "Content/CodeRescueData/curriculum_database.json"
db_text = read(db_path)
if db_text:
    try:
        db = json.loads(db_text)
        entries = db.get("entries", [])
        need(len(entries) >= 20, f"curriculum must have >=20 entries (has {len(entries)})")
        need("concept_graph" in db and len(db["concept_graph"]) >= 8,
             "curriculum must define a concept_graph of >=8 tiers")
        rich = [e for e in entries if "world_effect" in e and "visible_tests" in e and "micro_lesson" in e]
        need(len(rich) >= 12, f"need >=12 rich challenges w/ tests+world_effect+micro_lesson (has {len(rich)})")
        # every entry keeps the loader-required fields
        req = ["id", "title", "language", "concept", "difficulty", "strategies", "common_mistakes"]
        broken = [e.get("id", "?") for e in entries if not all(k in e for k in req)]
        need(not broken, f"entries missing loader-required fields: {broken[:3]}")
        # tests must have in/out
        for e in rich:
            for t in e.get("visible_tests", []):
                need("in" in t and "out" in t, f"{e['id']} visible_test needs in/out")
        target_langs = ["Java", "C", "C+", "C++", "Python", "MATLAB"]
        starter_keys = {
            "Java": "java",
            "C": "c",
            "C+": "cplus",
            "C++": "cpp",
            "Python": "python",
            "MATLAB": "matlab",
        }
        filter_entries = [
            e for e in entries
            if e.get("tier") == 4
            and (
                "evac-even-order" in e.get("id", "")
                or "filter" in e.get("concept", "").lower()
            )
        ]
        need(len(filter_entries) >= 5, f"tier-4 filter node must have >=5 challenge rows (has {len(filter_entries)})")
        need(all(e.get("validator") == "even_filter" for e in filter_entries),
             "tier-4 filter rows must declare validator=even_filter")
        coverage = Counter()
        for e in filter_entries:
            for field in ["micro_lesson", "worked_example", "prompt", "world_effect", "post_solve"]:
                need(bool(e.get(field)), f"{e.get('id', '?')} missing {field}")
            need(len(e.get("visible_tests", [])) >= 1, f"{e.get('id', '?')} needs visible tests")
            need(len(e.get("hidden_tests", [])) >= 2, f"{e.get('id', '?')} needs hidden tests")
            starter = e.get("starter", {})
            langs = e.get("languages")
            if not langs:
                langs = target_langs if e.get("language") == "All" else [e.get("language")]
            for lang in langs:
                if lang in starter_keys and starter_keys[lang] in starter:
                    coverage[lang] += 1
        for lang in target_langs:
            need(coverage[lang] >= 5, f"filter node needs >=5 playable challenges for {lang} (has {coverage[lang]})")
        boolean_entries = [
            e for e in entries
            if e.get("tier") == 2
            and (
                "airlock" in e.get("id", "")
                or "boolean" in e.get("concept", "").lower()
            )
        ]
        need(len(boolean_entries) >= 5,
             f"tier-2 boolean node must have >=5 challenge rows (has {len(boolean_entries)})")
        need(all(e.get("validator") == "boolean_lock" for e in boolean_entries),
             "tier-2 boolean rows must declare validator=boolean_lock")
        bool_coverage = Counter()
        for e in boolean_entries:
            for field in ["micro_lesson", "worked_example", "prompt", "world_effect", "post_solve"]:
                need(bool(e.get(field)), f"{e.get('id', '?')} missing {field}")
            need(len(e.get("visible_tests", [])) >= 1, f"{e.get('id', '?')} needs visible tests")
            need(len(e.get("hidden_tests", [])) >= 3, f"{e.get('id', '?')} needs complete truth-table hidden tests")
            starter = e.get("starter", {})
            langs = e.get("languages")
            if not langs:
                langs = target_langs if e.get("language") == "All" else [e.get("language")]
            for lang in langs:
                key = starter_keys.get(lang)
                if key and key in starter:
                    starter_text = starter[key]
                    if "shouldUnlock" in starter_text or "should_unlock" in starter_text:
                        bool_coverage[lang] += 1
        for lang in target_langs:
            need(bool_coverage[lang] >= 5,
                 f"boolean node needs >=5 playable shouldUnlock challenges for {lang} (has {bool_coverage[lang]})")
        reverse_entries = [
            e for e in entries
            if e.get("tier") == 5
            and "reverse" in e.get("id", "")
        ]
        need(len(reverse_entries) >= 5,
             f"tier-5 reverse node must have >=5 challenge rows (has {len(reverse_entries)})")
        need(all(e.get("validator") == "reverse_string" for e in reverse_entries),
             "tier-5 reverse rows must declare validator=reverse_string")
        reverse_coverage = Counter()
        for e in reverse_entries:
            for field in ["micro_lesson", "worked_example", "prompt", "world_effect", "post_solve"]:
                need(bool(e.get(field)), f"{e.get('id', '?')} missing {field}")
            need(len(e.get("visible_tests", [])) >= 1, f"{e.get('id', '?')} needs visible tests")
            need(len(e.get("hidden_tests", [])) >= 2, f"{e.get('id', '?')} needs hidden reverse tests")
            starter = e.get("starter", {})
            langs = e.get("languages")
            if not langs:
                langs = target_langs if e.get("language") == "All" else [e.get("language")]
            for lang in langs:
                key = starter_keys.get(lang)
                if key and key in starter:
                    starter_text = starter[key]
                    if "reverseString" in starter_text or "reverse_string" in starter_text:
                        reverse_coverage[lang] += 1
        for lang in target_langs:
            need(reverse_coverage[lang] >= 5,
                 f"reverse node needs >=5 playable reverseString challenges for {lang} (has {reverse_coverage[lang]})")
    except json.JSONDecodeError as exc:
        errors.append(f"FAIL: curriculum_database.json invalid JSON: {exc}")

# --- 2. learning library (additive C++) ---
lh = read(SRC / "CodeRescueLearning.h")
lc = read(SRC / "CodeRescueLearning.cpp")
for sym in ["UCodeRescueLearningLibrary", "FCodeRescueChallenge", "FCodeRescueTeachPayload",
            "BuildTeachPayload", "BuildPostSolve", "ShouldOfferScaffold", "BuildScaffold",
            "GetWorldEffect", "IsExternalValidationEnabled", "RecordAttempt", "SummarizeConcept",
            "SelectChallengeForCity"]:
    need(sym in lh, f"learning header must declare {sym}")
    need(sym in lc, f"learning cpp must implement {sym}")
need("#pragma once" in lh and "CodeRescueLearning.generated.h" in lh, "learning header UE hygiene")
need('#include "CodeRescueLearning.h"' in lc, "learning cpp must include its own header first")
need("curriculum_database.json" in lc, "learning cpp must read the curriculum data pack")
need("FILEWRITE_Append" in lc, "telemetry must append (not overwrite)")
need("Validator" in lh and '"validator"' in lc, "learning loader must expose the curriculum validator key")

# --- 2b. runtime terminal wiring ---
tw = read(SRC / "CodeTerminalWidget.cpp")
for token in ["SelectDataDrivenChallengeForTerminal", "BuildDataDrivenTeachApplySummary",
              "BuildTeachPayload", "RecordAttempt", "BuildPostSolve", "BuildScaffold",
              "ShouldOfferScaffold", "GetTerminalValidatorKey", "GetLearningChallengeValidatorKey",
              "BuildDataDrivenRuntimeChallengeForValidation", "DATA-DRIVEN VALIDATION PACK",
              "DATA-DRIVEN CURRICULUM NODE", "ADAPTIVE SCAFFOLD"]:
    need(token in tw, f"terminal widget must wire learning runtime token {token}")
need("IsDataDrivenStarterSignatureCompatible" in tw and "StarterCode = TeachPayload.StarterCode" in tw,
     "terminal must replace compatible data-driven starters with exercise skeletons")
runner = read(SRC / "CodeRunnerLibrary.cpp")
for token in ["CanUseDeclarativeTests", "BuildJavaDeclarativeHarness", "BuildCDeclarativeMain",
              "BuildCppDeclarativeMain", "BuildPythonDeclarativeTestBlock",
              "BuildMatlabDeclarativeRunner", "ApplyDeclarativeTestCaseCounts"]:
    need(token in runner, f"runner must execute/report curriculum-declared tests via {token}")
need("RuntimeChallenge.TestCases.Add" in tw and "ValidateChallenge(ValidationChallenge, UserCode)" in tw,
     "terminal must pass selected curriculum visible/hidden tests into validation")

# --- 3. intrinsic world-effect actor (cook-safe) ---
ah = read(SRC / "CodeRescueSolveEffectActor.h")
ac = read(SRC / "CodeRescueSolveEffectActor.cpp")
need("ACodeRescueSolveEffectActor" in ah and "ACodeRescueSolveEffectActor" in ac, "solve-effect actor present")
need("OutputMagnitude" in ah and "OutputMagnitude" in ac, "effect must be driven by solution OUTPUT (intrinsic integration)")
need("bReducedMotion" in ah and "bReducedMotion" in ac, "effect must honor reduced motion")
need("OnSolveEffectStarted" in ah, "effect must expose a Blueprint hook for authored FX")
need("/Engine/BasicShapes/" in ac, "effect must be cook-safe (engine primitives only)")
need("ConfigureSolveEffect" in ah and "ConfigureSolveEffect" in ac, "effect must expose a configure entry point")
gm = read(SRC / "CodeRescueGameMode.cpp")
for token in ["ACodeRescueSolveEffectActor", "ConfigureSolveEffect", "OutputMagnitudeWorldEffect",
              "DataDrivenFilterNode", "FilterNodeEvacOrder", "PhysicsSafeNonBlocking",
              "Mentor Point Gesture", "Survivor Boarding Pose Proxy",
              "DataDrivenBooleanNode", "BooleanAirlockWorldEffect", "TruthTableRescueArtifact",
              "Boolean Mentor Points At TT Row", "Boolean Survivor Exit Pose",
              "DataDrivenStringNode", "ReverseCodeWorldEffect", "ReversedSequenceArtifact",
              "Reverse Mentor Shows Last-To-First", "Reverse Survivor Unlock Pose"]:
    need(token in gm, f"GameMode solved route must include {token}")

# --- 4. documentation + wiring handoff ---
need((DOC / "LEARNING_VERTICAL_SLICE.md").exists(), "missing LEARNING_VERTICAL_SLICE.md")
need((DOC / "00_OVERVIEW.md").exists(), "missing 00_OVERVIEW.md")
need((ROOT / "Content/CodeRescueData/data_driven_filter_node_manifest.tsv").exists(),
     "missing data_driven_filter_node_manifest.tsv")
need((ROOT / "Content/CodeRescueData/data_driven_boolean_node_manifest.tsv").exists(),
     "missing data_driven_boolean_node_manifest.tsv")
need((ROOT / "Content/CodeRescueData/data_driven_reverse_node_manifest.tsv").exists(),
     "missing data_driven_reverse_node_manifest.tsv")
wiring = read(DOC / "LEARNING_VERTICAL_SLICE.md")
need("Mac" in wiring and ("compile" in wiring.lower()), "wiring doc must state the Mac-compile DoD gate")
need("Runtime terminal wiring" in wiring and "Solved-route world response" in wiring,
     "wiring doc must describe completed runtime wiring")
need("tier-2 boolean" in wiring and "DataDrivenBooleanNode" in wiring,
     "wiring doc must describe the boolean node runtime slice")
need("tier-5 strings" in wiring and "DataDrivenStringNode" in wiring,
     "wiring doc must describe the reverse node runtime slice")

if errors:
    print(f"[verify_learning_vertical_slice_pass] {len(errors)} problem(s):")
    for e in errors:
        print("  " + e)
    sys.exit(1)
print("[verify_learning_vertical_slice_pass] PASS - learning vertical data + C++ + docs present and consistent")
print("  NOTE: static check only; compile on Mac + playtest is the Definition-of-Done gate.")
