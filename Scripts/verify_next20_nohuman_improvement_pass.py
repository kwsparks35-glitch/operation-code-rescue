#!/usr/bin/env python3
"""Verify the no-human next-20 improvement pass contract."""

from __future__ import annotations

import csv
import json
from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC = PROJECT_ROOT / "Source/CodeRescueUnreal"
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOCS = PROJECT_ROOT / "Documentation"
errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def check(cond: bool, msg: str) -> None:
    if not cond:
        errors.append(msg)


manifest_path = DATA / "nohuman_next20_recommendations.tsv"
if manifest_path.exists():
    with manifest_path.open(encoding="utf-8", newline="") as fh:
        rows = list(csv.DictReader(fh, delimiter="\t"))
else:
    rows = []
    errors.append("missing Content/CodeRescueData/nohuman_next20_recommendations.tsv")

check(len(rows) == 20, f"expected 20 recommendation rows, found {len(rows)}")
statuses = {row.get("status", "") for row in rows}
for required in ("implemented", "automated_ready", "credential_boundary", "manual_boundary"):
    check(required in statuses, f"recommendation manifest missing status {required}")
check(any(row.get("boundary") == "physical_human" for row in rows), "human-testing boundary must be explicit")
check(any(row.get("boundary") == "external_credentials" for row in rows), "credential boundary must be explicit")

gi_h = read(SRC / "CodeRescueGameInstance.h")
gi_cpp = read(SRC / "CodeRescueGameInstance.cpp")
settings_h = read(SRC / "CodeRescueSettingsWidget.h")
settings_cpp = read(SRC / "CodeRescueSettingsWidget.cpp")
save = read(SRC / "CodeRescueSaveGame.h")
profile_script = read(PROJECT_ROOT / "Scripts/profile_city_layers_static.py")
control_script = read(PROJECT_ROOT / "Scripts/apply_control_remap_profile.py")
evidence_script = read(PROJECT_ROOT / "Scripts/generate_nohuman_next20_evidence.py")

for token in (
    "ControlProfileName",
    "ControlProfileExportCount",
):
    check(token in save and token in gi_h and token in gi_cpp, f"control profile save/runtime token missing: {token}")
check("GetControlProfileSummary" in gi_h and "GetControlProfileSummary" in gi_cpp,
      "GameInstance must expose control profile summary")
check("ExportControlProfileReviewFile" in gi_h and "runtime_controls_profile.json" in gi_cpp,
      "GameInstance must export runtime control profile JSON")
check("ExportControlsButton" in settings_h and "OnExportControlsClicked" in settings_cpp,
      "Settings widget must expose an export control profile button")
check("performance_city_layer_budget.tsv" in profile_script and "budget_contract" in profile_script,
      "static profiler must include performance budget contracts")
check("implementation_note" in control_script and "direct_cpp_controls" in gi_cpp,
      "control profile exports must document direct-binding safety limits")
check("nohuman_next20_evidence_latest.json" in evidence_script,
      "evidence generator must write latest no-human next20 JSON")

doc = read(DOCS / "NOHUMAN_NEXT20_IMPROVEMENT_PASS_2026-06-24.md")
doc_normalized = " ".join(doc.split())
for phrase in (
    "Human playthrough remains the primary excluded gate",
    "Developer ID signing and notarization remain credential-bound",
    "GameMode modularization remains an architecture continuation",
):
    check(phrase in doc_normalized, f"no-human pass doc missing phrase: {phrase}")

latest = PROJECT_ROOT / "Saved/Release/nohuman_next20_evidence_latest.json"
if latest.exists():
    try:
        payload = json.loads(latest.read_text(encoding="utf-8"))
        check(payload.get("recommendation_count") == 20, "latest evidence JSON must report 20 recommendations")
    except json.JSONDecodeError as exc:
        errors.append(f"latest evidence JSON is invalid: {exc}")

if errors:
    for error in errors:
        print(f"[verify_next20_nohuman_improvement_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_next20_nohuman_improvement_pass] PASS: no-human next20 improvement contract intact")
