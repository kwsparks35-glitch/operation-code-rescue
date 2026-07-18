#!/usr/bin/env python3
"""verify_pedagogy_slice_pass_2026_07_04b.py

Gate for the 2026-07-04 part-3 pedagogy slice: curriculum 60 (item 25),
interactive predict-the-output drill (27), concept mastery meter (32),
reflective debrief (33), minimap beacon glyph dots (46).
"""
from __future__ import annotations
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "Source" / "CodeRescueUnreal"
FAILURES: list[str] = []


def check(cond: bool, msg: str) -> None:
    print(f"[verify_pedagogy_slice_pass_2026_07_04b] {'PASS' if cond else 'FAIL'}: {msg}")
    if not cond:
        FAILURES.append(msg)


def has(path: Path, *needles: str) -> bool:
    try:
        t = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return False
    return all(n in t for n in needles)


# --- item 25: curriculum 60 with tiers 6-10 at full depth -----------------------
db = json.loads((ROOT / "Content/CodeRescueData/curriculum_database.json").read_text())
entries = db["entries"]
check(len(entries) == 60, f"curriculum has 60 entries (found {len(entries)})")
per_tier = {}
for e in entries:
    per_tier[e.get("tier", "legacy")] = per_tier.get(e.get("tier", "legacy"), 0) + 1
for tier in (6, 7, 8, 9, 10):
    check(per_tier.get(tier, 0) >= 6, f"tier {tier} has >= 6 entries ({per_tier.get(tier, 0)})")
new = [e for e in entries if e.get("authored") == "claude-2026-07-04-item25"]
check(len(new) == 24, f"24 new item-25 entries present ({len(new)})")
for e in new:
    st = e.get("starter", {})
    ok = all(k in st and st[k].strip() for k in ("python", "java", "c", "cplus", "cpp", "matlab"))
    if not ok:
        check(False, f"{e['id']}: missing one of the 6 starters")
        break
else:
    check(True, "every new entry ships all 6 language starters")
check(all((e.get("visible_tests") and e.get("hidden_tests")) for e in new),
      "every new entry declares visible AND hidden tests")
check(has(ROOT / "Scripts/build_curriculum_tiers6_10_2026_07_04.py",
          "REFERENCE EXECUTION", "lint_and_test"),
      "curriculum builder with reference-execution validation is committed")

# --- item 27: predict-the-output drill -------------------------------------------
term_h = SRC / "CodeTerminalWidget.h"
term_c = SRC / "CodeTerminalWidget.cpp"
check(has(term_h, "PredictionDrillRow", "OnPredictChoiceA", "AnswerPrediction"),
      "terminal owns the prediction drill row + handlers")
check(has(term_c, "PREDICT IT: for input", "PredictionCorrectIndex = static_cast<int32>(GetTypeHash"),
      "drill builds per-challenge question with deterministic choice slots")
check(has(term_c, "prediction-drill"),
      "prediction answers feed concept telemetry")
check(has(term_c, "SetupPredictionDrill();   // 2026-07-04 item 27"),
      "drill refreshes with the terminal content")

# --- item 32: mastery meter --------------------------------------------------------
j_h = SRC / "CodeRescueObjectiveJournalWidget.h"
j_c = SRC / "CodeRescueObjectiveJournalWidget.cpp"
check(has(j_h, "ConceptMasteryText"), "journal owns the mastery meter block")
check(has(j_c, "CONCEPT MASTERY (solves / attempts):", "SuccessfulValidations + P.FailedValidations"),
      "journal renders per-concept bars from saved progress")

# --- item 33: reflective debrief -----------------------------------------------------
check(has(term_c, "REFLECT (say it out loud or jot it):", "DebriefPrompts"),
      "solve output appends a rotating reflective debrief prompt")

# --- item 46: minimap beacon dots -----------------------------------------------------
m_c = SRC / "CodeRescueMinimapWidget.cpp"
check(has(m_c, "ACodeRescueBeaconMarkerActor", "BeaconCount"),
      "minimap draws beacon markers as their own POI family")

print()
if FAILURES:
    print(f"[verify_pedagogy_slice_pass_2026_07_04b] {len(FAILURES)} FAILURE(S)")
    sys.exit(1)
print("[verify_pedagogy_slice_pass_2026_07_04b] ALL CHECKS PASSED")
