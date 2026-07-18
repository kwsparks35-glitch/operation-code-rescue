#!/usr/bin/env python3
"""Fail if a PNG is effectively blank/black.

Used by launch-menu visual checks so a packaged app can boot successfully while
still failing QA if the first human-visible frame is not readable.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys
import warnings

from PIL import Image


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Verify that a PNG has visible non-black content.")
    parser.add_argument("image", type=Path, help="PNG file to inspect.")
    parser.add_argument("--min-mean-luma", type=float, default=6.0)
    parser.add_argument("--min-visible-ratio", type=float, default=0.01)
    parser.add_argument("--visible-threshold", type=int, default=12)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    image_path = args.image
    if not image_path.exists():
        print(f"[verify_png_not_black] FAIL: missing image {image_path}", file=sys.stderr)
        return 1

    with Image.open(image_path) as img:
        rgba = img.convert("RGBA")
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", DeprecationWarning)
            pixels = list(rgba.getdata())

    if not pixels:
        print(f"[verify_png_not_black] FAIL: image has no pixels: {image_path}", file=sys.stderr)
        return 1

    luma_values: list[float] = []
    visible_count = 0
    for r, g, b, a in pixels:
        alpha = a / 255.0
        luma = (0.2126 * r + 0.7152 * g + 0.0722 * b) * alpha
        luma_values.append(luma)
        if a > 0 and luma >= args.visible_threshold:
            visible_count += 1

    mean_luma = sum(luma_values) / len(luma_values)
    max_luma = max(luma_values)
    visible_ratio = visible_count / len(pixels)

    if mean_luma < args.min_mean_luma or visible_ratio < args.min_visible_ratio:
        print(
            "[verify_png_not_black] FAIL: image appears blank/black "
            f"(mean_luma={mean_luma:.2f}, max_luma={max_luma:.2f}, "
            f"visible_ratio={visible_ratio:.4f}): {image_path}",
            file=sys.stderr,
        )
        return 1

    print(
        "[verify_png_not_black] PASS: "
        f"mean_luma={mean_luma:.2f}, max_luma={max_luma:.2f}, "
        f"visible_ratio={visible_ratio:.4f}: {image_path}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
