# First-Level V5 Final Test and Package Record

Date: 2026-07-09

Final application: `PackagedMac/Mac/CodeRescueUnreal.app`

## Archive Identity

- Unreal Engine: 5.7.4
- Architecture: arm64
- Bundle ID: `com.operationcoderescue.CodeRescueUnreal`
- Bundle version: `51494982.0.196`
- Minimum macOS: 14.00
- Final size: 2,151,085,626 bytes (2,051.4 MB)
- Cook: 1,815 discovered packages; 1,807 runtime IoStore packages
- Signature: valid ad-hoc local signature; Designated Requirement satisfied
- Distribution boundary: Apple Developer ID signing and notarization remain
  external credentialed release steps

## Final Build Sequence

1. `./Recompile_Module.command` passed after correcting the V2 zombie animation
   object paths.
2. `./Package_Mac_App.command` completed build, full cook, stage, local sign,
   and archive with AutomationTool exit code 0.
3. `./Smoke_Test_Packaged_App.command null` passed the log scanner and curated
   runtime contract.
4. `./Smoke_Test_Packaged_App.command render` launched through Metal and passed
   the same gates.
5. The archived executable ran `-FirstLevelIntegratedAcceptanceAudit` and
   completed every first-level subsystem in a single session.
6. `Scripts/verify_package_integrity_pass.py` and
   `codesign --verify --deep --strict` both passed.

## Acceptance Matrix

| Validation | Result | Evidence |
| --- | --- | --- |
| V5 static implementation contract | PASS | `Scripts/verify_first_level_integrated_v5_pass_2026_07_09.py` |
| V5 Unreal meshes | PASS, 5/5 | `TestLogs/FirstLevelV5MeshAudit.log` |
| V5 persistent collision setup | PASS, 3/3 | `TestLogs/FirstLevelV5CollisionConfigure.log` |
| Arena and doorway access | PASS, 3/3 doors | `TestLogs/FirstLevelWorldAccessAuditV5Final.log` |
| Exact first-level solutions | PASS, 6/6 languages | `TestLogs/FirstLevelChallengeAuditV5.log` |
| Validator shape matrix | PASS, 48/48 | `TestLogs/CurriculumValidatorShapesV5.log` |
| Four-phase sky | PASS | `TestLogs/FirstLevelSkyAuditV5Retest.log` |
| Clean editor integration | PASS | `TestLogs/FirstLevelIntegratedAcceptanceAuditV5CleanPhysics.log` |
| Packaged null smoke | PASS | `TestLogs/PackagedSmoke_null.log` |
| Packaged Metal smoke | PASS | `TestLogs/PackagedSmoke_render.log` |
| Packaged single-run integration | PASS | `TestLogs/PackagedFirstLevelIntegratedAcceptanceAuditV5Final.log` |
| Package integrity | PASS, local ready | `TestLogs/package_integrity_latest.json` |
| Deep code-sign verification | PASS | `codesign --verify --deep --strict` |

## Single-Run Packaged Result

The final archived application reported:

```text
[FirstLevelAccessAudit] COMPLETE PASS ... enterable_buildings=3 clear_doors=3/3 level_doors=3/3
[FirstLevelChallengeAudit] COMPLETE PASS languages=6/6 challenge=new_york_ny_sum
[FirstLevelSkyAudit] COMPLETE PASS day=1 sunset=1 night=1 sunrise=1 stars=1 moon=1
[FirstLevelCombatAudit] COMPLETE PASS jump=1 bite=1 miss_locality=1 target_lock=1 trace_hits=2 corpse=1 fade=1 removed=1
[PauseArmoryAudit] COMPLETE PASS previews=17 final_index=16 equipped=1
[FirstLevelIntegratedAudit] COMPLETE PASS world=1 sky=1 challenges=1 target_lock=1 combat=1 corpse=1 armory=1
```

The deliberate miss test left target health at `100000.0 -> 100000.0`. The two
subsequent physical traces reduced health `70 -> 35 -> 0`, created localized
wounds, retained the corpse for 9.0 seconds, and completed the 2.8-second fade.

## Log Classification

There are no missing cooked packages, failed object loads, linker warnings,
errors, exceptions, assertions, ensures, fatals, invalid physical-animation
bodies, or stale V2 animation references in the final integrated log. The
project scanner permits three exact Development-environment diagnostics:

- a navigation dirty-area message during runtime-generated level startup;
- Unreal 5.7's `UViewport` hit-proxy message when the live armory preview opens;
- the crowd manager's no-Recast message during immediate test shutdown.

The first and third do not affect the explicit capsule/AI gameplay paths used
by this generated arena. The second is emitted only by non-Shipping builds; the
preview rendered and cycled all 17 entries successfully.

## Visual Review

The final packaged captures were inspected at 1280x720. They show distinct day,
sunset, night, and sunrise lighting; unobstructed streets; the raised two-arm
aim pose; grounded corpse and fade stages; and a complete armory layout with a
centered live weapon, description, ammunition, statistics, navigation, equip,
and retained run controls.

## Reproduction Commands

```text
python3 Scripts/verify_first_level_integrated_v5_pass_2026_07_09.py
python3 Scripts/verify_first_level_combat_experience_pass_2026_07_09.py
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
python3 Scripts/scan_audit_warnings.py Saved/Logs/PackagedFirstLevelIntegratedAcceptanceAuditV5Final.log
python3 Scripts/verify_package_integrity_pass.py
codesign --verify --deep --strict --verbose=2 PackagedMac/Mac/CodeRescueUnreal.app
```

The normal player launch must omit audit flags so the coding-language selector
and per-language Resume/New Run choices remain the first screen.
