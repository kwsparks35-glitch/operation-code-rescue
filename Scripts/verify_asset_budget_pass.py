#!/usr/bin/env python3
"""Enforce coarse asset/package budgets for demo-readiness review."""

from __future__ import annotations

import csv
from collections import defaultdict
from pathlib import Path
import sys


PROJECT_ROOT = Path(__file__).resolve().parents[1]
BUDGETS = PROJECT_ROOT / "Content/CodeRescueData/asset_budget_limits.tsv"
CONTENT = PROJECT_ROOT / "Content"
PACKAGE = PROJECT_ROOT / "PackagedMac/Mac/CodeRescueUnreal.app"
SCREENSHOTS = PROJECT_ROOT / "Saved/Screenshots"


def read_budgets() -> dict[str, dict[str, float]]:
    if not BUDGETS.exists():
        raise FileNotFoundError(BUDGETS)
    with BUDGETS.open(encoding="utf-8", newline="") as fh:
        rows = csv.DictReader(fh, delimiter="\t")
        return {
            row["asset_class"]: {
                "single": float(row["max_single_mb"]),
                "total": float(row["max_total_mb"]),
            }
            for row in rows
        }


def mb(size: int) -> float:
    return size / (1024 * 1024)


def total_size(path: Path) -> int:
    if not path.exists():
        return 0
    if path.is_file():
        return path.stat().st_size
    return sum(p.stat().st_size for p in path.rglob("*") if p.is_file())


def main() -> int:
    errors: list[str] = []
    budgets = read_budgets()
    totals: dict[str, int] = defaultdict(int)
    largest: dict[str, tuple[float, Path] | None] = defaultdict(lambda: None)

    suffix_to_class = {
        ".uasset": "uasset",
        ".umap": "umap",
        ".wav": "wav",
        ".png": "png",
        ".jpg": "jpg",
        ".jpeg": "jpg",
    }
    for path in CONTENT.rglob("*"):
        if not path.is_file():
            continue
        asset_class = suffix_to_class.get(path.suffix.lower())
        if not asset_class:
            continue
        size_mb = mb(path.stat().st_size)
        totals[asset_class] += path.stat().st_size
        current = largest[asset_class]
        if current is None or size_mb > current[0]:
            largest[asset_class] = (size_mb, path)
        max_single = budgets.get(asset_class, {}).get("single")
        if max_single is not None and size_mb > max_single:
            errors.append(f"{path.relative_to(PROJECT_ROOT)} is {size_mb:.1f} MB, over {max_single:.1f} MB")

    for asset_class, size in totals.items():
        max_total = budgets.get(asset_class, {}).get("total")
        if max_total is not None and mb(size) > max_total:
            errors.append(f"{asset_class} total is {mb(size):.1f} MB, over {max_total:.1f} MB")

    package_mb = mb(total_size(PACKAGE))
    if package_mb > budgets["packaged_app"]["total"]:
        errors.append(f"packaged app is {package_mb:.1f} MB, over {budgets['packaged_app']['total']:.1f} MB")
    screenshots_mb = mb(total_size(SCREENSHOTS))
    if screenshots_mb > budgets["screenshots"]["total"]:
        errors.append(f"screenshots total is {screenshots_mb:.1f} MB, over {budgets['screenshots']['total']:.1f} MB")

    for asset_class, entry in sorted(largest.items()):
        if entry:
            print(f"[verify_asset_budget_pass] largest {asset_class}: {entry[0]:.1f} MB {entry[1].relative_to(PROJECT_ROOT)}")
    print(f"[verify_asset_budget_pass] package={package_mb:.1f} MB screenshots={screenshots_mb:.1f} MB")

    if errors:
        for error in errors:
            print(f"[verify_asset_budget_pass] FAIL: {error}")
        return 1
    print("[verify_asset_budget_pass] PASS: asset budgets within configured limits")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
