# Non-Human Release Readiness Pass - 2026-06-18

## Scope

This continuation pass advances all work that can be completed without
physical human-testing. It adds automated package-integrity, signing-preflight,
and Maple audio-technical checks; records the gates in project data; and wires
the new evidence into release manifests, support bundles, and local CI.

Physical human-testing remains intentionally outside this pass. Apple
Developer signing/notarization also remains credential-only until the final
bundle identifier, Team ID, Developer ID certificate, and notary credentials
are supplied.

## Implemented

1. Added `Content/CodeRescueData/nonhuman_release_readiness_gates.tsv` to
   identify the automated gates, evidence paths, and boundaries for
   physical-human and credential-only work.
2. Added `Scripts/audit_maple_audio_assets.py` to verify all 230 expected Maple
   female-voice WAVs and imported SoundWave assets exist, are mono 16-bit
   22050 Hz WAVs, have sane durations, and contain non-silent signal.
3. Added `Scripts/verify_package_integrity_pass.py` to inspect the packaged
   Mac app, required cooked files, Info.plist fields, local code-signature
   state, Gatekeeper assessment, package size, and current distribution
   blockers.
4. Added `Scripts/verify_nonhuman_release_readiness_pass.py` to assert the
   new scripts, docs, manifest rows, latest JSON reports, and release manifest
   are all present and consistent.
5. Added `Run_NonHuman_Release_Readiness.command` as the focused wrapper for
   audio audit, package-integrity preflight, release manifest refresh,
   non-human readiness verification, and support-bundle creation.
6. Updated `Run_Local_CI_Readiness.command` so future full local CI runs also
   execute the new non-human release gates after package smoke tests.
7. Updated `Scripts/generate_release_manifest.py` and
   `Scripts/create_support_bundle.py` so package-integrity and Maple audio
   audit reports are captured in release evidence.

## Evidence Files

- `Saved/AudioAudit/maple_audio_audit_latest.json`
- `Saved/Release/package_integrity_latest.json`
- `Saved/Release/release_manifest_latest.json`
- `Saved/SupportBundles/`

## Current Expected Result

- Maple audio technical gate should pass with `230/230` expected WAVs and
  `230/230` expected imported SoundWave assets.
- Local package integrity should pass for the current development package.
- External distribution readiness is expected to remain false until the app is
  signed and notarized with final Apple Developer credentials. The earlier
  `com.YourCompany.CodeRescueUnreal` placeholder bundle identifier was replaced
  on 2026-06-23 with `com.operationcoderescue.CodeRescueUnreal`.

## Validation

Run the focused non-human release gate:

```bash
./Run_NonHuman_Release_Readiness.command
```

Run strict distribution mode only after final credentials and bundle identity
are in place:

```bash
python3 Scripts/verify_package_integrity_pass.py --strict-distribution
```

Strict mode should fail today because the final Apple signing/notarization
inputs are not present.
