# First-Level Test and Package Record

Date: 2026-07-09

Final application:
`PackagedMac/Mac/CodeRescueUnreal.app`

This V4 record is retained for history. The superseding final archive and V5
single-run results are documented in
`FIRST_LEVEL_V5_FINAL_TEST_AND_PACKAGE.md`.

## Final Archive Identity

- Unreal Engine: 5.7.4
- Bundle ID: `com.operationcoderescue.CodeRescueUnreal`
- Bundle version: `51494982.0.194`
- Locally measured package size: `2049.1 MB`
- Final IoStore mount: 1,764 packages
- Local signing: valid on disk and satisfies its Designated Requirement
- Distribution state: local Mac play ready; Developer ID signing and Apple
  notarization remain credentialed external-release steps

## Build and Cook

`./Recompile_Module.command` completed successfully after the character,
armory, world, and combat changes. `./Package_Mac_App.command` then completed a
clean BuildCookRun, staged the full Mac data tree, signed the application, and
archived the path above. The final cook adds the localized blood decal material
instances and dependencies that were absent from the immediately preceding
audit build.

## Acceptance Results

| Validation | Final result | Evidence |
| --- | --- | --- |
| First-level static acceptance | PASS, 41 contracts | `Scripts/verify_first_level_combat_experience_pass_2026_07_09.py` |
| V4 Unreal mesh commandlet | PASS, 9/9 assets | `TestLogs/FirstLevelV4MeshAudit.log` |
| Editor armory cycle | PASS, 17/17 previews and equip | `TestLogs/FirstLevelArmoryCycleAudit.log` |
| Editor integrated combat | PASS, full lifecycle | `TestLogs/FirstLevelCombatRuntimeAudit.log` |
| Packaged null-render launch | PASS, strict log and runtime contracts | `TestLogs/PackagedSmoke_null.log` |
| Packaged Metal launch | PASS, strict log and runtime contracts | `TestLogs/PackagedSmoke_render.log` |
| Packaged armory cycle | PASS, 17/17 previews and equip | `TestLogs/PackagedFirstLevelArmoryCycleAudit.log` |
| Packaged integrated combat | PASS, full lifecycle | `TestLogs/PackagedFirstLevelCombatRuntimeAudit.log` |
| Package integrity verifier | PASS, local ready | `TestLogs/package_integrity_latest.json` |
| Deep code-sign verification | PASS | `codesign --verify --deep --strict` |

The static verifier currently reports 41 explicit PASS checks plus its final
acceptance line. It covers input remapping, aim pose, live armory viewport,
selection/equip behavior, hit geometry, explosive constraints, localized
wounds, corpse lifecycle, cook inclusion, all nine Blender outputs, mesh audit,
and first-level-only world gating.

## Packaged Armory Audit

The final package logged each weapon from `1/17 Balanced Handgun` through
`17/17 Flash Grenade`, then reported:

```text
[PauseArmory] equipped selection index=16
[PauseArmoryAudit] COMPLETE PASS previews=17 final_index=16 equipped=1
```

The generated 1280x720 screenshot is
`Renders/packaged_first_level_armory_cycle_complete.png`. It verifies the final
Flash Grenade mesh, lighting, centered framing, description, ammunition,
statistics, clickable navigation, equip state, and retained run controls.

## Packaged Combat Audit

The packaged test used the actual first-level player and created a transient
target because the selected campaign save had already defeated its live
zombies. The target uses the same `ACodeZombieActor` collision, damage, wound,
death, and lifecycle code as normal encounter targets.

Observed results:

```text
jump=PASS velocity_z=500.1 stamina=87.8
solver channel=1 object=1 capsule=1 zombie_first=1
line_trace_shot=1 PASS health=70.0->35.0 assist=disabled
line_trace_shot=2 PASS health=35.0->0.0 assist=disabled
corpse_window=PASS trace_hits=2
gradual_fade=PASS sink=29.93 scale_reduction=0.605
COMPLETE PASS jump=1 bite=1 trace_hits=2 corpse=1 fade=1 removed=1
```

The two pistol shots therefore resolved through physical traces without aim
assist, hit the intended capsule before world geometry, generated wounds,
killed only the target, retained the body for the specified window, and removed
it only after the gradual fade. The final log contains no missing package,
failed object load, `LogTemp: Error`, fatal error, assertion, or ensure entry.

## Smoke-Test Results

Both launch modes passed `Scripts/scan_audit_warnings.py` and the profile-aware
`Scripts/verify_runtime_log_contracts.py` curated-production contract. The null
run retained only the allowed immediate-exit navigation dirty-area and missing
crowd-manager warnings. The Metal run retained those two plus the known macOS
CoreAudio sample-rate warning. The armory audit additionally emits Unreal's
nonfatal `Consoles don't need hitproxy storage` viewport warning; the viewport
render, capture, and 17-item cycle all complete successfully.

Machine-readable scanner output is retained in
`TestLogs/PackagedSmoke_null_scan.txt`,
`TestLogs/PackagedSmoke_null_contracts.txt`,
`TestLogs/PackagedSmoke_render_scan.txt`, and
`TestLogs/PackagedSmoke_render_contracts.txt`.

## Regression Coverage

Adjacent verifiers were also run after implementation, including gameplay
fixes, collision channels, combat juice, death physics, physical-animation hit
reaction, quick-slot armory, production camera/presentation, menu/movement, and
V3 art contracts. The runtime log-contract verifier was updated to distinguish
the curated production profile from the opt-in development showcase profile,
so production validation no longer demands intentionally disabled prototype
markers.

## Import Toolchain Note

The Blender 5.1.2 generation completed normally. Unreal imported and saved all
nine assets, after which the UE 5.7 editor process encountered a heap teardown
defect during shutdown. This occurred after successful asset saves; independent
commandlet loading verified all nine meshes have render triangles, LODs,
materials, and sane bounds. The package subsequently cooked and loaded those
same assets successfully.

## Reproduction Commands

```text
python3 Scripts/verify_first_level_combat_experience_pass_2026_07_09.py
./Smoke_Test_Packaged_App.command null
./Smoke_Test_Packaged_App.command render
```

The packaged audit flags are `-VisualReviewStart
-FirstLevelArmoryCycleAudit` and `-VisualReviewStart
-FirstLevelCombatRuntimeAudit`. These are review-only launch paths; they do not
replace or weaken the normal language-selection launch gate.
