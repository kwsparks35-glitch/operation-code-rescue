# Next 20 Implementation Pass - 2026-06-18

## Scope

This pass implements the next 20 continued-improvement recommendations as a
demo-readiness and production-hardening layer. The work adds runtime support,
data manifests, release tooling, verification scripts, and review documents so
future changes can be checked without relying on memory.

## Implemented Recommendations

1. Maple narration generation was unblocked by patching the Maple XTTS shim for
   Transformers 4.50+ generation behavior. The 230-city female-voice Maple
   subset was generated, imported into Unreal SoundWave assets, wired through
   `Import_And_Wire_Maple_Narrations.command`, verified at 230/230 coverage,
   and cooked into the packaged app. The full playable campaign currently has
   465 missions. `CodeRescueGameMode.cpp` also has a native
   slug-based cue load fallback so imported cues play even when the native game
   mode is used instead of the edited Blueprint asset.
2. Source-control guidance remains documented in
   `Documentation/SOURCE_CONTROL_HANDOFF_2026-06-18.md`; this pass adds new
   files without making a broad commit over the existing dirty worktree.
3. `Scripts/generate_release_manifest.py` records package evidence, logs,
   verifier files, Maple coverage, screenshots, feature flags, and git status.
4. `CodeRescueGameMode.cpp` remains split through
   `CodeRescueGameModeSpawning.cpp`; the new demo-readiness verifier keeps the
   split visible as a required contract.
5. `Scripts/generate_visual_regression_manifest.py`,
   `Run_Visual_Regression_Audit.command`, and
   `Content/CodeRescueData/visual_regression_targets.tsv` establish a visual
   regression baseline workflow.
6. `Content/CodeRescueData/human_qa_signoff_checklist.tsv` and the updated
   human QA checklist add explicit sign-off coverage for controls, difficulty,
   accessibility, combat, squad, curriculum, save/load, package, and release
   evidence.
7. `Scripts/verify_save_compatibility_pass.py` locks save-version,
   back-compat flags, explicit difficulty enum values, and new accessibility
   save fields.
8. `Content/CodeRescueData/first_ten_minutes_onboarding.tsv` defines the
   intended first-ten-minutes player learning path.
9. `Content/CodeRescueData/control_remap_manifest.tsv` and
   `Scripts/apply_control_remap_profile.py` document/export the control-remap
   contract for config/direct-binding review.
10. The difficulty enum now supports Story, Easy, Normal, Hard, Survival, and
    Nightmare while preserving old Easy/Normal/Hard serialized values. The
    pause menu cycles all six presets.
11. `Content/CodeRescueData/performance_city_layer_budget.tsv` defines target
    budgets for major city/runtime layers.
12. `Content/CodeRescueData/asset_budget_limits.tsv` and
    `Scripts/verify_asset_budget_pass.py` enforce coarse asset/package budgets.
13. `Content/CodeRescueData/enemy_readability_manifest.tsv` documents enemy
    telegraph, stagger, damage-direction, and elite readability contracts.
14. `Content/CodeRescueData/squad_personality_manifest.tsv` documents the five
    squad roles, mechanical identities, bark styles, and QA expectations.
15. `Content/CodeRescueData/curriculum_feedback_manifest.tsv` documents the
    terminal-feedback and learning-loop UX contracts.
16. Accessibility settings now include persistent subtitle size, high-contrast
    HUD, reduced motion, simplified input hints, and aim-assist scale. The
    settings widget exposes these controls, subtitle scale affects rendering,
    aim-assist scale affects assisted-hit behavior, reduced motion softens
    enemy-hit knockback, simplified hints shorten the HUD control line, and
    high contrast updates HUD text colors.
17. `Scripts/create_support_bundle.py` packages release manifests, visual
    manifests, logs, screenshots, data manifests, and key docs into a support
    zip.
18. `Documentation/SIGNING_NOTARIZATION_RUNBOOK_2026-06-18.md` documents the
    Mac signing/notarization path and bundle-identity decisions.
19. `Run_Local_CI_Readiness.command` provides a single local CI-style command
    that runs verifiers, manifest generation, compile, full QA, package, null
    smoke, render smoke, and support-bundle creation.
20. `Documentation/DEMO_READINESS_ROADMAP_2026-06-18.md` splits remaining work
    into Demo Blockers, Demo Polish, Content Expansion, Technical Debt, and
    Post-Demo.

## Files Added

- `Content/CodeRescueData/accessibility_settings_manifest.tsv`
- `Content/CodeRescueData/asset_budget_limits.tsv`
- `Content/CodeRescueData/control_remap_manifest.tsv`
- `Content/CodeRescueData/curriculum_feedback_manifest.tsv`
- `Content/CodeRescueData/difficulty_presets.tsv`
- `Content/CodeRescueData/enemy_readability_manifest.tsv`
- `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`
- `Content/CodeRescueData/human_qa_signoff_checklist.tsv`
- `Content/CodeRescueData/performance_city_layer_budget.tsv`
- `Content/CodeRescueData/squad_personality_manifest.tsv`
- `Content/CodeRescueData/visual_regression_targets.tsv`
- `Scripts/apply_control_remap_profile.py`
- `Scripts/create_support_bundle.py`
- `Scripts/generate_release_manifest.py`
- `Scripts/generate_visual_regression_manifest.py`
- `Scripts/profile_city_layers_static.py`
- `Scripts/verify_asset_budget_pass.py`
- `Scripts/verify_demo_readiness_pass.py`
- `Scripts/verify_save_compatibility_pass.py`
- `Run_Local_CI_Readiness.command`
- `Run_Visual_Regression_Audit.command`
- `Import_And_Wire_Maple_Narrations.command`
- `Documentation/DEMO_READINESS_ROADMAP_2026-06-18.md`
- `Documentation/NEXT_20_IMPLEMENTATION_PASS_2026-06-18.md`
- `Documentation/SAVE_COMPATIBILITY_TEST_PLAN_2026-06-18.md`
- `Documentation/SIGNING_NOTARIZATION_RUNBOOK_2026-06-18.md`
- `Documentation/VISUAL_REGRESSION_BASELINE_2026-06-18.md`

