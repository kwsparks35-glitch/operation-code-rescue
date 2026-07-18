# External Demo Preflight Slice

This pass closes the P2 signed external demo preparation row as far as it can be completed without Apple distribution credentials. The packaged Mac app is locally buildable, locally signable, package-integrity checked, smoke-tested, and release-manifested; final Developer ID signing, notarization, and Gatekeeper approval remain credential-bound.

## Runtime And Release Coverage

- `Scripts/verify_package_integrity_pass.py` inspects the packaged app bundle, required cooked files, Info.plist identity, local code signature, Gatekeeper assessment, package size, and strict distribution mode.
- `Scripts/generate_release_manifest.py` captures package stats, logs, static verifier evidence, Maple narration coverage, package-integrity report, visual evidence, profiling evidence, and git status.
- `Run_NonHuman_Release_Readiness.command` runs audio audit, package integrity, release manifest generation, non-human readiness verification, and support-bundle creation.
- `Content/CodeRescueData/external_demo_preflight_manifest.tsv` records the external demo gates, current local-ready/credential-blocked status, and handoff evidence for future reviewers.

## Current Package Evidence

The latest package-integrity run reported local package readiness for `PackagedMac/Mac/CodeRescueUnreal.app` with bundle ID `com.operationcoderescue.CodeRescueUnreal`. External distribution readiness remains false because Gatekeeper/notarization still requires Apple Developer credentials.

## Boundaries

This slice does not create, store, or request Apple signing credentials. It also does not claim public release sign-off without human playtest. The correct strict external-distribution command remains:

```bash
python3 Scripts/verify_package_integrity_pass.py --strict-distribution
```

That command should fail until the final Developer ID certificate, Team ID, notary credentials, notarized package, and Gatekeeper acceptance are in place.

## Validation

- `python3 Scripts/verify_external_demo_preflight_slice_pass.py`
- `python3 Scripts/verify_package_integrity_pass.py`
- `python3 Scripts/generate_release_manifest.py`
- `python3 Scripts/verify_nonhuman_release_readiness_pass.py`
- Packaged null and render smoke tests after package refreshes.
