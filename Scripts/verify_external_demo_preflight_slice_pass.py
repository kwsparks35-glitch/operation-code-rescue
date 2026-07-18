#!/usr/bin/env python3
"""Static verifier for the external demo preflight slice."""

from __future__ import annotations

from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
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


package_integrity = read(PROJECT_ROOT / "Scripts/verify_package_integrity_pass.py")
release_manifest = read(PROJECT_ROOT / "Scripts/generate_release_manifest.py")
nonhuman_verifier = read(PROJECT_ROOT / "Scripts/verify_nonhuman_release_readiness_pass.py")
support_bundle = read(PROJECT_ROOT / "Scripts/create_support_bundle.py")
preflight_manifest = read(DATA / "external_demo_preflight_manifest.tsv")
nonhuman_gates = read(DATA / "nonhuman_release_readiness_gates.tsv")
creative_plan = read(DATA / "creative_development_inclusion_plan.tsv")
human_qa = read(DATA / "human_qa_signoff_checklist.tsv")
run_nonhuman = read(PROJECT_ROOT / "Run_NonHuman_Release_Readiness.command")
full_qa = read(PROJECT_ROOT / "Run_Full_QA_Audit.command")
local_ci = read(PROJECT_ROOT / "Run_Local_CI_Readiness.command")
slice_doc = read(DOC / "EXTERNAL_DEMO_PREFLIGHT_SLICE.md")
demo_roadmap = read(PROJECT_ROOT / "Documentation/DEMO_READINESS_ROADMAP_2026-06-18.md")
nonhuman_doc = read(PROJECT_ROOT / "Documentation/NONHUMAN_RELEASE_READINESS_PASS_2026-06-18.md")
project_review = read(SOURCE_DOC / "PROJECT_STATE_REVIEW_2026-06-25.md")
top50 = read(SOURCE_DOC / "TOP_50_RECOMMENDATIONS_2026-06-25.md")
progress = read(PROJECT_ROOT / "progress.md")
self_source = read(PROJECT_ROOT / "Scripts/verify_external_demo_preflight_slice_pass.py")


check_all(
    package_integrity,
    [
        "--strict-distribution",
        "--expected-bundle-id",
        "codesign",
        "spctl",
        "credential_blockers",
        "local_package_ready",
        "external_distribution_ready",
        "CFBundleIdentifier",
        "Gatekeeper assessment",
    ],
    "package integrity script must expose local and strict distribution preflight checks",
)

check_all(
    release_manifest,
    [
        "package_integrity_report",
        "maple_audio_audit_report",
        "visual_regression_latest",
        "git_status_short",
        "nonhuman_release_readiness",
    ],
    "release manifest generator must capture package and support evidence",
)

check_all(
    nonhuman_verifier + support_bundle,
    [
        "package_integrity_latest.json",
        "maple_audio_audit_latest.json",
        "release_manifest_latest.json",
        "nonhuman_release_readiness_gates.tsv",
    ],
    "non-human verifier and support bundle must carry release evidence",
)

check_all(
    preflight_manifest,
    [
        "package_integrity",
        "bundle_identity",
        "local_codesign",
        "gatekeeper_notarization",
        "release_manifest",
        "support_bundle",
        "packaged_smokes",
        "human_playtest",
        "credential_blocked",
        "local_ready",
    ],
    "external demo preflight manifest must list local-ready and credential-bound gates",
)

check_all(
    nonhuman_gates,
    [
        "external_demo_preflight",
        "verify_external_demo_preflight_slice_pass.py",
        "signing_credentials",
        "physical_human_playtest",
        "Developer ID certificate",
        "notary credentials",
    ],
    "non-human release gates must include the external demo preflight",
)

check_all(
    creative_plan,
    [
        "signed external demo preparation",
        "verify_external_demo_preflight_slice_pass.py",
        "verify_package_integrity_pass.py",
        "generate_release_manifest.py",
        "verify_nonhuman_release_readiness_pass.py",
        "credential-bound notarization checklist",
    ],
    "creative plan must route the P2 packaging row through external-demo preflight checks",
)

check_all(
    human_qa,
    [
        "Release Package",
        "external demo preflight",
        "signing credential boundary",
        "support bundle",
    ],
    "human QA release row must include external-demo preflight review",
)

for script_name, script_text in (
    ("Run_NonHuman_Release_Readiness.command", run_nonhuman),
    ("Run_Full_QA_Audit.command", full_qa),
    ("Run_Local_CI_Readiness.command", local_ci),
):
    check(
        "verify_external_demo_preflight_slice_pass.py" in script_text,
        f"{script_name} must run the external demo preflight verifier",
    )

check_all(
    run_nonhuman,
    [
        "audit_maple_audio_assets.py",
        "verify_package_integrity_pass.py",
        "generate_release_manifest.py",
        "verify_nonhuman_release_readiness_pass.py",
        "create_support_bundle.py",
    ],
    "non-human release command must run all release evidence generators",
)

check_all(
    slice_doc,
    [
        "External Demo Preflight Slice",
        "Runtime And Release Coverage",
        "Current Package Evidence",
        "Boundaries",
        "verify_package_integrity_pass.py --strict-distribution",
        "Gatekeeper",
        "Developer ID",
        "notarization",
    ],
    "slice documentation must explain coverage and credential boundaries",
)

check_all(
    demo_roadmap,
    [
        "Run_NonHuman_Release_Readiness.command",
        "package_integrity_latest.json",
        "signed/notarized distribution",
        "verify_package_integrity_pass.py --strict-distribution",
    ],
    "demo readiness roadmap must contain the external distribution path",
)

check_all(
    nonhuman_doc,
    [
        "signing-preflight",
        "Developer signing/notarization",
        "Team ID",
        "notary credentials",
        "Strict mode should fail today",
    ],
    "non-human release doc must state signing and notarization boundaries",
)

check_all(
    project_review,
    [
        "compile, run, and package on Mac",
        "support bundle",
        "No human playtest / no signing & notarization / no hosted CI",
        "Apple Developer signing/notarization",
    ],
    "June 25 project review must describe release tooling and external boundaries",
)

check_all(
    top50,
    [
        "Apple Developer signing/notarization",
        "release pipeline",
        "release gates",
    ],
    "Top 50 source guidance must contain release pipeline recommendations",
)

check_all(
    progress,
    [
        "External demo preflight slice",
        "external_demo_preflight_manifest.tsv",
        "verify_external_demo_preflight_slice_pass.py",
        "Gatekeeper/notarization",
    ],
    "progress log must record the external demo preflight slice",
)

check("verify_external_demo_preflight_slice_pass.py" in self_source,
      "static verifier should identify itself")

if errors:
    print("[verify_external_demo_preflight_slice_pass] FAIL")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("[verify_external_demo_preflight_slice_pass] PASS: external demo preflight verified")
