#!/usr/bin/env python3
"""Generate the second no-human next-20 improvement evidence dashboard.

The first no-human pass turned recommendation status into a durable ledger.
This second pass goes one level deeper: it collects review dashboards for the
areas that can regress between builds without requiring a human playthrough.
Missing optional artifacts are recorded instead of treated as fatal so the JSON
can be used on fresh workstations and after partial QA runs.
"""

from __future__ import annotations

import csv
import json
import re
import subprocess
import warnings
import zipfile
from collections import Counter, defaultdict
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


PROJECT_ROOT = Path(__file__).resolve().parents[1]
DATA_DIR = PROJECT_ROOT / "Content/CodeRescueData"
OUT_DIR = PROJECT_ROOT / "Saved/Release"
RECOMMENDATIONS = DATA_DIR / "nohuman_next20_round2_recommendations.tsv"

KEY_ARTIFACTS = {
    "release_manifest": "Saved/Release/release_manifest_latest.json",
    "first_nohuman_evidence": "Saved/Release/nohuman_next20_evidence_latest.json",
    "round2_nohuman_evidence": "Saved/Release/nohuman_next20_round2_evidence_latest.json",
    "package_integrity": "Saved/Release/package_integrity_latest.json",
    "visual_regression_manifest": "Saved/VisualRegression/visual_regression_manifest_latest.json",
    "city_layer_static_profile": "Saved/Profiling/city_layer_static_profile_latest.json",
    "default_control_profile": "Saved/Config/ControlProfiles/default_controls_profile.json",
    "maple_audio_audit": "Saved/AudioAudit/maple_audio_audit_latest.json",
    "headless_full_qa_log": "Saved/Logs/HeadlessFullQASmoke.log",
    "packaged_null_smoke_log": "Saved/Logs/PackagedSmoke_null.log",
    "packaged_render_smoke_log": "Saved/Logs/PackagedSmoke_render.log",
}

ALLOWED_WARNING_FRAGMENTS = (
    "LogNavigationDirtyArea: Warning: Skipped some dirty area creation",
    "LogCrowdFollowing: Warning: Unable to find RecastNavMesh instance",
    "LogAudioMixerAudioUnit: Warning: Error querying Sample Rate",
)


def rel(path: Path) -> str:
    try:
        return str(path.relative_to(PROJECT_ROOT))
    except ValueError:
        return str(path)


