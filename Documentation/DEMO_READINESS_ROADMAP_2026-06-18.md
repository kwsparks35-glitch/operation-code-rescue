# Demo Readiness Roadmap - 2026-06-18

## Demo Blockers

- Maple narration batch, import/wire, native runtime fallback, and always-cook
  package coverage are complete at `230/230`.
- `Run_Local_CI_Readiness.command` has completed successfully after Maple
  import so compile, full QA, package, smoke, release manifest, and support
  bundle all reflect the same package.
- Complete a human first-ten-minutes QA pass using
  `Content/CodeRescueData/first_ten_minutes_onboarding.tsv`.
- Pick the final bundle identifier and complete the steps in
  `Documentation/SIGNING_NOTARIZATION_RUNBOOK_2026-06-18.md`.

## Automated Non-Human Gates

- Run `./Run_NonHuman_Release_Readiness.command` after package refreshes.
- Review `Saved/AudioAudit/maple_audio_audit_latest.json` for Maple technical
  audio coverage.
- Review `Saved/Release/package_integrity_latest.json` for package structure,
  plist identity, local code-signature state, and credential-only distribution
  blockers.
- Keep `Content/CodeRescueData/nonhuman_release_readiness_gates.tsv` aligned
  with the release gates that can run without physical human-testing.
- Run `./Run_NoHuman_Next20_Improvement.command` after substantial gameplay,
  tooling, or release-doc changes. Review
  `Saved/Release/nohuman_next20_evidence_latest.json` for the current status of
  the top twenty continued-improvement recommendations.
- Run `./Run_NoHuman_Next20_Round2_Improvement.command` after the first
  no-human pass when the release-support evidence needs a deeper dashboard.
  Review `Saved/Release/nohuman_next20_round2_evidence_latest.json` for input
  mapping, curriculum progression, localization, screenshot readability,
  artifact freshness, source-control slices, and support-bundle completeness.

## Demo Polish

- Capture fresh visual-regression screenshots for every target listed in
  `Content/CodeRescueData/visual_regression_targets.tsv`.
- Review settings at 1280x720 and native display size to confirm the scrollable
  settings panel remains readable.
- Do one hands-on pass for each difficulty preset: Story, Easy, Normal, Hard,
  Survival, and Nightmare.
- Validate that accessibility settings have observable effects: subtitles,
  subtitle size, high contrast, reduced motion, simplified hints, and aim
  assist.

## Content Expansion

- Replace remaining procedural role barks with authored squad audio.
- Add stronger city-specific audio beds for representative city families.
- Continue replacing placeholder geometry with authored/optimized assets where
  it improves readability.
- Add more post-solve curriculum explanation text for advanced lessons.

## Technical Debt

- Continue splitting `CodeRescueGameMode.cpp` by ownership: city identity,
  arena confinement, access cleanup, curriculum safehouse, district dressing,
  combat spawning, and release markers.
- Continue the control-remap architecture pass. Settings can now export the
  active control contract in-game, but arbitrary key capture should wait until
  direct C++ bindings are routed through a shared action table.
- Add a binary save-load commandlet test once save files can be constructed in
  a deterministic test fixture.
- Add runtime per-layer performance telemetry. Static source/budget evidence now
  exists through `Scripts/profile_city_layers_static.py`.

## Post-Demo

- Build CI in a hosted environment once Unreal licensing, storage, and Mac
  runner constraints are settled.
- Add signed/notarized distribution zips after the Apple Developer workflow is
  approved and `verify_package_integrity_pass.py --strict-distribution` passes.
- Expand visual regression from screenshot hashing to image comparison with
  target-specific tolerances.
- Add localization QA for subtitles, terminal prompts, and city radio text.