## Files Updated

- `Run_Full_QA_Audit.command`
- `Source/CodeRescueUnreal/CodeRescueTypes.h`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.h`
- `Source/CodeRescueUnreal/CodeRescueGameInstance.cpp`
- `Source/CodeRescueUnreal/CodeRescueSaveGame.h`
- `Source/CodeRescueUnreal/CodeRescuePauseWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueSettingsWidget.h`
- `Source/CodeRescueUnreal/CodeRescueSettingsWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueSubtitlesWidget.cpp`
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp`
- `Source/CodeRescueUnreal/CodeRescueGameMode.h`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp`
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp`
- `Source/CodeRescueUnreal/CompanionActor.h`
- `Source/CodeRescueUnreal/CompanionActor.cpp`
- `Config/DefaultGame.ini`
- `Scripts/import_and_wire_maple_narrations.py`
- `Scripts/wire_radio_cues.py`
- `Tools/MapleVoice/maple_voice_clone.py`
- `Tools/MapleVoice/README.md`
- `Documentation/QA_PLAYTEST_CHECKLIST.md`
- `Documentation/DISTRIBUTION_GUIDE_MAC.md`
- `Documentation/MAPLE_NARRATION_STATUS_2026-06-18.md`
- `progress.md`

## Verification Commands

Use this focused verification set for the pass:

```bash
python3 Scripts/verify_save_compatibility_pass.py
python3 Scripts/verify_asset_budget_pass.py
python3 Scripts/verify_demo_readiness_pass.py
python3 Scripts/verify_maple_sinister_narration_pass.py
python3 Scripts/generate_visual_regression_manifest.py --min-count 1
python3 Scripts/apply_control_remap_profile.py
python3 Scripts/generate_release_manifest.py
python3 Scripts/create_support_bundle.py
git diff --check
./Recompile_Module.command
./Import_And_Wire_Maple_Narrations.command
```

Use `Run_Local_CI_Readiness.command` for the full compile, QA, package, smoke,
manifest, and bundle path.

## Final Local Evidence

`Run_Local_CI_Readiness.command` completed successfully after the Maple import
and native cue-load fallback were in place. The run completed static verifiers,
visual/release manifest generation, module recompile, full QA, Mac package,
packaged null smoke, packaged render smoke, runtime log-contract scans, visual
regression manifest refresh, and support-bundle creation.

Latest generated evidence:

- Package:
  `/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix/PackagedMac/Mac/CodeRescueUnreal.app`
- Package size from the release manifest: `2.03 GB`
- Latest package timestamp from the release manifest:
  `2026-06-19T01:36:37Z` (`Jun 18 17:36 AKDT 2026`)
- Release manifest:
  `Saved/Release/release_manifest_latest.json`
- Visual regression manifest:
  `Saved/VisualRegression/visual_regression_manifest_latest.json`
- Support bundles:
  `Saved/SupportBundles/`
- Maple narration coverage: `230/230` female-voiced missions generated and
  imported as radio cues.
- Runtime smoke allowed only known unattended diagnostics: macOS CoreAudio
  sample-rate query, immediate-quit navigation dirty-area, and immediate-quit
  crowd-following RecastNavMesh warnings.

## Notes For Future Review

- Maple generation is long-running and resumable, but the June 18 local CI
  evidence shows complete 230/230 female-voice coverage. The release manifest
  reports live coverage by counting generated `*_radio_briefing.wav` files
  against the 230 female-voiced missions.
- The Maple import path is idempotent by default and skips already-imported
  SoundWave assets. Set `CODE_RESCUE_FORCE_MAPLE_IMPORT=1` before
  `Import_And_Wire_Maple_Narrations.command` only when the imported SoundWave
  assets must be overwritten from regenerated WAV files.
- Control remapping is now documented/exportable. Direct C++ bindings still
  need an approved in-game binding editor before arbitrary runtime rebinding is
  safe.
- Signing/notarization cannot be completed without the final Apple Developer
  Team ID, bundle identifier, certificate, and notarization credentials.
- The support-bundle script deliberately packages evidence under `Saved/` so
  review artifacts can be shared without staging unrelated source changes.

## Continuation

The non-physical continuation pass is documented in
`Documentation/NONHUMAN_RELEASE_READINESS_PASS_2026-06-18.md`. It adds package
integrity/signing preflight, Maple audio technical auditing, a non-human
release-readiness verifier, and `Run_NonHuman_Release_Readiness.command`.