def read_text(path: Path) -> str:
    if not path.exists():
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def read_tsv(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        return []
    with path.open(encoding="utf-8", newline="") as fh:
        return list(csv.DictReader(fh, delimiter="\t"))


def read_json(path: Path) -> Any | None:
    if not path.exists():
        return None
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        return {"parse_error": str(exc)}


def sha256_short(path: Path) -> str | None:
    if not path.is_file():
        return None
    import hashlib

    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()[:16]


def file_entry(path: Path) -> dict[str, Any]:
    if not path.exists():
        return {"path": rel(path), "exists": False}
    stat = path.stat()
    return {
        "path": rel(path),
        "exists": True,
        "size_bytes": stat.st_size,
        "mtime_utc": datetime.fromtimestamp(stat.st_mtime, timezone.utc).isoformat(),
        "sha256_short": sha256_short(path),
    }


def recommendation_rows() -> list[dict[str, str]]:
    return read_tsv(RECOMMENDATIONS)


def release_dashboard() -> dict[str, Any]:
    manifest_path = PROJECT_ROOT / KEY_ARTIFACTS["release_manifest"]
    manifest = read_json(manifest_path) or {}
    package = manifest.get("package", {}) if isinstance(manifest, dict) else {}
    maple = manifest.get("maple_narration", {}) if isinstance(manifest, dict) else {}
    static_verifiers = manifest.get("static_verifiers", []) if isinstance(manifest, dict) else []
    logs = manifest.get("logs", []) if isinstance(manifest, dict) else []
    return {
        "release_manifest": file_entry(manifest_path),
        "package_exists": bool(package.get("exists")),
        "package_size_gb": package.get("size_gb"),
        "maple_generated": maple.get("generated"),
        "maple_total": maple.get("female_total"),
        "static_verifier_count": len(static_verifiers),
        "log_artifact_count": len(logs),
        "has_round2_feature_flag": "nohuman_next20_round2_improvement"
        in (manifest.get("feature_flags", {}) if isinstance(manifest, dict) else {}),
        "has_round2_evidence_entry": "nohuman_next20_round2_evidence" in manifest
        if isinstance(manifest, dict)
        else False,
    }


def artifact_freshness() -> dict[str, Any]:
    now = datetime.now(timezone.utc)
    entries: dict[str, Any] = {}
    missing: list[str] = []
    stale_over_72h: list[str] = []
    for name, relative in KEY_ARTIFACTS.items():
        path = PROJECT_ROOT / relative
        entry = file_entry(path)
        if path.exists():
            age_hours = (now - datetime.fromtimestamp(path.stat().st_mtime, timezone.utc)).total_seconds() / 3600
            entry["age_hours"] = round(age_hours, 2)
            if age_hours > 72:
                stale_over_72h.append(name)
        else:
            missing.append(name)
        entries[name] = entry
    return {
        "artifacts": entries,
        "missing": missing,
        "stale_over_72h": stale_over_72h,
        "missing_count": len(missing),
        "stale_over_72h_count": len(stale_over_72h),
    }


def parse_mapping_keys(value: str) -> list[str]:
    value = value.strip()
    if not value or value.lower() == "none":
        return []
    if re.fullmatch(r"\d+-\d+", value):
        return [value]
    return [part.strip() for part in value.split("/") if part.strip() and part.strip().lower() != "none"]


def input_mapping_audit() -> dict[str, Any]:
    input_text = read_text(PROJECT_ROOT / "Config/DefaultInput.ini")
    action_rows = re.findall(r'\+ActionMappings=\(ActionName="([^"]+)".*?Key=([A-Za-z0-9_]+)\)', input_text)
    axis_rows = re.findall(r'\+AxisMappings=\(AxisName="([^"]+)".*?Scale=([-0-9.]+),Key=([A-Za-z0-9_]+)\)', input_text)
    control_rows = read_tsv(DATA_DIR / "control_remap_manifest.tsv")

    key_to_controls: dict[str, set[str]] = defaultdict(set)
    for row in control_rows:
        action = row.get("action", "")
        for column in ("default_keyboard", "alternate_keyboard", "gamepad"):
            for key in parse_mapping_keys(row.get(column, "")):
                key_to_controls[key].add(action)

    cross_control_conflicts = {
        key: sorted(actions)
        for key, actions in sorted(key_to_controls.items())
        if len(actions) > 1
    }
    direct_cpp = [row for row in control_rows if "Direct C++" in row.get("remap_status", "")]
    mixed = [row for row in control_rows if "Config-backed action plus direct C++" in row.get("remap_status", "")]
    config_action_names = sorted({name for name, _key in action_rows})
    config_axis_names = sorted({name for name, _scale, _key in axis_rows})
    manifest_actions = sorted(row.get("action", "") for row in control_rows if row.get("action"))
    manifest_mentions_config = sorted(
        row.get("action", "")
        for row in control_rows
        if "Config-backed" in row.get("remap_status", "")
    )
    missing_config_mentions = [
        action
        for action in manifest_mentions_config
        if action not in config_action_names and action not in config_axis_names
    ]

    return {
        "default_input_exists": (PROJECT_ROOT / "Config/DefaultInput.ini").exists(),
        "config_action_binding_count": len(action_rows),
        "config_axis_binding_count": len(axis_rows),
        "config_actions": config_action_names,
        "config_axes": config_axis_names,
        "control_manifest_controls": len(control_rows),
        "manifest_actions": manifest_actions,
        "direct_cpp_control_count": len(direct_cpp),
        "mixed_config_direct_control_count": len(mixed),
        "cross_control_conflicts": cross_control_conflicts,
        "cross_control_conflict_count": len(cross_control_conflicts),
        "config_backed_manifest_rows_missing_ini_binding": missing_config_mentions,
    }


def curriculum_progression_audit() -> dict[str, Any]:
    database = read_json(DATA_DIR / "curriculum_database.json") or {}
    entries = database.get("entries", []) if isinstance(database, dict) else []
    required_fields = ("id", "title", "language", "concept", "difficulty", "strategies", "common_mistakes")
    missing_required: list[dict[str, Any]] = []
    language_counts: Counter[str] = Counter()
    concept_counts: Counter[str] = Counter()
    difficulty_counts: Counter[str] = Counter()

    for entry in entries:
        if not isinstance(entry, dict):
            continue
        language_counts[str(entry.get("language", "unknown"))] += 1
        concept_counts[str(entry.get("concept", "unknown"))] += 1
        difficulty_counts[str(entry.get("difficulty", "unknown"))] += 1
        missing = [field for field in required_fields if not entry.get(field)]
        if missing:
            missing_required.append({"id": entry.get("id", "<missing-id>"), "missing": missing})

    return {
        "database_exists": (DATA_DIR / "curriculum_database.json").exists(),
        "entry_count": len(entries),
        "languages": dict(sorted(language_counts.items())),
        "concept_count": len(concept_counts),
        "top_concepts": concept_counts.most_common(12),
        "difficulty_counts": dict(sorted(difficulty_counts.items())),
        "entries_missing_required_fields": missing_required[:20],
        "missing_required_count": len(missing_required),
    }


def localization_audit() -> dict[str, Any]:
    rows = read_tsv(DATA_DIR / "localization_source.tsv")
    key_counts = Counter(row.get("key", "") for row in rows)
    namespace_counts = Counter(row.get("namespace", "unknown") for row in rows)
    blank_source = [row.get("key", "") for row in rows if not row.get("source_text", "").strip()]
    duplicate_keys = sorted(key for key, count in key_counts.items() if key and count > 1)
    return {
        "row_count": len(rows),
        "namespace_counts": dict(sorted(namespace_counts.items())),
        "blank_source_count": len(blank_source),
        "blank_source_keys": blank_source[:20],
        "duplicate_key_count": len(duplicate_keys),
        "duplicate_keys": duplicate_keys[:20],
    }


def png_size(path: Path) -> tuple[int | None, int | None]:
    import struct

    try:
        with path.open("rb") as fh:
            header = fh.read(24)
        if header.startswith(b"\x89PNG\r\n\x1a\n") and header[12:16] == b"IHDR":
            return struct.unpack(">II", header[16:24])
    except OSError:
        pass
    return None, None


def screenshot_readability_audit(limit: int = 24) -> dict[str, Any]:
    try:
        from PIL import Image
    except Exception as exc:
        return {
            "pillow_available": False,
            "pillow_error": str(exc),
            "screenshot_count": 0,
            "screenshots": [],
        }

    roots = (
        PROJECT_ROOT / "Saved/Screenshots/VisualReview",
        PROJECT_ROOT / "Saved/Screenshots/MacEditor",
        PROJECT_ROOT / "Saved/Screenshots/LaunchMenu",
    )
    screenshots: list[Path] = []
    for root in roots:
        if root.exists():
            screenshots.extend(p for p in root.glob("*.png") if p.is_file())
    screenshots.sort(key=lambda p: p.stat().st_mtime, reverse=True)

    entries: list[dict[str, Any]] = []
    low_luma: list[str] = []
    for path in screenshots[:limit]:
        width, height = png_size(path)
        with Image.open(path) as img:
            rgba = img.convert("RGBA")
            with warnings.catch_warnings():
                warnings.simplefilter("ignore", DeprecationWarning)
                pixels = list(rgba.getdata())
        lumas: list[float] = []
        visible = 0
        for r, g, b, a in pixels:
            alpha = a / 255.0
            luma = (0.2126 * r + 0.7152 * g + 0.0722 * b) * alpha
            lumas.append(luma)
            if a > 0 and luma >= 12:
                visible += 1
        mean_luma = sum(lumas) / len(lumas) if lumas else 0.0
        visible_ratio = visible / len(pixels) if pixels else 0.0
        if mean_luma < 6.0 or visible_ratio < 0.01:
            low_luma.append(rel(path))
        entries.append(
            {
                "path": rel(path),
                "width": width,
                "height": height,
                "mean_luma": round(mean_luma, 2),
                "visible_ratio": round(visible_ratio, 4),
            }
        )

    return {
        "pillow_available": True,
        "screenshot_count": len(screenshots),
        "sampled_count": len(entries),
        "low_luma_count": len(low_luma),
        "low_luma_screenshots": low_luma[:20],
        "screenshots": entries,
    }


def data_manifest_inventory() -> dict[str, Any]:
    files = sorted([p for p in DATA_DIR.iterdir() if p.suffix in {".tsv", ".json"}])
    entries: list[dict[str, Any]] = []
    empty_files: list[str] = []
    for path in files:
        entry = file_entry(path)
        if path.suffix == ".tsv":
            rows = read_tsv(path)
            headers: list[str] = []
            if path.exists():
                with path.open(encoding="utf-8", newline="") as fh:
                    reader = csv.reader(fh, delimiter="\t")
                    headers = next(reader, [])
            entry["row_count"] = len(rows)
            entry["columns"] = headers
            if not rows:
                empty_files.append(rel(path))
        elif path.suffix == ".json":
            payload = read_json(path)
            entry["json_type"] = type(payload).__name__
            if isinstance(payload, dict):
                entry["top_level_keys"] = sorted(payload.keys())[:20]
                if isinstance(payload.get("entries"), list):
                    entry["entry_count"] = len(payload["entries"])
            elif isinstance(payload, list):
                entry["entry_count"] = len(payload)
            if payload in (None, [], {}):
                empty_files.append(rel(path))
        entries.append(entry)
    return {
        "total_files": len(files),
        "tsv_count": sum(1 for p in files if p.suffix == ".tsv"),
        "json_count": sum(1 for p in files if p.suffix == ".json"),
        "empty_file_count": len(empty_files),
        "empty_files": empty_files,
        "files": entries,
    }


def source_control_slices() -> dict[str, Any]:
    try:
        result = subprocess.run(
            ["git", "status", "--short", "--untracked-files=all"],
            cwd=PROJECT_ROOT,
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
    except OSError as exc:
        return {"available": False, "error": str(exc)}

    def classify(path: str) -> str:
        if path.startswith("Source/"):
            return "source"
        if path.startswith("Scripts/") or path.endswith(".command"):
            return "automation"
        if path.startswith("Documentation/") or path in {"README_MAC.md", "progress.md"}:
            return "documentation"
        if path.startswith("Content/CodeRescueData/"):
            return "data_manifest"
        if path.startswith("Content/"):
            return "content_asset"
        if path.startswith("Config/"):
            return "config"
        if path.startswith("Saved/"):
            return "generated_saved"
        if path.startswith("Build/") or path.startswith("PackagedMac/"):
            return "build_package"
        return "other"

    counts: Counter[str] = Counter()
    samples: dict[str, list[str]] = defaultdict(list)
    lines = [line for line in result.stdout.splitlines() if line.strip()]
    for line in lines:
        path = line[3:] if len(line) > 3 else line
        bucket = classify(path)
        counts[bucket] += 1
        if len(samples[bucket]) < 12:
            samples[bucket].append(line)
    return {
        "available": result.returncode == 0,
        "line_count": len(lines),
        "slice_counts": dict(sorted(counts.items())),
        "samples": dict(sorted(samples.items())),
        "stderr": result.stderr.strip(),
    }


def qa_log_summary() -> dict[str, Any]:
    logs = {
        name: PROJECT_ROOT / relative
        for name, relative in KEY_ARTIFACTS.items()
        if relative.startswith("Saved/Logs/")
    }
    summaries: dict[str, Any] = {}
    for name, path in logs.items():
        entry = file_entry(path)
        warnings = 0
        allowed = 0
        errors = 0
        fatals = 0
        if path.exists():
            for line in path.read_text(encoding="utf-8", errors="ignore").splitlines():
                if "Warning" in line:
                    warnings += 1
                    if any(fragment in line for fragment in ALLOWED_WARNING_FRAGMENTS):
                        allowed += 1
                if "Error:" in line or "LogLinker: Warning" in line or "Failed to find object" in line:
                    errors += 1
                if "Fatal" in line or "Exception" in line:
                    fatals += 1
        entry.update(
            {
                "warning_count": warnings,
                "allowed_warning_count": allowed,
                "unexpected_warning_count": max(0, warnings - allowed),
                "error_like_count": errors,
                "fatal_like_count": fatals,
            }
        )
        summaries[name] = entry
    return summaries


def support_bundle_audit() -> dict[str, Any]:
    script = read_text(PROJECT_ROOT / "Scripts/create_support_bundle.py")
    expected_tokens = (
        "generate_nohuman_next20_round2_evidence.py",
        "nohuman_next20_round2_evidence_latest.json",
        "NOHUMAN_NEXT20_ROUND2_IMPROVEMENT_PASS_2026-06-24.md",
        "nohuman_next20_round2_recommendations.tsv",
    )
    latest_zip = None
    bundle_dir = PROJECT_ROOT / "Saved/SupportBundles"
    if bundle_dir.exists():
        zips = sorted(bundle_dir.glob("code_rescue_support_bundle_*.zip"), key=lambda p: p.stat().st_mtime)
        latest_zip = zips[-1] if zips else None

    zip_summary: dict[str, Any] = {"exists": False}
    if latest_zip is not None:
        with zipfile.ZipFile(latest_zip) as zf:
            names = set(zf.namelist())
        zip_summary = {
            "exists": True,
            "path": rel(latest_zip),
            "entry_count": len(names),
            "contains_round2_evidence": "Saved/Release/nohuman_next20_round2_evidence_latest.json" in names,
            "contains_round2_doc": "Documentation/NOHUMAN_NEXT20_ROUND2_IMPROVEMENT_PASS_2026-06-24.md" in names,
            "contains_round2_manifest": "Content/CodeRescueData/nohuman_next20_round2_recommendations.tsv" in names,
        }

    return {
        "script_mentions_expected_round2_artifacts": all(token in script for token in expected_tokens),
        "script_expected_tokens": {token: token in script for token in expected_tokens},
        "latest_bundle": zip_summary,
    }


def difficulty_onboarding_matrix() -> dict[str, Any]:
    difficulty_rows = read_tsv(DATA_DIR / "difficulty_presets.tsv")
    onboarding_rows = read_tsv(DATA_DIR / "first_ten_minutes_onboarding.tsv")
    return {
        "difficulty_preset_count": len(difficulty_rows),
        "difficulty_presets": [row.get("preset", "") for row in difficulty_rows],
        "onboarding_minute_count": len(onboarding_rows),
        "onboarding_minutes": [row.get("minute", "") for row in onboarding_rows],
        "matrix_review_cells": len(difficulty_rows) * len(onboarding_rows),
    }


def accessibility_audit() -> dict[str, Any]:
    rows = read_tsv(DATA_DIR / "accessibility_settings_manifest.tsv")
    source_text = "\n".join(
        read_text(PROJECT_ROOT / relative)
        for relative in (
            "Source/CodeRescueUnreal/CodeRescueGameInstance.h",
            "Source/CodeRescueUnreal/CodeRescueSettingsWidget.cpp",
            "Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp",
            "Source/CodeRescueUnreal/CodeRescueSubtitlesWidget.cpp",
        )
    )
    implemented_mentions = {
        row.get("setting", ""): row.get("setting", "") in source_text
        for row in rows
        if row.get("setting")
    }
    return {
        "manifest_row_count": len(rows),
        "implemented_mentions": implemented_mentions,
        "implemented_mention_count": sum(1 for value in implemented_mentions.values() if value),
        "needs_human_observation": True,
    }


def save_schema_inventory() -> dict[str, Any]:
    header = read_text(PROJECT_ROOT / "Source/CodeRescueUnreal/CodeRescueSaveGame.h")
    fields: list[str] = []
    lines = header.splitlines()
    for index, line in enumerate(lines):
        if "UPROPERTY" not in line:
            continue
        for candidate in lines[index + 1 : index + 5]:
            stripped = candidate.strip()
            if stripped and stripped.endswith(";"):
                fields.append(stripped.rstrip(";"))
                break
    return {
        "header_exists": bool(header),
        "field_count": len(fields),
        "fields": fields,
        "contains_control_profile_fields": all(
            token in header for token in ("ControlProfileName", "ControlProfileExportCount")
        ),
    }


def radio_content_audit() -> dict[str, Any]:
    rows = read_tsv(DATA_DIR / "radio_briefings.tsv")
    female_voices = {"Samantha", "Victoria", "Kyoko", "Tessa", "Karen"}
    female_rows = [row for row in rows if row.get("voice", "").strip() in female_voices]
    wav_missing = [
        row.get("slug", "")
        for row in female_rows
        if not (PROJECT_ROOT / "Content/CodeRescueAssets/Audio/RadioSamples" / f"{row.get('slug', '')}_radio_briefing.wav").exists()
    ]
    audit = read_json(PROJECT_ROOT / "Saved/AudioAudit/maple_audio_audit_latest.json")
    return {
        "radio_row_count": len(rows),
        "female_voice_row_count": len(female_rows),
        "female_voice_wav_missing_count": len(wav_missing),
        "female_voice_wav_missing": wav_missing[:20],
        "maple_audio_audit_exists": audit is not None,
    }


def asset_budget_audit() -> dict[str, Any]:
    rows = read_tsv(DATA_DIR / "asset_budget_limits.tsv")
    limits = {
        row.get("asset_class", ""): {
            "max_single_mb": row.get("max_single_mb", ""),
            "max_total_mb": row.get("max_total_mb", ""),
        }
        for row in rows
    }
    return {
        "budget_row_count": len(rows),
        "asset_classes": sorted(limits.keys()),
        "limits": limits,
        "verifier": file_entry(PROJECT_ROOT / "Scripts/verify_asset_budget_pass.py"),
    }


def build_evidence() -> dict[str, Any]:
    rows = recommendation_rows()
    status_counts = Counter(row.get("status", "unknown") for row in rows)
    boundary_counts = Counter(row.get("boundary", "unknown") for row in rows)
    return {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "recommendation_manifest": rel(RECOMMENDATIONS),
        "recommendation_count": len(rows),
        "status_counts": dict(sorted(status_counts.items())),
        "boundary_counts": dict(sorted(boundary_counts.items())),
        "recommendations": rows,
        "release_dashboard": release_dashboard(),
        "artifact_freshness": artifact_freshness(),
        "input_mapping_audit": input_mapping_audit(),
        "curriculum_progression_audit": curriculum_progression_audit(),
        "localization_audit": localization_audit(),
        "screenshot_readability_audit": screenshot_readability_audit(),
        "data_manifest_inventory": data_manifest_inventory(),
        "source_control_slices": source_control_slices(),
        "qa_log_summary": qa_log_summary(),
        "support_bundle_audit": support_bundle_audit(),
        "difficulty_onboarding_matrix": difficulty_onboarding_matrix(),
        "accessibility_audit": accessibility_audit(),
        "save_schema_inventory": save_schema_inventory(),
        "radio_content_audit": radio_content_audit(),
        "asset_budget_audit": asset_budget_audit(),
        "human_testing_boundary": "Human playthrough remains outside this pass.",
        "credential_boundary": "Apple signing, notarization, and hosted release credentials remain external.",
        "subjective_balance_boundary": "This pass does not claim subjective balance approval.",
    }


def main() -> int:
    if not RECOMMENDATIONS.exists():
        print(f"[generate_nohuman_next20_round2_evidence] FAIL: missing {rel(RECOMMENDATIONS)}")
        return 1

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    evidence = build_evidence()
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    latest = OUT_DIR / "nohuman_next20_round2_evidence_latest.json"
    stamped = OUT_DIR / f"nohuman_next20_round2_evidence_{stamp}.json"
    text = json.dumps(evidence, indent=2, sort_keys=True)
    latest.write_text(text + "\n", encoding="utf-8")
    stamped.write_text(text + "\n", encoding="utf-8")

    print(f"[generate_nohuman_next20_round2_evidence] wrote {latest}")
    print(
        "[generate_nohuman_next20_round2_evidence] "
        f"recommendations={evidence['recommendation_count']} "
        f"status_counts={evidence['status_counts']} "
        f"data_files={evidence['data_manifest_inventory']['total_files']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
