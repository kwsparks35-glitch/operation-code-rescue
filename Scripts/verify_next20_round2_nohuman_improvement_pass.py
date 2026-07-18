#!/usr/bin/env python3
"""Verify the second no-human next-20 improvement pass contract."""

from __future__ import annotations

import csv
import json
import os
from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
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


manifest_path = DATA / "nohuman_next20_round2_recommendations.tsv"
if manifest_path.exists():
    with manifest_path.open(encoding="utf-8", newline="") as fh:
        rows = list(csv.DictReader(fh, delimiter="\t"))
else:
    rows = []
    errors.append("missing Content/CodeRescueData/nohuman_next20_round2_recommendations.tsv")

check(len(rows) == 20, f"expected 20 round-two recommendation rows, found {len(rows)}")
statuses = {row.get("status", "") for row in rows}
for required in ("implemented", "manual_boundary", "credential_boundary"):
    check(required in statuses, f"round-two manifest missing status {required}")
check(any(row.get("boundary") == "physical_human" for row in rows), "human boundary must be explicit")
check(any(row.get("boundary") == "external_credentials" for row in rows), "credential boundary must be explicit")

generator = read(PROJECT_ROOT / "Scripts/generate_nohuman_next20_round2_evidence.py")
for token in (
    "input_mapping_audit",
    "curriculum_progression_audit",
    "screenshot_readability_audit",
    "source_control_slices",
    "support_bundle_audit",
    "save_schema_inventory",
):
    check(token in generator, f"round-two evidence generator missing {token}")

full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
release_manifest = read(PROJECT_ROOT / "Scripts/generate_release_manifest.py")
support_bundle = read(PROJECT_ROOT / "Scripts/create_support_bundle.py")
for text, label in ((full_qa, "full QA"), (local_ci, "local CI")):
    check("generate_nohuman_next20_round2_evidence.py" in text, f"{label} must generate round-two evidence")
    check("verify_next20_round2_nohuman_improvement_pass.py" in text, f"{label} must verify round-two evidence")
check("nohuman_next20_round2_evidence" in release_manifest, "release manifest must reference round-two evidence")
check("nohuman_next20_round2_improvement" in release_manifest, "release manifest must expose round-two feature flag")
check("generate_nohuman_next20_round2_evidence.py" in support_bundle, "support bundle must refresh round-two evidence")
check("nohuman_next20_round2_recommendations.tsv" in support_bundle, "support bundle must include round-two recommendations")

runner = PROJECT_ROOT / "Run_NoHuman_Next20_Round2_Improvement.command"
check(runner.exists(), "missing Run_NoHuman_Next20_Round2_Improvement.command")
if runner.exists():
    check(os.access(runner, os.X_OK), "round-two runner must be executable")

doc = read(DOCS / "NOHUMAN_NEXT20_ROUND2_IMPROVEMENT_PASS_2026-06-24.md")
doc_normalized = " ".join(doc.split())
for phrase in (
    "Second-cycle recommendations are implemented as dashboardable evidence",
    "Human playthrough remains outside this pass",
    "Apple signing, notarization, and hosted release credentials remain external",
    "This pass does not claim subjective balance approval",
):
    check(phrase in doc_normalized, f"round-two pass doc missing phrase: {phrase}")

for path in (
    PROJECT_ROOT / "README_MAC.md",
    DOCS / "RELEASE_CHECKLIST.md",
    DOCS / "DEMO_READINESS_ROADMAP_2026-06-18.md",
):
    check("Run_NoHuman_Next20_Round2_Improvement.command" in read(path), f"{path.name} must mention round-two runner")

latest = PROJECT_ROOT / "Saved/Release/nohuman_next20_round2_evidence_latest.json"
if latest.exists():
    try:
        payload = json.loads(latest.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        payload = {}
        errors.append(f"latest round-two evidence JSON is invalid: {exc}")
    check(payload.get("recommendation_count") == 20, "round-two evidence JSON must report 20 recommendations")
    check(payload.get("input_mapping_audit", {}).get("control_manifest_controls", 0) >= 18,
          "round-two evidence must include input mapping audit")
    check(payload.get("curriculum_progression_audit", {}).get("entry_count", 0) > 0,
          "round-two evidence must include curriculum progression entries")
    check(payload.get("data_manifest_inventory", {}).get("total_files", 0) >= 20,
          "round-two evidence must inventory data manifests")
    check("human_testing_boundary" in payload, "round-two evidence must record human boundary")
    check("credential_boundary" in payload, "round-two evidence must record credential boundary")
else:
    errors.append("missing Saved/Release/nohuman_next20_round2_evidence_latest.json")

if errors:
    for error in errors:
        print(f"[verify_next20_round2_nohuman_improvement_pass] FAIL: {error}")
    sys.exit(1)

print("[verify_next20_round2_nohuman_improvement_pass] PASS: round-two no-human next20 contract intact")
