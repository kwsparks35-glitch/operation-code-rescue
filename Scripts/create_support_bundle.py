#!/usr/bin/env python3
"""Bundle logs, manifests, screenshots, and key docs for support review."""

from __future__ import annotations

import subprocess
import sys
import zipfile
from datetime import datetime
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = PROJECT_ROOT / "Saved/SupportBundles"


def add_if_exists(zf: zipfile.ZipFile, path: Path) -> None:
    if path.is_file():
        zf.write(path, path.relative_to(PROJECT_ROOT))


def main() -> int:
    subprocess.run([sys.executable, "Scripts/generate_release_manifest.py"], cwd=PROJECT_ROOT, check=False)
    subprocess.run([sys.executable, "Scripts/generate_visual_regression_manifest.py"], cwd=PROJECT_ROOT, check=False)
    subprocess.run([sys.executable, "Scripts/generate_nohuman_next20_evidence.py"], cwd=PROJECT_ROOT, check=False)
    subprocess.run([sys.executable, "Scripts/generate_nohuman_next20_round2_evidence.py"], cwd=PROJECT_ROOT, check=False)

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    out = OUT_DIR / f"code_rescue_support_bundle_{datetime.now().strftime('%Y%m%d_%H%M%S')}.zip"
    added: set[Path] = set()

    def add_once(zf: zipfile.ZipFile, path: Path) -> None:
        resolved = path.resolve()
        if resolved in added:
            return
        added.add(resolved)
        add_if_exists(zf, path)

    with zipfile.ZipFile(out, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        for rel in (
            "Saved/Release/release_manifest_latest.json",
            "Saved/Release/package_integrity_latest.json",
            "Saved/Release/nohuman_next20_evidence_latest.json",
            "Saved/Release/nohuman_next20_round2_evidence_latest.json",
            "Saved/AudioAudit/maple_audio_audit_latest.json",
            "Saved/VisualRegression/visual_regression_manifest_latest.json",
            "Saved/Profiling/city_layer_static_profile_latest.json",
            "Saved/Config/ControlProfiles/default_controls_profile.json",
            "Saved/Config/ControlProfiles/runtime_controls_profile.json",
            "Saved/Logs/HeadlessFullQASmoke.log",
            "Saved/Logs/PackagedSmoke_null.log",
            "Saved/Logs/PackagedSmoke_render.log",
            "Saved/Logs/PerformanceProfile.log",
            "Saved/Logs/VisualReviewCapture.log",
            "Documentation/DISTRIBUTION_GUIDE_MAC.md",
            "Documentation/QA_PLAYTEST_CHECKLIST.md",
            "Documentation/NEXT_20_IMPLEMENTATION_PASS_2026-06-18.md",
            "Documentation/NONHUMAN_RELEASE_READINESS_PASS_2026-06-18.md",
            "Documentation/NOHUMAN_NEXT20_IMPROVEMENT_PASS_2026-06-24.md",
            "Documentation/NOHUMAN_NEXT20_ROUND2_IMPROVEMENT_PASS_2026-06-24.md",
            "Documentation/DEMO_READINESS_ROADMAP_2026-06-18.md",
            "Documentation/SIGNING_NOTARIZATION_RUNBOOK_2026-06-18.md",
            "Content/CodeRescueData/nonhuman_release_readiness_gates.tsv",
            "Content/CodeRescueData/nohuman_next20_recommendations.tsv",
            "Content/CodeRescueData/nohuman_next20_round2_recommendations.tsv",
        ):
            add_once(zf, PROJECT_ROOT / rel)
        for path in (PROJECT_ROOT / "Content/CodeRescueData").glob("*manifest*.tsv"):
            add_once(zf, path)
        for path in (PROJECT_ROOT / "Content/CodeRescueData").glob("*budget*.tsv"):
            add_once(zf, path)
        shots = []
        for root in (PROJECT_ROOT / "Saved/Screenshots/VisualReview", PROJECT_ROOT / "Saved/Screenshots/MacEditor"):
            if root.exists():
                shots.extend(p for p in root.glob("*.png") if p.is_file())
        shots.sort(key=lambda p: p.stat().st_mtime, reverse=True)
        for path in shots[:12]:
            add_once(zf, path)
    print(f"[create_support_bundle] wrote {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
