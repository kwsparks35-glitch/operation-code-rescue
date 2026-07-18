#!/usr/bin/env python3
"""Generate a reviewable local release manifest for Operation Code Rescue."""

from __future__ import annotations

import csv
import hashlib
import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
APP_PATH = PROJECT_ROOT / "PackagedMac/Mac/CodeRescueUnreal.app"
OUT_DIR = PROJECT_ROOT / "Saved/Release"
RADIO_DIR = PROJECT_ROOT / "Content/CodeRescueAssets/Audio/RadioSamples"
BRIEFINGS = PROJECT_ROOT / "Content/CodeRescueData/radio_briefings.tsv"
FEMALE_VOICES = {"Samantha", "Victoria", "Kyoko", "Tessa", "Karen"}


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def directory_stats(path: Path) -> dict:
    if not path.exists():
        return {"exists": False}
    files = [p for p in path.rglob("*") if p.is_file()]
    size = sum(p.stat().st_size for p in files)
    mtime = max((p.stat().st_mtime for p in files), default=path.stat().st_mtime)
    return {
        "exists": True,
        "path": str(path),
        "file_count": len(files),
        "size_bytes": size,
        "size_gb": round(size / (1024 ** 3), 2),
        "latest_mtime_utc": datetime.fromtimestamp(mtime, timezone.utc).isoformat(),
    }


def file_entry(path: Path) -> dict:
    if not path.exists():
        return {"path": str(path), "exists": False}
    stat = path.stat()
    return {
        "path": str(path),
        "exists": True,
        "size_bytes": stat.st_size,
        "mtime_utc": datetime.fromtimestamp(stat.st_mtime, timezone.utc).isoformat(),
        "sha256": sha256(path),
    }


def git_status() -> list[str]:
    try:
        result = subprocess.run(
            ["git", "status", "--short"],
            cwd=PROJECT_ROOT,
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        )
    except OSError:
        return ["git unavailable"]
    return result.stdout.splitlines()


def maple_coverage() -> dict:
    if not BRIEFINGS.exists():
        return {"female_total": 0, "generated": 0, "missing": [], "error": "missing radio_briefings.tsv"}
    with BRIEFINGS.open(encoding="utf-8", newline="") as fh:
        rows = [r for r in csv.DictReader(fh, delimiter="\t")
                if r.get("voice", "").strip() in FEMALE_VOICES]
    missing = [
        r["slug"]
        for r in rows
        if not (RADIO_DIR / f"{r['slug']}_radio_briefing.wav").exists()
    ]
    return {
        "female_total": len(rows),
        "generated": len(rows) - len(missing),
        "missing": missing[:30],
        "missing_count": len(missing),
    }


def latest_visual_screenshots(limit: int = 12) -> list[dict]:
    roots = [
        PROJECT_ROOT / "Saved/Screenshots/VisualReview",
        PROJECT_ROOT / "Saved/Screenshots/MacEditor",
    ]
    shots: list[Path] = []
    for root in roots:
        if root.exists():
            shots.extend(p for p in root.glob("*.png") if p.is_file())
    shots.sort(key=lambda p: p.stat().st_mtime, reverse=True)
    return [file_entry(p) for p in shots[:limit]]


def build_manifest() -> dict:
    logs = [
        "Saved/Logs/HeadlessFullQASmoke.log",
        "Saved/Logs/PackagedSmoke_null.log",
        "Saved/Logs/PackagedSmoke_render.log",
        "Saved/Logs/PerformanceProfile.log",
        "Saved/Logs/VisualReviewCapture.log",
    ]
    static_verifiers = [
        "Scripts/verify_june18_public_hardening_pass.py",
        "Scripts/verify_maple_sinister_narration_pass.py",
        "Scripts/verify_june12_city_realization_pass.py",
        "Scripts/verify_save_compatibility_pass.py",
        "Scripts/verify_asset_budget_pass.py",
        "Scripts/verify_demo_readiness_pass.py",
        "Scripts/audit_maple_audio_assets.py",
        "Scripts/verify_package_integrity_pass.py",
        "Scripts/verify_nonhuman_release_readiness_pass.py",
        "Scripts/verify_next20_nohuman_improvement_pass.py",
        "Scripts/verify_next20_round2_nohuman_improvement_pass.py",
        "Scripts/verify_runtime_log_contracts.py",
    ]
    return {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "project_root": str(PROJECT_ROOT),
        "package": directory_stats(APP_PATH),
        "logs": [file_entry(PROJECT_ROOT / rel) for rel in logs],
        "static_verifiers": [file_entry(PROJECT_ROOT / rel) for rel in static_verifiers],
        "maple_narration": maple_coverage(),
        "package_integrity_report": file_entry(PROJECT_ROOT / "Saved/Release/package_integrity_latest.json"),
        "nohuman_next20_evidence": file_entry(PROJECT_ROOT / "Saved/Release/nohuman_next20_evidence_latest.json"),
        "nohuman_next20_round2_evidence": file_entry(PROJECT_ROOT / "Saved/Release/nohuman_next20_round2_evidence_latest.json"),
        "maple_audio_audit_report": file_entry(PROJECT_ROOT / "Saved/AudioAudit/maple_audio_audit_latest.json"),
        "visual_regression_latest": latest_visual_screenshots(),
        "city_layer_profile": file_entry(PROJECT_ROOT / "Saved/Profiling/city_layer_static_profile_latest.json"),
        "git_status_short": git_status(),
        "known_allowed_warnings": [
            "LogNavigationDirtyArea immediate-quit dirty area diagnostic",
            "LogCrowdFollowing immediate-quit RecastNavMesh diagnostic",
            "macOS CoreAudio sample-rate query warning in render smoke",
        ],
        "feature_flags": {
            "external_validation_default": "CodeRescue.AllowExternalCodeValidation=0",
            "external_validation_trusted_qa": "-AllowExternalCodeValidation",
            "maple_generation": "resumable via Generate_Maple_Sinister_Narrations.command",
            "nonhuman_release_readiness": "Run_NonHuman_Release_Readiness.command",
            "nohuman_next20_improvement": "Run_NoHuman_Next20_Improvement.command",
            "nohuman_next20_round2_improvement": "Run_NoHuman_Next20_Round2_Improvement.command",
            "difficulty_presets": ["Story", "Easy", "Normal", "Hard", "Survival", "Nightmare"],
        },
    }


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest = build_manifest()
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    stamped = OUT_DIR / f"release_manifest_{stamp}.json"
    latest = OUT_DIR / "release_manifest_latest.json"
    text = json.dumps(manifest, indent=2, sort_keys=True)
    stamped.write_text(text + "\n", encoding="utf-8")
    latest.write_text(text + "\n", encoding="utf-8")
    print(f"[generate_release_manifest] wrote {latest}")
    print(f"[generate_release_manifest] package exists={manifest['package'].get('exists')} maple={manifest['maple_narration'].get('generated')}/{manifest['maple_narration'].get('female_total')}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
