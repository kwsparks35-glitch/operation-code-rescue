#!/usr/bin/env python3
"""Verify non-human release readiness automation and latest evidence."""

from __future__ import annotations

import csv
import json
from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
DATA = PROJECT_ROOT / "Content/CodeRescueData"
DOCS = PROJECT_ROOT / "Documentation"
SCRIPTS = PROJECT_ROOT / "Scripts"
PACKAGE_REPORT = PROJECT_ROOT / "Saved/Release/package_integrity_latest.json"
AUDIO_REPORT = PROJECT_ROOT / "Saved/AudioAudit/maple_audio_audit_latest.json"
RELEASE_MANIFEST = PROJECT_ROOT / "Saved/Release/release_manifest_latest.json"

errors: list[str] = []
infos: list[str] = []


def check(cond: bool, msg: str) -> None:
    if not cond:
        errors.append(msg)


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def load_json(path: Path) -> dict:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return {}
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        errors.append(f"invalid JSON {path.relative_to(PROJECT_ROOT)}: {exc}")
        return {}


def rows(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        errors.append(f"missing {path.relative_to(PROJECT_ROOT)}")
        return []
    with path.open(encoding="utf-8", newline="") as fh:
        return list(csv.DictReader(fh, delimiter="\t"))


gate_rows = rows(DATA / "nonhuman_release_readiness_gates.tsv")
check(len(gate_rows) >= 8, "nonhuman readiness gate manifest must cover release gates")
for gate in ("package_integrity", "maple_audio_technical", "release_manifest", "support_bundle", "signing_credentials"):
    check(any(row.get("gate_id") == gate for row in gate_rows), f"missing nonhuman readiness gate: {gate}")

for rel in (
    "audit_maple_audio_assets.py",
    "verify_package_integrity_pass.py",
    "verify_nonhuman_release_readiness_pass.py",
):
    check((SCRIPTS / rel).exists(), f"missing script: Scripts/{rel}")

local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
for token in (
    "audit_maple_audio_assets.py",
    "verify_package_integrity_pass.py",
    "verify_nonhuman_release_readiness_pass.py",
):
    check(token in local_ci, f"Run_Local_CI_Readiness.command must run {token}")

support = read(SCRIPTS / "create_support_bundle.py")
for token in (
    "package_integrity_latest.json",
    "maple_audio_audit_latest.json",
    "NONHUMAN_RELEASE_READINESS_PASS_2026-06-18.md",
    "nonhuman_release_readiness_gates.tsv",
):
    check(token in support, f"support bundle must include {token}")

manifest_script = read(SCRIPTS / "generate_release_manifest.py")
for token in (
    "package_integrity_report",
    "maple_audio_audit_report",
    "verify_nonhuman_release_readiness_pass.py",
):
    check(token in manifest_script, f"release manifest generator must include {token}")

package = load_json(PACKAGE_REPORT)
if package:
    check(package.get("local_package_ready") is True, "package integrity report must be locally ready")
    check(package.get("hard_error_count") == 0, "package integrity report must have zero hard errors")
    if package.get("credential_blocker_count", 0):
        infos.append(
            f"distribution credential blockers remain: {package.get('credential_blocker_count')}"
        )

audio = load_json(AUDIO_REPORT)
if audio:
    check(audio.get("expected_rows") == 230, "Maple audio audit must cover 230 expected rows")
    check(audio.get("expected_wavs_present") == 230, "Maple audio audit must find 230 expected WAVs")
    check(audio.get("expected_uassets_present") == 230, "Maple audio audit must find 230 expected SoundWave assets")
    check(audio.get("error_count") == 0, "Maple audio audit must have zero errors")

manifest = load_json(RELEASE_MANIFEST)
if manifest:
    check(manifest.get("package_integrity_report", {}).get("exists") is True,
          "release manifest must include package integrity report")
    check(manifest.get("maple_audio_audit_report", {}).get("exists") is True,
          "release manifest must include Maple audio audit report")

doc = read(DOCS / "NONHUMAN_RELEASE_READINESS_PASS_2026-06-18.md")
check("physical human-testing" in doc.lower(), "nonhuman pass doc must state physical human-testing boundary")
check("credential-only" in doc.lower(), "nonhuman pass doc must state credential-only signing boundary")

roadmap = read(DOCS / "DEMO_READINESS_ROADMAP_2026-06-18.md")
check("Automated Non-Human Gates" in roadmap, "demo roadmap must include automated non-human gates")

if errors:
    for error in errors:
        print(f"[verify_nonhuman_release_readiness_pass] FAIL: {error}")
    sys.exit(1)
for info in infos:
    print(f"[verify_nonhuman_release_readiness_pass] INFO: {info}")
print("[verify_nonhuman_release_readiness_pass] PASS: non-human release readiness evidence intact")
