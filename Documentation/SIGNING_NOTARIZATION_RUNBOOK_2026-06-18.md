# Mac Signing And Notarization Runbook - 2026-06-18

## Required Decisions

- Confirm the final bundle identifier. Current source-controlled default:
  `com.operationcoderescue.CodeRescueUnreal`.
- Apple Developer Team ID.
- Distribution type: internal development share, Developer ID distribution, or
  Mac App Store.
- Whether the package needs hardened runtime entitlements beyond Unreal's
  default local-development signing.

## Local Evidence Before Signing

Run these first:

```bash
./Run_Full_QA_Audit.command
./Package_Mac_App.command
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
python3 Scripts/generate_release_manifest.py
python3 Scripts/create_support_bundle.py
```

The release manifest and support bundle should be archived with the signed
artifact.

For the focused non-human preflight, run:

```bash
./Run_NonHuman_Release_Readiness.command
```

This writes `Saved/Release/package_integrity_latest.json` and records whether
the current package is locally valid, whether the bundle ID is still a
placeholder, and whether Gatekeeper/distribution checks remain blocked by
credential-only work.

## Bundle Identity

Update Unreal Project Settings or the generated Mac plist so the bundle ID is a
real reverse-DNS identifier owned by the project or organization.

Current bundle identifier:

```text
com.operationcoderescue.CodeRescueUnreal
```

Verify it explicitly:

```bash
python3 Scripts/verify_package_integrity_pass.py --expected-bundle-id com.operationcoderescue.CodeRescueUnreal
```

## Developer ID Signing Outline

1. Package the app with `Package_Mac_App.command`.
2. Verify the app contains cooked data under `Contents/UE`.
3. Sign nested binaries/frameworks according to Unreal's generated app bundle
   layout.
4. Sign the `.app` with the Developer ID Application certificate and hardened
   runtime as appropriate.
5. Create a zip or DMG for notarization.
6. Submit with `xcrun notarytool submit`.
7. Staple with `xcrun stapler staple`.
8. Verify Gatekeeper with `spctl --assess --verbose`.
9. Run packaged null and render smoke against the signed app.
10. Run strict distribution preflight:

```bash
python3 Scripts/verify_package_integrity_pass.py --strict-distribution
```

## Not Yet Completed

This pass does not complete Developer ID signing or notarization because the
Apple Developer certificate and Team ID are not present in the repository. The
bundle identifier placeholder has been removed; strict distribution preflight is
now expected to remain blocked only by Gatekeeper/notarization until credentials
are available.
