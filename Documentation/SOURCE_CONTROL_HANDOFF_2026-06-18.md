# Source Control Handoff - 2026-06-18

## Purpose

This handoff captures the source-control cleanup completed during the June 18
public-hardening pass and the remaining steps needed before a clean public
release branch.

## Completed

- Added project `.gitattributes` with Git LFS rules for Unreal binary assets
  (`*.uasset`, `*.umap`, cooked package chunks, audio, image, and DCC files).
- Normalized code/script/data files as LF text so future diffs remain readable.
- Expanded `.gitignore` for Python caches and local virtual environments.
- Removed generated `.DS_Store` files and Python `__pycache__` folders.
- Removed an empty top-level scratch `Source/` directory outside the Unreal
  project root.

## Commit Guidance

The worktree already contains a large set of pre-existing modified and
untracked gameplay, asset, script, and documentation changes. On 2026-06-23,
review found 48 modified tracked files and 633 untracked paths before the
recommendation-closure edits. Do not squash all of it into one opaque release
commit. Recommended slices:

1. Recommendation-closure runtime fixes: cooked-radio priority, MATLAB PATH
   detection, validator sentinels, skill tree widget/application, and bundle ID.
2. No-human next-20 evidence pass: control-profile export/save metadata,
   recommendation ledger, evidence generator, verifier, wrapper command,
   release/support-bundle wiring, and documentation.
3. No-human next-20 round-two evidence pass: release dashboard, input mapping
   audit, curriculum/localization coverage audits, screenshot readability,
   source-control slice classifier, support-bundle wiring, and documentation.
4. Existing gameplay/source expansion: campaign, character, combat, squad,
   save/load, HUD, pause/settings, and terminal systems.
5. QA verifier additions and script wiring.
6. Maple narration assets and documentation/status correction.
7. GameMode spawning split and any future GameMode modularization.
8. Asset/content batches, grouped by source and feature.

Before committing large existing assets, run:

```bash
git lfs install
git lfs track
git status --short
git diff --check
```

For already-tracked large binaries, verify whether migration is required before
pushing to a remote:

```bash
git lfs migrate info --everything
```

Only run an actual LFS migration after confirming branch/remote policy, because
history rewriting affects anyone with an existing clone.

## Review Notes

- No commit was created during this pass because the worktree contains many
  unrelated pre-existing changes, including files that also needed targeted
  recommendation-closure edits.
- Generated package/build folders remain ignored and should not be staged.
- Keep `Saved/Logs` out of normal commits unless a specific log is needed as
  release evidence.
