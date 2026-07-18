# Operation Code Rescue - Release Checklist

This checklist is the repeatable release process for a Mac Development package.

## Preflight

- Confirm Unreal Engine is discoverable with `Scripts/find_unreal_mac.sh`.
- Confirm `CodeRescueUnreal.uproject` opens from the project root.
- Confirm no critical C++ changes are pending verification.
- Confirm generated folders are ignored by git.

## Build

1. Run `Recompile_Module.command` or the equivalent UBT editor build.
2. Run `Package_Mac_App.command`.
3. Confirm output exists at `PackagedMac/Mac/CodeRescueUnreal.app`.
4. Confirm `PackagedMac/Mac/CodeRescueUnreal.app/Contents/UE` exists.
5. Confirm pak data exists at `Contents/UE/CodeRescueUnreal/Content/Paks`.
6. Run `Run_NoHuman_Next20_Improvement.command` to refresh the top-twenty
   continued-improvement evidence ledger.
7. Run `Run_NoHuman_Next20_Round2_Improvement.command` to refresh the
   second-cycle release dashboard, input/curriculum/localization audits,
   visual-readability metrics, and source-control slice evidence.

## Smoke Tests

Run both:

```bash
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
```

Pass criteria:

- Exit code is 0.
- Pak/utoc containers mount.
- `/Engine/Maps/Entry` loads.
- `CodeRescueGameMode` initializes.
- App exits cleanly from `ExecCmds=Quit`.

## Manual QA

Complete `Documentation/QA_PLAYTEST_CHECKLIST.md`.

Pass criteria:

- First city can be completed.
- City transition to Los Angeles works.
- Save/load preserves solved/rescued/neutralized state.
- Journal, terminal, pause, death, and HUD are readable.
- Combat feels playable on Normal.

## Distribution Prep

- Confirm `CFBundleIdentifier` is `com.operationcoderescue.CodeRescueUnreal`
  unless the Apple developer organization has intentionally changed ownership.
- Confirm `Saved/Release/nohuman_next20_evidence_latest.json` exists and
  reports twenty recommendation rows.
- Confirm `Saved/Release/nohuman_next20_round2_evidence_latest.json` exists
  and reports twenty second-cycle recommendation rows.
- Decide whether this is a local unsigned build, signed developer build, or notarized build.
- For public sharing, sign with Developer ID, notarize, staple, and run
  `python3 Scripts/verify_package_integrity_pass.py --strict-distribution`.
- Zip or DMG the final app only after smoke tests pass.
- Include `README_MAC.md` and the current completion report with the shared build.

## Release Record

Create `Documentation/RELEASE_NOTES_YYYY-MM-DD.md` with:

- Build date.
- Engine version.
- Package path.
- Verification commands run.
- Known limitations.
- Manual playtest result.
