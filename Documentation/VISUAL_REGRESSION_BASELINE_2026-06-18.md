# Visual Regression Baseline - 2026-06-18

## Workflow

Use:

```bash
./Run_Visual_Regression_Audit.command
```

That command captures a visual review screenshot through
`Run_Visual_Review_Capture.command` and then writes:

```text
Saved/VisualRegression/visual_regression_manifest_latest.json
```

The manifest records screenshot paths, file sizes, PNG dimensions, timestamps,
and SHA-256 hashes.

## Target Surfaces

The required target list lives in:

```text
Content/CodeRescueData/visual_regression_targets.tsv
```

It currently covers New York entry, city arena boundary, terminal safehouse,
armory, survivor area, helipad extraction, squad HUD, and settings
accessibility.

## Review Guidance

- Screenshot hashing catches accidental capture changes and missing artifacts.
- Human review is still required for framing, readability, overlap, color, and
  whether a target actually shows the intended surface.
- Future work should add target-specific camera commands and pixel-tolerance
  image comparison.
