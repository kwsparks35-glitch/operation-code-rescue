#!/usr/bin/env python3
"""Verify packaged Mac app integrity and signing/notarization preflight state."""

from __future__ import annotations

import argparse
import json
import plistlib
import subprocess
from datetime import datetime, timezone
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
APP_PATH = PROJECT_ROOT / "PackagedMac/Mac/CodeRescueUnreal.app"
OUT_DIR = PROJECT_ROOT / "Saved/Release"
REQUIRED_FILES = (
    "Contents/MacOS/CodeRescueUnreal",
    "Contents/Info.plist",
    "Contents/UE/UECommandLine.txt",
    "Contents/UE/CodeRescueUnreal/Content/Paks/CodeRescueUnreal-Mac.pak",
    "Contents/UE/CodeRescueUnreal/Content/Paks/CodeRescueUnreal-Mac.ucas",
    "Contents/UE/CodeRescueUnreal/Content/Paks/CodeRescueUnreal-Mac.utoc",
    "Contents/UE/CodeRescueUnreal/Content/Paks/global.ucas",
    "Contents/UE/CodeRescueUnreal/Content/Paks/global.utoc",
)


def run_command(args: list[str]) -> dict:
    try:
        result = subprocess.run(
            args,
            cwd=PROJECT_ROOT,
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
    except OSError as exc:
        return {
            "available": False,
            "returncode": None,
            "stdout": "",
            "stderr": str(exc),
        }
    return {
        "available": True,
        "returncode": result.returncode,
        "stdout": result.stdout.strip(),
        "stderr": result.stderr.strip(),
    }


def directory_size(path: Path) -> int:
    if not path.exists():
        return 0
    return sum(p.stat().st_size for p in path.rglob("*") if p.is_file())


def file_entry(rel: str) -> dict:
    path = APP_PATH / rel
    if not path.exists():
        return {"relative_path": rel, "exists": False}
    return {
        "relative_path": rel,
        "exists": True,
        "size_bytes": path.stat().st_size,
        "mtime_utc": datetime.fromtimestamp(path.stat().st_mtime, timezone.utc).isoformat(),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--strict-distribution",
        action="store_true",
        help="Fail on distribution-only blockers such as placeholder bundle ID or Gatekeeper rejection.",
    )
    parser.add_argument(
        "--expected-bundle-id",
        default="",
        help="Optional final bundle identifier to enforce.",
    )
    args = parser.parse_args()

    hard_errors: list[str] = []
    credential_blockers: list[str] = []
    warnings: list[str] = []

    if not APP_PATH.exists():
        hard_errors.append(f"missing packaged app: {APP_PATH}")

    required = [file_entry(rel) for rel in REQUIRED_FILES]
    for entry in required:
        if not entry["exists"]:
            hard_errors.append(f"missing required app file: {entry['relative_path']}")

    plist_path = APP_PATH / "Contents/Info.plist"
    plist: dict = {}
    if plist_path.exists():
        try:
            with plist_path.open("rb") as fh:
                plist = plistlib.load(fh)
        except (OSError, plistlib.InvalidFileException) as exc:
            hard_errors.append(f"cannot read Info.plist: {exc}")
    else:
        hard_errors.append("missing Contents/Info.plist")

    bundle_id = str(plist.get("CFBundleIdentifier", ""))
    executable = str(plist.get("CFBundleExecutable", ""))
    bundle_name = str(plist.get("CFBundleName", ""))
    minimum_os = str(plist.get("LSMinimumSystemVersion", ""))

    if executable != "CodeRescueUnreal":
        hard_errors.append(f"unexpected CFBundleExecutable: {executable!r}")
    if bundle_name != "CodeRescueUnreal":
        warnings.append(f"unexpected CFBundleName: {bundle_name!r}")
    if not minimum_os:
        hard_errors.append("missing LSMinimumSystemVersion")
    if not bundle_id:
        hard_errors.append("missing CFBundleIdentifier")
    if "YourCompany" in bundle_id:
        credential_blockers.append(
            "CFBundleIdentifier still uses placeholder com.YourCompany.CodeRescueUnreal"
        )
    if args.expected_bundle_id and bundle_id != args.expected_bundle_id:
        hard_errors.append(
            f"CFBundleIdentifier {bundle_id!r} does not match expected {args.expected_bundle_id!r}"
        )

    size_bytes = directory_size(APP_PATH)
    size_mb = size_bytes / (1024 * 1024)
    if APP_PATH.exists() and size_mb < 500:
        hard_errors.append(f"packaged app is unexpectedly small: {size_mb:.1f} MB")
    if size_mb > 3072:
        hard_errors.append(f"packaged app exceeds demo budget: {size_mb:.1f} MB")

    codesign_verify = run_command(["codesign", "--verify", "--deep", "--strict", "--verbose=2", str(APP_PATH)])
    codesign_display = run_command(["codesign", "-dv", "--verbose=4", str(APP_PATH)])
    spctl_assess = run_command(["spctl", "--assess", "--verbose=4", "--type", "execute", str(APP_PATH)])

    if codesign_verify["available"] and codesign_verify["returncode"] != 0:
        hard_errors.append("codesign verification failed for packaged app")
    elif not codesign_verify["available"]:
        warnings.append("codesign tool is unavailable on this machine")

    if spctl_assess["available"] and spctl_assess["returncode"] != 0:
        credential_blockers.append(
            "Gatekeeper assessment does not pass yet; signed/notarized distribution still pending"
        )
    elif not spctl_assess["available"]:
        warnings.append("spctl tool is unavailable on this machine")

    if args.strict_distribution:
        hard_errors.extend(credential_blockers)

    report = {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "project_root": str(PROJECT_ROOT),
        "app_path": str(APP_PATH),
        "app_exists": APP_PATH.exists(),
        "app_size_bytes": size_bytes,
        "app_size_mb": round(size_mb, 1),
        "info_plist": {
            "CFBundleIdentifier": bundle_id,
            "CFBundleExecutable": executable,
            "CFBundleName": bundle_name,
            "CFBundleShortVersionString": plist.get("CFBundleShortVersionString", ""),
            "CFBundleVersion": plist.get("CFBundleVersion", ""),
            "LSMinimumSystemVersion": minimum_os,
            "LSApplicationCategoryType": plist.get("LSApplicationCategoryType", ""),
        },
        "required_files": required,
        "codesign_verify": codesign_verify,
        "codesign_display": codesign_display,
        "spctl_assess": spctl_assess,
        "strict_distribution": args.strict_distribution,
        "hard_error_count": len(hard_errors),
        "credential_blocker_count": len(credential_blockers),
        "warning_count": len(warnings),
        "hard_errors": hard_errors,
        "credential_blockers": credential_blockers,
        "warnings": warnings,
        "local_package_ready": len(hard_errors) == 0,
        "external_distribution_ready": len(hard_errors) == 0 and len(credential_blockers) == 0,
    }

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    text = json.dumps(report, indent=2, sort_keys=True)
    (OUT_DIR / f"package_integrity_{stamp}.json").write_text(text + "\n", encoding="utf-8")
    latest = OUT_DIR / "package_integrity_latest.json"
    latest.write_text(text + "\n", encoding="utf-8")

    print(
        "[verify_package_integrity_pass] "
        f"local_ready={report['local_package_ready']} "
        f"external_ready={report['external_distribution_ready']} "
        f"bundle_id={bundle_id or 'missing'} size={report['app_size_mb']} MB"
    )
    for blocker in credential_blockers:
        print(f"[verify_package_integrity_pass] CREDENTIAL: {blocker}")
    for warning in warnings:
        print(f"[verify_package_integrity_pass] WARN: {warning}")
    if hard_errors:
        for error in hard_errors:
            print(f"[verify_package_integrity_pass] FAIL: {error}")
        print(f"[verify_package_integrity_pass] wrote {latest}")
        return 1
    print(f"[verify_package_integrity_pass] PASS: local package integrity wrote {latest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
