# No-Human Next 20 Round-Two Improvement Pass - 2026-06-24

## Scope

Second-cycle recommendations are implemented as dashboardable evidence. This
pass focuses on the next twenty improvements that can be completed without a
current human playthrough, Apple Developer credentials, hosted Mac CI, or new
approved art/audio authoring.

Human playthrough remains outside this pass. Apple signing, notarization, and
hosted release credentials remain external. This pass does not claim subjective
balance approval.

## Implemented In This Pass

1. Added `Content/CodeRescueData/nohuman_next20_round2_recommendations.tsv` as
   the second-cycle machine-readable recommendation ledger.
2. Added `Scripts/generate_nohuman_next20_round2_evidence.py`. It writes
   `Saved/Release/nohuman_next20_round2_evidence_latest.json` and a stamped
   copy for review history.
3. Added `Scripts/verify_next20_round2_nohuman_improvement_pass.py` to verify
   the manifest, generated JSON sections, docs, QA wiring, release-manifest
   wiring, support-bundle wiring, and wrapper command.
4. Added `Run_NoHuman_Next20_Round2_Improvement.command` as the focused local
   entrypoint for this second-cycle pass.
5. Wired the pass into `Run_Full_QA_Audit.command`,
   `Run_Local_CI_Readiness.command`, `Scripts/generate_release_manifest.py`,
   and `Scripts/create_support_bundle.py`.

## Dashboard Coverage

The round-two evidence JSON now records:

- release manifest/package state and whether round-two evidence is represented;
- artifact freshness and missing-artifact counts;
- input mapping coverage, direct-C++ control counts, mixed config/direct
  controls, and cross-control key overlaps;
- curriculum progression coverage by language, concept, and difficulty;
- localization source row counts, blank source checks, and duplicate key checks;
- screenshot luminance and visible-pixel metrics for recent visual captures;
- every `.tsv` and `.json` file in `Content/CodeRescueData`, including row
  counts and empty-file flags;
- source-control slice counts for source, automation, docs, data, assets,
  config, generated files, and package/build files;
- QA log warning/error/fatal summaries with known allowed warnings separated;
- support-bundle script coverage and latest bundle round-two contents when a
  bundle has already been generated;
- difficulty/onboarding matrix size;
- accessibility settings manifest mentions in implementation files;
- save-game schema field inventory;
- radio briefing and Maple female-voice WAV coverage;
- asset-budget rows and verifier presence.

## Recommendation Status

- Rows 21-38 are implemented as local dashboards, scripts, or release/support
  wiring.
- Row 39 remains a physical-human boundary: first-ten-minutes playthrough,
  accessibility observation, per-difficulty balance feel, audio review, and UI
  comfort still need people.
- Row 40 remains an external-credential boundary: Developer ID signing,
  notarization, stapling, and hosted Mac CI need the appropriate accounts and
  infrastructure.

## Verification

Run the focused pass:

```bash
./Run_NoHuman_Next20_Round2_Improvement.command
```

Or run the component commands:

```bash
python3 Scripts/apply_control_remap_profile.py
python3 Scripts/profile_city_layers_static.py
python3 Scripts/generate_nohuman_next20_evidence.py
python3 Scripts/generate_nohuman_next20_round2_evidence.py
python3 Scripts/verify_next20_nohuman_improvement_pass.py
python3 Scripts/verify_next20_round2_nohuman_improvement_pass.py
python3 Scripts/verify_save_compatibility_pass.py
python3 Scripts/generate_release_manifest.py
```

Refresh support artifacts after verification:

```bash
python3 Scripts/create_support_bundle.py
```

## Review Notes

The generated evidence can reveal known work without failing the pass. For
example, direct C++ key bindings and intentional key overlaps are listed so the
future arbitrary remap refactor has a reliable target list. Screenshot metrics
flag low-luma captures, but human review still decides whether visual design is
comfortable and legible in motion.
