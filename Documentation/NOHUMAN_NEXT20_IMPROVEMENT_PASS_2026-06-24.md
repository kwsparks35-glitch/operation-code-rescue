# No-Human Next 20 Improvement Pass - 2026-06-24

## Scope

This pass implements or automates the next twenty continued-improvement
recommendations as far as possible without current physical human testing,
Apple Developer credentials, hosted CI infrastructure, or new authored art/audio
approval.

Human playthrough remains the primary excluded gate. Developer ID signing and
notarization remain credential-bound. GameMode modularization remains an
architecture continuation because the current 12k-line source file needs careful
ownership extraction rather than a broad risky rewrite inside an already-dirty
worktree.

## Implemented In This Pass

1. Added `Content/CodeRescueData/nohuman_next20_recommendations.tsv` as the
   machine-readable status ledger for the twenty recommendations. Each row
   records implementation status, boundary, evidence path, and next review step.
2. Added `Scripts/generate_nohuman_next20_evidence.py`. It writes
   `Saved/Release/nohuman_next20_evidence_latest.json`, summarizing the twenty
   recommendation rows, optional generated artifacts, dirty-tree status, and
   human/credential boundaries.
3. Added `Scripts/verify_next20_nohuman_improvement_pass.py` and wired its
   expected contract around the manifest, docs, control-profile export path,
   static performance profile, and generated evidence.
4. Extended the save schema with `ControlProfileName` and
   `ControlProfileExportCount`, then serialized them through
   `UCodeRescueGameInstance::SavePersistentRun()` and
   `LoadPersistentRun()`.
5. Added `UCodeRescueGameInstance::GetControlProfileSummary()` and
   `ExportControlProfileReviewFile()`. The runtime export writes
   `Saved/Config/ControlProfiles/runtime_controls_profile.json` from inside the
   game.
6. Added an in-game Settings-panel control-profile status line and
   "Export Control Profile" button. This does not mutate live combat bindings;
   it produces a safe review artifact before any future arbitrary key-capture
   refactor.
7. Upgraded `Scripts/apply_control_remap_profile.py` so the generated
   `default_controls_profile.json` includes counts and safety notes for
   config-backed, direct-C++, and mixed bindings.
8. Upgraded `Scripts/profile_city_layers_static.py` so the static profile
   includes the `performance_city_layer_budget.tsv` contract for each layer.

## Recommendation Status

- Human playthrough: explicitly not claimed by this pass.
- Full package/QA refresh: automation is present and documented; run
  `Run_Full_QA_Audit.command`, `Package_Mac_App.command`, package smoke tests,
  `Run_Visual_Regression_Audit.command`, and
  `Run_NonHuman_Release_Readiness.command` after a package refresh.
- Signing/notarization: still requires Apple Developer Team ID, certificate,
  notary credentials, staple, and strict distribution verification.
- Source-control cleanup: still requires intentional staging because the repo
  has many pre-existing modified/untracked files.
- GameMode modularization: partially complete through the existing spawning
  split and new static ownership evidence; further extraction should be done in
  focused files such as audio, city identity, curriculum safehouse, combat, and
  arena confinement.
- Control profile/remap: now visible and exportable in-game and externally;
  arbitrary rebinding remains a future input-architecture step.
- Performance telemetry: static source/budget profile is implemented; runtime
  frame timing should be added when a profiling session is available.
- Visual regression: existing capture/manifest automation remains the correct
  no-human gate.
- Difficulty/accessibility/onboarding: manifests and automated contracts are
  present; subjective feel and visible-effect confirmation still need human
  observation.
- Art, animation, AI, audio, curriculum, and terminal UX: the pass records
  current evidence and next steps, but asset-authoring and subjective review
  remain separate production passes.

## Verification

Run this focused pass:

```bash
python3 Scripts/apply_control_remap_profile.py
python3 Scripts/profile_city_layers_static.py
python3 Scripts/generate_nohuman_next20_evidence.py
python3 Scripts/verify_next20_nohuman_improvement_pass.py
python3 Scripts/verify_save_compatibility_pass.py
```

Run the wider automated suite after C++ changes:

```bash
./Recompile_Module.command
git diff --check
```

Run the full package/release chain when time allows:

```bash
./Run_Full_QA_Audit.command
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
./Run_Visual_Regression_Audit.command
./Run_NonHuman_Release_Readiness.command
```
